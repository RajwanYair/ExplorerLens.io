// DecoderPerformanceCounters.cpp — Per-format decode performance metrics implementation
// Copyright (c) 2026 ExplorerLens Project
//
#include "DecoderPerformanceCounters.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// LatencyHistogram — bucket boundaries
// ---------------------------------------------------------------------------
// Bucket boundaries (ms): 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048+
// Bucket i covers: [2^(i-1), 2^i) for i>=1, bucket 0 covers [0,1)
// Buckets 12..BUCKET_COUNT-1 hold overflow (>4096 ms)

int LatencyHistogram::BucketFor(double ms) noexcept {
    if (ms < 0.0) ms = 0.0;
    if (ms < 1.0)  return 0;
    // log2(ms) + 1, clamped to BUCKET_COUNT-1
    int b = static_cast<int>(std::log2(ms)) + 1;
    return (b < static_cast<int>(BUCKET_COUNT)) ? b : static_cast<int>(BUCKET_COUNT) - 1;
}

void LatencyHistogram::Record(double ms) noexcept {
    int b = BucketFor(ms);
    m_buckets[static_cast<size_t>(b)].fetch_add(1, std::memory_order_relaxed);
    auto us = static_cast<uint64_t>(ms * 1000.0);
    m_sumUs.fetch_add(us, std::memory_order_relaxed);
}

uint64_t LatencyHistogram::SampleCount() const noexcept {
    uint64_t total = 0;
    for (const auto& b : m_buckets) {
        total += b.load(std::memory_order_relaxed);
    }
    return total;
}

double LatencyHistogram::Mean() const noexcept {
    uint64_t n = SampleCount();
    if (n == 0) return 0.0;
    return static_cast<double>(m_sumUs.load(std::memory_order_relaxed)) / (static_cast<double>(n) * 1000.0);
}

double LatencyHistogram::Percentile(double p) const noexcept {
    if (p <= 0.0) p = 0.0;
    if (p >= 1.0) p = 1.0;

    uint64_t total = SampleCount();
    if (total == 0) return 0.0;

    uint64_t target = static_cast<uint64_t>(std::ceil(static_cast<double>(total) * p));
    uint64_t cumulative = 0;

    for (size_t i = 0; i < BUCKET_COUNT; ++i) {
        cumulative += m_buckets[i].load(std::memory_order_relaxed);
        if (cumulative >= target) {
            // Return upper bound of bucket i: 2^(i) ms (minimum 1 ms for bucket 0)
            double upper = (i == 0) ? 1.0 : std::pow(2.0, static_cast<double>(i));
            return upper;
        }
    }
    return static_cast<double>(1ull << (BUCKET_COUNT - 1));
}

void LatencyHistogram::Reset() noexcept {
    for (auto& b : m_buckets) { b.store(0, std::memory_order_relaxed); }
    m_sumUs.store(0, std::memory_order_relaxed);
}

// ---------------------------------------------------------------------------
// PerFormatReport::Serialize
// ---------------------------------------------------------------------------

std::string PerFormatReport::Serialize() const {
    std::ostringstream ss;
    ss << "{";
    ss << "\"format\":\"" << format << "\",";
    ss << "\"attempts\":"   << totalAttempts   << ",";
    ss << "\"successes\":"  << totalSuccesses  << ",";
    ss << "\"failures\":"   << totalFailures   << ",";
    ss << "\"hit_rate\":"   << hitRate         << ",";
    ss << "\"error_rate\":" << errorRate       << ",";
    ss << "\"cold_p50_ms\":" << coldP50Ms      << ",";
    ss << "\"cold_p95_ms\":" << coldP95Ms      << ",";
    ss << "\"cold_p99_ms\":" << coldP99Ms      << ",";
    ss << "\"warm_p50_ms\":" << warmP50Ms      << ",";
    ss << "\"cold_mean_ms\":" << coldMeanMs    << ",";
    ss << "\"gpu_decodes\":"  << gpuDecodes    << ",";
    ss << "\"cpu_decodes\":"  << cpuDecodes;
    ss << "}";
    return ss.str();
}

// ---------------------------------------------------------------------------
// DecoderPerformanceCounters — singleton
// ---------------------------------------------------------------------------

DecoderPerformanceCounters& DecoderPerformanceCounters::Instance() noexcept {
    static DecoderPerformanceCounters s_instance;
    return s_instance;
}

// Spin-lock acquire (ATOMIC_FLAG is not recursive)
namespace {
struct SpinLock {
    std::atomic_flag& flag;
    SpinLock(std::atomic_flag& f) : flag(f) {
        while (flag.test_and_set(std::memory_order_acquire)) { /* spin */ }
    }
    ~SpinLock() { flag.clear(std::memory_order_release); }
};
} // namespace

