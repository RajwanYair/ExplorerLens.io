// DecoderPerformanceCounters.h — Per-Decoder Performance Metrics
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks decode time, success rate, memory usage, and throughput per decoder.
// Thread-safe counters using atomic operations. Provides real-time performance
// visibility and historical statistics for adaptive decoder routing decisions.
//
#pragma once

#include <windows.h>
#include <cstdint>
#include <atomic>
#include <string>
#include <array>
#include <chrono>
#include <algorithm>
#include <sstream>

namespace ExplorerLens {
namespace Engine {

/// Individual decoder metrics snapshot
struct PerfCounterMetrics {
    std::string     decoderName;
    uint64_t        totalDecodes = 0;
    uint64_t        successfulDecodes = 0;
    uint64_t        failedDecodes = 0;
    double          avgDecodeTimeMs = 0.0;
    double          minDecodeTimeMs = 0.0;
    double          maxDecodeTimeMs = 0.0;
    double          p95DecodeTimeMs = 0.0;
    uint64_t        totalBytesProcessed = 0;
    double          throughputMBps = 0.0;
    double          successRate = 0.0;
};

/// Maximum number of decoders we track (25+ supported, reserve 32)
constexpr uint32_t MAX_TRACKED_DECODERS = 32;

/// Atomic counter block for a single decoder (lock-free)
struct AtomicDecoderCounters {
    std::atomic<uint64_t> totalDecodes{ 0 };
    std::atomic<uint64_t> successCount{ 0 };
    std::atomic<uint64_t> failCount{ 0 };
    std::atomic<uint64_t> totalTimeUs{ 0 };      // Microseconds
    std::atomic<uint64_t> minTimeUs{ UINT64_MAX };
    std::atomic<uint64_t> maxTimeUs{ 0 };
    std::atomic<uint64_t> totalBytes{ 0 };
    std::atomic<uint64_t> recentTimesUs[16]{};  // Ring buffer for P95
    std::atomic<uint32_t> recentIndex{ 0 };
    char name[64] = {};
    std::atomic<bool> active{ false };
};

/// RAII scope guard for timing a decode operation
class DecoderTimingScope;

/// Centralized performance counter registry for all decoders.
/// Thread-safe, lock-free reads, designed for high-frequency thumbnail pipeline.
///
/// Usage:
///   auto& counters = DecoderPerformanceCounters::Instance();
///   uint32_t id = counters.RegisterDecoder("JPEG");
///   { auto scope = counters.BeginTiming(id); /* decode... */ scope.RecordSuccess(fileSize); }
///
class DecoderPerformanceCounters {
public:
    static DecoderPerformanceCounters& Instance() {
        static DecoderPerformanceCounters instance;
        return instance;
    }

    /// Register a decoder by name. Returns decoder ID (0..MAX-1).
    /// Thread-safe. Same name returns same ID.
    uint32_t RegisterDecoder(const char* name) {
        // Check if already registered
        for (uint32_t i = 0; i < MAX_TRACKED_DECODERS; i++) {
            if (m_counters[i].active.load(std::memory_order_acquire)) {
                if (strncmp(m_counters[i].name, name, 63) == 0) return i;
            }
        }
        // Register new
        uint32_t slot = m_nextSlot.fetch_add(1, std::memory_order_acq_rel);
        if (slot >= MAX_TRACKED_DECODERS) return MAX_TRACKED_DECODERS - 1;
        strncpy_s(m_counters[slot].name, name, 63);
        m_counters[slot].active.store(true, std::memory_order_release);
        return slot;
    }

    /// Record a successful decode
    void RecordSuccess(uint32_t decoderId, uint64_t durationUs, uint64_t bytesProcessed) {
        if (decoderId >= MAX_TRACKED_DECODERS) return;
        auto& c = m_counters[decoderId];
        c.totalDecodes.fetch_add(1, std::memory_order_relaxed);
        c.successCount.fetch_add(1, std::memory_order_relaxed);
        c.totalTimeUs.fetch_add(durationUs, std::memory_order_relaxed);
        c.totalBytes.fetch_add(bytesProcessed, std::memory_order_relaxed);

        // Update min/max atomically (CAS loop)
        uint64_t oldMin = c.minTimeUs.load(std::memory_order_relaxed);
        while (durationUs < oldMin && !c.minTimeUs.compare_exchange_weak(oldMin, durationUs));

        uint64_t oldMax = c.maxTimeUs.load(std::memory_order_relaxed);
        while (durationUs > oldMax && !c.maxTimeUs.compare_exchange_weak(oldMax, durationUs));

        // Ring buffer for P95
        uint32_t idx = c.recentIndex.fetch_add(1, std::memory_order_relaxed) % 16;
        c.recentTimesUs[idx].store(durationUs, std::memory_order_relaxed);
    }

    /// Record a failed decode
    void RecordFailure(uint32_t decoderId, uint64_t durationUs) {
        if (decoderId >= MAX_TRACKED_DECODERS) return;
        auto& c = m_counters[decoderId];
        c.totalDecodes.fetch_add(1, std::memory_order_relaxed);
        c.failCount.fetch_add(1, std::memory_order_relaxed);
        c.totalTimeUs.fetch_add(durationUs, std::memory_order_relaxed);
    }

