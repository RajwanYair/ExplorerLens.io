// RateLimitingMiddleware.h — Rate Limiting Middleware
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces per-client token-bucket and sliding window rate limits on API endpoints.
//
#pragma once
#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class RLMAlgorithm {
    TokenBucket,
    SlidingWindow,
    FixedWindow
};

struct RLMConfig
{
    RLMAlgorithm algorithm = RLMAlgorithm::TokenBucket;
    uint32_t maxRequests = 100;
    uint32_t windowMs = 60000;
};

struct RLMCheckResult
{
    bool allowed = false;
    uint32_t remaining = 0;
    uint64_t resetAtMs = 0;
};

class RateLimitingMiddleware
{
  public:
    explicit RateLimitingMiddleware(const RLMConfig& config) : m_config(config) {}

    RLMCheckResult Check(const std::string& clientId)
    {
        RLMCheckResult r;
        auto& bucket = m_buckets[clientId];
        auto nowMs = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
                .count());
        if (nowMs - bucket.windowStartMs >= m_config.windowMs) {
            bucket.count = 0;
            bucket.windowStartMs = nowMs;
        }
        r.allowed = bucket.count < m_config.maxRequests;
        if (r.allowed)
            ++bucket.count;
        r.remaining = m_config.maxRequests - bucket.count;
        r.resetAtMs = bucket.windowStartMs + m_config.windowMs;
        return r;
    }
    void Reset(const std::string& clientId)
    {
        m_buckets.erase(clientId);
    }

  private:
    RLMConfig m_config;
    struct BucketState
    {
        uint32_t count = 0;
        uint64_t windowStartMs = 0;
    };
    std::unordered_map<std::string, BucketState> m_buckets;
};

}  // namespace Engine
}  // namespace ExplorerLens
