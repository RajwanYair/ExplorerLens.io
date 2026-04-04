// PresenceIndicatorEngine.h — Presence Indicator Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks active collaborator presence and idle timeout for real-time annotation sessions.
//
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PIEPresenceState {
    Active,
    Idle,
    Offline
};

struct PIEUser
{
    std::string userId;
    PIEPresenceState state = PIEPresenceState::Offline;
    uint64_t lastSeenMs = 0;
    std::string displayName;
};

class PresenceIndicatorEngine
{
  public:
    void UpdatePresence(const PIEUser& user)
    {
        m_users[user.userId] = user;
    }

    PIEPresenceState GetPresence(const std::string& userId) const
    {
        auto it = m_users.find(userId);
        return it != m_users.end() ? it->second.state : PIEPresenceState::Offline;
    }
    std::vector<PIEUser> ActiveUsers() const
    {
        std::vector<PIEUser> out;
        for (const auto& [id, u] : m_users)
            if (u.state == PIEPresenceState::Active)
                out.push_back(u);
        return out;
    }
    void SetOffline(const std::string& userId)
    {
        auto it = m_users.find(userId);
        if (it != m_users.end())
            it->second.state = PIEPresenceState::Offline;
    }

  private:
    std::unordered_map<std::string, PIEUser> m_users;
};

}  // namespace Engine
}  // namespace ExplorerLens
