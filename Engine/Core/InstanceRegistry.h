// InstanceRegistry.h — Multi-Instance Registry with Heartbeat & Stale Cleanup
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks all active ExplorerLens COM server instances. Each instance registers on
// load and sends periodic heartbeats. Stale entries (missed heartbeats) are pruned
// to enable correct load-balancing and cross-instance coordination.
//
#pragma once
#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

using InstanceId = uint64_t;

struct InstanceRecord
{
    InstanceId id = 0;
    uint32_t processId = 0;
    uint32_t sessionId = 0;
    std::string version;
    std::chrono::steady_clock::time_point registeredAt = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastHeartbeat = std::chrono::steady_clock::now();
    bool isAlive = true;
    int activePipelines = 0;

    bool IsStale(int staleThresholdMs = 5000) const noexcept
    {
        auto age =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - lastHeartbeat)
                .count();
        return age > staleThresholdMs;
    }
};

class InstanceRegistry
{
  public:
    static InstanceRegistry& Instance()
    {
        static InstanceRegistry inst;
        return inst;
    }

    InstanceId Register(uint32_t pid, uint32_t sessionId, const std::string& version = {})
    {
        InstanceId id = ++m_nextId;
        InstanceRecord r;
        r.id = id;
        r.processId = pid;
        r.sessionId = sessionId;
        r.version = version;
        m_records[id] = r;
        return id;
    }

    void Heartbeat(InstanceId id)
    {
        auto it = m_records.find(id);
        if (it != m_records.end())
            it->second.lastHeartbeat = std::chrono::steady_clock::now();
    }

    void Unregister(InstanceId id)
    {
        m_records.erase(id);
    }

    int PruneStale(int thresholdMs = 5000)
    {
        int pruned = 0;
        for (auto it = m_records.begin(); it != m_records.end();) {
            if (it->second.IsStale(thresholdMs)) {
                it = m_records.erase(it);
                pruned++;
            } else
                ++it;
        }
        return pruned;
    }

    const InstanceRecord* Get(InstanceId id) const
    {
        auto it = m_records.find(id);
        return it != m_records.end() ? &it->second : nullptr;
    }

    std::vector<InstanceRecord> AllInstances() const
    {
        std::vector<InstanceRecord> v;
        for (const auto& kv : m_records)
            v.push_back(kv.second);
        return v;
    }

    int Count() const noexcept
    {
        return (int)m_records.size();
    }
    bool IsEmpty() const noexcept
    {
        return m_records.empty();
    }

  private:
    InstanceRegistry() = default;
    std::unordered_map<InstanceId, InstanceRecord> m_records;
    InstanceId m_nextId = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
