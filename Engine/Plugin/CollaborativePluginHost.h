// CollaborativePluginHost.h — Collaborative Plugin Host (Multi-User Plugin Sessions)
// Copyright (c) 2026 ExplorerLens Project
//
// Extends the plugin host to support multi-user collaborative plugin sessions,
// where multiple participants can interact with the same plugin instance simultaneously.
//
#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class CollabPluginPolicy  { SingleOwner, MultiEditor, ViewOnly };
enum class CollabPluginState   { Idle, Active, Locked, Error };

struct CollabPluginSession {
    std::string         pluginId;
    std::string         ownerUserId;
    CollabPluginPolicy  policy   = CollabPluginPolicy::MultiEditor;
    CollabPluginState   state    = CollabPluginState::Idle;
    std::vector<std::string> participants;
};

struct CollabPluginInvokeResult {
    bool        success = false;
    std::string output;
    std::string invokerUserId;
    std::string errorMsg;
    bool Ok() const noexcept { return success; }
};

using CollabPluginHandler = std::function<CollabPluginInvokeResult(
    const std::string& pluginId, const std::string& userId, const std::string& input)>;

class CollaborativePluginHost {
public:
    explicit CollaborativePluginHost() = default;
    void SetHandler(CollabPluginHandler handler) { m_handler = std::move(handler); }

    bool CreateSession(const CollabPluginSession& session) {
        m_sessions[session.pluginId] = session;
        return true;
    }

    bool JoinSession(const std::string& pluginId, const std::string& userId) {
        auto it = m_sessions.find(pluginId);
        if (it == m_sessions.end()) return false;
        it->second.participants.push_back(userId);
        return true;
    }

    CollabPluginInvokeResult Invoke(const std::string& pluginId,
                                     const std::string& userId,
                                     const std::string& input) {
        if (!m_handler) return { false, {}, userId, "No handler" };
        auto it = m_sessions.find(pluginId);
        if (it == m_sessions.end()) return { false, {}, userId, "Session not found" };
        if (it->second.state == CollabPluginState::Locked)
            return { false, {}, userId, "Session locked" };
        return m_handler(pluginId, userId, input);
    }

    int SessionCount() const noexcept { return static_cast<int>(m_sessions.size()); }

    static std::string PolicyName(CollabPluginPolicy p) noexcept {
        switch (p) {
        case CollabPluginPolicy::SingleOwner: return "SingleOwner";
        case CollabPluginPolicy::MultiEditor: return "MultiEditor";
        case CollabPluginPolicy::ViewOnly:    return "ViewOnly";
        }
        return "Unknown";
    }

private:
    std::unordered_map<std::string, CollabPluginSession> m_sessions;
    CollabPluginHandler m_handler;
};

} // namespace Engine
} // namespace ExplorerLens
