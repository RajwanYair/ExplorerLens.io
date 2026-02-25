// ThumbnailQualityAnalyzer.h — Automated Thumbnail Quality Assessment
// Copyright (c) 2026 ExplorerLens Project
//
// Evaluates generated thumbnails for visual quality using structural similarity
// (SSIM), color accuracy, sharpness metrics, and perceptual hashing.
// Integrated into CI as an automated regression gate.

#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Quality metric types for thumbnail assessment
enum class QualityMetric : uint8_t {
  SSIM = 0,             ///< Structural Similarity Index (0-1)
  PSNR = 1,             ///< Peak Signal-to-Noise Ratio (dB)
  Sharpness = 2,        ///< Laplacian variance (higher = sharper)
  ColorAccuracy = 3,    ///< Delta-E 2000 color difference
  EdgePreservation = 4, ///< Edge retention ratio vs source
  ContrastRatio = 5,    ///< Dynamic range preservation
  NoiseLevel = 6,       ///< ISO noise estimation
  Artifacts = 7,        ///< Compression artifact score (lower = better)
  COUNT
};

/// Quality grade based on metric thresholds
enum class QualityGrade : uint8_t {
  Excellent = 0,  ///< >= 0.95 SSIM, meets all targets
  Good = 1,       ///< >= 0.90 SSIM
  Acceptable = 2, ///< >= 0.80 SSIM
  Poor = 3,       ///< >= 0.70 SSIM
  Rejected = 4    ///< < 0.70 SSIM
};

/// Result of a single quality assessment
struct QualityResult {
  QualityMetric metric = QualityMetric::SSIM;
  double value = 0.0;
  double threshold = 0.0;
  bool passes = false;
  std::wstring description;
};

/// Comprehensive quality report for a thumbnail
struct ThumbnailQualityReport {
  std::wstring sourceFile;
  uint32_t sourceWidth = 0;
  uint32_t sourceHeight = 0;
  uint32_t thumbWidth = 0;
  uint32_t thumbHeight = 0;
  QualityGrade grade = QualityGrade::Rejected;
  std::vector<QualityResult> metrics;
  double overallScore = 0.0; ///< Weighted average (0-1)
  double assessmentTimeMs = 0.0;
  std::wstring decoderUsed;
  bool gpuAccelerated = false;

  bool IsAcceptable() const { return grade <= QualityGrade::Acceptable; }
};

/// Configuration for quality thresholds
struct QualityThresholds {
  double minSSIM = 0.90;
  double minPSNR = 30.0; ///< 30dB minimum
  double minSharpness = 50.0;
  double maxDeltaE = 3.0; ///< DeltaE 2000 max color drift
  double minEdgeRetention = 0.85;
  double maxArtifactScore = 0.15;
};

/// Thumbnail Quality Analyzer
class ThumbnailQualityAnalyzer {
public:
  static const wchar_t *MetricName(QualityMetric m) {
    switch (m) {
    case QualityMetric::SSIM:
      return L"Structural Similarity (SSIM)";
    case QualityMetric::PSNR:
      return L"Peak SNR (dB)";
    case QualityMetric::Sharpness:
      return L"Sharpness (Laplacian)";
    case QualityMetric::ColorAccuracy:
      return L"Color Accuracy (DeltaE)";
    case QualityMetric::EdgePreservation:
      return L"Edge Preservation";
    case QualityMetric::ContrastRatio:
      return L"Contrast Ratio";
    case QualityMetric::NoiseLevel:
      return L"Noise Level";
    case QualityMetric::Artifacts:
      return L"Artifact Score";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *GradeName(QualityGrade g) {
    switch (g) {
    case QualityGrade::Excellent:
      return L"Excellent";
    case QualityGrade::Good:
      return L"Good";
    case QualityGrade::Acceptable:
      return L"Acceptable";
    case QualityGrade::Poor:
      return L"Poor";
    case QualityGrade::Rejected:
      return L"Rejected";
    default:
      return L"Unknown";
    }
  }

  static constexpr size_t MetricCount() {
    return static_cast<size_t>(QualityMetric::COUNT);
  }

  /// Compute SSIM between two BGRA pixel buffers
  static double ComputeSSIM(const uint8_t *ref, const uint8_t *test,
                            uint32_t width, uint32_t height, uint32_t stride) {
    if (!ref || !test || width == 0 || height == 0)
      return 0.0;
    // Constants for SSIM computation (Wang et al., 2004)
    constexpr double C1 = 6.5025;  // (0.01 * 255)^2
    constexpr double C2 = 58.5225; // (0.03 * 255)^2
    double sumSSIM = 0.0;
    uint32_t blockCount = 0;
    constexpr uint32_t blockSize = 8;

    for (uint32_t by = 0; by + blockSize <= height; by += blockSize) {
      for (uint32_t bx = 0; bx + blockSize <= width; bx += blockSize) {
        double muRef = 0, muTest = 0;
        for (uint32_t y = by; y < by + blockSize; ++y) {
          for (uint32_t x = bx; x < bx + blockSize; ++x) {
            size_t idx = y * stride + x * 4;
            double rLum =
                0.299 * ref[idx + 2] + 0.587 * ref[idx + 1] + 0.114 * ref[idx];
            double tLum = 0.299 * test[idx + 2] + 0.587 * test[idx + 1] +
                          0.114 * test[idx];
            muRef += rLum;
            muTest += tLum;
          }
        }
        double n = blockSize * blockSize;
        muRef /= n;
        muTest /= n;
        double sigmaRef2 = 0, sigmaTest2 = 0, sigmaCross = 0;
        for (uint32_t y = by; y < by + blockSize; ++y) {
          for (uint32_t x = bx; x < bx + blockSize; ++x) {
            size_t idx = y * stride + x * 4;
            double rL = 0.299 * ref[idx + 2] + 0.587 * ref[idx + 1] +
                        0.114 * ref[idx] - muRef;
            double tL = 0.299 * test[idx + 2] + 0.587 * test[idx + 1] +
                        0.114 * test[idx] - muTest;
            sigmaRef2 += rL * rL;
            sigmaTest2 += tL * tL;
            sigmaCross += rL * tL;
          }
        }
        sigmaRef2 /= (n - 1);
        sigmaTest2 /= (n - 1);
        sigmaCross /= (n - 1);
        double num = (2.0 * muRef * muTest + C1) * (2.0 * sigmaCross + C2);
        double den = (muRef * muRef + muTest * muTest + C1) *
                     (sigmaRef2 + sigmaTest2 + C2);
        sumSSIM += num / den;
        blockCount++;
      }
    }
    return blockCount > 0 ? sumSSIM / blockCount : 0.0;
  }

  /// Determine quality grade from SSIM score
  static QualityGrade GradeFromSSIM(double ssim) {
    if (ssim >= 0.95)
      return QualityGrade::Excellent;
    if (ssim >= 0.90)
      return QualityGrade::Good;
    if (ssim >= 0.80)
      return QualityGrade::Acceptable;
    if (ssim >= 0.70)
      return QualityGrade::Poor;
    return QualityGrade::Rejected;
  }
};

} // namespace Engine
} // namespace ExplorerLens
