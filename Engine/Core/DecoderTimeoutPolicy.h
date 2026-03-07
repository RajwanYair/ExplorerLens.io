// DecoderTimeoutPolicy.h — Per-Decoder Timeout Configuration
// Copyright (c) 2026 ExplorerLens Project
//
// Configurable timeout policies per decoder type to prevent slow
// decoders from blocking the thumbnail pipeline. Integrates with
// circuit breaker for progressive backoff.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct TimeoutPolicy {
    uint32_t maxDecodeMs = 5000;
    uint32_t maxFileReadMs = 2000;
    uint32_t maxGPUMs = 1000;
    uint32_t backoffMultiplier = 2;
    uint32_t maxRetries = 3;
    bool killOnTimeout = true;
};

struct TimeoutEvent {
    std::wstring decoderName;
    std::wstring filePath;
    uint32_t elapsedMs = 0;
    uint32_t limitMs = 0;
    bool wasKilled = false;
};

class DecoderTimeoutPolicy {
public:
    static DecoderTimeoutPolicy& Instance() {
        static DecoderTimeoutPolicy s;
        return s;
    }

    void SetPolicy(const std::wstring& decoderName, TimeoutPolicy policy) {
        m_policies[decoderName] = policy;
    }

    TimeoutPolicy GetPolicy(const std::wstring& decoderName) const {
        auto it = m_policies.find(decoderName);
        return (it != m_policies.end()) ? it->second : m_defaultPolicy;
    }

    void SetDefaultPolicy(TimeoutPolicy policy) {
        m_defaultPolicy = policy;
    }

    bool IsTimedOut(const std::wstring& decoderName, uint32_t elapsedMs) const {
        auto policy = GetPolicy(decoderName);
        return elapsedMs > policy.maxDecodeMs;
    }

    void RecordTimeout(const TimeoutEvent& evt) {
        m_totalTimeouts++;
        m_timeoutsByDecoder[evt.decoderName]++;
    }

    uint64_t TotalTimeouts() const { return m_totalTimeouts; }
    size_t PolicyCount() const { return m_policies.size(); }

private:
    DecoderTimeoutPolicy() = default;
    TimeoutPolicy m_defaultPolicy;
    std::unordered_map<std::wstring, TimeoutPolicy> m_policies;
    std::unordered_map<std::wstring, uint64_t> m_timeoutsByDecoder;
    uint64_t m_totalTimeouts = 0;
};

} // namespace Engine
} // namespace ExplorerLens
