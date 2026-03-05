// PluginSecurityAuditor.h — Runtime Plugin Behavior Auditing
// Copyright (c) 2026 ExplorerLens Project
//
// Runtime plugin behavior auditing. Monitors syscall patterns, file I/O, and
// network access. Flags security violations and generates audit reports.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <mutex>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

enum class PluginAuditEventType : uint8_t {
    FileRead,
    FileWrite,
    FileDelete,
    NetworkConnect,
    NetworkListen,
    NetworkSend,
    RegistryRead,
    RegistryWrite,
    ProcessSpawn,
    MemoryAllocLarge,
    DllLoad
};

enum class ViolationSeverity : uint8_t {
    Info,
    Warning,
    Critical,
    Blocked
};

struct PluginSecurityAuditEvent {
    PluginAuditEventType type = PluginAuditEventType::FileRead;
    uint32_t pluginId = 0;
    std::string target;
    uint64_t timestamp = 0;
    bool allowed = true;
    ViolationSeverity severity = ViolationSeverity::Info;
    std::string reason;
};

struct SecurityPolicy {
    std::unordered_set<std::string> allowedPaths;
    std::unordered_set<std::string> blockedPaths;
    bool allowNetworkAccess = false;
    bool allowProcessSpawn = false;
    bool allowRegistryWrite = false;
    bool allowDllLoad = false;
    size_t maxMemoryAllocBytes = 512 * 1024 * 1024;
    uint32_t maxFileWriteCount = 100;
};

struct AuditReport {
    uint32_t pluginId = 0;
    std::string pluginName;
    uint64_t totalEvents = 0;
    uint64_t allowedEvents = 0;
    uint64_t blockedEvents = 0;
    uint64_t warningCount = 0;
    uint64_t criticalCount = 0;
    std::vector<PluginSecurityAuditEvent> violations;
    double auditDurationSeconds = 0.0;
    bool passedAudit = true;
};

class PluginSecurityAuditor {
public:
    static PluginSecurityAuditor& Instance() {
        static PluginSecurityAuditor instance;
        return instance;
    }

    inline void SetPolicy(uint32_t pluginId, const SecurityPolicy& policy) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_policies[pluginId] = policy;
    }

    inline PluginSecurityAuditEvent AuditAction(uint32_t pluginId, PluginAuditEventType type, const std::string& target) {
        std::lock_guard<std::mutex> lock(m_mutex);
        PluginSecurityAuditEvent event;
        event.type = type;
        event.pluginId = pluginId;
        event.target = target;
        event.timestamp = CurrentTimestamp();

        auto policyIt = m_policies.find(pluginId);
        SecurityPolicy policy = policyIt != m_policies.end() ? policyIt->second : SecurityPolicy{};

        EvaluatePolicy(event, policy);

        m_auditLog[pluginId].push_back(event);

        auto& counts = m_eventCounts[pluginId];
        counts.totalEvents++;
        if (event.allowed) counts.allowedEvents++;
        else counts.blockedEvents++;

        return event;
    }

    inline AuditReport GenerateReport(uint32_t pluginId, const std::string& pluginName = "") const {
        std::lock_guard<std::mutex> lock(m_mutex);
        AuditReport report;
        report.pluginId = pluginId;
        report.pluginName = pluginName;

        auto countIt = m_eventCounts.find(pluginId);
        if (countIt != m_eventCounts.end()) {
            report.totalEvents = countIt->second.totalEvents;
            report.allowedEvents = countIt->second.allowedEvents;
            report.blockedEvents = countIt->second.blockedEvents;
        }

        auto logIt = m_auditLog.find(pluginId);
        if (logIt != m_auditLog.end()) {
            for (const auto& event : logIt->second) {
                if (!event.allowed || event.severity >= ViolationSeverity::Warning) {
                    report.violations.push_back(event);
                    if (event.severity == ViolationSeverity::Warning) report.warningCount++;
                    if (event.severity >= ViolationSeverity::Critical) report.criticalCount++;
                }
            }
        }

        report.passedAudit = report.criticalCount == 0 && report.blockedEvents == 0;
        return report;
    }

    inline bool IsActionAllowed(uint32_t pluginId, PluginAuditEventType type, const std::string& target) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto policyIt = m_policies.find(pluginId);
        if (policyIt == m_policies.end()) return true;

        PluginSecurityAuditEvent temp;
        temp.type = type;
        temp.target = target;
        EvaluatePolicy(temp, policyIt->second);
        return temp.allowed;
    }

    inline std::string EventTypeToString(PluginAuditEventType type) const {
        switch (type) {
        case PluginAuditEventType::FileRead:        return "FileRead";
        case PluginAuditEventType::FileWrite:       return "FileWrite";
        case PluginAuditEventType::FileDelete:      return "FileDelete";
        case PluginAuditEventType::NetworkConnect:   return "NetworkConnect";
        case PluginAuditEventType::NetworkListen:    return "NetworkListen";
        case PluginAuditEventType::NetworkSend:      return "NetworkSend";
        case PluginAuditEventType::RegistryRead:     return "RegistryRead";
        case PluginAuditEventType::RegistryWrite:    return "RegistryWrite";
        case PluginAuditEventType::ProcessSpawn:     return "ProcessSpawn";
        case PluginAuditEventType::MemoryAllocLarge: return "MemoryAllocLarge";
        case PluginAuditEventType::DllLoad:          return "DllLoad";
        default:                               return "Unknown";
        }
    }

    inline std::string SeverityToString(ViolationSeverity severity) const {
        switch (severity) {
        case ViolationSeverity::Info:     return "Info";
        case ViolationSeverity::Warning:  return "Warning";
        case ViolationSeverity::Critical: return "Critical";
        case ViolationSeverity::Blocked:  return "Blocked";
        default:                          return "Unknown";
        }
    }

    inline void ClearAuditLog(uint32_t pluginId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_auditLog.erase(pluginId);
        m_eventCounts.erase(pluginId);
    }

