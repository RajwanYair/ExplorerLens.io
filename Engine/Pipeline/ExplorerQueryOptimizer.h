// ExplorerQueryOptimizer.h — Shell Query Plan Optimization
// Copyright (c) 2026 ExplorerLens Project
//
// Analyzes patterns in Explorer thumbnail requests and optimizes the
// decode order using batch prefetching and predictive scheduling.
//
#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class QueryPattern : uint8_t {
    Sequential = 0,  // User scrolling through folder
    Random = 1,      // Jumping between locations
    Repeated = 2,    // Same folder re-opened
    Batch = 3        // Folder opened for first time (bulk)
};

struct QueryHint
{
    std::wstring folderPath;
    uint32_t estimatedFileCount = 0;
    QueryPattern pattern = QueryPattern::Sequential;
    uint32_t visibleWindowSize = 40;
    double avgDecodeMs = 0.0;
};

struct OptimizedPlan
{
    std::vector<std::wstring> prefetchOrder;
    uint32_t batchSize = 8;
    uint32_t concurrency = 4;
    bool enableSpeculativeDecode = false;
    double estimatedCompletionMs = 0.0;
};

struct QueryOptimizerStats
{
    uint64_t totalQueries = 0;
    uint64_t prefetchHits = 0;
    uint64_t prefetchMisses = 0;
    double avgPlanTimeMs = 0.0;
    double hitRate() const
    {
        return totalQueries > 0 ? 100.0 * prefetchHits / totalQueries : 0.0;
    }
};

class ExplorerQueryOptimizer
{
  public:
    QueryPattern DetectPattern(const std::deque<std::wstring>& recentPaths) const
    {
        if (recentPaths.size() < 2)
            return QueryPattern::Batch;
        // Check if paths share same parent directory
        bool sameFolder = true;
        auto parentOf = [](const std::wstring& p) -> std::wstring {
            auto pos = p.find_last_of(L"\\/");
            return pos != std::wstring::npos ? p.substr(0, pos) : p;
        };
        std::wstring firstParent = parentOf(recentPaths.front());
        for (const auto& p : recentPaths) {
            if (parentOf(p) != firstParent) {
                sameFolder = false;
                break;
            }
        }
        if (!sameFolder)
            return QueryPattern::Random;
        return QueryPattern::Sequential;
    }

    OptimizedPlan CreatePlan(const QueryHint& hint) const
    {
        OptimizedPlan plan;
        switch (hint.pattern) {
            case QueryPattern::Sequential:
                plan.batchSize = 16;
                plan.concurrency = 4;
                plan.enableSpeculativeDecode = true;
                break;
            case QueryPattern::Batch:
                plan.batchSize = 32;
                plan.concurrency = 8;
                plan.enableSpeculativeDecode = false;
                break;
            case QueryPattern::Random:
                plan.batchSize = 4;
                plan.concurrency = 2;
                plan.enableSpeculativeDecode = false;
                break;
            case QueryPattern::Repeated:
                plan.batchSize = 8;
                plan.concurrency = 4;
                plan.enableSpeculativeDecode = false;
                break;
        }
        plan.estimatedCompletionMs = hint.avgDecodeMs * hint.estimatedFileCount / plan.concurrency;
        return plan;
    }

    QueryOptimizerStats GetStats() const
    {
        return m_stats;
    }

  private:
    QueryOptimizerStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
