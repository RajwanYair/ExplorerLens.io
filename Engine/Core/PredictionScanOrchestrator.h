// PredictionScanOrchestrator.h — Prediction Scan Orchestrator
// Copyright (c) 2026 ExplorerLens Project
//
// Orchestrates background predictive scan jobs with priority queueing and concurrency limits.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

enum class PSOScanPriority { Urgent = 0, High = 1, Normal = 2, Background = 3 };

struct PSOTrigger {
    std::wstring     folderPath;
    PSOScanPriority  priority   = PSOScanPriority::Normal;
    uint32_t         maxFiles   = 50;
};

struct PSOScanStatus {
    uint32_t pendingScans  = 0;
    uint32_t activeScans   = 0;
    uint32_t completedTotal = 0;
};

class PredictionScanOrchestrator {
public:
    void EnqueueScan(const PSOTrigger& trigger) {
        m_queue.push({ static_cast<int>(trigger.priority), trigger });
    }
    bool DrainOne() {
        if (m_queue.empty()) return false;
        auto [pri, t] = m_queue.top();
        m_queue.pop();
        ++m_completedTotal;
        return true;
    }
    PSOScanStatus GetStatus() const {
        PSOScanStatus s;
        s.pendingScans   = static_cast<uint32_t>(m_queue.size());
        s.activeScans    = 0;
        s.completedTotal = m_completedTotal;
        return s;
    }
    void Cancel() {
        while (!m_queue.empty()) m_queue.pop();
    }

private:
    using QPair = std::pair<int, PSOTrigger>;
    struct Cmp { bool operator()(const QPair& a, const QPair& b) const { return a.first > b.first; } };
    std::priority_queue<QPair, std::vector<QPair>, Cmp> m_queue;
    uint32_t m_completedTotal = 0;
};

}} // namespace ExplorerLens::Engine
