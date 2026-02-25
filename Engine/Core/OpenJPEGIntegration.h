// OpenJPEGIntegration.h — OpenJPEG 2.5.x JPEG 2000 Decoder Integration
// Copyright (c) 2026 ExplorerLens Project
//
// Provides JPEG 2000 Part 1/2 decoding via OpenJPEG library, replacing the
// limited WIC fallback. Supports JP2, J2K, J2C, JPX formats with proper
// color management and multi-resolution decoding.

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// JPEG 2000 profile/part
enum class JP2Profile : uint8_t {
  Part1_Core = 0,     ///< ISO 15444-1 (core codec)
  Part2_Extended = 1, ///< ISO 15444-2 (extensions)
  Cinema2K = 2,       ///< Digital Cinema 2K profile
  Cinema4K = 3,       ///< Digital Cinema 4K profile
  BroadcastSDI = 4,   ///< Broadcast profile
  Unknown = 255,
  Part1 = Part1_Core,
  Part2 = Part2_Extended,
  COUNT = 6
};

/// Alias for backward compatibility
using JPEG2000Profile = JP2Profile;

/// JPEG 2000 color space
enum class JP2ColorSpace : uint8_t {
  Unknown = 0,
  Grayscale = 1,
  sRGB = 2,
  sYCC = 3,
  CMYK = 4,
  EYCC = 5,
  UNSPECIFIED = 6
};

/// Decoded JPEG 2000 image data
struct JP2DecodeResult {
  bool success = false;
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t numComponents = 0;
  uint32_t bitsPerComponent = 0;
  JP2Profile profile = JP2Profile::Unknown;
  JP2ColorSpace colorSpace = JP2ColorSpace::Unknown;
  std::vector<uint8_t> pixelData;
  std::string errorMessage;
  double decodeTimeMs = 0.0;
  uint32_t resolutionLevels = 0;
};

/// OpenJPEG integration layer
class OpenJPEGIntegration {
public:
  static OpenJPEGIntegration &Instance() {
    static OpenJPEGIntegration instance;
    return instance;
  }

  /// Check if OpenJPEG is available
  bool IsAvailable() const {
#ifdef HAS_OPENJPEG
    return true;
#else
    return false;
#endif
  }

  /// Get library version
  const char *GetVersion() const {
#ifdef HAS_OPENJPEG
    return "2.5.3";
#else
    return "not available";
#endif
  }

  /// Decode a JPEG 2000 file
  JP2DecodeResult Decode(const wchar_t *filePath, uint32_t targetWidth = 0,
                         uint32_t targetHeight = 0) {
    JP2DecodeResult result;
#ifdef HAS_OPENJPEG
    result = DecodeInternal(filePath, targetWidth, targetHeight);
#else
    (void)filePath;
    (void)targetWidth;
    (void)targetHeight;
    result.errorMessage = "OpenJPEG not built (HAS_OPENJPEG=OFF)";
#endif
    return result;
  }

  /// Generate HBITMAP thumbnail from JP2 file
  HBITMAP GenerateThumbnail(const wchar_t *filePath, uint32_t cx, uint32_t cy) {
    auto result = Decode(filePath, cx, cy);
    if (!result.success || result.pixelData.empty())
      return nullptr;
    return CreateBitmapFromPixels(result.pixelData.data(), result.width,
                                  result.height);
  }

  /// Detect JPEG 2000 sub-format from file header
  static bool IsJPEG2000(const uint8_t *header, size_t len) {
    if (len < 12)
      return false;
    if (header[0] == 0x00 && header[1] == 0x00 && header[2] == 0x00 &&
        header[3] == 0x0C && header[4] == 0x6A && header[5] == 0x50) {
      return true;
    }
    if (header[0] == 0xFF && header[1] == 0x4F && header[2] == 0xFF &&
        header[3] == 0x51) {
      return true;
    }
    return false;
  }

  /// Profile name lookup (narrow string)
  static const char *ProfileNameA(JP2Profile p) {
    switch (p) {
    case JP2Profile::Part1_Core:
      return "Part1_Core";
    case JP2Profile::Part2_Extended:
      return "Part2_Extended";
    case JP2Profile::Cinema2K:
      return "Cinema2K";
    case JP2Profile::Cinema4K:
      return "Cinema4K";
    case JP2Profile::BroadcastSDI:
      return "BroadcastSDI";
    case JP2Profile::Unknown:
      return "Unknown";
    default:
      return "?";
    }
  }

  /// Color space name lookup
  static const char *ColorSpaceName(JP2ColorSpace cs) {
    switch (cs) {
    case JP2ColorSpace::Unknown:
      return "Unknown";
    case JP2ColorSpace::Grayscale:
      return "Grayscale";
    case JP2ColorSpace::sRGB:
      return "sRGB";
    case JP2ColorSpace::sYCC:
      return "sYCC";
    case JP2ColorSpace::CMYK:
      return "CMYK";
    case JP2ColorSpace::EYCC:
      return "EYCC";
    case JP2ColorSpace::UNSPECIFIED:
      return "Unspecified";
    default:
      return "?";
    }
  }

  static constexpr uint32_t GetProfileCount() { return 6; }
  static constexpr size_t ProfileCount() { return 6; }
  static constexpr uint32_t GetColorSpaceCount() { return 7; }

  /// Wide-string profile name for GUI display
  static const wchar_t *ProfileName(JP2Profile p) {
    switch (p) {
    case JP2Profile::Part1_Core:
      return L"Part 1 (JP2)";
    case JP2Profile::Part2_Extended:
      return L"Part 2 (JPX)";
    case JP2Profile::Cinema2K:
      return L"Cinema 2K";
    case JP2Profile::Cinema4K:
      return L"Cinema 4K";
    case JP2Profile::BroadcastSDI:
      return L"Broadcast SDI";
    case JP2Profile::Unknown:
      return L"Unknown";
    default:
      return L"Unknown";
    }
  }

private:
  OpenJPEGIntegration() = default;

#ifdef HAS_OPENJPEG
  JP2DecodeResult DecodeInternal(const wchar_t *filePath, uint32_t tw,
                                 uint32_t th);
#endif

  HBITMAP CreateBitmapFromPixels(const uint8_t *pixels, uint32_t w,
                                 uint32_t h) {
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = static_cast<LONG>(w);
    bmi.bmiHeader.biHeight = -static_cast<LONG>(h);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    void *bits = nullptr;
    HBITMAP hbmp =
        CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (hbmp && bits)
      memcpy(bits, pixels, static_cast<size_t>(w) * h * 4);
    return hbmp;
  }
};

} // namespace Engine
} // namespace ExplorerLens
