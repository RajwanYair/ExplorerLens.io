//==============================================================================
// ExplorerLens Engine — APNG & Animated Format Enhancement
// Validates APNG via WIC, improves animated WebP/JXL first-frame extraction.
// Core types (AnimatedFormat, FrameStrategy, AnimationInfo) are defined
// in Core/AnimatedThumbnailEngine.h — this handler adds detection and
// selection.
//==============================================================================
#pragma once
#include "../Core/AnimatedThumbnailEngine.h"

namespace ExplorerLens {
namespace Engine {

/// Animated format handler — extends the core AnimatedThumbnailEngine types
class AnimatedFormatHandler {
public:
  /// Detect animated format from extension
  static AnimatedFormat DetectFormat(const std::wstring &ext) {
    if (ext == L".apng")
      return AnimatedFormat::APNG;
    if (ext == L".webp")
      return AnimatedFormat::WebPAnim;
    if (ext == L".jxl")
      return AnimatedFormat::JXLAnim;
    if (ext == L".gif")
      return AnimatedFormat::GIF;
    if (ext == L".avif")
      return AnimatedFormat::AVIFSeq;
    return AnimatedFormat::GIF; // default fallback
  }

  /// Format name
  static const wchar_t *FormatName(AnimatedFormat f) {
    switch (f) {
    case AnimatedFormat::APNG:
      return L"Animated PNG";
    case AnimatedFormat::WebPAnim:
      return L"Animated WebP";
    case AnimatedFormat::JXLAnim:
      return L"Animated JPEG XL";
    case AnimatedFormat::GIF:
      return L"Animated GIF";
    case AnimatedFormat::AVIFSeq:
      return L"Animated AVIF";
    case AnimatedFormat::FLIF:
      return L"Animated FLIF";
    default:
      return L"Unknown";
    }
  }

  /// Strategy name
  static const wchar_t *StrategyName(FrameStrategy s) {
    switch (s) {
    case FrameStrategy::First:
      return L"FirstFrame";
    case FrameStrategy::Keyframe:
      return L"KeyFrame";
    case FrameStrategy::Middle:
      return L"MiddleFrame";
    case FrameStrategy::MostDetail:
      return L"MostDetail";
    case FrameStrategy::Composite:
      return L"Composite";
    default:
      return L"Unknown";
    }
  }

  /// Select best frame index for thumbnail
  static uint32_t SelectFrame(const AnimationInfo &info,
                              FrameStrategy strategy) {
    if (info.frameCount == 0)
      return 0;
    switch (strategy) {
    case FrameStrategy::First:
      return 0;
    case FrameStrategy::Middle:
      return info.frameCount / 2;
    case FrameStrategy::Keyframe:
      return 0; // simplified
    case FrameStrategy::MostDetail:
      return 0; // needs decode
    default:
      return 0;
    }
  }

  /// Check if APNG from magic bytes (PNG + acTL chunk)
  static bool IsAPNG(const uint8_t *data, size_t size) {
    if (size < 8)
      return false;
    // PNG magic
    if (data[0] != 0x89 || data[1] != 'P' || data[2] != 'N' || data[3] != 'G')
      return false;
    // Search for acTL chunk (animated control)
    for (size_t i = 8; i + 8 < size && i < 4096; i++) {
      if (data[i] == 'a' && data[i + 1] == 'c' && data[i + 2] == 'T' &&
          data[i + 3] == 'L')
        return true;
    }
    return false;
  }

  /// Count of supported animated formats
  static constexpr size_t FormatCount() {
    return static_cast<size_t>(AnimatedFormat::FormatCount);
  }
};

} // namespace Engine
} // namespace ExplorerLens