    /// Get snapshot of metrics for a specific decoder
    PerfCounterMetrics GetMetrics(uint32_t decoderId) const {
        PerfCounterMetrics m;
        if (decoderId >= MAX_TRACKED_DECODERS) return m;
        const auto& c = m_counters[decoderId];
        if (!c.active.load(std::memory_order_acquire)) return m;

        m.decoderName = c.name;
        m.totalDecodes = c.totalDecodes.load(std::memory_order_relaxed);
        m.successfulDecodes = c.successCount.load(std::memory_order_relaxed);
        m.failedDecodes = c.failCount.load(std::memory_order_relaxed);
        m.totalBytesProcessed = c.totalBytes.load(std::memory_order_relaxed);

        uint64_t totalUs = c.totalTimeUs.load(std::memory_order_relaxed);
        if (m.totalDecodes > 0) {
            m.avgDecodeTimeMs = static_cast<double>(totalUs) / m.totalDecodes / 1000.0;
        }

        uint64_t minUs = c.minTimeUs.load(std::memory_order_relaxed);
        m.minDecodeTimeMs = (minUs == UINT64_MAX) ? 0.0 : minUs / 1000.0;
        m.maxDecodeTimeMs = c.maxTimeUs.load(std::memory_order_relaxed) / 1000.0;
        m.successRate = (m.totalDecodes > 0) ?
            static_cast<double>(m.successfulDecodes) / m.totalDecodes * 100.0 : 0.0;

        // Throughput: MB/s based on total bytes and total time
        if (totalUs > 0) {
            m.throughputMBps = static_cast<double>(m.totalBytesProcessed) / (totalUs / 1e6) / (1024.0 * 1024.0);
        }

        // P95 from ring buffer (approximate)
        std::array<uint64_t, 16> recent{};
        uint32_t validCount = 0;
        for (int i = 0; i < 16; i++) {
            uint64_t v = c.recentTimesUs[i].load(std::memory_order_relaxed);
            if (v > 0) recent[validCount++] = v;
        }
        if (validCount > 0) {
            std::sort(recent.begin(), recent.begin() + validCount);
            uint32_t p95Idx = static_cast<uint32_t>(validCount * 0.95);
            if (p95Idx >= validCount) p95Idx = validCount - 1;
            m.p95DecodeTimeMs = recent[p95Idx] / 1000.0;
        }

        return m;
    }

    /// Get all active decoder metrics
    std::vector<PerfCounterMetrics> GetAllMetrics() const {
        std::vector<PerfCounterMetrics> all;
        for (uint32_t i = 0; i < MAX_TRACKED_DECODERS; i++) {
            if (m_counters[i].active.load(std::memory_order_acquire)) {
                all.push_back(GetMetrics(i));
            }
        }
        return all;
    }

    /// Reset all counters
    void Reset() {
        for (uint32_t i = 0; i < MAX_TRACKED_DECODERS; i++) {
            if (m_counters[i].active.load(std::memory_order_acquire)) {
                m_counters[i].totalDecodes.store(0, std::memory_order_relaxed);
                m_counters[i].successCount.store(0, std::memory_order_relaxed);
                m_counters[i].failCount.store(0, std::memory_order_relaxed);
                m_counters[i].totalTimeUs.store(0, std::memory_order_relaxed);
                m_counters[i].minTimeUs.store(UINT64_MAX, std::memory_order_relaxed);
                m_counters[i].maxTimeUs.store(0, std::memory_order_relaxed);
                m_counters[i].totalBytes.store(0, std::memory_order_relaxed);
                m_counters[i].recentIndex.store(0, std::memory_order_relaxed);
                for (auto& t : m_counters[i].recentTimesUs)
                    t.store(0, std::memory_order_relaxed);
            }
        }
    }

    /// Generate a formatted report string
    std::string GenerateReport() const {
        std::ostringstream ss;
        ss << "=== Decoder Performance Counters ===\n";
        auto all = GetAllMetrics();
        for (const auto& m : all) {
            ss << "\n[" << m.decoderName << "]\n"
                << "  Decodes: " << m.totalDecodes
                << " (OK: " << m.successfulDecodes << ", Fail: " << m.failedDecodes << ")\n"
                << "  Success Rate: " << m.successRate << "%\n"
                << "  Avg: " << m.avgDecodeTimeMs << " ms"
                << "  Min: " << m.minDecodeTimeMs << " ms"
                << "  Max: " << m.maxDecodeTimeMs << " ms"
                << "  P95: " << m.p95DecodeTimeMs << " ms\n"
                << "  Throughput: " << m.throughputMBps << " MB/s\n";
        }
        return ss.str();
    }

private:
    DecoderPerformanceCounters() = default;
    std::array<AtomicDecoderCounters, MAX_TRACKED_DECODERS> m_counters;
    std::atomic<uint32_t> m_nextSlot{ 0 };
};

/// RAII timing scope — records duration on destruction
class DecoderTimingScope {
public:
    DecoderTimingScope(uint32_t decoderId)
        : m_decoderId(decoderId)
        , m_start(std::chrono::high_resolution_clock::now()) {
    }

    void RecordSuccess(uint64_t bytesProcessed = 0) {
        auto elapsed = std::chrono::high_resolution_clock::now() - m_start;
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        DecoderPerformanceCounters::Instance().RecordSuccess(m_decoderId, us, bytesProcessed);
        m_recorded = true;
    }

    void RecordFailure() {
        auto elapsed = std::chrono::high_resolution_clock::now() - m_start;
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        DecoderPerformanceCounters::Instance().RecordFailure(m_decoderId, us);
        m_recorded = true;
    }

    ~DecoderTimingScope() {
        if (!m_recorded) RecordFailure(); // Default to failure if not explicitly recorded
    }

private:
    uint32_t m_decoderId;
    std::chrono::high_resolution_clock::time_point m_start;
    bool m_recorded = false;
};

} // namespace Engine
} // namespace ExplorerLens
