// ClusterAutoScaler.h — Cluster Auto Scaler
// Copyright (c) 2026 ExplorerLens Project
//
// Makes scale-up/scale-down decisions for the render cluster based on queue depth and load.
//
#pragma once
#include <cstdint>
#include <string>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

enum class CASScaleAction { ScaleUp, ScaleDown, NoChange };

struct CASMetrics {
    uint32_t queueDepth    = 0;
    float    avgCpuPercent = 0.0f;
    uint32_t activeNodes   = 0;
};

struct CASDecision {
    CASScaleAction action      = CASScaleAction::NoChange;
    uint32_t       targetNodes = 0;
    std::string    reason;
};

class ClusterAutoScaler {
public:
    ClusterAutoScaler(uint32_t minNodes, uint32_t maxNodes)
        : m_min(minNodes), m_max(maxNodes) {}

    CASDecision Evaluate(const CASMetrics& metrics) {
        CASDecision d;
        d.targetNodes = metrics.activeNodes;
        if (metrics.queueDepth > 20 || metrics.avgCpuPercent > 80.0f) {
            d.action      = (metrics.activeNodes < m_max) ? CASScaleAction::ScaleUp
                                                          : CASScaleAction::NoChange;
            d.targetNodes = std::min(m_max, metrics.activeNodes + 2u);
            d.reason      = "High load";
        } else if (metrics.queueDepth < 2 && metrics.avgCpuPercent < 20.0f) {
            d.action      = (metrics.activeNodes > m_min) ? CASScaleAction::ScaleDown
                                                          : CASScaleAction::NoChange;
            uint32_t dec  = (metrics.activeNodes > 1u) ? metrics.activeNodes - 1u : 1u;
            d.targetNodes = std::max(m_min, dec);
            d.reason      = "Low load";
        }
        return d;
    }

    uint32_t MinNodes() const { return m_min; }
    uint32_t MaxNodes() const { return m_max; }

private:
    uint32_t m_min;
    uint32_t m_max;
};

}} // namespace ExplorerLens::Engine