FormatCounters& DecoderPerformanceCounters::GetOrCreate(std::string_view format) noexcept {
    SpinLock lk(m_lock);
    auto it = m_counters.find(std::string(format));
    if (it == m_counters.end()) {
        auto [ins, _] = m_counters.emplace(std::string(format), std::make_unique<FormatCounters>());
        return *ins->second;
    }
    return *it->second;
}

void DecoderPerformanceCounters::RecordDecode(
    std::string_view format,
    double           latencyMs,
    bool             success,
    bool             usedGpu,
    uint64_t         bytes) noexcept {
    auto& c = GetOrCreate(format);
    c.decodeAttempts.fetch_add(1, std::memory_order_relaxed);
    if (success) {
        c.decodeSuccesses.fetch_add(1, std::memory_order_relaxed);
        c.coldLatency.Record(latencyMs);
        c.bytesDecoded.fetch_add(bytes, std::memory_order_relaxed);
        if (usedGpu) {
            c.gpuDecodes.fetch_add(1, std::memory_order_relaxed);
        } else {
            c.cpuDecodes.fetch_add(1, std::memory_order_relaxed);
        }
    } else {
        c.decodeFailures.fetch_add(1, std::memory_order_relaxed);
    }
}

void DecoderPerformanceCounters::RecordCacheHit(std::string_view format, double latencyMs) noexcept {
    auto& c = GetOrCreate(format);
    c.cacheHits.fetch_add(1, std::memory_order_relaxed);
    c.warmLatency.Record(latencyMs);
}

void DecoderPerformanceCounters::RecordCacheMiss(std::string_view format) noexcept {
    GetOrCreate(format).cacheMisses.fetch_add(1, std::memory_order_relaxed);
}

const FormatCounters* DecoderPerformanceCounters::GetCounters(std::string_view format) const noexcept {
    SpinLock lk(m_lock);
    auto it = m_counters.find(std::string(format));
    return (it != m_counters.end()) ? it->second.get() : nullptr;
}

std::vector<PerFormatReport> DecoderPerformanceCounters::GenerateReport() const {
    SpinLock lk(m_lock);
    std::vector<PerFormatReport> out;
    out.reserve(m_counters.size());
    for (const auto& [fmt, c] : m_counters) {
        PerFormatReport r;
        r.format          = fmt;
        r.totalAttempts   = c->decodeAttempts .load(std::memory_order_relaxed);
        r.totalSuccesses  = c->decodeSuccesses.load(std::memory_order_relaxed);
        r.totalFailures   = c->decodeFailures .load(std::memory_order_relaxed);
        r.hitRate         = c->HitRate();
        r.errorRate       = c->ErrorRate();
        r.coldP50Ms       = c->coldLatency.P50();
        r.coldP95Ms       = c->coldLatency.P95();
        r.coldP99Ms       = c->coldLatency.P99();
        r.coldMeanMs      = c->coldLatency.Mean();
        r.warmP50Ms       = c->warmLatency.P50();
        r.gpuDecodes      = c->gpuDecodes.load(std::memory_order_relaxed);
        r.cpuDecodes      = c->cpuDecodes.load(std::memory_order_relaxed);
        out.push_back(std::move(r));
    }
    std::sort(out.begin(), out.end(), [](const PerFormatReport& a, const PerFormatReport& b){
        return a.totalAttempts > b.totalAttempts;
    });
    return out;
}

std::string DecoderPerformanceCounters::SerializeJson() const {
    auto reports = GenerateReport();
    std::ostringstream ss;
    ss << "{\"decoder_performance\":[";
    for (size_t i = 0; i < reports.size(); ++i) {
        if (i > 0) ss << ",";
        ss << reports[i].Serialize();
    }
    ss << "]}";
    return ss.str();
}

void DecoderPerformanceCounters::ResetAll() noexcept {
    SpinLock lk(m_lock);
    for (auto& [fmt, c] : m_counters) { c->Reset(); }
}

uint64_t DecoderPerformanceCounters::TotalAttempts() const noexcept {
    SpinLock lk(m_lock);
    uint64_t total = 0;
    for (const auto& [_, c] : m_counters) {
        total += c->decodeAttempts.load(std::memory_order_relaxed);
    }
    return total;
}

uint64_t DecoderPerformanceCounters::TotalSuccesses() const noexcept {
    SpinLock lk(m_lock);
    uint64_t total = 0;
    for (const auto& [_, c] : m_counters) {
        total += c->decodeSuccesses.load(std::memory_order_relaxed);
    }
    return total;
}

uint64_t DecoderPerformanceCounters::TotalFailures() const noexcept {
    SpinLock lk(m_lock);
    uint64_t total = 0;
    for (const auto& [_, c] : m_counters) {
        total += c->decodeFailures.load(std::memory_order_relaxed);
    }
    return total;
}

} // namespace Engine
} // namespace ExplorerLens
