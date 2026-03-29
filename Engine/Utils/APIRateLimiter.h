// APIRateLimiter.h — API Rate Limiter (Token Bucket + Sliding Window)
// Copyright (c) 2026 ExplorerLens Project
//
// Implements token-bucket and sliding-window rate limiting for the REST/gRPC API
// surface, protecting the engine from request floods and ensuring fair resource sharing.
//
#pragma once
#include <string>
#include <unordered_map>
#include <chrono>
#include <atomic>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

enum class RateLimitAlgorithm { TokenBucket, SlidingWindow, FixedWindow };
enum class RateLimitDecision  { Allow, Throttle, Block };

struct RateLimitPolicy {
    int                  requestsPerWindow = 100;
    std::chrono::seconds windowDuration    = std::chrono::seconds{60};
    int                  burstAllowance    = 20;
    RateLimitAlgorithm   algorithm         = RateLimitAlgorithm::TokenBucket;
};

struct RateLimitState {
    int     tokens            = 0;
    int64_t windowStartMs     = 0;
    int     windowCount       = 0;
};

struct APIRateLimitResult {
    RateLimitDecision decision    = RateLimitDecision::Allow;
    int               remaining   = 0;
    int64_t           retryAfterMs = 0;
    bool IsAllowed() const noexcept { return decision == RateLimitDecision::Allow; }
};

class APIRateLimiter {
public:
    explicit APIRateLimiter(RateLimitPolicy policy = {}) : m_policy(std::move(policy)) {
        m_defaultTokens = m_policy.requestsPerWindow + m_policy.burstAllowance;
    }

    APIRateLimitResult Check(const std::string& clientKey) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

        auto& state = m_states[clientKey];
        const int64_t windowMs = static_cast<int64_t>(m_policy.windowDuration.count()) * 1000;

        if (m_policy.algorithm == RateLimitAlgorithm::TokenBucket) {
            if (state.tokens == 0 && state.windowStartMs == 0) {
                state.tokens         = m_defaultTokens;
                state.windowStartMs  = now;
            }
            // Refill tokens if window elapsed
            if (now - state.windowStartMs >= windowMs) {
                state.tokens        = m_defaultTokens;
                state.windowStartMs = now;
            }
            if (state.tokens > 0) {
                state.tokens--;
                return { RateLimitDecision::Allow, state.tokens, 0 };
            }
            int64_t retry = windowMs - (now - state.windowStartMs);
            return { RateLimitDecision::Throttle, 0, retry };
        }
        // SlidingWindow / FixedWindow fallback
        if (now - state.windowStartMs >= windowMs) {
            state.windowStartMs = now;
            state.windowCount   = 0;
        }
        state.windowCount++;
        if (state.windowCount <= m_policy.requestsPerWindow)
            return { RateLimitDecision::Allow, m_policy.requestsPerWindow - state.windowCount, 0 };

        int64_t retry = windowMs - (now - state.windowStartMs);
        return { RateLimitDecision::Throttle, 0, retry };
    }

    void ResetClient(const std::string& clientKey) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_states.erase(clientKey);
    }

    const RateLimitPolicy& Policy() const noexcept { return m_policy; }
    size_t TrackedClients() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_states.size();
    }

private:
    RateLimitPolicy   m_policy;
    int               m_defaultTokens = 0;
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, RateLimitState> m_states;
};

} // namespace Engine
} // namespace ExplorerLens
