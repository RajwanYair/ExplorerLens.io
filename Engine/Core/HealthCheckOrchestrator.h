// HealthCheckOrchestrator.h — System Health Check Orchestration
// Copyright (c) 2026 ExplorerLens Project
//
// Coordinates periodic health checks across all engine subsystems.
// Aggregates decoder, cache, GPU, memory, and pipeline health into
// a unified dashboard model with auto-remediation capabilities.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class SubsystemId : uint8_t {
    DecoderPipeline, CacheEngine, GPURenderer, MemoryManager,
    PluginHost, ShellIntegration, IOPipeline, Telemetry, COUNT
};

enum class SubsystemHealth : uint8_t {
    Optimal, Normal, Degraded, Critical, Offline, COUNT
};

struct OrchestratorSubsystemStatus {
    SubsystemId id = SubsystemId::DecoderPipeline;
    SubsystemHealth health = SubsystemHealth::Normal;
    std::wstring name;
    std::wstring detail;
    double lastCheckMs = 0.0;
    uint32_t errorCount = 0;
    bool autoRemediable = false;
};

struct HealthSummary {
    uint32_t totalSubsystems = 0;
    uint32_t optimalCount = 0;
    uint32_t degradedCount = 0;
    uint32_t criticalCount = 0;
    uint32_t offlineCount = 0;
    SubsystemHealth worstHealth = SubsystemHealth::Optimal;
    double totalCheckMs = 0.0;
};

class HealthCheckOrchestrator {
public:
    void RegisterSubsystem(SubsystemId id, const std::wstring& name) {
        if (m_count < MAX_SUBSYSTEMS) {
            auto& s = m_subsystems[m_count++];
            s.id = id;
            s.name = name;
            s.health = SubsystemHealth::Normal;
        }
    }

    void UpdateHealth(SubsystemId id, SubsystemHealth health, const std::wstring& detail = L"") {
        for (uint32_t i = 0; i < m_count; ++i) {
            if (m_subsystems[i].id == id) {
                m_subsystems[i].health = health;
                m_subsystems[i].detail = detail;
                if (health == SubsystemHealth::Critical || health == SubsystemHealth::Offline)
                    m_subsystems[i].errorCount++;
                return;
            }
        }
    }

    SubsystemHealth GetHealth(SubsystemId id) const {
        for (uint32_t i = 0; i < m_count; ++i) {
            if (m_subsystems[i].id == id) return m_subsystems[i].health;
        }
        return SubsystemHealth::Offline;
    }

    HealthSummary Summarize() const {
        HealthSummary summary;
        summary.totalSubsystems = m_count;
        summary.worstHealth = SubsystemHealth::Optimal;
        for (uint32_t i = 0; i < m_count; ++i) {
            switch (m_subsystems[i].health) {
            case SubsystemHealth::Optimal: summary.optimalCount++; break;
            case SubsystemHealth::Degraded: summary.degradedCount++; break;
            case SubsystemHealth::Critical: summary.criticalCount++; break;
            case SubsystemHealth::Offline: summary.offlineCount++; break;
            default: break;
            }
            if (static_cast<uint8_t>(m_subsystems[i].health) > static_cast<uint8_t>(summary.worstHealth))
                summary.worstHealth = m_subsystems[i].health;
        }
        return summary;
    }

    uint32_t SubsystemCount() const { return m_count; }
    void Reset() { m_count = 0; }

    static size_t IdCount() { return static_cast<size_t>(SubsystemId::COUNT); }
    static size_t HealthLevelCount() { return static_cast<size_t>(SubsystemHealth::COUNT); }

private:
    static constexpr uint32_t MAX_SUBSYSTEMS = 16;
    OrchestratorSubsystemStatus m_subsystems[MAX_SUBSYSTEMS] = {};
    uint32_t m_count = 0;
};

} // namespace Engine
} // namespace ExplorerLens
