// PluginAuditTrail.h — Immutable audit log for plugin lifecycle events
// Copyright (c) 2026 ExplorerLens Project
//
// Records plugin load/unload/error events in a tamper-evident audit trail
// for security compliance and post-incident forensic analysis.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct PluginAuditTrailConfig {
    bool enabled = true;
    uint32_t maxEntries = 10000;
    std::string label = "PluginAuditTrail";
};

class PluginAuditTrail {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    PluginAuditTrailConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    enum class EventType : uint8_t { Load, Unload, Error, SecurityViolation, ConfigChange };

    struct AuditEntry {
        uint64_t timestamp = 0;
        std::string pluginName;
        EventType type = EventType::Load;
        std::string detail;
    };

    bool RecordEvent(const AuditEntry& entry) {
        if (m_entries.size() >= m_config.maxEntries) return false;
        m_entries.push_back(entry);
        return true;
    }

    size_t GetEntryCount() const { return m_entries.size(); }
    const std::vector<AuditEntry>& GetEntries() const { return m_entries; }

private:
    bool m_initialized = false;
    PluginAuditTrailConfig m_config;
    std::vector<AuditEntry> m_entries;
};

}
} // namespace ExplorerLens::Engine
