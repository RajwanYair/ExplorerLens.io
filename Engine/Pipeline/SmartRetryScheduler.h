// SmartRetryScheduler.h — Intelligent Decode Retry Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Implements exponential backoff retry logic for failed thumbnail decodes
// with circuit breaker patterns and format-specific retry strategies.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class RetryStrategy : uint8_t {
    None,
    Immediate,
    LinearBackoff,
    ExponentialBackoff,
    CircuitBreaker
};

enum class FailureCategory : uint8_t {
    Transient,
    Corrupt,
    Unsupported,
    ResourceExhausted,
    Timeout,
    Unknown
};

struct SmartRetryPolicy {
    RetryStrategy strategy = RetryStrategy::ExponentialBackoff;
    uint32_t maxRetries = 3;
    uint32_t baseDelayMs = 100;
    float backoffMultiplier = 2.0f;
    uint32_t maxDelayMs = 5000;
};

struct RetryRecord {
    std::wstring filePath;
    FailureCategory lastFailure = FailureCategory::Unknown;
    uint32_t attemptCount = 0;
    uint32_t nextDelayMs = 0;
    bool circuitOpen = false;
    uint64_t lastAttemptTimestamp = 0;
};

struct RetryStats {
    uint64_t totalRetries = 0;
    uint64_t successfulRetries = 0;
    uint64_t permanentFailures = 0;
    uint64_t circuitBreaks = 0;
};

class SmartRetryScheduler {
public:
    SmartRetryScheduler() = default;

    bool ShouldRetry(const std::wstring& filePath, FailureCategory failure) {
        auto& record = m_records[filePath];
        record.filePath = filePath;
        record.lastFailure = failure;
        record.attemptCount++;

        if (failure == FailureCategory::Corrupt || failure == FailureCategory::Unsupported) {
            record.circuitOpen = true;
            m_stats.permanentFailures++;
            return false;
        }

        if (record.attemptCount > m_policy.maxRetries) {
            record.circuitOpen = true;
            m_stats.circuitBreaks++;
            return false;
        }

        record.nextDelayMs = CalculateDelay(record.attemptCount);
        m_stats.totalRetries++;
        return true;
    }

    void RecordSuccess(const std::wstring& filePath) {
        m_records.erase(filePath);
        m_stats.successfulRetries++;
    }

    RetryRecord GetRecord(const std::wstring& filePath) const {
        auto it = m_records.find(filePath);
        return it != m_records.end() ? it->second : RetryRecord{};
    }

    void SetPolicy(const SmartRetryPolicy& policy) { m_policy = policy; }
    SmartRetryPolicy GetPolicy() const { return m_policy; }
    RetryStats GetStats() const { return m_stats; }
    size_t GetPendingRetries() const { return m_records.size(); }
    void ClearRecords() { m_records.clear(); }

private:
    uint32_t CalculateDelay(uint32_t attempt) const {
        if (m_policy.strategy == RetryStrategy::LinearBackoff)
            return m_policy.baseDelayMs * attempt;
        uint32_t delay = m_policy.baseDelayMs;
        for (uint32_t i = 1; i < attempt; i++)
            delay = static_cast<uint32_t>(delay * m_policy.backoffMultiplier);
        return delay < m_policy.maxDelayMs ? delay : m_policy.maxDelayMs;
    }

    std::unordered_map<std::wstring, RetryRecord> m_records;
    SmartRetryPolicy m_policy;
    RetryStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
