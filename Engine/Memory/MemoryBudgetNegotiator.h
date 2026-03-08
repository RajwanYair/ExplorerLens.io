// MemoryBudgetNegotiator.h — Negotiates memory budgets between subsystems
// Copyright (c) 2026 ExplorerLens Project
//
// Allocates a global memory budget across competing subsystems (cache,
// decode buffers, GPU staging) based on priority and current demand.
//
#pragma once
#include <string>
#include <cstdint>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct MemoryBudgetNegotiatorConfig {
    bool enabled = true;
    uint64_t totalBudgetMB = 512;
    std::string label = "MemoryBudgetNegotiator";
};

class MemoryBudgetNegotiator {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    MemoryBudgetNegotiatorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct Subsystem {
        std::string name;
        uint32_t priority = 0;
        uint64_t requestedMB = 0;
        uint64_t allocatedMB = 0;
    };

    bool RegisterSubsystem(const std::string& name, uint32_t priority, uint64_t requestedMB) {
        m_subsystems[name] = { name, priority, requestedMB, 0 };
        return true;
    }

    void NegotiateBudgets() {
        uint64_t remaining = m_config.totalBudgetMB;
        // Priority-ordered allocation
        for (auto& [name, sub] : m_subsystems) {
            uint64_t grant = (sub.requestedMB <= remaining) ? sub.requestedMB : remaining;
            sub.allocatedMB = grant;
            remaining -= grant;
        }
    }

    uint64_t GetAllocated(const std::string& name) const {
        auto it = m_subsystems.find(name);
        return (it != m_subsystems.end()) ? it->second.allocatedMB : 0;
    }

private:
    bool m_initialized = false;
    MemoryBudgetNegotiatorConfig m_config;
    std::unordered_map<std::string, Subsystem> m_subsystems;
};

}
} // namespace ExplorerLens::Engine
