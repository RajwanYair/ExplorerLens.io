// AdaptiveFidelitySelector.cpp — Adaptive Decode-Fidelity Selector
// Copyright (c) 2026 ExplorerLens Project
//
#include "AdaptiveFidelitySelector.h"

namespace ExplorerLens { namespace Engine {

AdaptiveFidelitySelector::AdaptiveFidelitySelector(const Config& cfg)
    : m_config(cfg)
{}

FidelityHint AdaptiveFidelitySelector::Select(
    const NetworkBandwidthEstimate& bw, bool gpuAvailable) const
{
    FidelityHint hint = FidelityHint::BALANCED;

    // Tight time budget
    if (m_config.timeBudgetMs < 50) {
        hint = FidelityHint::PLACEHOLDER_ONLY;
    } else if (m_config.timeBudgetMs < 150) {
        hint = FidelityHint::FAST;
    } else if (bw.bytesPerSecond == 0) {
        // Offline — fall back to fast local decode
        hint = FidelityHint::FAST;
    } else if (bw.bytesPerSecond >= m_config.minBandwidthForQuality
               && (!bw.isMetered || !m_config.degradeOnMetered)) {
        hint = (gpuAvailable && m_config.preferQualityWhenGPU)
             ? FidelityHint::QUALITY
             : FidelityHint::BALANCED;
    } else if (bw.bytesPerSecond >= m_config.minBandwidthForBalance) {
        hint = FidelityHint::BALANCED;
    } else {
        hint = FidelityHint::FAST;
    }

    // Degrade on metered network
    if (bw.isMetered && m_config.degradeOnMetered) {
        if (hint == FidelityHint::QUALITY) hint = FidelityHint::BALANCED;
    }

    m_lastHint = hint;
    return hint;
}

void AdaptiveFidelitySelector::UpdateBandwidth(const NetworkBandwidthEstimate& sample)
{
    // Simple exponential moving average (alpha=0.25)
    constexpr uint64_t ALPHA_NUM = 1, ALPHA_DEN = 4;
    if (m_rollingBw.bytesPerSecond == 0) {
        m_rollingBw = sample;
    } else {
        m_rollingBw.bytesPerSecond =
            (m_rollingBw.bytesPerSecond * (ALPHA_DEN - ALPHA_NUM)
             + sample.bytesPerSecond * ALPHA_NUM) / ALPHA_DEN;
        m_rollingBw.isMetered    = sample.isMetered;
        m_rollingBw.isBehindProxy= sample.isBehindProxy;
    }
}

void AdaptiveFidelitySelector::SetTimeBudgetMs(uint32_t ms)
{
    m_config.timeBudgetMs = ms;
}

void AdaptiveFidelitySelector::Reset()
{
    m_rollingBw = {};
    m_lastHint  = FidelityHint::BALANCED;
}

const AdaptiveFidelitySelector::Config& AdaptiveFidelitySelector::GetConfig() const
{
    return m_config;
}

FidelityHint AdaptiveFidelitySelector::LastHint() const
{
    return m_lastHint;
}

const char* AdaptiveFidelitySelector::FidelityLabel(FidelityHint hint)
{
    switch (hint) {
        case FidelityHint::PLACEHOLDER_ONLY: return "PlaceholderOnly";
        case FidelityHint::FAST:             return "Fast";
        case FidelityHint::BALANCED:         return "Balanced";
        case FidelityHint::QUALITY:          return "Quality";
        default:                             return "Unknown";
    }
}

}} // namespace ExplorerLens::Engine
