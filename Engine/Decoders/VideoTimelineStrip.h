// VideoTimelineStrip.h — Video Timeline Filmstrip Generator
// Copyright (c) 2026 ExplorerLens Project
//
// Generates a filmstrip-style thumbnail by extracting evenly-spaced
// keyframes from video files. Creates a horizontal strip of frame
// thumbnails showing progression through the video.

#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct TimelineFrame
{
    double timestampSeconds = 0.0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t thumbnailX = 0;  // X position in strip
    bool isKeyframe = false;
    std::vector<uint8_t> pixelData;  // BGRA
};

struct VideoTimelineConfig
{
    uint32_t frameCount = 5;    // Number of frames to extract
    uint32_t frameWidth = 64;   // Width per frame in strip
    uint32_t frameHeight = 48;  // Height per frame
    uint32_t spacing = 2;       // Pixel spacing between frames
    bool skipBlackFrames = true;
};

struct VideoTimelineInfo
{
    double durationSeconds = 0.0;
    uint32_t totalFrameCount = 0;
    uint32_t keyframeCount = 0;
    uint32_t stripWidth = 0;
    uint32_t stripHeight = 0;
    std::vector<double> extractedTimestamps;
};

struct VideoTimelineStats
{
    uint32_t stripsGenerated = 0;
    uint64_t totalFramesExtracted = 0;
    uint32_t blackFramesSkipped = 0;
};

class VideoTimelineStrip
{
  public:
    VideoTimelineStrip() = default;
    ~VideoTimelineStrip() = default;

    static const wchar_t* GetName()
    {
        return L"VideoTimelineStrip";
    }

    bool CanProcess(const wchar_t* ext) const
    {
        if (!ext)
            return false;
        std::wstring e(ext);
        for (auto& c : e)
            c = towlower(c);
        return e == L".mp4" || e == L".avi" || e == L".mkv" || e == L".mov" || e == L".wmv" || e == L".webm"
               || e == L".flv" || e == L".m4v";
    }

    /// Calculate evenly-spaced timestamps for frame extraction.
    std::vector<double> CalculateTimestamps(double duration, uint32_t frameCount) const
    {
        std::vector<double> timestamps;
        if (duration <= 0.0 || frameCount == 0)
            return timestamps;
        timestamps.reserve(frameCount);
        // Skip first and last 5% to avoid black intro/outro
        double start = duration * 0.05;
        double end = duration * 0.95;
        double range = end - start;
        if (range <= 0.0) {
            timestamps.push_back(duration / 2);
            return timestamps;
        }
        double step = (frameCount > 1) ? range / (frameCount - 1) : 0.0;
        for (uint32_t i = 0; i < frameCount; ++i)
            timestamps.push_back(start + i * step);
        return timestamps;
    }

    /// Calculate strip dimensions for the output thumbnail.
    VideoTimelineInfo CalculateLayout(double duration, const VideoTimelineConfig& config) const
    {
        VideoTimelineInfo info;
        info.durationSeconds = duration;
        info.extractedTimestamps = CalculateTimestamps(duration, config.frameCount);
        info.stripWidth = config.frameCount * config.frameWidth
                          + (config.frameCount > 0 ? config.frameCount - 1 : 0) * config.spacing;
        info.stripHeight = config.frameHeight;
        return info;
    }

    /// Check if a frame is "black" (average luminance below threshold).
    bool IsBlackFrame(const uint8_t* bgra, uint32_t width, uint32_t height, float threshold = 10.0f) const
    {
        if (!bgra || width == 0 || height == 0)
            return true;
        double sum = 0.0;
        uint32_t pixelCount = width * height;
        for (uint32_t i = 0; i < pixelCount; ++i) {
            sum += bgra[i * 4] * 0.114 + bgra[i * 4 + 1] * 0.587 + bgra[i * 4 + 2] * 0.299;
        }
        return (sum / pixelCount) < threshold;
    }

    VideoTimelineStats GetStats() const
    {
        return m_stats;
    }

  private:
    mutable VideoTimelineStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
