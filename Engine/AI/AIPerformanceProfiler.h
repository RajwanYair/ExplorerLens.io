// AIPerformanceProfiler.h — AI Module Latency and Energy Cost Profiler
// Copyright (c) 2026 ExplorerLens Project
//
// Measures per-inference latency, VRAM delta, and GPU energy draw for each
// AI module in the thumbnail pipeline, enabling per-device calibration of
// which AI stages are within the 17ms shell extension latency budget.
//
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---- Per-Module Profile -----------------------------------------------------

struct AIModuleProfile
{
    std::string moduleName;  // e.g. "ContentCategoryClassifier"
    uint64_t sampleCount = 0;
    float p50Ms = 0.0f;
    float p95Ms = 0.0f;
    float p99Ms = 0.0f;
    float maxMs = 0.0f;
    uint64_t vramDeltaKB = 0;     // Peak VRAM increase during inference
    float energyUJPerInf = 0.0f;  // Micro-joules per inference (GPU PM)
    bool withinBudget = true;     // p95Ms <= budgetMs
    float budgetMs = 3.0f;        // Per-module SLO contribution
};

// ---- Profiler ---------------------------------------------------------------

class AIPerformanceProfiler
{
  public:
    AIPerformanceProfiler();
    ~AIPerformanceProfiler();

    // Begin recording a timed inference span.
    uint64_t BeginSpan(const std::string& moduleName);

    // End a span and record the sample.
    void EndSpan(uint64_t spanId, uint64_t vramDeltaKB = 0);

    // Get aggregated profile for a module.
    AIModuleProfile GetProfile(const std::string& moduleName) const;

    // Get all module profiles.
    std::vector<AIModuleProfile> GetAllProfiles() const;

    // Recommend which AI stages to disable to stay within total budget.
    std::vector<std::string> RecommendDisable(float totalBudgetMs = 5.0f) const;

    // Export profiles as JSON string.
    std::string ToJson() const;

    // Reset all counters.
    void Reset();

    static AIPerformanceProfiler& Instance();

  private:
    struct Impl
    {};
    std::unique_ptr<Impl> m_impl;
};

}  // namespace Engine
}  // namespace ExplorerLens
