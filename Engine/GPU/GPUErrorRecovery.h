// GPUErrorRecovery.h — GPU Fault Detection and Recovery
// Copyright (c) 2026 ExplorerLens Project
//
// Detects GPU device removal, TDR events, and shader compilation failures,
// then orchestrates recovery by falling back to alternate APIs or CPU paths.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace ExplorerLens {
namespace Engine {

enum class GPUErrorType : uint8_t {
    None = 0,
    DeviceRemoved = 1,     // DXGI_ERROR_DEVICE_REMOVED
    DeviceHung = 2,        // Device timeout (TDR)
    DriverCrash = 3,       // Internal driver error
    OutOfMemory = 4,       // GPU VRAM exhausted
    ShaderCompileFail = 5, // PSO/shader creation failure
    PresentFail = 6        // Swap chain / present error
};

enum class GPURecoveryAction : uint8_t {
    Retry = 0,          // Retry same operation
    FallbackAPI = 1,    // Switch DX12 -> DX11 -> WARP -> CPU
    ReduceLoad = 2,     // Lower resolution / disable features
    ResetDevice = 3,    // Full device recreation
    DisableGPU = 4      // Permanent CPU-only mode for session
};

struct GPUErrorEvent {
    GPUErrorType type = GPUErrorType::None;
    uint32_t hresult = 0;
    std::string description;
    uint64_t timestampMs = 0;
    uint32_t consecutiveCount = 0;
};

struct RecoveryResult {
    GPURecoveryAction action = GPURecoveryAction::Retry;
    bool recovered = false;
    std::string newApiName;
    uint32_t recoveryTimeMs = 0;
};

struct GPURecoveryStats {
    uint32_t totalErrors = 0;
    uint32_t successfulRecoveries = 0;
    uint32_t fallbacksTriggered = 0;
    uint32_t permanentDisables = 0;
    GPUErrorType lastError = GPUErrorType::None;
};

class GPUErrorRecovery {
public:
    using FallbackCallback = std::function<void(GPURecoveryAction)>;

    GPURecoveryAction DetermineAction(const GPUErrorEvent& error) const {
        if (error.consecutiveCount >= 3)
            return GPURecoveryAction::DisableGPU;
        switch (error.type) {
        case GPUErrorType::DeviceRemoved:
        case GPUErrorType::DriverCrash:
            return GPURecoveryAction::ResetDevice;
        case GPUErrorType::DeviceHung:
            return error.consecutiveCount >= 2
                ? GPURecoveryAction::FallbackAPI
                : GPURecoveryAction::ResetDevice;
        case GPUErrorType::OutOfMemory:
            return GPURecoveryAction::ReduceLoad;
        case GPUErrorType::ShaderCompileFail:
            return GPURecoveryAction::FallbackAPI;
        case GPUErrorType::PresentFail:
            return GPURecoveryAction::Retry;
        default:
            return GPURecoveryAction::Retry;
        }
    }

    void OnFallback(FallbackCallback cb) { m_fallbackCb = std::move(cb); }

    void RecordError(const GPUErrorEvent& error) {
        m_stats.totalErrors++;
        m_stats.lastError = error.type;
        m_history.push_back(error);
        if (m_history.size() > 100) m_history.erase(m_history.begin());
    }

    void RecordRecovery(bool success) {
        if (success) m_stats.successfulRecoveries++;
    }

    GPURecoveryStats GetStats() const { return m_stats; }

private:
    FallbackCallback m_fallbackCb;
    GPURecoveryStats m_stats;
    std::vector<GPUErrorEvent> m_history;
};

} // namespace Engine
} // namespace ExplorerLens
