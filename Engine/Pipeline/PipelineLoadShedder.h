// PipelineLoadShedder.h — Overload protection via request dropping
// Copyright (c) 2026 ExplorerLens Project
//
// Implements load-shedding for the decode pipeline when request rate exceeds
// capacity, gracefully dropping low-priority requests to maintain responsiveness.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct PipelineLoadShedderConfig {
    bool enabled = true;
    uint32_t maxQueueDepth = 256;
    uint32_t shedThreshold = 200;
    uint32_t minPriorityToKeep = 50;
    std::string label = "PipelineLoadShedder";
};

class PipelineLoadShedder {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    PipelineLoadShedderConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    enum class Decision : uint8_t { Accept, Shed, Defer };

    Decision Evaluate(uint32_t currentQueueDepth, uint32_t requestPriority) const {
        if (currentQueueDepth >= m_config.maxQueueDepth)
            return Decision::Shed;
        if (currentQueueDepth >= m_config.shedThreshold &&
            requestPriority < m_config.minPriorityToKeep)
            return Decision::Defer;
        return Decision::Accept;
    }

    void RecordShed() { m_totalShed++; }
    void RecordAccepted() { m_totalAccepted++; }
    uint64_t GetTotalShed() const { return m_totalShed; }
    uint64_t GetTotalAccepted() const { return m_totalAccepted; }
    double GetShedRatio() const {
        uint64_t total = m_totalShed + m_totalAccepted;
        return total > 0 ? static_cast<double>(m_totalShed) / total : 0.0;
    }

private:
    bool m_initialized = false;
    PipelineLoadShedderConfig m_config;
    uint64_t m_totalShed = 0;
    uint64_t m_totalAccepted = 0;
};

}
} // namespace ExplorerLens::Engine
