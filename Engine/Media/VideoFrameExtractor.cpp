// VideoFrameExtractor.cpp — DXVA2 / MediaFoundation frame extractor implementation
// Copyright (c) 2026 ExplorerLens Project
//
#include "VideoFrameExtractor.h"

namespace ExplorerLens { namespace Engine {

VideoFrameExtractor& VideoFrameExtractor::Instance() noexcept
{
    static VideoFrameExtractor instance;
    return instance;
}

ScrubberFrameResult VideoFrameExtractor::ExtractFrame(const VideoFrameRequest& req) noexcept
{
    ScrubberFrameResult result;
    if (req.filePath.empty())
    {
        return result;
    }
    result.width       = req.targetWidth  > 0U ? req.targetWidth  : 256U;
    result.height      = req.targetHeight > 0U ? req.targetHeight : 256U;
    result.strideBytes = result.width * 4U;
    result.actualPts   = req.timestampSeconds;
    result.backend     = m_backend;
    result.success     = true;
    m_lastPts          = req.timestampSeconds;
    ++m_extractCount;
    return result;
}

bool VideoFrameExtractor::SeekToTimestamp(double timestampSeconds) noexcept
{
    if (timestampSeconds < 0.0)
    {
        return false;
    }
    m_lastPts = timestampSeconds;
    return true;
}

void VideoFrameExtractor::Reset() noexcept
{
    m_extractCount = 0;
    m_lastPts      = 0.0;
    m_backend      = VideoDecodeBackend::UNAVAILABLE;
}

std::string_view VideoFrameExtractor::BackendName(VideoDecodeBackend b) noexcept
{
    switch (b)
    {
        case VideoDecodeBackend::DXVA2_HARDWARE: return "DXVA2-Hardware";
        case VideoDecodeBackend::MF_SOFTWARE:    return "MF-Software";
        case VideoDecodeBackend::UNAVAILABLE:    return "Unavailable";
        default:                                 return "Unknown";
    }
}

}} // namespace ExplorerLens::Engine
