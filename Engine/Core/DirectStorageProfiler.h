// DirectStorageProfiler.h — Latency profiler for DirectStorage vs CPU decode paths
// Copyright (c) 2026 ExplorerLens Project
//
// Instruments DirectStorage I/O + GPU decompression latency alongside the
// traditional CPU decode path so the adaptive scheduler can detect regressions
// and route workloads in real time. Reports P50/P95/P99 per decode path
// and feeds budget data into ThumbnailPipelineMetrics.
//
#pragma once

#include <cstdint>
#include <string_view>

namespace ExplorerLens { namespace Engine {

enum class DSDecodePath : uint8_t { DIRECT_STORAGE, CPU_DECODE };

struct DSProfileSample {
    DSDecodePath path          = DSDecodePath::CPU_DECODE;
    float        ioMs          = 0.0f;
    float        decompressMs  = 0.0f;
    float        decodeMs      = 0.0f;
    float        totalMs       = 0.0f;
    uint32_t     fileSizeBytes = 0;
};

struct DSProfileStats {
    float    p50TotalMs   = 0.0f;
    float    p95TotalMs   = 0.0f;
    float    p99TotalMs   = 0.0f;
    uint32_t sampleCount  = 0;
    uint32_t dsPathCount  = 0;
    uint32_t cpuPathCount = 0;
};

class DirectStorageProfiler {
public:
    static DirectStorageProfiler& Instance();

    void           AddSample(const DSProfileSample& s) noexcept;
    DSProfileStats ComputeStats()                       const noexcept;
    void           Reset()                              noexcept;
    DSDecodePath   RecommendedPath(uint32_t fileSizeBytes) const noexcept;

    static std::string_view PathName(DSDecodePath p) noexcept;

private:
    static constexpr uint32_t MAX_SAMPLES = 1024;

    DSProfileSample m_samples[MAX_SAMPLES]{};
    uint32_t        m_count = 0;
};

}} // namespace ExplorerLens::Engine
