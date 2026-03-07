// GPUFenceOrchestrator.h — GPU Fence Management and Synchronization
// Copyright (c) 2026 ExplorerLens Project
//
// Manages GPU fence objects for synchronization between command buffer
// submissions, CPU/GPU coordination, and multi-engine scheduling.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

enum class OrchestratorFenceState : uint8_t {
    Unsignaled,
    Signaled,
    WaitingCPU,
    WaitingGPU,
    Retired
};

struct GPUFence {
    uint64_t fenceId = 0;
    uint64_t fenceValue = 0;
    OrchestratorFenceState state = OrchestratorFenceState::Unsignaled;
    uint64_t createdTimestamp = 0;
    uint64_t signaledTimestamp = 0;
};

struct FenceOrchestratorMetrics {
    uint64_t totalFencesCreated = 0;
    uint64_t totalFencesSignaled = 0;
    uint64_t totalWaitsIssued = 0;
    uint64_t totalWaitsCompleted = 0;
    double avgWaitTimeUs = 0.0;
    uint32_t activeFences = 0;
};

class GPUFenceOrchestrator {
public:
    GPUFenceOrchestrator() = default;

    uint64_t CreateFence() {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t id = ++m_nextFenceId;
        GPUFence fence;
        fence.fenceId = id;
        fence.fenceValue = id;
        m_fences.push_back(fence);
        m_metrics.totalFencesCreated++;
        m_metrics.activeFences++;
        return id;
    }

    bool Signal(uint64_t fenceId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& fence : m_fences) {
            if (fence.fenceId == fenceId && fence.state == OrchestratorFenceState::Unsignaled) {
                fence.state = OrchestratorFenceState::Signaled;
                m_metrics.totalFencesSignaled++;
                return true;
            }
        }
        return false;
    }

    bool IsSignaled(uint64_t fenceId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& fence : m_fences) {
            if (fence.fenceId == fenceId) return fence.state == OrchestratorFenceState::Signaled;
        }
        return false;
    }

    void RetireFence(uint64_t fenceId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& fence : m_fences) {
            if (fence.fenceId == fenceId) {
                fence.state = OrchestratorFenceState::Retired;
                m_metrics.activeFences--;
                break;
            }
        }
    }

    void RetireAllSignaled() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& fence : m_fences) {
            if (fence.state == OrchestratorFenceState::Signaled) {
                fence.state = OrchestratorFenceState::Retired;
                m_metrics.activeFences--;
            }
        }
    }

    FenceOrchestratorMetrics GetMetrics() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_metrics;
    }

    uint32_t GetActiveFenceCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_metrics.activeFences;
    }

private:
    mutable std::mutex m_mutex;
    std::vector<GPUFence> m_fences;
    uint64_t m_nextFenceId = 0;
    FenceOrchestratorMetrics m_metrics;
};

} // namespace Engine
} // namespace ExplorerLens
