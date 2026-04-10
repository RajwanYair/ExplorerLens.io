// CollaborativeCacheCoordinator.cpp — Collaborative Cache Invalidation Coordinator
// Copyright (c) 2026 ExplorerLens Project
//
#include "CollaborativeCacheCoordinator.h"
#include <filesystem>

namespace ExplorerLens { namespace Engine {

SessionId CollaborativeCacheCoordinator::RegisterSession(InvalidationCallback cb)
{
    const SessionId id = m_nextId++;
    m_sessions[id] = Session{ id, std::move(cb), {} };
    return id;
}

void CollaborativeCacheCoordinator::DeregisterSession(SessionId id)
{
    m_sessions.erase(id);
}

void CollaborativeCacheCoordinator::Invalidate(
    const std::wstring& filePath, SessionId origin)
{
    // Determine parent folder of the changed file.
    const std::filesystem::path p(filePath);
    const std::wstring parentFolder = p.parent_path().wstring();

    for (auto& [sid, session] : m_sessions) {
        if (sid == origin) continue;  // Don't echo back to originator

        // Only notify sessions watching the same folder.
        bool matches = false;
        for (const auto& watched : session.watchedFolders) {
            if (parentFolder.find(watched) != std::wstring::npos) {
                matches = true;
                break;
            }
        }
        if (!matches) continue;

        if (session.cb) {
            session.cb(filePath, origin);
            ++m_invalidationsFired;
        }
    }
}

void CollaborativeCacheCoordinator::WatchFolder(
    SessionId id, const std::wstring& folderPath)
{
    auto it = m_sessions.find(id);
    if (it != m_sessions.end())
        it->second.watchedFolders.push_back(folderPath);
}

void CollaborativeCacheCoordinator::UnwatchFolder(
    SessionId id, const std::wstring& folderPath)
{
    auto it = m_sessions.find(id);
    if (it == m_sessions.end()) return;
    auto& folders = it->second.watchedFolders;
    folders.erase(
        std::remove(folders.begin(), folders.end(), folderPath),
        folders.end());
}

uint32_t CollaborativeCacheCoordinator::ActiveSessionCount() const
{
    return static_cast<uint32_t>(m_sessions.size());
}

uint32_t CollaborativeCacheCoordinator::InvalidationsFiredTotal() const
{
    return m_invalidationsFired;
}

void CollaborativeCacheCoordinator::Reset()
{
    m_sessions.clear();
    m_nextId             = 1;
    m_invalidationsFired = 0;
}

}} // namespace ExplorerLens::Engine
