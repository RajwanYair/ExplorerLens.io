// NodeHealthMonitor.h — Node Health Monitor
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks heartbeats and health metrics for distributed render cluster nodes.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

enum class NHMHealthState { Healthy, Degraded, Unresponsive };

struct NHMHeartbeat {
    std::string nodeId;
    float       cpuPercent  = 0.0f;
    float       memPercent  = 0.0f;
    uint64_t    timestampMs = 0;
};

struct NHMHealthReport {
    NHMHealthState state       = NHMHealthState::Healthy;
    uint32_t       missedBeats = 0;
    float          avgCpu      = 0.0f;
};

class NodeHealthMonitor {
public:
    void RecordHeartbeat(const NHMHeartbeat& hb) {
        m_latestHb[hb.nodeId]    = hb;
        m_missedBeats[hb.nodeId] = 0;
    }

    NHMHealthReport GetReport(const std::string& nodeId) const {
        NHMHealthReport r;
        auto it = m_latestHb.find(nodeId);
        if (it == m_latestHb.end()) {
            r.state = NHMHealthState::Unresponsive;
            return r;
        }
        r.avgCpu = it->second.cpuPercent;
        auto mb  = m_missedBeats.find(nodeId);
        r.missedBeats = (mb != m_missedBeats.end()) ? mb->second : 0u;
        r.state = (r.missedBeats >= 3)  ? NHMHealthState::Unresponsive :
                  (r.avgCpu > 85.0f)    ? NHMHealthState::Degraded
                                        : NHMHealthState::Healthy;
        return r;
    }

    void IncrementMissed(const std::string& nodeId) { m_missedBeats[nodeId]++; }

    uint32_t TrackedNodeCount() const {
        return static_cast<uint32_t>(m_latestHb.size());
    }

private:
    std::unordered_map<std::string, NHMHeartbeat> m_latestHb;
    std::unordered_map<std::string, uint32_t>     m_missedBeats;
};

}} // namespace ExplorerLens::Engine
