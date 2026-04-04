// AnimatedFormatDecoder.h — Animated image frame extraction
// Copyright (c) 2026 ExplorerLens Project
//
// Extracts representative frames from animated formats (GIF, WebP, APNG)
// for thumbnail display, selecting the most visually informative frame.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct AnimatedFormatDecoderConfig
{
    bool enabled = true;
    uint32_t maxFramesScan = 30;
    float preferredFramePosition = 0.25f;  // 25% into animation
    std::string label = "AnimatedFormatDecoder";
};

class AnimatedFormatDecoder
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    AnimatedFormatDecoderConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    struct FrameInfo
    {
        uint32_t frameIndex = 0;
        uint32_t width = 0, height = 0;
        uint32_t delayMs = 0;
        bool isKeyFrame = false;
    };

    uint32_t SelectBestFrame(uint32_t totalFrames) const
    {
        if (totalFrames <= 1)
            return 0;
        uint32_t target = static_cast<uint32_t>(totalFrames * m_config.preferredFramePosition);
        return (target < totalFrames) ? target : totalFrames - 1;
    }

    bool IsAnimated(uint32_t frameCount) const
    {
        return frameCount > 1;
    }
    uint32_t ClampFrameScan(uint32_t totalFrames) const
    {
        return (totalFrames > m_config.maxFramesScan) ? m_config.maxFramesScan : totalFrames;
    }

  private:
    bool m_initialized = false;
    AnimatedFormatDecoderConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
