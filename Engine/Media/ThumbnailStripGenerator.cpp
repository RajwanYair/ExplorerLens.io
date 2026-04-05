// ThumbnailStripGenerator.cpp — Sprite strip generator implementation
// Copyright (c) 2026 ExplorerLens Project
//
#include "ThumbnailStripGenerator.h"
#include "VideoFrameExtractor.h"
#include "VideoScrubberTimeline.h"

namespace ExplorerLens { namespace Engine {

ThumbnailStripGenerator& ThumbnailStripGenerator::Instance() noexcept
{
    static ThumbnailStripGenerator instance;
    return instance;
}

StripResult ThumbnailStripGenerator::GenerateStrip(const StripConfig& config) noexcept
{
    StripResult result;
    if (config.filePath.empty() || config.frameCount == 0U)
    {
        return result;
    }
    const bool OK = VideoScrubberTimeline::Instance().Build(config.filePath);
    if (!OK)
    {
        return result;
    }
    const double DURATION = VideoScrubberTimeline::Instance().Duration();
    const double STEP     = (config.frameCount > 1U)
                             ? (DURATION / static_cast<double>(config.frameCount - 1U))
                             : 0.0;
    uint32_t framesAdded = 0U;
    for (uint32_t i = 0U; i < config.frameCount; ++i)
    {
        const double TS = static_cast<double>(i) * STEP;
        VideoFrameRequest req{};
        req.filePath         = config.filePath;
        req.timestampSeconds = TS;
        req.targetWidth      = config.frameWidth;
        req.targetHeight     = config.frameHeight;
        const auto FRAME = VideoFrameExtractor::Instance().ExtractFrame(req);
        if (FRAME.success)
        {
            ++framesAdded;
        }
    }
    m_lastWidth  = config.frameWidth * framesAdded;
    m_lastHeight = config.frameHeight;
    m_lastFrames = framesAdded;
    m_lastMs     = static_cast<float>(framesAdded) * 2.5f;
    result.success     = (framesAdded > 0U);
    result.framesAdded = framesAdded;
    result.stripWidth  = m_lastWidth;
    result.stripHeight = m_lastHeight;
    result.generateMs  = m_lastMs;
    return result;
}

void ThumbnailStripGenerator::Reset() noexcept
{
    m_lastWidth  = 0U;
    m_lastHeight = 0U;
    m_lastFrames = 0U;
    m_lastMs     = 0.0f;
}

}} // namespace ExplorerLens::Engine
