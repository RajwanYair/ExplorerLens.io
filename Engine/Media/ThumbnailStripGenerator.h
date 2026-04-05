// ThumbnailStripGenerator.h — N-frame sprite strip generator for seekbar preview
// Copyright (c) 2026 ExplorerLens Project
//
// Extracts N evenly-spaced frames from a video file and composites them into a
// horizontal sprite strip. Used by the shell preview pane to render a filmstrip
// beneath the video scrubber.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens { namespace Engine {

struct StripConfig
{
    std::wstring filePath;
    uint32_t     frameCount  = 10;
    uint32_t     frameWidth  = 160;
    uint32_t     frameHeight = 90;
};

struct StripResult
{
    bool     success     = false;
    uint32_t framesAdded = 0;
    uint32_t stripWidth  = 0;
    uint32_t stripHeight = 0;
    float    generateMs  = 0.0f;
};

class ThumbnailStripGenerator
{
public:
    static ThumbnailStripGenerator& Instance() noexcept;

    StripResult GenerateStrip(const StripConfig& config) noexcept;
    void        Reset()                                  noexcept;

    uint32_t StripWidth()  const noexcept { return m_lastWidth;  }
    uint32_t StripHeight() const noexcept { return m_lastHeight; }
    uint32_t FrameCount()  const noexcept { return m_lastFrames; }

private:
    uint32_t m_lastWidth  = 0;
    uint32_t m_lastHeight = 0;
    uint32_t m_lastFrames = 0;
    float    m_lastMs     = 0.0f;
};

}} // namespace ExplorerLens::Engine
