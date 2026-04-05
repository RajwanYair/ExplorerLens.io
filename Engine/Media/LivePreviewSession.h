// LivePreviewSession.h — Header-only live preview session aggregator
// Copyright (c) 2026 ExplorerLens Project
//
// Ties VideoFrameExtractor and VideoScrubberTimeline into a single session handle.
// Manages the open/seek/close lifecycle for a single video file preview.
//
#pragma once
#include "VideoFrameExtractor.h"
#include "VideoScrubberTimeline.h"
#include <string>

namespace ExplorerLens { namespace Engine {

class LivePreviewSession
{
public:
    bool Open(const std::wstring& filePath) noexcept
    {
        if (filePath.empty())
        {
            return false;
        }
        m_filePath = filePath;
        m_isOpen   = VideoScrubberTimeline::Instance().Build(filePath);
        return m_isOpen;
    }

    ScrubberFrameResult SeekTo(double timestampSeconds) noexcept
    {
        if (!m_isOpen)
        {
            return ScrubberFrameResult{};
        }
        const double PTS = VideoScrubberTimeline::Instance().NearestKeyframePts(timestampSeconds);
        VideoFrameRequest req{};
        req.filePath         = m_filePath;
        req.timestampSeconds = PTS;
        return VideoFrameExtractor::Instance().ExtractFrame(req);
    }

    void Close() noexcept
    {
        m_filePath.clear();
        m_isOpen = false;
        VideoScrubberTimeline::Instance().Clear();
    }

    bool         IsOpen()   const noexcept { return m_isOpen;   }
    std::wstring FilePath() const noexcept { return m_filePath; }

private:
    std::wstring m_filePath;
    bool         m_isOpen = false;
};

}} // namespace ExplorerLens::Engine
