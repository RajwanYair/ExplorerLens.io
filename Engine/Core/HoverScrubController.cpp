// HoverScrubController.cpp — Mouse-Position-Based Thumbnail Frame Scrubber
// Copyright (c) 2026 ExplorerLens Project
//
#include "Core/HoverScrubController.h"
#include <algorithm>
#include <cmath>

namespace ExplorerLens { namespace Engine {

uint32_t HoverScrubController::PosToFrame(float pos, uint32_t frameCount) noexcept
{
    if (frameCount <= 1) return 0;
    pos = std::clamp(pos, 0.0f, 1.0f);
    const auto idx = static_cast<uint32_t>(std::floor(pos * (frameCount - 1) + 0.5f));
    return std::min(idx, frameCount - 1u);
}

void HoverScrubController::SetFrameCount(uint32_t count) noexcept
{
    m_state.frameCount = count;
    if (m_state.frameIndex >= count && count > 0)
        m_state.frameIndex = count - 1u;
}

void HoverScrubController::OnMouseEnter(int32_t x, uint32_t thumbnailWidth) noexcept
{
    m_state.active = true;
    if (thumbnailWidth == 0) return;
    m_state.normalizedPos = std::clamp(
        static_cast<float>(x) / static_cast<float>(thumbnailWidth), 0.0f, 1.0f);
    const uint32_t frame = PosToFrame(m_state.normalizedPos, m_state.frameCount);
    if (frame != m_state.frameIndex) {
        m_state.frameIndex = frame;
        if (m_callback) m_callback(frame);
    }
}

void HoverScrubController::OnMouseMove(int32_t x, uint32_t thumbnailWidth) noexcept
{
    if (!m_state.active || thumbnailWidth == 0) return;
    m_state.normalizedPos = std::clamp(
        static_cast<float>(x) / static_cast<float>(thumbnailWidth), 0.0f, 1.0f);
    const uint32_t frame = PosToFrame(m_state.normalizedPos, m_state.frameCount);
    if (frame != m_state.frameIndex) {
        m_state.frameIndex = frame;
        if (m_callback) m_callback(frame);
    }
}

void HoverScrubController::OnMouseLeave() noexcept
{
    m_state.active        = false;
    m_state.normalizedPos = 0.0f;
    if (m_state.frameIndex != 0) {
        m_state.frameIndex = 0;
        if (m_callback) m_callback(0);
    }
}

void HoverScrubController::SetFrameChangedCallback(FrameChangedCallback cb) noexcept
{
    m_callback = std::move(cb);
}

}} // namespace ExplorerLens::Engine
