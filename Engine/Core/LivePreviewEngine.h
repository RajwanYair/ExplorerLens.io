// LivePreviewEngine.h — Real-Time Live Preview for Animated/Video Content
// Copyright (c) 2026 ExplorerLens Project
//
// Generates animated preview sequences for GIF, WebP, APNG, video files
// that play on hover in Windows Explorer. Manages frame extraction,
// timing, memory budget, and smooth playback transitions.

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Preview animation mode
enum class LivePreviewMode : uint8_t {
 Static = 0, ///< Single frame thumbnail (default)
 AnimatedLoop = 1, ///< Looping preview (GIF/WebP animated)
 VideoScrub = 2, ///< Scrubbing through video keyframes
 Slideshow = 3, ///< Slideshow of representative frames
 Hover3D = 4, ///< 3D model rotation preview
 COUNT
};

/// Frame timing for animation playback
struct PreviewFrame {
 uint32_t width = 0;
 uint32_t height = 0;
 std::vector<uint8_t> pixelData; ///< BGRA8
 double timestampMs = 0.0;
 double durationMs = 33.33; ///< Frame duration (default 30fps)
 uint32_t frameIndex = 0;
 bool isKeyframe = false;
};

/// Preview generation configuration
struct LivePreviewConfig {
 LivePreviewMode mode = LivePreviewMode::Static;
 uint32_t maxFrames = 12; ///< Max frames to extract
 uint32_t targetWidth = 256;
 uint32_t targetHeight = 256;
 double maxDurationSec = 3.0; ///< Max preview duration
 double fpsTarget = 15.0; ///< Target playback FPS
 uint64_t maxMemoryBytes = 16 * 1024 * 1024; ///< 16MB memory budget
 bool useGPUDecode = true;
 bool generateOnHover = true; ///< Only generate when user hovers
};

/// Preview generation result
struct PreviewResult {
 bool success = false;
 LivePreviewMode mode = LivePreviewMode::Static;
 std::vector<PreviewFrame> frames;
 double totalDurationMs = 0.0;
 uint64_t totalMemoryBytes = 0;
 double generationTimeMs = 0.0;
 std::wstring errorMessage;
 bool gpuAccelerated = false;
 std::wstring sourceFormat;
};

/// Live Preview Engine
class LivePreviewEngine {
public:
 static const wchar_t *ModeName(LivePreviewMode m) {
 switch (m) {
 case LivePreviewMode::Static:
 return L"Static";
 case LivePreviewMode::AnimatedLoop:
 return L"Animated Loop";
 case LivePreviewMode::VideoScrub:
 return L"Video Scrub";
 case LivePreviewMode::Slideshow:
 return L"Slideshow";
 case LivePreviewMode::Hover3D:
 return L"3D Hover";
 default:
 return L"Unknown";
 }
 }

 static constexpr size_t ModeCount() {
 return static_cast<size_t>(LivePreviewMode::COUNT);
 }

 /// Determine optimal preview mode for a given file format
 static LivePreviewMode RecommendMode(const std::wstring &extension) {
 if (extension == L".gif" || extension == L".webp" || extension == L".apng")
 return LivePreviewMode::AnimatedLoop;
 if (extension == L".mp4" || extension == L".mkv" || extension == L".avi" ||
 extension == L".mov" || extension == L".webm" || extension == L".flv")
 return LivePreviewMode::VideoScrub;
 if (extension == L".cbz" || extension == L".cbr" || extension == L".pdf")
 return LivePreviewMode::Slideshow;
 if (extension == L".obj" || extension == L".fbx" || extension == L".gltf" ||
 extension == L".glb" || extension == L".stl" || extension == L".usd")
 return LivePreviewMode::Hover3D;
 return LivePreviewMode::Static;
 }

 /// Calculate memory budget for N frames at given dimensions
 static uint64_t EstimateMemory(uint32_t width, uint32_t height,
 uint32_t frameCount) {
 return static_cast<uint64_t>(width) * height * 4 * frameCount;
 }

 /// Adjust frame count to fit within memory budget
 static uint32_t FitToBudget(uint32_t width, uint32_t height,
 uint64_t budgetBytes, uint32_t maxFrames) {
 uint64_t frameSize = static_cast<uint64_t>(width) * height * 4;
 if (frameSize == 0)
 return 0;
 uint32_t maxByBudget = static_cast<uint32_t>(budgetBytes / frameSize);
 return (maxByBudget < maxFrames) ? maxByBudget : maxFrames;
 }
};

} // namespace Engine
} // namespace ExplorerLens
