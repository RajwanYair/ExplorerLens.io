#pragma once
// ContentAwareThumbnailSelector.h — Content-Aware Thumbnail Selector
// ML-driven selection of the most visually interesting region for thumbnail
// cropping, using saliency maps, rule-of-thirds, and face-aware composition.
#include <algorithm>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Saliency detection algorithm
enum class SaliencyAlgorithm : uint8_t {
  CenterBias = 0,    // Simple center-weighted
  HistogramContrast, // Color histogram difference
  SpectralResidual,  // Frequency domain saliency
  FineGrained,       // Fine-grained saliency map
  DeepSaliency,      // Neural network (requires DirectML)
  COUNT
};

/// Crop composition rule
enum class CompositionRule : uint8_t {
  CenterCrop = 0, // Simple center crop
  RuleOfThirds,   // Align to thirds grid
  GoldenRatio,    // Fibonacci spiral placement
  FaceAware,      // Center on detected faces
  SubjectAware,   // Center on main subject
  COUNT
};

struct SaliencyRegion {
  float x = 0.0f; // normalized 0-1
  float y = 0.0f;
  float width = 1.0f;
  float height = 1.0f;
  float score = 0.0f; // 0-1 saliency strength
};

struct CropResult {
  uint32_t srcX = 0;
  uint32_t srcY = 0;
  uint32_t srcW = 0;
  uint32_t srcH = 0;
  CompositionRule ruleUsed = CompositionRule::CenterCrop;
  float confidence = 0.0f;
};

class ContentAwareThumbnailSelector {
public:
  static constexpr size_t AlgorithmCount() {
    return static_cast<size_t>(SaliencyAlgorithm::COUNT);
  }
  static constexpr size_t RuleCount() {
    return static_cast<size_t>(CompositionRule::COUNT);
  }

  static const wchar_t *AlgorithmName(SaliencyAlgorithm a) {
    switch (a) {
    case SaliencyAlgorithm::CenterBias:
      return L"Center Bias";
    case SaliencyAlgorithm::HistogramContrast:
      return L"Histogram Contrast";
    case SaliencyAlgorithm::SpectralResidual:
      return L"Spectral Residual";
    case SaliencyAlgorithm::FineGrained:
      return L"Fine-Grained";
    case SaliencyAlgorithm::DeepSaliency:
      return L"Deep Saliency (ML)";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *RuleName(CompositionRule r) {
    switch (r) {
    case CompositionRule::CenterCrop:
      return L"Center Crop";
    case CompositionRule::RuleOfThirds:
      return L"Rule of Thirds";
    case CompositionRule::GoldenRatio:
      return L"Golden Ratio";
    case CompositionRule::FaceAware:
      return L"Face-Aware";
    case CompositionRule::SubjectAware:
      return L"Subject-Aware";
    default:
      return L"Unknown";
    }
  }

  /// Calculate center crop region for target aspect ratio
  static CropResult CenterCropCalc(uint32_t srcW, uint32_t srcH,
                                   float targetAspect) {
    CropResult r;
    r.ruleUsed = CompositionRule::CenterCrop;
    r.confidence = 1.0f;
    float srcAspect = (srcH > 0) ? (float)srcW / srcH : 1.0f;
    if (srcAspect > targetAspect) {
      r.srcH = srcH;
      r.srcW = static_cast<uint32_t>(srcH * targetAspect);
      r.srcX = (srcW - r.srcW) / 2;
      r.srcY = 0;
    } else {
      r.srcW = srcW;
      r.srcH = static_cast<uint32_t>(srcW / targetAspect);
      r.srcX = 0;
      r.srcY = (srcH - r.srcH) / 2;
    }
    return r;
  }
};

} // namespace Engine
} // namespace ExplorerLens
