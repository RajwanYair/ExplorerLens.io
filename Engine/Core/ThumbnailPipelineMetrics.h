// ThumbnailPipelineMetrics.h — Per-Stage Pipeline Telemetry & Bottleneck Detection
// Copyright (c) 2026 ExplorerLens Project
//
// Captures per-stage latency (stream, decode, render, deliver) and computes
// P50/P95/P99 distributions using a lock-free circular histogram. Feeds into
// ETW events and the observability dashboard. (roadmap T2)
//
#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <atomic>
#include <algorithm>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PipelineStage : uint8_t {
    FileRead = 0,
    Decompress,
    Decode,
    ColorConvert,
    Scale,
    Render,
    ShellDeliver,
    Count           // Sentinel — number of tracked stages
};

enum class BottleneckStage : uint8_t {
    None,
    IO,             // File read dominates (HDD / slow NVMe)
    CPU,            // CPU decompression / decode dominates
    GPU,            // GPU render / VRAM upload dominates
    ShellDeliver    // IPC to Explorer dominates
};

struct StageStats {
    double   minMs   = 1e9;
    double   maxMs   = 0.0;
    double   p50Ms   = 0.0;
    double   p95Ms   = 0.0;
    double   p99Ms   = 0.0;
    double   meanMs  = 0.0;
    uint64_t samples = 0;
};

struct PipelineSnapshot {
    std::array<StageStats, static_cast<size_t>(PipelineStage::Count)> stages;
    double   totalP50Ms    = 0.0;
    double   totalP99Ms    = 0.0;
    double   throughput    = 0.0;   // Images/sec
    uint64_t totalSamples  = 0;
    BottleneckStage bottleneck = BottleneckStage::None;
    std::string bottleneckDescription;
};

class ThumbnailPipelineMetrics {
public:
    static constexpr int HIST_BUCKETS = 1000;   // 0.0–1000 ms in 1 ms buckets

    static ThumbnailPipelineMetrics& Instance() {
        static ThumbnailPipelineMetrics s_instance;
        return s_instance;
    }

    void RecordSample(PipelineStage stage, double latencyMs) {
        auto idx = static_cast<size_t>(stage);
        if (idx >= static_cast<size_t>(PipelineStage::Count)) return;

        int bucket = static_cast<int>(latencyMs);
        if (bucket < 0) bucket = 0;
        if (bucket >= HIST_BUCKETS) bucket = HIST_BUCKETS - 1;

        m_histograms[idx][bucket].fetch_add(1, std::memory_order_relaxed);
        m_totals[idx].fetch_add(1, std::memory_order_relaxed);
    }

    StageStats ComputeStats(PipelineStage stage) const {
        auto idx = static_cast<size_t>(stage);
        if (idx >= static_cast<size_t>(PipelineStage::Count)) return {};

        StageStats s;
        uint64_t total = m_totals[idx].load(std::memory_order_relaxed);
        if (total == 0) return s;

        s.samples = total;

        double sum       = 0.0;
        uint64_t cumul   = 0;
        bool p50set      = false;
        bool p95set      = false;
        bool p99set      = false;
        double minVal    = 1e9;
        double maxVal    = 0.0;

        for (int b = 0; b < HIST_BUCKETS; ++b) {
            uint64_t cnt = m_histograms[idx][b].load(std::memory_order_relaxed);
            if (cnt == 0) continue;

            double ms = static_cast<double>(b) + 0.5;
            sum  += ms * static_cast<double>(cnt);
            cumul += cnt;
            if (ms < minVal) minVal = ms;
            if (ms > maxVal) maxVal = ms;

            if (!p50set && cumul * 2 >= total) { s.p50Ms = ms; p50set = true; }
            if (!p95set && cumul * 20 >= total * 19) { s.p95Ms = ms; p95set = true; }
            if (!p99set && cumul * 100 >= total * 99) { s.p99Ms = ms; p99set = true; }
        }

        s.meanMs = (total > 0) ? (sum / static_cast<double>(total)) : 0.0;
        s.minMs  = (minVal < 1e9) ? minVal : 0.0;
        s.maxMs  = maxVal;
        return s;
    }

