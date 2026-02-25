#pragma once
// Sprint 406: Animated Format Controller
// Intelligent frame selection for animated formats (GIF, APNG, WebP, AVIF
// sequences), choosing the most representative frame or generating a filmstrip
// composite.
#include <algorithm>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Frame selection mode for animated thumbnails
enum class FrameSelectionMode : uint8_t {
  FirstFrame = 0,  // Always frame 0
  KeyFrame,        // Most visually distinct frame
  MiddleFrame,     // Middle of animation
  MostColorful,    // Highest color information
  Filmstrip,       // 4-up composite grid
  AnimatedPreview, // Generate short loop
  COUNT
};

/// Animation format capability
enum class AnimCapability : uint8_t {
  None = 0,
  LoopOnly,     // Simple loop (GIF)
  BlendModes,   // Alpha blending (APNG)
  FullVideo,    // Full video decode (WebP anim)
  HDRAnimation, // HDR frames (AVIF seq)
  COUNT
};

struct AnimFormatInfo {
  uint32_t frameCount = 0;
  uint32_t loopCount = 0; // 0 = infinite
  double totalDurationMs = 0.0;
  double avgFrameTimeMs = 0.0;
  uint32_t width = 0;
  uint32_t height = 0;
  bool hasAlpha = false;
};

class AnimatedFormatController {
public:
  static constexpr size_t ModeCount() {
    return static_cast<size_t>(FrameSelectionMode::COUNT);
  }
  static constexpr size_t CapabilityCount() {
    return static_cast<size_t>(AnimCapability::COUNT);
  }

  static const wchar_t *ModeName(FrameSelectionMode m) {
    switch (m) {
    case FrameSelectionMode::FirstFrame:
      return L"First Frame";
    case FrameSelectionMode::KeyFrame:
      return L"Key Frame";
    case FrameSelectionMode::MiddleFrame:
      return L"Middle Frame";
    case FrameSelectionMode::MostColorful:
      return L"Most Colorful";
    case FrameSelectionMode::Filmstrip:
      return L"Filmstrip";
    case FrameSelectionMode::AnimatedPreview:
      return L"Animated Preview";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *CapabilityName(AnimCapability c) {
    switch (c) {
    case AnimCapability::None:
      return L"None";
    case AnimCapability::LoopOnly:
      return L"Loop Only";
    case AnimCapability::BlendModes:
      return L"Blend Modes";
    case AnimCapability::FullVideo:
      return L"Full Video";
    case AnimCapability::HDRAnimation:
      return L"HDR Animation";
    default:
      return L"Unknown";
    }
  }

  static uint32_t SelectFrame(FrameSelectionMode mode, uint32_t totalFrames) {
    if (totalFrames == 0)
      return 0;
    switch (mode) {
    case FrameSelectionMode::FirstFrame:
      return 0;
    case FrameSelectionMode::MiddleFrame:
      return totalFrames / 2;
    case FrameSelectionMode::KeyFrame:
      return (std::min)(totalFrames / 3, totalFrames - 1);
    default:
      return 0;
    }
  }

  /// Calculate filmstrip grid cell size for 4-up composite
  static uint32_t FilmstripCellSize(uint32_t thumbnailSize) {
    return thumbnailSize / 2; // 2x2 grid
  }
};

} // namespace Engine
} // namespace ExplorerLens
