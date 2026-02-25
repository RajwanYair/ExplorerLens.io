#pragma once
// NeuralTextureCompression.h — Neural Texture Compression
// ML-driven texture compression using trained neural codecs,
// achieving superior quality at same bitrate vs traditional BC7.
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Neural codec backend
enum class NeuralCodecBackend : uint8_t {
  CPU_Reference = 0, // Slow CPU reference decoder
  DirectML,          // DirectML inference
  ONNX_Runtime,      // ONNX Runtime (CPU/GPU)
  TensorRT,          // NVIDIA TensorRT
  OpenVINO,          // Intel OpenVINO
  COUNT
};

/// Compression model complexity
enum class NeuralModelTier : uint8_t {
  Tiny = 0, // <1M params, fastest decode
  Small,    // ~5M params, balanced
  Medium,   // ~20M params, high quality
  Large,    // ~80M params, best quality
  COUNT
};

struct NeuralCompressionStats {
  double compressionRatio = 0.0;
  double psnrDb = 0.0;
  double ssim = 0.0;
  double encodeTimeMs = 0.0;
  double decodeTimeMs = 0.0;
  size_t modelSizeBytes = 0;
  uint32_t batchesProcessed = 0;
  NeuralCodecBackend activeBackend = NeuralCodecBackend::CPU_Reference;
};

struct NeuralCompressionConfig {
  bool enabled = false; // Experimental feature
  NeuralCodecBackend backend = NeuralCodecBackend::DirectML;
  NeuralModelTier modelTier = NeuralModelTier::Small;
  float qualityTarget = 0.90f; // SSIM target
  uint32_t batchSize = 16;
  bool quantizeWeights = true; // INT8 quantization
  bool cacheDecoded = true;
};

class NeuralTextureCompression {
public:
  static constexpr size_t BackendCount() {
    return static_cast<size_t>(NeuralCodecBackend::COUNT);
  }
  static constexpr size_t ModelTierCount() {
    return static_cast<size_t>(NeuralModelTier::COUNT);
  }

  static const wchar_t *BackendName(NeuralCodecBackend b) {
    switch (b) {
    case NeuralCodecBackend::CPU_Reference:
      return L"CPU Reference";
    case NeuralCodecBackend::DirectML:
      return L"DirectML";
    case NeuralCodecBackend::ONNX_Runtime:
      return L"ONNX Runtime";
    case NeuralCodecBackend::TensorRT:
      return L"TensorRT";
    case NeuralCodecBackend::OpenVINO:
      return L"OpenVINO";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *ModelTierName(NeuralModelTier t) {
    switch (t) {
    case NeuralModelTier::Tiny:
      return L"Tiny (<1M params)";
    case NeuralModelTier::Small:
      return L"Small (~5M params)";
    case NeuralModelTier::Medium:
      return L"Medium (~20M params)";
    case NeuralModelTier::Large:
      return L"Large (~80M params)";
    default:
      return L"Unknown";
    }
  }

  /// Estimate decode time multiplier relative to BC7
  static constexpr double DecodeTimeFactor(NeuralModelTier tier,
                                           NeuralCodecBackend backend) {
    double baseFactor = 1.0;
    switch (tier) {
    case NeuralModelTier::Tiny:
      baseFactor = 1.2;
      break;
    case NeuralModelTier::Small:
      baseFactor = 2.5;
      break;
    case NeuralModelTier::Medium:
      baseFactor = 5.0;
      break;
    case NeuralModelTier::Large:
      baseFactor = 12.0;
      break;
    default:
      break;
    }
    // GPU backends are much faster
    if (backend == NeuralCodecBackend::DirectML ||
        backend == NeuralCodecBackend::TensorRT)
      baseFactor *= 0.15;
    else if (backend == NeuralCodecBackend::ONNX_Runtime ||
             backend == NeuralCodecBackend::OpenVINO)
      baseFactor *= 0.3;
    return baseFactor;
  }
};

} // namespace Engine
} // namespace ExplorerLens
