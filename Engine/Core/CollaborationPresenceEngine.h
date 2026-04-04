// CollaborationPresenceEngine.h — Collaboration Presence Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Manages real-time user presence state for collaborative annotation sessions,
// tracking active participants, cursor positions, and last-seen timestamps.
//
#pragma once
#include <array>
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PresenceState {
    Online,
    Idle,
    Away,
    Offline
};
enum class CollabSessionRole {
    Owner,
    Editor,
    Viewer
};

struct PresenceUser
{
    std::string userId;
    std::string displayName;
    PresenceState state = PresenceState::Online;
    CollabSessionRole role = CollabSessionRole::Viewer;
    std::array<float, 2> cursorPos = {0.0f, 0.0f};
    std::array<uint8_t, 3> color = {255, 128, 0};
    int64_t lastSeenMs = 0;
};

struct PresenceEvent
{
    std::string userId;
    PresenceState newState = PresenceState::Online;
};

struct PresenceSnapshot
{
    std::vector<PresenceUser> users;
    int onlineCount() const noexcept
    {
        int c = 0;
        for (const auto& u : users)
            if (u.state == PresenceState::Online)
                c++;
        return c;
    }
};

class CollaborationPresenceEngine
{
  public:
    explicit CollaborationPresenceEngine() = default;

    void JoinSession(const PresenceUser& user)
    {
        m_users[user.userId] = user;
    }

    void LeaveSession(const std::string& userId)
    {
        auto it = m_users.find(userId);
        if (it != m_users.end())
            it->second.state = PresenceState::Offline;
    }

    void UpdatePresence(const std::string& userId, PresenceState state)
    {
        auto it = m_users.find(userId);
        if (it != m_users.end())
            it->second.state = state;
    }

    void UpdateCursor(const std::string& userId, float x, float y)
    {
        auto it = m_users.find(userId);
        if (it != m_users.end()) {
            it->second.cursorPos = {x, y};
        }
    }

    PresenceSnapshot GetSnapshot() const
    {
        PresenceSnapshot snap;
        for (const auto& [_, u] : m_users)
            snap.users.push_back(u);
        return snap;
    }

    int UserCount() const noexcept
    {
        return static_cast<int>(m_users.size());
    }

    static std::string StateName(PresenceState s) noexcept
    {
        switch (s) {
            case PresenceState::Online:
                return "Online";
            case PresenceState::Idle:
                return "Idle";
            case PresenceState::Away:
                return "Away";
            case PresenceState::Offline:
                return "Offline";
        }
        return "Unknown";
    }

  private:
    std::unordered_map<std::string, PresenceUser> m_users;
};

}  // namespace Engine
}  // namespace ExplorerLens
