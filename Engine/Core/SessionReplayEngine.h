// SessionReplayEngine.h — Collaboration Session Replay Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Records and replays annotation editing sessions for training, audit, and
// undo-history purposes, with scrubbing and variable-speed playback support.
//
#pragma once
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class ReplayState    { Idle, Recording, Playing, Paused };
enum class ReplaySpeed    { Slow_0_25x, Normal_1x, Fast_2x, Super_4x };

struct ReplayFrame {
    int64_t      timestampMs = 0;
    std::string  authorId;
    std::string  operation;   // serialised annotation op
    std::string  filePath;
};

struct ReplayRecording {
    std::string              sessionId;
    std::vector<ReplayFrame> frames;
    int64_t                  durationMs = 0;
    int FrameCount() const noexcept { return static_cast<int>(frames.size()); }
};

struct ReplayPlaybackResult {
    bool   success        = false;
    int    framesPlayed   = 0;
    int    totalFrames    = 0;
    double playbackMs     = 0.0;
    std::string errorMsg;
    bool Ok() const noexcept { return success; }
};

using ReplayFrameCallback = std::function<void(const ReplayFrame&)>;

class SessionReplayEngine {
public:
    explicit SessionReplayEngine() = default;
    void SetFrameCallback(ReplayFrameCallback cb) { m_callback = std::move(cb); }

    void StartRecording(const std::string& sessionId) {
        m_current.sessionId = sessionId;
        m_current.frames.clear();
        m_state = ReplayState::Recording;
        m_recordStart = 0;
    }

    void RecordFrame(const ReplayFrame& frame) {
        if (m_state != ReplayState::Recording) return;
        m_current.frames.push_back(frame);
    }

    ReplayRecording StopRecording() {
        m_state = ReplayState::Idle;
        m_current.durationMs = m_current.frames.empty() ? 0
            : m_current.frames.back().timestampMs - m_current.frames.front().timestampMs;
        return m_current;
    }

    ReplayPlaybackResult Play(const ReplayRecording& rec, ReplaySpeed speed = ReplaySpeed::Normal_1x) {
        m_state = ReplayState::Playing;
        int played = 0;
        for (const auto& f : rec.frames) {
            if (m_callback) m_callback(f);
            played++;
        }
        m_state = ReplayState::Idle;
        double factor = SpeedFactor(speed);
        return { true, played, rec.FrameCount(), rec.durationMs / factor, {} };
    }

    ReplayState GetState() const noexcept { return m_state; }

    static double SpeedFactor(ReplaySpeed s) noexcept {
        switch (s) {
        case ReplaySpeed::Slow_0_25x:  return 0.25;
        case ReplaySpeed::Normal_1x:   return 1.0;
        case ReplaySpeed::Fast_2x:     return 2.0;
        case ReplaySpeed::Super_4x:    return 4.0;
        }
        return 1.0;
    }

private:
    ReplayState      m_state      = ReplayState::Idle;
    ReplayRecording  m_current;
    int64_t          m_recordStart = 0;
    ReplayFrameCallback m_callback;
};

} // namespace Engine
} // namespace ExplorerLens
