// RenderClusterManager.h — Render Cluster Manager
// Copyright (c) 2026 ExplorerLens Project
//
// Manages a pool of render nodes for distributed thumbnail workload dispatch.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

enum class RCMNodeStatus { Idle, Busy, Offline, Draining };

struct RCMNode {
    std::string   nodeId;
    std::string   address;
    RCMNodeStatus status      = RCMNodeStatus::Idle;
    uint32_t      capacity    = 4;
    float         loadPercent = 0.0f;
};

struct RCMSubmitResult {
    bool        success   = false;
    std::string assignedNodeId;
    uint32_t    jobId     = 0;
    std::string errorMsg;
};

class RenderClusterManager {
public:
    void RegisterNode(const RCMNode& node) { m_nodes[node.nodeId] = node; }

    RCMSubmitResult Submit(const std::string& workload) {
        RCMSubmitResult r;
        for (auto& [id, n] : m_nodes) {
            if (n.status == RCMNodeStatus::Idle) {
                n.status         = RCMNodeStatus::Busy;
                n.loadPercent    = 50.0f;
                r.assignedNodeId = id;
                r.jobId          = ++m_jobCounter;
                r.success        = true;
                m_jobs[r.jobId]  = workload;
                return r;
            }
        }
        r.errorMsg = "No idle nodes";
        return r;
    }

    bool CompleteJob(uint32_t jobId) {
        auto jit = m_jobs.find(jobId);
        if (jit == m_jobs.end()) return false;
        m_jobs.erase(jit);
        return true;
    }

    uint32_t ActiveNodeCount() const {
        uint32_t n = 0;
        for (const auto& [k, node] : m_nodes)
            if (node.status != RCMNodeStatus::Offline) ++n;
        return n;
    }

private:
    std::unordered_map<std::string, RCMNode>  m_nodes;
    std::unordered_map<uint32_t, std::string> m_jobs;
    uint32_t                                  m_jobCounter = 0;
};

}} // namespace ExplorerLens::Engine
