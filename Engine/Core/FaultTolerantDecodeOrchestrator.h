// FaultTolerantDecodeOrchestrator.h — Fault-Tolerant Decode Pipeline Orchestrator
// Copyright (c) 2026 ExplorerLens Project
//
// Orchestrates decode operations with automatic fault recovery, fallback decoder
// selection, and graceful degradation. Tracks per-decoder reliability and routes
// future requests away from unstable decoders. Singleton lifecycle.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DecodeOutcome : uint8_t {
    Success,
    PartialSuccess,
    RecoveredViaFallback,
    TimedOut,
    CrashedAndRecovered,
    PermanentFailure
};

enum class FallbackStrategy : uint8_t {
    None,
    NextDecoder,
    GenericDecoder,
    CPUOnly,
    ReducedQuality,
    SkipFile
};

struct DecodeAttemptRecord {
    std::wstring decoderName;
    DecodeOutcome outcome = DecodeOutcome::Success;
    float durationMs = 0.0f;
    uint32_t attemptNumber = 1;
    FallbackStrategy fallbackUsed = FallbackStrategy::None;
};

struct DecoderReliability {
    std::wstring decoderName;
    uint64_t totalAttempts = 0;
    uint64_t successCount = 0;
    uint64_t fallbackCount = 0;
    uint64_t failureCount = 0;
    float reliabilityScore = 1.0f;
};

struct FaultToleranceStats {
    uint64_t totalDecodes = 0;
    uint64_t successfulDecodes = 0;
    uint64_t fallbackRecoveries = 0;
    uint64_t permanentFailures = 0;
    float overallReliability = 1.0f;
    bool initialized = false;
};

class FaultTolerantDecodeOrchestrator {
public:
    static FaultTolerantDecodeOrchestrator& Instance() {
        static FaultTolerantDecodeOrchestrator instance;
        return instance;
    }

    void Initialize(uint32_t maxRetries = 3) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxRetries = maxRetries;
        m_decoderReliability.clear();
        m_stats = {};
        m_stats.initialized = true;
    }

    DecodeAttemptRecord RecordAttempt(const std::wstring& decoderName,
                                      DecodeOutcome outcome,
                                      float durationMs) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.totalDecodes++;

        auto& rel = m_decoderReliability[decoderName];
        rel.decoderName = decoderName;
        rel.totalAttempts++;

        DecodeAttemptRecord record;
        record.decoderName = decoderName;
        record.outcome = outcome;
        record.durationMs = durationMs;

        if (outcome == DecodeOutcome::Success || outcome == DecodeOutcome::PartialSuccess) {
            rel.successCount++;
            m_stats.successfulDecodes++;
        } else if (outcome == DecodeOutcome::RecoveredViaFallback) {
            rel.fallbackCount++;
            m_stats.fallbackRecoveries++;
            record.fallbackUsed = FallbackStrategy::NextDecoder;
        } else if (outcome == DecodeOutcome::PermanentFailure) {
            rel.failureCount++;
            m_stats.permanentFailures++;
        }

        if (rel.totalAttempts > 0) {
            rel.reliabilityScore = static_cast<float>(rel.successCount) /
                                   static_cast<float>(rel.totalAttempts);
        }

        if (m_stats.totalDecodes > 0) {
            m_stats.overallReliability =
                static_cast<float>(m_stats.successfulDecodes + m_stats.fallbackRecoveries) /
                static_cast<float>(m_stats.totalDecodes);
        }

        return record;
    }

    FallbackStrategy GetRecommendedFallback(const std::wstring& decoderName) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_decoderReliability.find(decoderName);
        if (it == m_decoderReliability.end()) return FallbackStrategy::None;

        if (it->second.reliabilityScore < 0.3f) return FallbackStrategy::SkipFile;
        if (it->second.reliabilityScore < 0.5f) return FallbackStrategy::GenericDecoder;
        if (it->second.reliabilityScore < 0.7f) return FallbackStrategy::CPUOnly;
        if (it->second.reliabilityScore < 0.9f) return FallbackStrategy::NextDecoder;
        return FallbackStrategy::None;
    }

    DecoderReliability GetDecoderReliability(const std::wstring& decoderName) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_decoderReliability.find(decoderName);
        if (it != m_decoderReliability.end()) return it->second;
        return {};
    }

    bool IsInitialized() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats.initialized;
    }

    FaultToleranceStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    void Shutdown() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.initialized = false;
        m_decoderReliability.clear();
    }

private:
    FaultTolerantDecodeOrchestrator() = default;
    ~FaultTolerantDecodeOrchestrator() = default;
    FaultTolerantDecodeOrchestrator(const FaultTolerantDecodeOrchestrator&) = delete;
    FaultTolerantDecodeOrchestrator& operator=(const FaultTolerantDecodeOrchestrator&) = delete;

    mutable std::mutex m_mutex;
    uint32_t m_maxRetries = 3;
    std::unordered_map<std::wstring, DecoderReliability> m_decoderReliability;
    FaultToleranceStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
