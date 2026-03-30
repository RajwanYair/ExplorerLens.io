// LivePreviewScrubber.h — Event-Driven Thumbnail Scrubber Controller
// Copyright (c) 2026 ExplorerLens Project
//
// Mouse-enter triggered progressive decode controller for live preview scrubbing
// across video, animation, audio, document, shader, font, and spreadsheet formats.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <atomic>

namespace ExplorerLens {
namespace Engine {

enum class ScrubMode : uint8_t {
    Video,
    Animation,
    Audio,
    Document,
    Shader,
    Font,
    Spreadsheet
};

struct ScrubState {
    uint32_t currentFrame = 0;
    uint32_t totalFrames = 0;
    float progress = 0.0f;
    bool isActive = false;
    bool isPaused = false;
    std::chrono::steady_clock::time_point startTime{};
    uint64_t elapsedMs = 0;
};

struct ScrubConfig {
    uint32_t maxFps = 30;
    uint32_t preloadFrames = 8;
    uint32_t cacheSize = 64;
    uint32_t decodeTimeoutMs = 500;
    bool loopPlayback = true;
    bool reverseOnExit = false;
    float mouseSensitivity = 1.0f;
};

using FrameCallback = std::function<void(uint32_t frameIndex, const uint8_t* pixels, uint32_t width, uint32_t height)>;
using ScrubEventCallback = std::function<void(ScrubMode mode, const ScrubState& state)>;

class LivePreviewScrubber {
public:
    explicit LivePreviewScrubber(ScrubConfig config = {})
        : m_config(config) {}

    ~LivePreviewScrubber() { EndScrub(); }

    bool BeginScrub(ScrubMode mode, const std::wstring& filePath) {
        if (m_state.isActive) EndScrub();
        m_mode = mode;
        m_filePath = filePath;
        m_state.isActive = true;
        m_state.currentFrame = 0;
        m_state.startTime = std::chrono::steady_clock::now();
        if (m_eventCallback) m_eventCallback(m_mode, m_state);
        return true;
    }

    bool AdvanceFrame() {
        if (!m_state.isActive || m_state.totalFrames == 0) return false;
        uint32_t next = m_state.currentFrame + 1;
        if (next >= m_state.totalFrames) {
            if (m_config.loopPlayback) next = 0;
            else return false;
        }
        m_state.currentFrame = next;
        m_state.progress = static_cast<float>(next) / static_cast<float>(m_state.totalFrames);
        m_state.elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - m_state.startTime).count();
        if (m_frameCallback) m_frameCallback(next, nullptr, m_frameWidth, m_frameHeight);
        return true;
    }

    bool SeekToPosition(float normalizedPos) {
        if (!m_state.isActive || m_state.totalFrames == 0) return false;
        float clamped = (normalizedPos < 0.0f) ? 0.0f : (normalizedPos > 1.0f) ? 1.0f : normalizedPos;
        m_state.currentFrame = static_cast<uint32_t>(clamped * (m_state.totalFrames - 1));
        m_state.progress = clamped;
        return true;
    }

    void EndScrub() {
        if (!m_state.isActive) return;
        m_state.isActive = false;
        m_state.progress = 0.0f;
        if (m_eventCallback) m_eventCallback(m_mode, m_state);
        m_preloadBuffer.clear();
    }

    const ScrubState& GetState() const { return m_state; }
    ScrubMode GetMode() const { return m_mode; }
    void SetFrameCallback(FrameCallback cb) { m_frameCallback = std::move(cb); }
    void SetEventCallback(ScrubEventCallback cb) { m_eventCallback = std::move(cb); }

    void SetTotalFrames(uint32_t count) { m_state.totalFrames = count; }
    void SetFrameDimensions(uint32_t w, uint32_t h) { m_frameWidth = w; m_frameHeight = h; }

    uint32_t GetPreloadedCount() const { return static_cast<uint32_t>(m_preloadBuffer.size()); }
    const ScrubConfig& GetConfig() const { return m_config; }

private:
    ScrubConfig m_config;
    ScrubState m_state;
    ScrubMode m_mode = ScrubMode::Video;
    std::wstring m_filePath;
    uint32_t m_frameWidth = 0;
    uint32_t m_frameHeight = 0;
    FrameCallback m_frameCallback;
    ScrubEventCallback m_eventCallback;
    std::vector<std::vector<uint8_t>> m_preloadBuffer;
};

} // namespace Engine
} // namespace ExplorerLens
