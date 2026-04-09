// HoverScrubController.h — Mouse-Position-Based Thumbnail Frame Scrubber
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks mouse position over an Explorer thumbnail and maps it to a frame
// index for animated/sequence formats. As the mouse moves left→right across
// the thumbnail, the visible frame scrubs linearly through all keyframes.
// Hooks into the IThumbnailProvider pipeline via frame-index callbacks.
//
#pragma once
#include <cstdint>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct ScrubState {
    uint32_t frameIndex    = 0;     // Current frame to display [0, frameCount)
    float    normalizedPos = 0.0f;  // Mouse X normalised [0.0, 1.0]
    uint32_t frameCount    = 0;
    bool     active        = false; // True while mouse is over thumbnail
};

class HoverScrubController {
public:
    // Callback invoked when the displayed frame should change.
    using FrameChangedCallback = std::function<void(uint32_t frameIndex)>;

    HoverScrubController() = default;
    ~HoverScrubController() = default;

    // Set the total number of frames available for scrubbing.
    void SetFrameCount(uint32_t count) noexcept;

    // Report mouse enter with initial position (x in [0, thumbnailWidth]).
    void OnMouseEnter(int32_t x, uint32_t thumbnailWidth) noexcept;

    // Report mouse move.
    void OnMouseMove(int32_t x, uint32_t thumbnailWidth) noexcept;

    // Report mouse leave — returns to frame 0.
    void OnMouseLeave() noexcept;

    // Register frame-changed callback.
    void SetFrameChangedCallback(FrameChangedCallback cb) noexcept;

    ScrubState GetState() const noexcept { return m_state; }

    // Map normalised position [0,1] and frame count to a frame index.
    static uint32_t PosToFrame(float normalizedPos, uint32_t frameCount) noexcept;

private:
    ScrubState            m_state;
    FrameChangedCallback  m_callback;
};

}} // namespace ExplorerLens::Engine