    PipelineSnapshot Snapshot() const {
        PipelineSnapshot snap;
        snap.totalSamples = m_totals[0].load(std::memory_order_relaxed);

        for (int i = 0; i < static_cast<int>(PipelineStage::Count); ++i) {
            snap.stages[i] = ComputeStats(static_cast<PipelineStage>(i));
        }

        snap.totalP50Ms = snap.stages[0].p50Ms;
        snap.totalP99Ms = snap.stages[0].p99Ms;
        for (int i = 0; i < static_cast<int>(PipelineStage::Count); ++i) {
            snap.totalP50Ms += snap.stages[i].p50Ms;
            snap.totalP99Ms += snap.stages[i].p99Ms;
        }

        snap.bottleneck = DetectBottleneck(snap);
        snap.bottleneckDescription = DescribeBottleneck(snap.bottleneck);
        return snap;
    }

    void Reset() {
        for (auto& hist : m_histograms)
            for (auto& b : hist) b.store(0, std::memory_order_relaxed);
        for (auto& t : m_totals) t.store(0, std::memory_order_relaxed);
    }

    static const char* StageName(PipelineStage s) {
        switch (s) {
        case PipelineStage::FileRead:     return "FileRead";
        case PipelineStage::Decompress:   return "Decompress";
        case PipelineStage::Decode:       return "Decode";
        case PipelineStage::ColorConvert: return "ColorConvert";
        case PipelineStage::Scale:        return "Scale";
        case PipelineStage::Render:       return "Render";
        case PipelineStage::ShellDeliver: return "ShellDeliver";
        default:                          return "Unknown";
        }
    }

    static BottleneckStage DetectBottleneck(const PipelineSnapshot& snap) {
        double ioMs  = snap.stages[static_cast<size_t>(PipelineStage::FileRead)].p99Ms +
                       snap.stages[static_cast<size_t>(PipelineStage::Decompress)].p99Ms;
        double cpuMs = snap.stages[static_cast<size_t>(PipelineStage::Decode)].p99Ms +
                       snap.stages[static_cast<size_t>(PipelineStage::ColorConvert)].p99Ms +
                       snap.stages[static_cast<size_t>(PipelineStage::Scale)].p99Ms;
        double gpuMs = snap.stages[static_cast<size_t>(PipelineStage::Render)].p99Ms;
        double dlvMs = snap.stages[static_cast<size_t>(PipelineStage::ShellDeliver)].p99Ms;

        double maxMs = std::max({ioMs, cpuMs, gpuMs, dlvMs});
        if (maxMs < 1.0) return BottleneckStage::None;
        if (maxMs == ioMs)  return BottleneckStage::IO;
        if (maxMs == cpuMs) return BottleneckStage::CPU;
        if (maxMs == gpuMs) return BottleneckStage::GPU;
        return BottleneckStage::ShellDeliver;
    }

    static const char* DescribeBottleneck(BottleneckStage b) {
        switch (b) {
        case BottleneckStage::IO:           return "File I/O & decompression are primary bottleneck";
        case BottleneckStage::CPU:          return "CPU decode and colour conversion are primary bottleneck";
        case BottleneckStage::GPU:          return "GPU render path is primary bottleneck";
        case BottleneckStage::ShellDeliver: return "IPC to Explorer Shell is primary bottleneck";
        default:                            return "No bottleneck detected — pipeline balanced";
        }
    }

private:
    ThumbnailPipelineMetrics() {
        for (auto& hist : m_histograms)
            for (auto& b : hist) b.store(0);
        for (auto& t : m_totals) t.store(0);
    }

    using Histogram = std::array<std::atomic<uint64_t>, HIST_BUCKETS>;

    std::array<Histogram, static_cast<size_t>(PipelineStage::Count)> m_histograms;
    std::array<std::atomic<uint64_t>, static_cast<size_t>(PipelineStage::Count)> m_totals;
};

}  // namespace Engine
}  // namespace ExplorerLens
