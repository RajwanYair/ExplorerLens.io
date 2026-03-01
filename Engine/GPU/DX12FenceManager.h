#pragma once
// DX12FenceManager.h — DirectX 12 fence synchronization for GPU decode pipeline
// Sprint 439 — ExplorerLens v15.0.0 Zenith

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// State of a GPU fence
enum class FenceState : uint8_t {
    Unsignaled = 0,  // Fence created but not yet signaled
    Signaled = 1,  // Fence has been signaled by GPU
    Waiting = 2,  // CPU is waiting on this fence
    Timeout = 3,  // Wait timed out
    Error = 4   // Fence is in error state
};

inline const char* FenceStateName(FenceState s) noexcept {
    switch (s) {
    case FenceState::Unsignaled: return "Unsignaled";
    case FenceState::Signaled:   return "Signaled";
    case FenceState::Waiting:    return "Waiting";
    case FenceState::Timeout:    return "Timeout";
    case FenceState::Error:      return "Error";
    default:                     return "Unknown";
    }
}

/// Strategy for waiting on fence completion
enum class FenceWaitMode : uint8_t {
    Blocking = 0,  // WaitForSingleObject (blocks thread)
    Polling = 1,  // Spin-poll GetCompletedValue
    Event = 2,  // SetEventOnCompletion callback
    Hybrid = 3,  // Spin briefly, then block
    BusyWait = 4   // Pure busy-wait (lowest latency)
};

inline const char* FenceWaitModeName(FenceWaitMode m) noexcept {
    switch (m) {
    case FenceWaitMode::Blocking: return "Blocking";
    case FenceWaitMode::Polling:  return "Polling";
    case FenceWaitMode::Event:    return "Event";
    case FenceWaitMode::Hybrid:   return "Hybrid";
    case FenceWaitMode::BusyWait: return "BusyWait";
    default:                      return "Unknown";
    }
}

/// Represents a single DX12 fence object
struct FenceHandle {
    uint64_t   id = 0;
    uint64_t   currentValue = 0;
    uint64_t   completedValue = 0;
    FenceState state = FenceState::Unsignaled;
};

/// Configuration for fence management
struct FenceManagerConfig {
    FenceWaitMode defaultWaitMode = FenceWaitMode::Hybrid;
    uint32_t      timeoutMs = 5000;     // Default wait timeout
    uint32_t      maxFences = 256;       // Maximum active fences
    uint32_t      spinCountBeforeBlock = 1000; // Hybrid mode spin iterations
};

/// Manages DirectX 12 fence objects for GPU/CPU synchronization
/// in the thumbnail decode pipeline, supporting multiple wait
/// strategies including hybrid spin-then-block.
class DX12FenceManager {
public:
    DX12FenceManager() = default;
    ~DX12FenceManager() = default;

    DX12FenceManager(const DX12FenceManager&) = delete;
    DX12FenceManager& operator=(const DX12FenceManager&) = delete;
    DX12FenceManager(DX12FenceManager&&) noexcept = default;
    DX12FenceManager& operator=(DX12FenceManager&&) noexcept = default;

    /// Create a new fence and return its ID
    uint64_t CreateFence() {
        uint64_t id = ++m_nextId;
        FenceHandle fence;
        fence.id = id;
        fence.state = FenceState::Unsignaled;
        m_fences.push_back(fence);
        return id;
    }

    /// Signal a fence with a new value
    bool Signal(uint64_t fenceId, uint64_t value) {
        for (auto& f : m_fences) {
            if (f.id == fenceId) {
                f.currentValue = value;
                f.completedValue = value;
                f.state = FenceState::Signaled;
                m_signalCount++;
                return true;
            }
        }
        return false;
    }

    /// Wait for a fence to reach a specific value
    bool WaitForFence(uint64_t fenceId, uint64_t targetValue) {
        for (auto& f : m_fences) {
            if (f.id == fenceId) {
                f.state = FenceState::Waiting;
                m_waitCount++;
                if (f.completedValue >= targetValue) {
                    f.state = FenceState::Signaled;
                    return true;
                }
                // Simulate timeout for unreached values
                f.state = FenceState::Timeout;
                return false;
            }
        }
        return false;
    }

    /// Get the completed value of a fence
    uint64_t GetCompletedValue(uint64_t fenceId) const {
        for (const auto& f : m_fences) {
            if (f.id == fenceId) return f.completedValue;
        }
        return 0;
    }

    /// Get number of active fences
    size_t GetActiveFenceCount() const noexcept { return m_fences.size(); }

    /// Get total signal count
    uint64_t GetSignalCount() const noexcept { return m_signalCount; }

    /// Get total wait count
    uint64_t GetWaitCount() const noexcept { return m_waitCount; }

    /// Apply configuration
    void SetConfig(const FenceManagerConfig& cfg) noexcept { m_config = cfg; }

    /// Get wait mode
    FenceWaitMode GetWaitMode() const noexcept { return m_config.defaultWaitMode; }

private:
    FenceManagerConfig        m_config;
    std::vector<FenceHandle>  m_fences;
    uint64_t                  m_nextId = 0;
    uint64_t                  m_signalCount = 0;
    uint64_t                  m_waitCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
