// AdaptiveFidelitySelector.h — Adaptive Decode-Fidelity Selector
// Copyright (c) 2026 ExplorerLens Project
//
// Chooses the optimal decode fidelity (placeholder / fast / balanced / quality)
// for the current network bandwidth, GPU availability, and per-thumbnail time budget.
// The selector feeds directly into MultiStageThumbnailEmitter.Config to gate which
// progressive stages are emitted.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens { namespace Engine {

/// Target decode fidelity.
enum class FidelityHint : uint8_t {
    PLACEHOLDER_ONLY = 0,  ///< Too constrained — emit a grey square only
    FAST             = 1,  ///< 256×256 single-pass, skip colour management
    BALANCED         = 2,  ///< 512×512 with basic colour management
    QUALITY          = 3,  ///< Full-res with GPU colour pipeline
};

/// Live network bandwidth estimate.
struct NetworkBandwidthEstimate {
    uint64_t bytesPerSecond = 0;        ///< 0 = offline / unknown
    bool     isMetered      = false;
    bool     isBehindProxy  = false;
};

/// Adaptive fidelity selector.
class AdaptiveFidelitySelector {
public:
    struct Config {
        uint32_t timeBudgetMs           = 500;   ///< Per-thumbnail time budget
        uint64_t minBandwidthForQuality = 10 * 1024 * 1024; ///< 10 MB/s threshold
        uint64_t minBandwidthForBalance = 1 * 1024 * 1024;  ///<  1 MB/s threshold
        bool     preferQualityWhenGPU   = true;
        bool     degradeOnMetered       = true;  ///< Clamp to FAST on metered networks
    };

    explicit AdaptiveFidelitySelector(const Config& cfg = {});

    /// Select fidelity based on runtime conditions.
    FidelityHint Select(const NetworkBandwidthEstimate& bw, bool gpuAvailable) const;

    /// Update the persisted bandwidth estimate (rolling average).
    void UpdateBandwidth(const NetworkBandwidthEstimate& sample);

    /// Override time budget at runtime (e.g. when Explorer tab is not visible).
    void SetTimeBudgetMs(uint32_t ms);

    /// Reset rolling average; reverts to Config defaults.
    void Reset();

    const Config& GetConfig() const;
    FidelityHint  LastHint()  const;

    static const char* FidelityLabel(FidelityHint hint);

private:
    Config            m_config;
    FidelityHint      m_lastHint    = FidelityHint::BALANCED;
    NetworkBandwidthEstimate m_rollingBw;
};

}} // namespace ExplorerLens::Engine
