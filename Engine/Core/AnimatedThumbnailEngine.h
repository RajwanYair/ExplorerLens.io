#pragma once
//==============================================================================
// AnimatedThumbnailEngine
// Extracts and renders representative frames from animated image formats
// (GIF, APNG, WebP animation, AVIF sequences).
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class AnimatedFormat : uint8_t {
 GIF = 0,
 APNG = 1,
 WebPAnim = 2,
 AVIFSeq = 3,
 JXLAnim = 4,
 FLIF = 5,
 FormatCount = 6
};

enum class FrameStrategy : uint8_t {
 First = 0, // Use first frame
 Middle = 1, // Use middle frame
 Keyframe = 2, // Use first keyframe
 MostDetail = 3, // Frame with most visual detail
 Composite = 4, // Composite multiple frames
 StrategyCount = 5
};

struct AnimationInfo {
 AnimatedFormat format = AnimatedFormat::GIF;
 uint32_t frameCount = 0;
 uint32_t width = 0;
 uint32_t height = 0;
 double durationMs = 0.0;
 double averageFpsMs = 0.0;
 bool hasAlpha = false;
 bool isLooping = true;
};

struct FrameExtractionResult {
 bool success = false;
 uint32_t selectedFrame = 0;
 uint32_t width = 0;
 uint32_t height = 0;
 double extractionTimeMs = 0.0;
 std::vector<uint8_t> pixelData;
};

class AnimatedThumbnailEngine {
public:
 AnimatedThumbnailEngine();

 AnimationInfo Probe(const std::wstring& filePath) const;
 FrameExtractionResult ExtractFrame(const std::wstring& filePath,
 FrameStrategy strategy = FrameStrategy::MostDetail) const;

 void SetMaxFrameScan(uint32_t maxFrames) { m_maxFrameScan = maxFrames; }
 uint32_t GetMaxFrameScan() const { return m_maxFrameScan; }

 static AnimatedFormat DetectFormat(const std::wstring& filePath);
 static const wchar_t* GetFormatName(AnimatedFormat format);
 static const wchar_t* GetStrategyName(FrameStrategy strategy);
 static uint32_t GetFormatCount() { return static_cast<uint32_t>(AnimatedFormat::FormatCount); }
 static uint32_t SelectBestFrame(uint32_t totalFrames, FrameStrategy strategy);

private:
 uint32_t m_maxFrameScan = 100;
};

}} // namespace ExplorerLens::Engine

