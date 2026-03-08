// DecodeQueueInspector.h — Diagnostic inspection of pending decode queue
// Copyright (c) 2026 ExplorerLens Project
//
// Provides read-only diagnostic access to the decode queue state — pending
// items, queue depth, estimated wait time, and per-format breakdown.
//
#pragma once
#include <string>
#include <cstdint>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct DecodeQueueInspectorConfig {
    bool enabled = true;
    uint32_t snapshotIntervalMs = 1000;
    std::string label = "DecodeQueueInspector";
};

class DecodeQueueInspector {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    DecodeQueueInspectorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct QueueSnapshot {
        uint32_t totalPending = 0;
        uint32_t inFlight = 0;
        double estimatedWaitMs = 0.0;
    };

    void UpdateSnapshot(uint32_t pending, uint32_t inflight, double waitMs) {
        m_snapshot = { pending, inflight, waitMs };
    }

    QueueSnapshot GetSnapshot() const { return m_snapshot; }
    bool IsIdle() const { return m_snapshot.totalPending == 0 && m_snapshot.inFlight == 0; }

private:
    bool m_initialized = false;
    DecodeQueueInspectorConfig m_config;
    QueueSnapshot m_snapshot;
};

}
} // namespace ExplorerLens::Engine
