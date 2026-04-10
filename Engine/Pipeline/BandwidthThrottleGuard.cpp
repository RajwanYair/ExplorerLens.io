// BandwidthThrottleGuard.cpp — Token-Bucket Bandwidth Rate Limiter
// Copyright (c) 2026 ExplorerLens Project
//
#include "BandwidthThrottleGuard.h"
#include <algorithm>

namespace ExplorerLens { namespace Engine {

BandwidthThrottleGuard::BandwidthThrottleGuard(const Config& cfg)
    : m_cfg(cfg)
    , m_tokensKb(static_cast<double>(cfg.bucketCapKb))
{
}

bool BandwidthThrottleGuard::TryConsume(uint32_t bytes, uint64_t nowMs)
{
    if (m_cfg.maxKbps == 0) {
        m_totalAllowed += bytes;
        return true;
    }
    Tick(nowMs);
    const double requiredKb = static_cast<double>(bytes) / 1024.0;
    if (m_tokensKb >= requiredKb) {
        m_tokensKb    -= requiredKb;
        m_totalAllowed += bytes;
        return true;
    }
    m_totalRejected += bytes;
    return false;
}

void BandwidthThrottleGuard::Tick(uint64_t nowMs)
{
    if (m_lastTickMs == 0) { m_lastTickMs = nowMs; return; }
    if (nowMs <= m_lastTickMs) return;

    const double elapsedSec = static_cast<double>(nowMs - m_lastTickMs) / 1000.0;
    const double addKb      = elapsedSec * static_cast<double>(m_cfg.maxKbps);
    const double cap        = static_cast<double>(m_cfg.bucketCapKb + m_cfg.burstKb);
    m_tokensKb  = (std::min)(m_tokensKb + addKb, cap);
    m_lastTickMs = nowMs;
}

uint32_t BandwidthThrottleGuard::AvailableTokensKb() const
{
    return static_cast<uint32_t>(m_tokensKb);
}

void BandwidthThrottleGuard::SetMaxKbps(uint32_t kbps)
{
    m_cfg.maxKbps = kbps;
}

void BandwidthThrottleGuard::Reset()
{
    m_tokensKb      = static_cast<double>(m_cfg.bucketCapKb);
    m_lastTickMs    = 0;
    m_totalAllowed  = 0;
    m_totalRejected = 0;
}

}} // namespace ExplorerLens::Engine
