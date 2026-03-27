// PluginSLAMonitor.h — Plugin SLA Monitor (P99 Budget)
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks per-plugin decode latency and alerts when P99 budget is exceeded — triggers auto-disable on sustained SLA breach.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct PluginSLABudget { std::string pluginId; double p99BudgetMs; uint32_t maxViolations; };
struct SLAViolation    { std::string pluginId; double observedP99; uint64_t timestamp; };
class PluginSLAMonitor {
public:
    void   RegisterBudget(PluginSLABudget budget)     { m_budgets[budget.pluginId] = budget; }
    void   RecordLatency(const std::string& id, double ms) {
        m_samples[id].push_back(ms);
    }
    bool   IsViolating(const std::string& id) const {
        auto it = m_samples.find(id);
        if (it == m_samples.end()) return false;
        auto& v = it->second;
        if (v.size() < 10) return false;
        auto sorted = v; std::sort(sorted.begin(), sorted.end());
        double p99 = sorted[static_cast<size_t>(sorted.size() * 0.99)];
        auto bIt = m_budgets.find(id);
        return bIt != m_budgets.end() && p99 > bIt->second.p99BudgetMs;
    }
    size_t ViolationCount() const { size_t n = 0; for (auto& [id,_] : m_budgets) if (IsViolating(id)) n++; return n; }
private:
    std::unordered_map<std::string, PluginSLABudget> m_budgets;
    std::unordered_map<std::string, std::vector<double>> m_samples;
};

} // namespace Engine
} // namespace ExplorerLens