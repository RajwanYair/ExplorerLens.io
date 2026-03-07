// DecodeProfilingHarness.h — Decoder Performance Instrumentation
// Copyright (c) 2026 ExplorerLens Project
//
// Wraps decoder invocations with timing, memory tracking, and success rate
// metrics to identify bottlenecks and regression in decode performance.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

struct DecodeProfile {
    std::string decoderName;
    uint64_t totalCalls = 0;
    uint64_t successCount = 0;
    uint64_t failureCount = 0;
    double minMs = 1e9;
    double maxMs = 0.0;
    double totalMs = 0.0;
    uint64_t peakMemoryBytes = 0;

    double AvgMs() const { return totalCalls > 0 ? totalMs / totalCalls : 0.0; }
    double SuccessRate() const { return totalCalls > 0 ? 100.0 * successCount / totalCalls : 0.0; }
};

class DecodeProfilingHarness {
public:
    class ScopedProfile {
    public:
        ScopedProfile(DecodeProfilingHarness& harness, const std::string& decoder)
            : m_harness(harness), m_decoder(decoder),
            m_start(std::chrono::steady_clock::now()) {
        }

        void MarkSuccess() { m_success = true; }
        void SetPeakMemory(uint64_t bytes) { m_peakMem = bytes; }

        ~ScopedProfile() {
            auto elapsed = std::chrono::steady_clock::now() - m_start;
            double ms = std::chrono::duration<double, std::milli>(elapsed).count();
            m_harness.Record(m_decoder, ms, m_success, m_peakMem);
        }

        ScopedProfile(const ScopedProfile&) = delete;
        ScopedProfile& operator=(const ScopedProfile&) = delete;

    private:
        DecodeProfilingHarness& m_harness;
        std::string m_decoder;
        std::chrono::steady_clock::time_point m_start;
        bool m_success = false;
        uint64_t m_peakMem = 0;
    };

    ScopedProfile Begin(const std::string& decoder) {
        return ScopedProfile(*this, decoder);
    }

    DecodeProfile GetProfile(const std::string& decoder) const {
        std::lock_guard lock(m_mutex);
        auto it = m_profiles.find(decoder);
        return it != m_profiles.end() ? it->second : DecodeProfile{};
    }

    std::vector<DecodeProfile> GetAllProfiles() const {
        std::lock_guard lock(m_mutex);
        std::vector<DecodeProfile> result;
        result.reserve(m_profiles.size());
        for (const auto& [k, v] : m_profiles) result.push_back(v);
        return result;
    }

    void Reset() {
        std::lock_guard lock(m_mutex);
        m_profiles.clear();
    }

private:
    void Record(const std::string& decoder, double ms, bool success, uint64_t peakMem) {
        std::lock_guard lock(m_mutex);
        auto& p = m_profiles[decoder];
        p.decoderName = decoder;
        p.totalCalls++;
        if (success) p.successCount++; else p.failureCount++;
        p.totalMs += ms;
        if (ms < p.minMs) p.minMs = ms;
        if (ms > p.maxMs) p.maxMs = ms;
        if (peakMem > p.peakMemoryBytes) p.peakMemoryBytes = peakMem;
    }

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, DecodeProfile> m_profiles;
};

} // namespace Engine
} // namespace ExplorerLens
