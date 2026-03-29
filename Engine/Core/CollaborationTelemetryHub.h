// CollaborationTelemetryHub.h — Collaboration Telemetry Hub
// Copyright (c) 2026 ExplorerLens Project
//
// Aggregates collaboration session telemetry (user activity, edit rates, conflict
// frequency, latency) and exposes structured metrics for dashboards and observability.
//
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>

namespace ExplorerLens {
namespace Engine {

enum class CollabTelemetryEvent {
    SessionStarted,
    SessionEnded,
    AnnotationEdited,
    ConflictDetected,
    ConflictResolved,
    UserJoined,
    UserLeft,
    SyncCompleted
};

struct CollabTelemetryRecord {
    CollabTelemetryEvent event     = CollabTelemetryEvent::AnnotationEdited;
    std::string          sessionId;
    std::string          userId;
    int64_t              timestampMs = 0;
    std::string          extra;
};

struct CollabSessionMetrics {
    std::string sessionId;
    int         edits          = 0;
    int         conflicts      = 0;
    int         resolvedCount  = 0;
    int         usersTotal     = 0;
    double      avgEditLatencyMs = 0.0;
    double ConflictRate() const noexcept {
        return edits > 0 ? (100.0 * conflicts / edits) : 0.0;
    }
};

class CollaborationTelemetryHub {
public:
    explicit CollaborationTelemetryHub() = default;

    void Record(const CollabTelemetryRecord& rec) {
        m_records.push_back(rec);
        UpdateMetrics(rec);
    }

    CollabSessionMetrics GetMetrics(const std::string& sessionId) const {
        auto it = m_metrics.find(sessionId);
        if (it != m_metrics.end()) return it->second;
        return {};
    }

    int TotalEvents() const noexcept { return static_cast<int>(m_records.size()); }

    std::vector<CollabSessionMetrics> AllMetrics() const {
        std::vector<CollabSessionMetrics> result;
        for (const auto& [_, m] : m_metrics) result.push_back(m);
        return result;
    }

    static std::string EventName(CollabTelemetryEvent e) noexcept {
        switch (e) {
        case CollabTelemetryEvent::SessionStarted:   return "SessionStarted";
        case CollabTelemetryEvent::SessionEnded:     return "SessionEnded";
        case CollabTelemetryEvent::AnnotationEdited: return "AnnotationEdited";
        case CollabTelemetryEvent::ConflictDetected: return "ConflictDetected";
        case CollabTelemetryEvent::ConflictResolved: return "ConflictResolved";
        case CollabTelemetryEvent::UserJoined:       return "UserJoined";
        case CollabTelemetryEvent::UserLeft:         return "UserLeft";
        case CollabTelemetryEvent::SyncCompleted:    return "SyncCompleted";
        }
        return "Unknown";
    }

private:
    void UpdateMetrics(const CollabTelemetryRecord& rec) {
        auto& m = m_metrics[rec.sessionId];
        m.sessionId = rec.sessionId;
        switch (rec.event) {
        case CollabTelemetryEvent::AnnotationEdited: m.edits++;         break;
        case CollabTelemetryEvent::ConflictDetected: m.conflicts++;     break;
        case CollabTelemetryEvent::ConflictResolved: m.resolvedCount++; break;
        case CollabTelemetryEvent::UserJoined:       m.usersTotal++;    break;
        default: break;
        }
    }

    std::vector<CollabTelemetryRecord> m_records;
    std::unordered_map<std::string, CollabSessionMetrics> m_metrics;
};

} // namespace Engine
} // namespace ExplorerLens
