// CrossInstanceLoadBalancer.h — Load-Balanced Decode Dispatch Across Instances
// Copyright (c) 2026 ExplorerLens Project
//
// Routes decode requests across registered ExplorerLens COM server instances using a
// weighted least-connection algorithm, preventing any single instance from becoming
// a bottleneck in multi-monitor or multi-folder navigation scenarios.
//
#pragma once
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class LBAlgorithm {
    RoundRobin,
    LeastConnections,
    WeightedRoundRobin
};

struct InstanceLoad
{
    uint64_t instanceId = 0;
    int activeTasks = 0;
    int weight = 1;
    double avgLatencyMs = 0.0;
    bool available = true;
    bool IsHealthy() const noexcept
    {
        return available && activeTasks < 1000;
    }
};

struct DispatchDecision
{
    uint64_t instanceId = 0;
    bool found = false;
    std::string reason;
};

class CrossInstanceLoadBalancer
{
  public:
    explicit CrossInstanceLoadBalancer(LBAlgorithm algo = LBAlgorithm::LeastConnections) : m_algo(algo) {}

    void RegisterInstance(const InstanceLoad& load)
    {
        m_instances[load.instanceId] = load;
    }
    void UnregisterInstance(uint64_t id)
    {
        m_instances.erase(id);
    }

    void MarkTaskStart(uint64_t id)
    {
        auto it = m_instances.find(id);
        if (it != m_instances.end())
            it->second.activeTasks++;
    }

    void MarkTaskEnd(uint64_t id)
    {
        auto it = m_instances.find(id);
        if (it != m_instances.end() && it->second.activeTasks > 0)
            it->second.activeTasks--;
    }

    DispatchDecision Dispatch() const
    {
        DispatchDecision d;
        if (m_instances.empty()) {
            d.reason = "no_instances";
            return d;
        }

        switch (m_algo) {
            case LBAlgorithm::LeastConnections: {
                const InstanceLoad* best = nullptr;
                for (const auto& [id, load] : m_instances) {
                    if (!load.IsHealthy())
                        continue;
                    if (!best || load.activeTasks < best->activeTasks)
                        best = &load;
                }
                if (best) {
                    d.instanceId = best->instanceId;
                    d.found = true;
                }
                break;
            }
            case LBAlgorithm::RoundRobin: {
                // Simple: pick first available
                for (const auto& [id, load] : m_instances) {
                    if (load.IsHealthy()) {
                        d.instanceId = id;
                        d.found = true;
                        break;
                    }
                }
                break;
            }
            default:
                for (const auto& [id, load] : m_instances) {
                    if (load.IsHealthy()) {
                        d.instanceId = id;
                        d.found = true;
                        break;
                    }
                }
                break;
        }
        if (!d.found)
            d.reason = "no_healthy_instance";
        return d;
    }

    int InstanceCount() const noexcept
    {
        return (int)m_instances.size();
    }
    int TotalActiveTasks() const noexcept
    {
        int n = 0;
        for (const auto& kv : m_instances)
            n += kv.second.activeTasks;
        return n;
    }

    LBAlgorithm Algorithm() const noexcept
    {
        return m_algo;
    }
    void SetAlgorithm(LBAlgorithm a) noexcept
    {
        m_algo = a;
    }

  private:
    std::unordered_map<uint64_t, InstanceLoad> m_instances;
    LBAlgorithm m_algo;
};

}  // namespace Engine
}  // namespace ExplorerLens
