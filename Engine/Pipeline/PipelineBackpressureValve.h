// PipelineBackpressureValve.h — Flow control for decode pipeline overload
// Copyright (c) 2026 ExplorerLens Project
//
// Implements backpressure signaling when downstream pipeline stages fall
// behind, throttling upstream producers to prevent memory exhaustion.
//
#pragma once
#include <string>
#include <cstdint>
#include <atomic>

namespace ExplorerLens {
namespace Engine {

struct PipelineBackpressureValveConfig {
    bool enabled = true;
    uint32_t highWaterMark = 128;
    uint32_t lowWaterMark = 32;
    std::string label = "PipelineBackpressureValve";
};

class PipelineBackpressureValve {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_pendingCount.store(0, std::memory_order_relaxed);
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    PipelineBackpressureValveConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    bool ShouldThrottle() const {
        return m_pendingCount.load(std::memory_order_acquire) >= m_config.highWaterMark;
    }

    bool IsRelieved() const {
        return m_pendingCount.load(std::memory_order_acquire) <= m_config.lowWaterMark;
    }

    void IncrementPending() { m_pendingCount.fetch_add(1, std::memory_order_release); }
    void DecrementPending() { m_pendingCount.fetch_sub(1, std::memory_order_release); }
    uint32_t GetPendingCount() const { return m_pendingCount.load(std::memory_order_acquire); }

private:
    bool m_initialized = false;
    PipelineBackpressureValveConfig m_config;
    std::atomic<uint32_t> m_pendingCount{ 0 };
};

}
} // namespace ExplorerLens::Engine
