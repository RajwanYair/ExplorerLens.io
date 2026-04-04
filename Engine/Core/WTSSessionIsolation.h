// WTSSessionIsolation.h — Multi-User WTS Session Isolation (RDS/Citrix)
// Copyright (c) 2026 ExplorerLens Project
//
// Isolates thumbnail cache and engine state per Windows Terminal Services session,
// enabling correct multi-user operation in RDS, Citrix, and VDI environments.
//
#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

using WTSSessionId = unsigned int;
static constexpr WTSSessionId WTS_INVALID_SESSION = 0xFFFFFFFFu;

enum class WTSSessionState {
    Active,
    Disconnected,
    Remote,
    Idle
};

struct WTSSessionInfo
{
    WTSSessionId sessionId = 0;
    std::wstring userName;
    std::wstring domainName;
    WTSSessionState state = WTSSessionState::Idle;
    bool isConsole = false;

    std::string StateName() const noexcept
    {
        switch (state) {
            case WTSSessionState::Active:
                return "Active";
            case WTSSessionState::Disconnected:
                return "Disconnected";
            case WTSSessionState::Remote:
                return "Remote";
            case WTSSessionState::Idle:
                return "Idle";
        }
        return "Unknown";
    }
};

struct SessionCacheScope
{
    WTSSessionId sessionId = 0;
    std::string cacheRoot;
    bool isIsolated = true;
};

class WTSSessionIsolation
{
  public:
    static WTSSessionIsolation& Instance()
    {
        static WTSSessionIsolation inst;
        return inst;
    }

    WTSSessionId CurrentSessionId() const noexcept
    {
        return m_currentSession;
    }
    void SetCurrentSession(WTSSessionId id) noexcept
    {
        m_currentSession = id;
    }

    void RegisterSession(const WTSSessionInfo& info)
    {
        m_sessions[info.sessionId] = info;
    }

    SessionCacheScope GetCacheScope(WTSSessionId id) const
    {
        SessionCacheScope scope;
        scope.sessionId = id;
        scope.cacheRoot = "lens_cache_" + std::to_string(id);
        scope.isIsolated = true;
        return scope;
    }

    SessionCacheScope CurrentCacheScope() const
    {
        return GetCacheScope(m_currentSession);
    }

    bool IsRDSEnvironment() const noexcept
    {
        return m_sessions.size() > 1;
    }
    int SessionCount() const noexcept
    {
        return (int)m_sessions.size();
    }

    std::vector<WTSSessionInfo> ActiveSessions() const
    {
        std::vector<WTSSessionInfo> v;
        for (const auto& kv : m_sessions)
            if (kv.second.state == WTSSessionState::Active)
                v.push_back(kv.second);
        return v;
    }

  private:
    WTSSessionIsolation() = default;
    WTSSessionId m_currentSession = 1;
    std::unordered_map<WTSSessionId, WTSSessionInfo> m_sessions;
};

}  // namespace Engine
}  // namespace ExplorerLens
