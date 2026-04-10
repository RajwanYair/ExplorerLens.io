// CollaborativeCacheCoordinator.h — Collaborative Cache Invalidation Coordinator
// Copyright (c) 2026 ExplorerLens Project
//
// Fan-out cache-invalidation signals to all active Explorer sessions that have
// the same cloud folder open.  When one user saves a file, every other session's
// cached thumbnail for that file is marked stale so ExplorerLens re-decodes it.
//
#pragma once
#include <cstdint>
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>

namespace ExplorerLens { namespace Engine {

/// Opaque session identifier.
using SessionId = uint32_t;

/// Callback invoked on a registered session when a watched path is invalidated.
using InvalidationCallback = std::function<void(const std::wstring& filePath, SessionId origin)>;

/// Tracks active sessions and fans out cache-invalidation on file change.
class CollaborativeCacheCoordinator {
public:
    /// Register a session.  Returns the assigned SessionId.
    SessionId RegisterSession(InvalidationCallback cb);

    /// Deregister a session cleanly (e.g. Explorer window closed).
    void DeregisterSession(SessionId id);

    /// Mark a file as stale and notify all other sessions.
    /// The session that originated the change passes its own id as `origin` so
    /// it is not notified redundantly.
    void Invalidate(const std::wstring& filePath, SessionId origin);

    /// Watch a folder so all files under it trigger invalidation on change.
    void WatchFolder(SessionId id, const std::wstring& folderPath);
    void UnwatchFolder(SessionId id, const std::wstring& folderPath);

    uint32_t ActiveSessionCount()     const;
    uint32_t InvalidationsFiredTotal()const;

    void Reset();

private:
    struct Session {
        SessionId          id;
        InvalidationCallback cb;
        std::vector<std::wstring> watchedFolders;
    };

    SessionId               m_nextId = 1;
    std::unordered_map<SessionId, Session> m_sessions;
    uint32_t               m_invalidationsFired = 0;
};

}} // namespace ExplorerLens::Engine
