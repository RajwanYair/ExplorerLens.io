#pragma once
// SmartQualityPredictor.h — Smart Quality Predictor
// Predicts optimal output quality (JPEG quality, scale factor, sharpening)
// based on source characteristics — avoiding wasted compute on simple images.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Image complexity tier
enum class ImageComplexity : uint8_t {
  Trivial = 0, // Solid color / gradient
  Simple,      // Icons, diagrams
  Moderate,    // Photos with uniform areas
  Complex,     // Detailed photos, textures
  Extreme,     // High-frequency noise, fractals
  COUNT
};

/// Quality prediction confidence
enum class PredictionConfidence : uint8_t {
  Low = 0,
  Medium,
  High,
  VeryHigh,
  COUNT
};

struct QualityPrediction {
  uint8_t jpegQuality = 85; // 1-100
  float scaleFactor = 1.0f;
  float sharpenStrength = 0.0f; // 0-1
  bool useChromaSubsampling = true;
  ImageComplexity complexity = ImageComplexity::Moderate;
  PredictionConfidence confidence = PredictionConfidence::Medium;
};

class SmartQualityPredictor {
public:
  static constexpr size_t ComplexityCount() {
    return static_cast<size_t>(ImageComplexity::COUNT);
  }
  static constexpr size_t ConfidenceCount() {
    return static_cast<size_t>(PredictionConfidence::COUNT);
  }

  static const wchar_t *ComplexityName(ImageComplexity c) {
    switch (c) {
    case ImageComplexity::Trivial:
      return L"Trivial";
    case ImageComplexity::Simple:
      return L"Simple";
    case ImageComplexity::Moderate:
      return L"Moderate";
    case ImageComplexity::Complex:
      return L"Complex";
    case ImageComplexity::Extreme:
      return L"Extreme";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *ConfidenceName(PredictionConfidence c) {
    switch (c) {
    case PredictionConfidence::Low:
      return L"Low";
    case PredictionConfidence::Medium:
      return L"Medium";
    case PredictionConfidence::High:
      return L"High";
    case PredictionConfidence::VeryHigh:
      return L"Very High";
    default:
      return L"Unknown";
    }
  }

  /// Predict JPEG quality from image complexity
  static uint8_t PredictJPEGQuality(ImageComplexity c) {
    switch (c) {
    case ImageComplexity::Trivial:
      return 70;
    case ImageComplexity::Simple:
      return 75;
    case ImageComplexity::Moderate:
      return 85;
    case ImageComplexity::Complex:
      return 90;
    case ImageComplexity::Extreme:
      return 95;
    default:
      return 85;
    }
  }

  /// Predict sharpening strength from complexity
  static float PredictSharpen(ImageComplexity c) {
    switch (c) {
    case ImageComplexity::Trivial:
      return 0.0f;
    case ImageComplexity::Simple:
      return 0.1f;
    case ImageComplexity::Moderate:
      return 0.3f;
    case ImageComplexity::Complex:
      return 0.5f;
    case ImageComplexity::Extreme:
      return 0.2f; // back off to avoid noise
    default:
      return 0.3f;
    }
  }
};

} // namespace Engine
} // namespace ExplorerLens
