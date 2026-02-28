//==============================================================================
// ExplorerLens Engine — Thumbnail Animation Engine V2
// Smooth animated thumbnail playback with frame interpolation, WebP/APNG
// loop control, GIF optimization, and DWM composition-aware rendering.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class AnimThumbnailFormat : uint8_t {
 AnimatedWebP = 0,
 APNG,
 GIF,
 AnimatedJXL,
 AnimatedHEIF,
 COUNT
};
enum class AnimLoopMode : uint8_t {
 Once = 0,
 Loop,
 Bounce,
 LoopCount,
 Infinite = Loop, // compat alias
 COUNT = LoopCount + 1
};
enum class AnimInterpolation : uint8_t { None = 0, Linear, Blend, COUNT };

struct AnimThumbnailConfig {
 AnimLoopMode loopMode = AnimLoopMode::Loop;
 AnimInterpolation interpolation = AnimInterpolation::Linear;
 uint32_t maxFrames = 60;
 uint32_t targetFPS = 24;
 uint32_t loopCount = 0; // 0 = infinite
 bool autoPlay = true;
 bool pauseOnHover = false;
};

struct AnimThumbnailInfo {
 AnimThumbnailFormat format = AnimThumbnailFormat::AnimatedWebP;
 uint32_t frameCount = 0;
 float durationSec = 0.0f;
 float fps = 0.0f;
 bool hasAlpha = false;
};

class ThumbnailAnimationEngineV2 {
public:
 static const wchar_t *FormatName(AnimThumbnailFormat f) {
 switch (f) {
 case AnimThumbnailFormat::AnimatedWebP:
 return L"Animated WebP";
 case AnimThumbnailFormat::APNG:
 return L"APNG";
 case AnimThumbnailFormat::GIF:
 return L"GIF";
 case AnimThumbnailFormat::AnimatedJXL:
 return L"Animated JXL";
 case AnimThumbnailFormat::AnimatedHEIF:
 return L"Animated HEIF";
 default:
 return L"Unknown";
 }
 }
 static const wchar_t *LoopModeName(AnimLoopMode m) {
 switch (m) {
 case AnimLoopMode::Once:
 return L"Once";
 case AnimLoopMode::Loop:
 return L"Loop";
 case AnimLoopMode::Bounce:
 return L"Bounce";
 case AnimLoopMode::LoopCount:
 return L"Loop Count";
 default:
 return L"Unknown";
 }
 }
 static const wchar_t *InterpolationName(AnimInterpolation i) {
 switch (i) {
 case AnimInterpolation::None:
 return L"None";
 case AnimInterpolation::Linear:
 return L"Linear";
 case AnimInterpolation::Blend:
 return L"Blend";
 default:
 return L"Unknown";
 }
 }
 static constexpr size_t FormatCount() {
 return static_cast<size_t>(AnimThumbnailFormat::COUNT);
 }
 static constexpr size_t LoopModeCount() {
 return static_cast<size_t>(AnimLoopMode::COUNT);
 }
 static constexpr size_t InterpolationCount() {
 return static_cast<size_t>(AnimInterpolation::COUNT);
 }
 static AnimThumbnailConfig DefaultConfig() { return AnimThumbnailConfig{}; }
};

} // namespace Engine
} // namespace ExplorerLens
