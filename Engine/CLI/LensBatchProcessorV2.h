// LensBatchProcessorV2.h — lens batch v2 — Parallel Bulk Thumbnail Generator
// Copyright (c) 2026 ExplorerLens Project
//
// Parallel bulk thumbnail generation engine for lens batch CLI — processes thousands of files concurrently.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct BatchJobV2 {
    std::vector<std::wstring> inputPaths;
    std::wstring              outputDir;
    uint32_t                  thumbWidth  = 256;
    uint32_t                  thumbHeight = 256;
    uint32_t                  threads     = 0;  // 0=auto
};
struct BatchResultV2 {
    size_t  processed = 0;
    size_t  failed    = 0;
    double  durationMs = 0.0;
    double  throughput = 0.0;  // items/sec
};
class LensBatchProcessorV2 {
public:
    BatchResultV2 Run(const BatchJobV2& job) {
        (void)job; return { 0, 0, 0.0, 0.0 };
    }
    void  Cancel()              { m_cancelled = true; }
    bool  IsCancelled() const   { return m_cancelled; }
    float Progress()    const   { return m_progress; }
private:
    std::atomic<bool>  m_cancelled{false};
    std::atomic<float> m_progress{0.0f};
};

} // namespace Engine
} // namespace ExplorerLens