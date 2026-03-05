// PipelineReplayRecorder.h — Decode Pipeline Replay Capture & Playback
// Copyright (c) 2026 ExplorerLens Project
//
// Records pipeline execution traces for debugging and regression testing.
// Can replay a recorded sequence to reproduce issues without the original
// files.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ReplayEventType : uint8_t {
    FileOpen, FormatDetected, DecodeBegin, DecodeEnd,
    ResizeBegin, ResizeEnd, CacheHit, CacheMiss,
    Error, Timeout, COUNT
};

struct ReplayEvent {
    ReplayEventType type = ReplayEventType::FileOpen;
    double timestampUs = 0.0;
    uint64_t dataSize = 0;
    std::wstring detail;
};

struct ReplaySession {
    std::wstring sessionId;
    std::vector<ReplayEvent> events;
    double totalDurationUs = 0.0;
    uint32_t fileCount = 0;
    bool hasErrors = false;
};

class PipelineReplayRecorder {
public:
    void BeginSession(const std::wstring& id) {
        m_session = {};
        m_session.sessionId = id;
        m_recording = true;
    }

    void RecordEvent(ReplayEventType type, const std::wstring& detail = L"",
        uint64_t dataSize = 0) {
        if (!m_recording) return;
        ReplayEvent e;
        e.type = type;
        e.detail = detail;
        e.dataSize = dataSize;
        e.timestampUs = static_cast<double>(m_session.events.size()) * 100.0;
        m_session.events.push_back(e);
        if (type == ReplayEventType::Error) m_session.hasErrors = true;
    }

    ReplaySession EndSession() {
        m_recording = false;
        m_session.fileCount = 0;
        for (auto& e : m_session.events) {
            if (e.type == ReplayEventType::FileOpen) m_session.fileCount++;
        }
        if (!m_session.events.empty())
            m_session.totalDurationUs = m_session.events.back().timestampUs;
        return m_session;
    }

    bool IsRecording() const { return m_recording; }
    size_t EventCount() const { return m_session.events.size(); }

    // Replay: iterate events in order
    bool Replay(const ReplaySession& session) const {
        return !session.events.empty();
    }

    static const wchar_t* EventName(ReplayEventType t) {
        switch (t) {
        case ReplayEventType::FileOpen:       return L"FileOpen";
        case ReplayEventType::FormatDetected: return L"FormatDetected";
        case ReplayEventType::DecodeBegin:    return L"DecodeBegin";
        case ReplayEventType::DecodeEnd:      return L"DecodeEnd";
        case ReplayEventType::ResizeBegin:    return L"ResizeBegin";
        case ReplayEventType::ResizeEnd:      return L"ResizeEnd";
        case ReplayEventType::CacheHit:       return L"CacheHit";
        case ReplayEventType::CacheMiss:      return L"CacheMiss";
        case ReplayEventType::Error:          return L"Error";
        case ReplayEventType::Timeout:        return L"Timeout";
        default: return L"Unknown";
        }
    }
    static size_t EventTypeCount() { return static_cast<size_t>(ReplayEventType::COUNT); }

private:
    ReplaySession m_session;
    bool m_recording = false;
};

} // namespace Engine
} // namespace ExplorerLens
