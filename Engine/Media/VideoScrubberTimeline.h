// VideoScrubberTimeline.h — Video timeline index for scrubber navigation
// Copyright (c) 2026 ExplorerLens Project
//
// Builds and queries the keyframe index and chapter markers for a video file.
// Used by LivePreviewSession to map a scrubber position to a decodable PTS.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

struct KeyframeEntry
{
    uint32_t index  = 0;
    double   ptsSec = 0.0;
    bool     isIDR  = false;
};

struct ChapterMarker
{
    std::wstring title;
    double       startSec = 0.0;
};

struct ScrubberTimelineStats
{
    uint32_t keyframeCount = 0;
    uint32_t chapterCount  = 0;
    double   durationSec   = 0.0;
    float    buildMs       = 0.0f;
};

class VideoScrubberTimeline
{
public:
    static VideoScrubberTimeline& Instance() noexcept;

    bool Build(const std::wstring& filePath) noexcept;
    void Clear()                             noexcept;

    double   Duration()      const noexcept { return m_durationSec; }
    uint32_t KeyframeCount() const noexcept { return static_cast<uint32_t>(m_keyframes.size()); }
    uint32_t ChapterCount()  const noexcept { return static_cast<uint32_t>(m_chapters.size());  }

    KeyframeEntry KeyframeAt(uint32_t index)       const noexcept;
    ChapterMarker ChapterAt(uint32_t index)         const noexcept;
    double        NearestKeyframePts(double target) const noexcept;

    ScrubberTimelineStats GetStats() const noexcept;

private:
    std::vector<KeyframeEntry> m_keyframes;
    std::vector<ChapterMarker> m_chapters;
    double                     m_durationSec = 0.0;
    float                      m_buildMs     = 0.0f;
};

}} // namespace ExplorerLens::Engine
