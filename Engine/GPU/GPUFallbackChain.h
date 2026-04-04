// GPUFallbackChain.h — Ordered GPU Backend Fallback Chain
// Copyright (c) 2026 ExplorerLens Project
//
// Manages an ordered chain of GPU rendering backends (DX12 -> DX11 ->
// Vulkan -> CPU) and selects the best available one for each request.
// Backends that accumulate consecutive failures are temporarily demoted
// and skip-listed; after a cooldown they are automatically re-promoted.
// CPU is always the last-resort fallback that cannot be demoted.
//
// Thread-safe singleton.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <array>
#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Backends listed in preference order; first available wins.
enum class GPUBackendId : uint32_t {
    DX12 = 0,
    DX11 = 1,
    Vulkan = 2,
    CPU = 3,
    Count = 4,
    None = 0xFFFFFFFF
};

static const wchar_t* GPUBackendName(GPUBackendId id)
{
    static const wchar_t* names[] = {L"DirectX 12", L"DirectX 11", L"Vulkan", L"CPU"};
    auto idx = static_cast<uint32_t>(id);
    return (idx < static_cast<uint32_t>(GPUBackendId::Count)) ? names[idx] : L"Unknown";
}

struct GPUBackendStatus
{
    GPUBackendId id = GPUBackendId::None;
    bool available = false;
    bool demoted = false;  // Temporarily demoted due to failures
    uint32_t failCount = 0;
    uint64_t lastFailure = 0;
    uint64_t lastSuccess = 0;
    double avgLatencyMs = 0.0;
};

struct GPUFallbackConfig
{
    uint32_t maxConsecutiveFailures = 3;  // Failures before demotion
    uint32_t demotionCooldownMs = 60000;  // 60s cooldown
    bool enableDX12 = true;
    bool enableDX11 = true;
    bool enableVulkan = true;
    bool alwaysFallbackCPU = true;  // CPU always available as last resort
};

struct GPUBackendSelection
{
    GPUBackendId selectedBackend = GPUBackendId::CPU;
    uint32_t attemptIndex = 0;  // Which position in chain (0=primary)
    bool isFallback = false;    // True if not the first choice
    const wchar_t* reason = L"";
};

struct GPUFallbackStats
{
    uint64_t totalSelections = 0;
    uint64_t fallbackSelections = 0;
    uint64_t demotionEvents = 0;
    uint64_t promotionEvents = 0;
    uint64_t backendSuccesses[4] = {};
    uint64_t backendFailures[4] = {};
};

// ========================================================================
// GPUFallbackChain — Ordered GPU backend selection with demotion
// ========================================================================
class GPUFallbackChain
{
  public:
    static GPUFallbackChain& Instance()
    {
        static GPUFallbackChain instance;
        return instance;
    }

    void Initialize(const GPUFallbackConfig& config = {})
    {
        m_config = config;
        m_stats = {};

        // Build default chain order
        m_chain.clear();
        if (config.enableDX12)
            m_chain.push_back(GPUBackendId::DX12);
        if (config.enableDX11)
            m_chain.push_back(GPUBackendId::DX11);
        if (config.enableVulkan)
            m_chain.push_back(GPUBackendId::Vulkan);
        if (config.alwaysFallbackCPU)
            m_chain.push_back(GPUBackendId::CPU);

        // Initialize backend statuses
        for (auto& s : m_backendStatus) {
            s = {};
        }
        for (auto id : m_chain) {
            auto idx = static_cast<uint32_t>(id);
            m_backendStatus[idx].id = id;
            m_backendStatus[idx].available = true;
        }

        m_initialized = true;
    }

    bool IsInitialized() const
    {
        return m_initialized;
    }

    // Select the best available backend
    GPUBackendSelection Select()
    {
        GPUBackendSelection sel;
        m_stats.totalSelections++;
        uint64_t now = GetTickCount64();

        for (uint32_t i = 0; i < static_cast<uint32_t>(m_chain.size()); ++i) {
            auto id = m_chain[i];
            auto& status = m_backendStatus[static_cast<uint32_t>(id)];

            if (!status.available)
                continue;

            // Check demotion cooldown
            if (status.demoted) {
                if (now - status.lastFailure >= m_config.demotionCooldownMs) {
                    status.demoted = false;
                    status.failCount = 0;
                    m_stats.promotionEvents++;
                } else {
                    continue;
                }
            }

            sel.selectedBackend = id;
            sel.attemptIndex = i;
            sel.isFallback = (i > 0);
            sel.reason = (i == 0) ? L"Primary" : L"Fallback";

            if (sel.isFallback)
                m_stats.fallbackSelections++;
            return sel;
        }

        // Ultimate fallback: CPU
        sel.selectedBackend = GPUBackendId::CPU;
        sel.isFallback = true;
        sel.reason = L"All GPU backends unavailable";
        m_stats.fallbackSelections++;
        return sel;
    }

    // Record success for a backend
    void RecordSuccess(GPUBackendId id)
    {
        auto idx = static_cast<uint32_t>(id);
        if (idx >= static_cast<uint32_t>(GPUBackendId::Count))
            return;
        m_backendStatus[idx].lastSuccess = GetTickCount64();
        m_backendStatus[idx].failCount = 0;
        m_stats.backendSuccesses[idx]++;
    }

    // Record failure — may trigger demotion
    void RecordFailure(GPUBackendId id)
    {
        auto idx = static_cast<uint32_t>(id);
        if (idx >= static_cast<uint32_t>(GPUBackendId::Count))
            return;
        auto& status = m_backendStatus[idx];
        status.failCount++;
        status.lastFailure = GetTickCount64();
        m_stats.backendFailures[idx]++;

        if (status.failCount >= m_config.maxConsecutiveFailures && id != GPUBackendId::CPU) {
            status.demoted = true;
            m_stats.demotionEvents++;
        }
    }

    // Force enable/disable a backend
    void SetBackendAvailable(GPUBackendId id, bool available)
    {
        auto idx = static_cast<uint32_t>(id);
        if (idx < static_cast<uint32_t>(GPUBackendId::Count)) {
            m_backendStatus[idx].available = available;
        }
    }

    // Get backend status
    GPUBackendStatus GetBackendStatus(GPUBackendId id) const
    {
        auto idx = static_cast<uint32_t>(id);
        if (idx < static_cast<uint32_t>(GPUBackendId::Count))
            return m_backendStatus[idx];
        return {};
    }

    // Get chain length
    uint32_t GetChainLength() const
    {
        return static_cast<uint32_t>(m_chain.size());
    }

    // Get stats
    GPUFallbackStats GetStats() const
    {
        return m_stats;
    }

  private:
    GPUFallbackChain() = default;

    GPUFallbackConfig m_config;
    GPUFallbackStats m_stats;
    std::vector<GPUBackendId> m_chain;
    std::array<GPUBackendStatus, static_cast<size_t>(GPUBackendId::Count)> m_backendStatus;
    bool m_initialized = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
