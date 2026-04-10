// BandwidthThrottleGuard.h — Token-Bucket Bandwidth Rate Limiter
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces a per-connection bandwidth cap on remote thumbnail fetch operations
// using a token-bucket algorithm. Fed externally with elapsed-time ticks.
//
#pragma once
#include <cstdint>

namespace ExplorerLens { namespace Engine {

class BandwidthThrottleGuard {
public:
    struct Config {
        uint32_t maxKbps       = 4096;  // 0 = unlimited
        uint32_t bucketCapKb   = 256;   // Token bucket capacity
        uint32_t burstKb       = 64;    // Max burst above nominal
    };

    explicit BandwidthThrottleGuard(const Config& cfg = {});

    // Attempt to consume `bytes`. Returns true if permitted (tokens available).
    // `nowMs` must be monotonically increasing.
    bool TryConsume(uint32_t bytes, uint64_t nowMs);

    // Refill the bucket based on elapsed wall time since last refill.
    void Tick(uint64_t nowMs);

    uint32_t AvailableTokensKb()  const;
    uint64_t TotalBytesAllowed()  const { return m_totalAllowed; }
    uint64_t TotalBytesRejected() const { return m_totalRejected; }

    void SetMaxKbps(uint32_t kbps);
    void Reset();

    const Config& GetConfig() const { return m_cfg; }

private:
    Config   m_cfg;
    double   m_tokensKb      = 0.0;
    uint64_t m_lastTickMs    = 0;
    uint64_t m_totalAllowed  = 0;
    uint64_t m_totalRejected = 0;
};

}} // namespace ExplorerLens::Engine
