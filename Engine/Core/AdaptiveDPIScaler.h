#pragma once
// Sprint 402: Adaptive DPI Scaler
// Per-monitor DPI-aware thumbnail scaling with fractional scale factors,
// ensuring pixel-perfect rendering on mixed-DPI multi-monitor setups.
#include <algorithm>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// DPI scaling strategy
enum class DPIStrategy : uint8_t {
  NearestNeighbor = 0, // Fast, pixelated at non-integer scales
  Bilinear,            // Smooth, slight blur
  Bicubic,             // Sharper edges, good default
  Lanczos3,            // Best quality, slowest
  AdaptiveHybrid,      // Switch strategy based on scale factor
  COUNT
};

/// Display DPI tier for quick classification
enum class DPITier : uint8_t {
  Standard = 0, // 96 DPI (100%)
  Medium,       // 120 DPI (125%)
  High,         // 144 DPI (150%)
  VeryHigh,     // 192 DPI (200%)
  Ultra,        // 288+ DPI (300%+)
  COUNT
};

struct DPIScaleConfig {
  float scaleFactor = 1.0f;
  DPIStrategy strategy = DPIStrategy::Bicubic;
  bool perMonitorAware = true;
  bool fractionalScaling = true;
  uint32_t baseThumbnailPx = 256;
};

class AdaptiveDPIScaler {
public:
  static constexpr size_t StrategyCount() {
    return static_cast<size_t>(DPIStrategy::COUNT);
  }
  static constexpr size_t TierCount() {
    return static_cast<size_t>(DPITier::COUNT);
  }

  static const wchar_t *StrategyName(DPIStrategy s) {
    switch (s) {
    case DPIStrategy::NearestNeighbor:
      return L"Nearest Neighbor";
    case DPIStrategy::Bilinear:
      return L"Bilinear";
    case DPIStrategy::Bicubic:
      return L"Bicubic";
    case DPIStrategy::Lanczos3:
      return L"Lanczos-3";
    case DPIStrategy::AdaptiveHybrid:
      return L"Adaptive Hybrid";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *TierName(DPITier t) {
    switch (t) {
    case DPITier::Standard:
      return L"Standard (96 DPI)";
    case DPITier::Medium:
      return L"Medium (120 DPI)";
    case DPITier::High:
      return L"High (144 DPI)";
    case DPITier::VeryHigh:
      return L"Very High (192 DPI)";
    case DPITier::Ultra:
      return L"Ultra (288+ DPI)";
    default:
      return L"Unknown";
    }
  }

  static DPITier ClassifyDPI(uint32_t dpi) {
    if (dpi <= 96)
      return DPITier::Standard;
    if (dpi <= 120)
      return DPITier::Medium;
    if (dpi <= 144)
      return DPITier::High;
    if (dpi <= 192)
      return DPITier::VeryHigh;
    return DPITier::Ultra;
  }

  static uint32_t ScaledSize(uint32_t basePx, float scaleFactor) {
    return static_cast<uint32_t>(basePx * scaleFactor + 0.5f);
  }

  static DPIStrategy RecommendStrategy(float scaleFactor) {
    float frac = scaleFactor - static_cast<int>(scaleFactor);
    if (frac < 0.01f)
      return DPIStrategy::NearestNeighbor; // integer scale
    if (scaleFactor < 1.5f)
      return DPIStrategy::Bilinear;
    return DPIStrategy::Lanczos3;
  }
};

} // namespace Engine
} // namespace ExplorerLens