private:
    PluginSecurityAuditor() = default;

    struct EventCounts {
        uint64_t totalEvents = 0;
        uint64_t allowedEvents = 0;
        uint64_t blockedEvents = 0;
    };

    inline void EvaluatePolicy(PluginSecurityAuditEvent& event, const SecurityPolicy& policy) const {
        event.allowed = true;
        event.severity = ViolationSeverity::Info;

        switch (event.type) {
        case PluginAuditEventType::FileWrite:
        case PluginAuditEventType::FileDelete:
            if (!policy.blockedPaths.empty()) {
                for (const auto& blocked : policy.blockedPaths) {
                    if (event.target.find(blocked) != std::string::npos) {
                        event.allowed = false;
                        event.severity = ViolationSeverity::Blocked;
                        event.reason = "Path in blocked list: " + blocked;
                        return;
                    }
                }
            }
            if (!policy.allowedPaths.empty()) {
                bool inAllowed = false;
                for (const auto& allowed : policy.allowedPaths) {
                    if (event.target.find(allowed) != std::string::npos) {
                        inAllowed = true;
                        break;
                    }
                }
                if (!inAllowed) {
                    event.severity = ViolationSeverity::Warning;
                    event.reason = "File write outside allowed paths";
                }
            }
            break;

        case PluginAuditEventType::NetworkConnect:
        case PluginAuditEventType::NetworkListen:
        case PluginAuditEventType::NetworkSend:
            if (!policy.allowNetworkAccess) {
                event.allowed = false;
                event.severity = ViolationSeverity::Critical;
                event.reason = "Network access not permitted";
            }
            break;

        case PluginAuditEventType::ProcessSpawn:
            if (!policy.allowProcessSpawn) {
                event.allowed = false;
                event.severity = ViolationSeverity::Critical;
                event.reason = "Process spawning not permitted";
            }
            break;

        case PluginAuditEventType::RegistryWrite:
            if (!policy.allowRegistryWrite) {
                event.allowed = false;
                event.severity = ViolationSeverity::Critical;
                event.reason = "Registry write not permitted";
            }
            break;

        case PluginAuditEventType::DllLoad:
            if (!policy.allowDllLoad) {
                event.severity = ViolationSeverity::Warning;
                event.reason = "DLL loading should be verified";
            }
            break;

        default:
            break;
        }
    }

    inline uint64_t CurrentTimestamp() const {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
    }

    mutable std::mutex m_mutex;
    std::unordered_map<uint32_t, SecurityPolicy> m_policies;
    std::unordered_map<uint32_t, std::vector<PluginSecurityAuditEvent>> m_auditLog;
    std::unordered_map<uint32_t, EventCounts> m_eventCounts;
};

}
} // namespace ExplorerLens::Engine
