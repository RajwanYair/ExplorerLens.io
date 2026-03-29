// SharedARSpaceManager.h — Shared AR Space Manager
// Copyright (c) 2026 ExplorerLens Project
//
// Synchronizes AR anchor state and thumbnail overlays across multiple
// concurrent users in shared AR sessions for collaborative file review.
//
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct ARParticipant {
    std::string  userId;
    std::string  displayName;
    bool         isHost = false;
    uint64_t     joinedAt = 0;
};

struct ARSpaceUpdate {
    std::string  userId;
    uint64_t     anchorId;
    std::string  action;   // "add" | "move" | "remove"
    std::string  payload;
    uint64_t     timestampUs = 0;
};

class SharedARSpaceManager {
public:
    using UpdateCallback = std::function<void(const ARSpaceUpdate&)>;

    SharedARSpaceManager() = default;

    bool CreateSpace(const std::string& spaceId, const std::string& hostUserId) {
        m_spaceId = spaceId;
        ARParticipant host;
        host.userId = hostUserId;
        host.isHost = true;
        m_participants[hostUserId] = host;
        m_active = true;
        return true;
    }

    bool JoinSpace(const std::string& spaceId, const std::string& userId) {
        if (m_spaceId != spaceId || !m_active) return false;
        ARParticipant p;
        p.userId = userId;
        m_participants[userId] = p;
        return true;
    }

    bool LeaveSpace(const std::string& userId) {
        return m_participants.erase(userId) > 0;
    }

    bool BroadcastUpdate(const ARSpaceUpdate& update) {
        if (!m_active) return false;
        m_updateLog.push_back(update);
        if (m_onUpdate) m_onUpdate(update);
        return true;
    }

    void SetUpdateCallback(UpdateCallback cb) { m_onUpdate = cb; }

    uint32_t GetParticipantCount() const { return static_cast<uint32_t>(m_participants.size()); }
    const std::string& GetSpaceId() const { return m_spaceId; }
    bool IsActive() const { return m_active; }

    uint64_t GetUpdateCount() const { return static_cast<uint64_t>(m_updateLog.size()); }

    void CloseSpace() { m_active = false; m_participants.clear(); }

private:
    std::string  m_spaceId;
    bool         m_active = false;
    std::unordered_map<std::string, ARParticipant> m_participants;
    std::vector<ARSpaceUpdate> m_updateLog;
    UpdateCallback m_onUpdate;
};

}} // namespace ExplorerLens::Engine
