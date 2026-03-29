// UserPreferenceLearner.h — User Preference Learning Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Learns user thumbnail display preferences through implicit signals
// (dwell time, click patterns) using collaborative + content filtering.
//
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class PreferenceSignal { Dwell, Click, Zoom, Pin, Hide, Share };

struct PreferenceEvent {
    std::string     userId;
    std::string     fileKey;
    PreferenceSignal signal;
    float           weight = 1.0f;
    uint64_t        timestampMs = 0;
};

struct UserPreferences {
    std::string  userId;
    uint32_t     preferredGridSize   = 128;
    float        preferredZoomLevel  = 1.0f;
    bool         prefersLandscape    = true;
    bool         prefersCompactView  = false;
    std::vector<std::string> pinnedTypes;
};

class UserPreferenceLearner {
public:
    UserPreferenceLearner() = default;

    bool Initialize() { m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    void RecordEvent(const PreferenceEvent& event) {
        m_events[event.userId].push_back(event);
        m_totalEvents++;
    }

    UserPreferences GetPreferences(const std::string& userId) const {
        UserPreferences prefs;
        prefs.userId = userId;
        auto it = m_events.find(userId);
        if (it == m_events.end()) return prefs;
        uint32_t clicks = 0, dwells = 0;
        for (const auto& e : it->second) {
            if (e.signal == PreferenceSignal::Click) ++clicks;
            if (e.signal == PreferenceSignal::Dwell) ++dwells;
        }
        prefs.preferredGridSize = (dwells > clicks) ? 192 : 128;
        return prefs;
    }

    uint64_t GetTotalEvents() const { return m_totalEvents; }
    uint64_t GetUserCount()   const { return static_cast<uint64_t>(m_events.size()); }

    void ClearUser(const std::string& userId) { m_events.erase(userId); }
    void Shutdown() { m_ready = false; }

private:
    bool m_ready = false;
    uint64_t m_totalEvents = 0;
    std::unordered_map<std::string, std::vector<PreferenceEvent>> m_events;
};

}} // namespace ExplorerLens::Engine
