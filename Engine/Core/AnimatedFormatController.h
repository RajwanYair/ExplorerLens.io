#pragma once
// AnimatedFormatController.h — Animated Format Controller
// Intelligent frame selection for animated formats (GIF, APNG, WebP, AVIF
// sequences), choosing the most representative frame or generating a filmstrip
// composite.
#include <algorithm>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Frame selection mode for animated thumbnails
enum class FrameSelectionMode : uint8_t {
 FirstFrame = 0,
 // Most visually distinct frame in the animation
 KeyFrame,
 MiddleFrame,
 // Selects frame with highest color information density
 MostColorful,
 // Generates a 4-up composite grid
 Filmstrip,
 // Generates a short preview loop
 AnimatedPreview,
 COUNT
};

/// Animation format capability
enum class AnimCapability : uint8_t {
 None = 0,
 // Simple frame loop (GIF-style)
 LoopOnly,
 // Per-frame alpha blending (APNG-style)
 BlendModes,
 // Full video decode pipeline (WebP anim)
 FullVideo,
 // HDR frame sequences (AVIF seq)
 HDRAnimation,
 COUNT
};

struct AnimFormatInfo {
 uint32_t frameCount = 0;
 // 0 = infinite loop; any positive value limits playback count
 uint32_t loopCount = 0;
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
 // Divide by 2 for a 2x2 grid cell layout
 return thumbnailSize / 2;
 }
};

class FormatGalleryCompositor {
public:
    static int LayoutCount() { return 6; }
    static int GridCellSize(int containerWidth, int columns, float scaleFactor) {
        if (columns <= 0) return 0;
        return static_cast<int>(containerWidth / columns * scaleFactor);
    }
    FormatGalleryCompositor() = delete;
};

} // namespace Engine
} // namespace ExplorerLens
