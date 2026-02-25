#pragma once
// GPUTextureCompressionPipeline.h — GPU Texture Compression Pipeline
// Real-time BCn/ASTC/ETC2 compression for thumbnail atlas storage,
// reducing VRAM footprint by 4-8x while maintaining visual quality.
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

/// GPU texture compression format
enum class TextureFormat : uint8_t {
  BC1_RGB = 0,       // DXT1 — 4bpp, opaque
  BC3_RGBA,          // DXT5 — 8bpp, full alpha
  BC4_R,             // Single channel — 4bpp
  BC5_RG,            // Two channel — 8bpp (normal maps)
  BC7_RGBA,          // High quality RGBA — 8bpp
  ASTC_4x4,          // ASTC 4x4 — mobile/ARM
  ASTC_6x6,          // ASTC 6x6 — lower quality, smaller
  ETC2_RGB,          // ETC2 — OpenGL ES 3.0+
  Uncompressed_BGRA, // 32bpp reference
  COUNT
};

/// Compression quality
enum class CompressionQuality : uint8_t {
  UltraFast = 0, // Fastest, lowest quality (~1ms/256x256)
  Fast,          // Good speed, reasonable quality
  Balanced,      // Default — quality/speed tradeoff
  High,          // Slow, high quality
  Ultra,         // Slowest, highest quality
  COUNT
};

/// Compression statistics
struct TextureCompressionStats {
  uint64_t inputSizeBytes = 0;
  uint64_t outputSizeBytes = 0;
  float compressionRatio = 1.0f;
  double compressTimeMs = 0.0;
  float psnrDB = 0.0f; // quality metric
  float ssim = 0.0f;   // structural similarity
};

class GPUTextureCompressionPipeline {
public:
  static constexpr size_t FormatCount() {
    return static_cast<size_t>(TextureFormat::COUNT);
  }
  static constexpr size_t QualityCount() {
    return static_cast<size_t>(CompressionQuality::COUNT);
  }

  static const wchar_t *FormatName(TextureFormat f) {
    switch (f) {
    case TextureFormat::BC1_RGB:
      return L"BC1 (DXT1)";
    case TextureFormat::BC3_RGBA:
      return L"BC3 (DXT5)";
    case TextureFormat::BC4_R:
      return L"BC4 (R)";
    case TextureFormat::BC5_RG:
      return L"BC5 (RG)";
    case TextureFormat::BC7_RGBA:
      return L"BC7 (RGBA)";
    case TextureFormat::ASTC_4x4:
      return L"ASTC 4x4";
    case TextureFormat::ASTC_6x6:
      return L"ASTC 6x6";
    case TextureFormat::ETC2_RGB:
      return L"ETC2 RGB";
    case TextureFormat::Uncompressed_BGRA:
      return L"BGRA 32bpp";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *QualityName(CompressionQuality q) {
    switch (q) {
    case CompressionQuality::UltraFast:
      return L"Ultra Fast";
    case CompressionQuality::Fast:
      return L"Fast";
    case CompressionQuality::Balanced:
      return L"Balanced";
    case CompressionQuality::High:
      return L"High";
    case CompressionQuality::Ultra:
      return L"Ultra";
    default:
      return L"Unknown";
    }
  }

  /// Calculate compressed size in bytes for given dimensions and format
  static uint64_t CalcCompressedSize(uint32_t width, uint32_t height,
                                     TextureFormat fmt) {
    uint32_t blocksW = (width + 3) / 4;
    uint32_t blocksH = (height + 3) / 4;
    uint32_t blockCount = blocksW * blocksH;
    switch (fmt) {
    case TextureFormat::BC1_RGB:
    case TextureFormat::BC4_R:
      return blockCount * 8ULL; // 8 bytes per 4x4 block
    case TextureFormat::BC3_RGBA:
    case TextureFormat::BC5_RG:
    case TextureFormat::BC7_RGBA:
      return blockCount * 16ULL; // 16 bytes per 4x4 block
    case TextureFormat::ASTC_4x4:
      return blockCount * 16ULL;
    case TextureFormat::ASTC_6x6: {
      uint32_t bw6 = (width + 5) / 6, bh6 = (height + 5) / 6;
      return bw6 * bh6 * 16ULL;
    }
    case TextureFormat::ETC2_RGB:
      return blockCount * 8ULL;
    case TextureFormat::Uncompressed_BGRA:
      return (uint64_t)width * height * 4;
    default:
      return (uint64_t)width * height * 4;
    }
  }

  /// Bits per pixel for a given format
  static float BitsPerPixel(TextureFormat fmt) {
    switch (fmt) {
    case TextureFormat::BC1_RGB:
      return 4.0f;
    case TextureFormat::BC3_RGBA:
      return 8.0f;
    case TextureFormat::BC4_R:
      return 4.0f;
    case TextureFormat::BC5_RG:
      return 8.0f;
    case TextureFormat::BC7_RGBA:
      return 8.0f;
    case TextureFormat::ASTC_4x4:
      return 8.0f;
    case TextureFormat::ASTC_6x6:
      return 3.56f;
    case TextureFormat::ETC2_RGB:
      return 4.0f;
    case TextureFormat::Uncompressed_BGRA:
      return 32.0f;
    default:
      return 32.0f;
    }
  }
};

} // namespace Engine
} // namespace ExplorerLens
