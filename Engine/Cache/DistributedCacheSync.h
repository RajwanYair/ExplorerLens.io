// DistributedCacheSync.h — Network-Distributed Cache Synchronization
// Copyright (c) 2026 ExplorerLens Project
//
// Network-distributed cache synchronization. Uses UDP multicast for cache
// invalidation broadcasts with eventual consistency protocol.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <array>

namespace ExplorerLens {
namespace Engine {

enum class SyncMessageType : uint8_t {
    Invalidate,
    Update,
    Heartbeat,
    JoinCluster,
    LeaveCluster,
    RequestSync,
    FullSync
};

enum class NodeStatus : uint8_t {
    Online,
    Offline,
    Syncing,
    Degraded
};

struct CacheSyncMessage {
    SyncMessageType type = SyncMessageType::Heartbeat;
    uint64_t sequenceNumber = 0;
    uint64_t timestamp = 0;
    std::string cacheKey;
    uint32_t version = 0;
    uint32_t sourceNodeId = 0;
    uint32_t payloadSize = 0;
};

struct ClusterNode {
    uint32_t nodeId = 0;
    std::string address;
    uint16_t port = 0;
    NodeStatus status = NodeStatus::Offline;
    uint64_t lastHeartbeat = 0;
    uint64_t lastSequence = 0;
    uint32_t cacheEntryCount = 0;
};

struct SyncStats {
    uint64_t messagesSent = 0;
    uint64_t messagesReceived = 0;
    uint64_t invalidationsSent = 0;
    uint64_t conflictsResolved = 0;
    uint64_t bytesTransferred = 0;
    uint32_t activeNodes = 0;
    double avgSyncLatencyMs = 0.0;
};

class DistributedCacheSync {
public:
    static DistributedCacheSync& Instance() {
        static DistributedCacheSync instance;
        return instance;
    }

    inline void RegisterNode(uint32_t nodeId, const std::string& address, uint16_t port) {
        std::lock_guard<std::mutex> lock(m_mutex);
        ClusterNode node;
        node.nodeId = nodeId;
        node.address = address;
        node.port = port;
        node.status = NodeStatus::Online;
        node.lastHeartbeat = CurrentTimestamp();
        m_nodes[nodeId] = node;
        m_stats.activeNodes = CountActiveNodes();
    }

    inline void RemoveNode(uint32_t nodeId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_nodes.find(nodeId);
        if (it != m_nodes.end()) {
            it->second.status = NodeStatus::Offline;
            m_stats.activeNodes = CountActiveNodes();
        }
    }

    inline CacheSyncMessage CreateInvalidation(const std::string& cacheKey, uint32_t version) {
        std::lock_guard<std::mutex> lock(m_mutex);
        CacheSyncMessage msg;
        msg.type = SyncMessageType::Invalidate;
        msg.sequenceNumber = m_nextSequence++;
        msg.timestamp = CurrentTimestamp();
        msg.cacheKey = cacheKey;
        msg.version = version;
        msg.sourceNodeId = m_localNodeId;
        m_stats.invalidationsSent++;
        m_stats.messagesSent++;
        return msg;
    }

    inline bool ProcessIncomingMessage(const CacheSyncMessage& msg) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.messagesReceived++;

        auto nodeIt = m_nodes.find(msg.sourceNodeId);
        if (nodeIt != m_nodes.end()) {
            nodeIt->second.lastHeartbeat = CurrentTimestamp();
            nodeIt->second.lastSequence = msg.sequenceNumber;
        }

        switch (msg.type) {
        case SyncMessageType::Invalidate:
            return ProcessInvalidation(msg);
        case SyncMessageType::Update:
            return ProcessUpdate(msg);
        case SyncMessageType::Heartbeat:
            return ProcessHeartbeat(msg);
        default:
            return false;
        }
    }

    inline bool ResolveConflict(const std::string& key, uint32_t localVersion, uint32_t remoteVersion) {
        (void)key;
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.conflictsResolved++;
        return remoteVersion > localVersion;
    }

    inline SyncStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    inline std::vector<ClusterNode> GetActiveNodes() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<ClusterNode> active;
        for (const auto& [id, node] : m_nodes) {
            if (node.status == NodeStatus::Online) active.push_back(node);
        }
        return active;
    }

    inline void SetLocalNodeId(uint32_t nodeId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_localNodeId = nodeId;
    }

private:
    DistributedCacheSync() = default;

    inline uint64_t CurrentTimestamp() const {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
    }

    inline uint32_t CountActiveNodes() const {
        uint32_t count = 0;
        for (const auto& [id, node] : m_nodes) {
            if (node.status == NodeStatus::Online) ++count;
        }
        return count;
    }

    inline bool ProcessInvalidation(const CacheSyncMessage& msg) {
        auto it = m_versionMap.find(msg.cacheKey);
        if (it == m_versionMap.end() || msg.version > it->second) {
            m_versionMap[msg.cacheKey] = msg.version;
            return true;
        }
        return false;
    }

    inline bool ProcessUpdate(const CacheSyncMessage& msg) {
        m_versionMap[msg.cacheKey] = msg.version;
        return true;
    }

    inline bool ProcessHeartbeat(const CacheSyncMessage& msg) {
        auto it = m_nodes.find(msg.sourceNodeId);
        if (it != m_nodes.end()) {
            it->second.status = NodeStatus::Online;
            it->second.lastHeartbeat = msg.timestamp;
            return true;
        }
        return false;
    }

    mutable std::mutex m_mutex;
    std::unordered_map<uint32_t, ClusterNode> m_nodes;
    std::unordered_map<std::string, uint32_t> m_versionMap;
    SyncStats m_stats;
    uint32_t m_localNodeId = 0;
    uint64_t m_nextSequence = 1;
};

}
} // namespace ExplorerLens::Engine
