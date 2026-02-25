#pragma once
// CacheEfficiencyAnalyzer.h — Cache Efficiency Analyzer
// Analyzes cache performance patterns — identifies cold spots, wasted capacity,
// optimal eviction parameters, and recommends budget adjustments.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Cache zone classification
enum class CacheZone : uint8_t {
  Hot = 0, // Frequently accessed (> 10 hits/min)
  Warm,    // Moderately accessed
  Cold,    // Rarely accessed (< 1 hit/hour)
  Frozen,  // Never re-accessed after initial cache
  Evicted, // Currently evicted
  COUNT
};

/// Cache recommendation
enum class CacheRecommendation : uint8_t {
  NoChange = 0,
  IncreaseBudget,
  DecreaseBudget,
  ChangeTTL,
  ChangeEvictionPolicy,
  EnableCompression,
  COUNT
};

struct CacheAnalysis {
  float hitRate = 0.0f;
  float missRate = 0.0f;
  float evictionRate = 0.0f;
  float hotZonePct = 0.0f;
  float coldZonePct = 0.0f;
  float wastedCapacityMB = 0.0f;
  uint64_t totalEntries = 0;
  uint64_t totalSizeBytes = 0;
  CacheRecommendation recommendation = CacheRecommendation::NoChange;
};

class CacheEfficiencyAnalyzer {
public:
  static constexpr size_t ZoneCount() {
    return static_cast<size_t>(CacheZone::COUNT);
  }
  static constexpr size_t RecommendationCount() {
    return static_cast<size_t>(CacheRecommendation::COUNT);
  }

  static const wchar_t *ZoneName(CacheZone z) {
    switch (z) {
    case CacheZone::Hot:
      return L"Hot";
    case CacheZone::Warm:
      return L"Warm";
    case CacheZone::Cold:
      return L"Cold";
    case CacheZone::Frozen:
      return L"Frozen";
    case CacheZone::Evicted:
      return L"Evicted";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *RecommendationName(CacheRecommendation r) {
    switch (r) {
    case CacheRecommendation::NoChange:
      return L"No Change";
    case CacheRecommendation::IncreaseBudget:
      return L"Increase Budget";
    case CacheRecommendation::DecreaseBudget:
      return L"Decrease Budget";
    case CacheRecommendation::ChangeTTL:
      return L"Change TTL";
    case CacheRecommendation::ChangeEvictionPolicy:
      return L"Change Eviction Policy";
    case CacheRecommendation::EnableCompression:
      return L"Enable Compression";
    default:
      return L"Unknown";
    }
  }

  /// Recommend action based on cache analysis
  static CacheRecommendation Analyze(float hitRate, float coldPct,
                                     float wastedMB) {
    if (hitRate < 0.5f)
      return CacheRecommendation::IncreaseBudget;
    if (coldPct > 0.4f)
      return CacheRecommendation::ChangeTTL;
    if (wastedMB > 100.0f)
      return CacheRecommendation::DecreaseBudget;
    if (coldPct > 0.2f)
      return CacheRecommendation::EnableCompression;
    return CacheRecommendation::NoChange;
  }
};

} // namespace Engine
} // namespace ExplorerLens
