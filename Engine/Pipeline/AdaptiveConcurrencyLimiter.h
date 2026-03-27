// AdaptiveConcurrencyLimiter.h — Adaptive Concurrency Limiter (AIMD)
// Copyright (c) 2026 ExplorerLens Project
//
// Additive-increase/multiplicative-decrease concurrency window to prevent overload during bursty decode workloads.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct AIMDConfig {
    uint32_t initialWindow  = 4;
    uint32_t maxWindow      = 64;
    uint32_t minWindow      = 1;
    double   increaseFactor = 1.0;    // additive increase per success
    double   decreaseFactor = 0.5;    // multiplicative decrease on overload
};
class AdaptiveConcurrencyLimiter {
public:
    explicit AdaptiveConcurrencyLimiter(AIMDConfig cfg = {}) : m_cfg(cfg), m_window(cfg.initialWindow) {}
    bool  TryAcquire()    { if (m_inflight >= m_window) return false; m_inflight++; return true; }
    void  Release(bool success) {
        m_inflight = m_inflight > 0 ? m_inflight - 1 : 0;
        if (success) m_window = std::min<uint32_t>(m_window + 1, m_cfg.maxWindow);
        else         m_window = std::max<uint32_t>(static_cast<uint32_t>(m_window * m_cfg.decreaseFactor), m_cfg.minWindow);
    }
    uint32_t Window()   const { return m_window; }
    uint32_t Inflight() const { return m_inflight; }
private:
    AIMDConfig            m_cfg;
    std::atomic<uint32_t> m_window;
    std::atomic<uint32_t> m_inflight{0};
};

} // namespace Engine
} // namespace ExplorerLens