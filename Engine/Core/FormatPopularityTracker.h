#pragma once
// Sprint 438: Format Popularity Tracker
// Tracks format usage statistics to optimize decoder loading order,
// cache eviction priority, and resource allocation.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Popularity tier
enum class PopularityTier : uint8_t {
  Dominant = 0, // > 30% of all thumbnails
  Popular,      // 10-30%
  Common,       // 1-10%
  Rare,         // 0.1-1%
  Exotic,       // < 0.1%
  COUNT
};

/// Trend direction
enum class TrendDirection : uint8_t {
  Rising = 0,
  Stable,
  Declining,
  NewFormat, // Recently first seen
  COUNT
};

struct FormatPopularity {
  const wchar_t *extension = nullptr;
  uint64_t totalRequests = 0;
  float percentage = 0.0f;
  PopularityTier tier = PopularityTier::Rare;
  TrendDirection trend = TrendDirection::Stable;
  double avgDecodeMs = 0.0;
};

struct PopularityConfig {
  uint32_t trackingWindowDays = 30;
  uint32_t maxTrackedFormats = 200;
  bool adjustDecoderOrder = true;
  bool adjustCachePriority = true;
};

class FormatPopularityTracker {
public:
  static constexpr size_t TierCount() {
    return static_cast<size_t>(PopularityTier::COUNT);
  }
  static constexpr size_t DirectionCount() {
    return static_cast<size_t>(TrendDirection::COUNT);
  }

  static const wchar_t *TierName(PopularityTier t) {
    switch (t) {
    case PopularityTier::Dominant:
      return L"Dominant";
    case PopularityTier::Popular:
      return L"Popular";
    case PopularityTier::Common:
      return L"Common";
    case PopularityTier::Rare:
      return L"Rare";
    case PopularityTier::Exotic:
      return L"Exotic";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *DirectionName(TrendDirection d) {
    switch (d) {
    case TrendDirection::Rising:
      return L"Rising";
    case TrendDirection::Stable:
      return L"Stable";
    case TrendDirection::Declining:
      return L"Declining";
    case TrendDirection::NewFormat:
      return L"New";
    default:
      return L"Unknown";
    }
  }

  /// Classify format into popularity tier
  static PopularityTier Classify(float percentage) {
    if (percentage > 30.0f)
      return PopularityTier::Dominant;
    if (percentage > 10.0f)
      return PopularityTier::Popular;
    if (percentage > 1.0f)
      return PopularityTier::Common;
    if (percentage > 0.1f)
      return PopularityTier::Rare;
    return PopularityTier::Exotic;
  }
};

} // namespace Engine
} // namespace ExplorerLens
