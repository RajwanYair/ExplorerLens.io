// DecoderFaultIsolator.h — Decoder Fault Isolation and Containment
// Copyright (c) 2026 ExplorerLens Project
//
// Isolates decoder faults to prevent cascade failures across the pipeline.
// Tracks per-decoder fault counts, applies quarantine after threshold breaches,
// and provides automatic un-quarantine after cooldown. Singleton lifecycle.
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

enum class FaultSeverity : uint8_t {
    Minor,
    Moderate,
    Major,
    Critical,
    Fatal
};

enum class QuarantineStatus : uint8_t {
    Active,
    Quarantined,
    CooldownPending,
    PermanentlyDisabled
};

struct DecoderFaultRecord
{
    std::wstring decoderName;
    FaultSeverity severity = FaultSeverity::Minor;
    std::wstring faultDescription;
    uint64_t faultTimestamp = 0;
};

struct DecoderIsolationState
{
    std::wstring decoderName;
    QuarantineStatus status = QuarantineStatus::Active;
    uint32_t totalFaults = 0;
    uint32_t consecutiveFaults = 0;
    uint32_t quarantineCount = 0;
    FaultSeverity worstSeverity = FaultSeverity::Minor;
};

struct FaultIsolationStats
{
    uint64_t totalFaults = 0;
    uint64_t quarantineEvents = 0;
    uint64_t decodersCurrentlyQuarantined = 0;
    uint64_t permanentlyDisabled = 0;
    bool initialized = false;
};

class DecoderFaultIsolator
{
  public:
    static DecoderFaultIsolator& Instance()
    {
        static DecoderFaultIsolator instance;
        return instance;
    }

    void Initialize(uint32_t faultThreshold = 5, uint32_t maxQuarantines = 3)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_faultThreshold = faultThreshold;
        m_maxQuarantines = maxQuarantines;
        m_decoderStates.clear();
        m_stats = {};
        m_stats.initialized = true;
    }

    void RecordFault(const std::wstring& decoderName, FaultSeverity severity, const std::wstring& description = L"")
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.totalFaults++;

        auto& state = m_decoderStates[decoderName];
        state.decoderName = decoderName;
        state.totalFaults++;
        state.consecutiveFaults++;
        if (severity > state.worstSeverity)
            state.worstSeverity = severity;

        if (severity == FaultSeverity::Fatal
            || (state.consecutiveFaults >= m_faultThreshold && state.status == QuarantineStatus::Active)) {
            if (state.quarantineCount >= m_maxQuarantines) {
                state.status = QuarantineStatus::PermanentlyDisabled;
                m_stats.permanentlyDisabled++;
            } else {
                state.status = QuarantineStatus::Quarantined;
                state.quarantineCount++;
                m_stats.quarantineEvents++;
            }
            UpdateQuarantineCount();
        }

        (void)description;
    }

    void RecordSuccess(const std::wstring& decoderName)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_decoderStates.find(decoderName);
        if (it != m_decoderStates.end()) {
            it->second.consecutiveFaults = 0;
        }
    }

    bool IsQuarantined(const std::wstring& decoderName) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_decoderStates.find(decoderName);
        if (it == m_decoderStates.end())
            return false;
        return it->second.status == QuarantineStatus::Quarantined
               || it->second.status == QuarantineStatus::PermanentlyDisabled;
    }

    void ReleaseFromQuarantine(const std::wstring& decoderName)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_decoderStates.find(decoderName);
        if (it != m_decoderStates.end() && it->second.status == QuarantineStatus::Quarantined) {
            it->second.status = QuarantineStatus::Active;
            it->second.consecutiveFaults = 0;
            UpdateQuarantineCount();
        }
    }

    DecoderIsolationState GetDecoderState(const std::wstring& decoderName) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_decoderStates.find(decoderName);
        if (it != m_decoderStates.end())
            return it->second;
        return {};
    }

    bool IsInitialized() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats.initialized;
    }

    FaultIsolationStats GetStats() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    void Shutdown()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.initialized = false;
        m_decoderStates.clear();
    }

  private:
    DecoderFaultIsolator() = default;
    ~DecoderFaultIsolator() = default;
    DecoderFaultIsolator(const DecoderFaultIsolator&) = delete;
    DecoderFaultIsolator& operator=(const DecoderFaultIsolator&) = delete;

    void UpdateQuarantineCount()
    {
        uint64_t count = 0;
        for (const auto& [name, state] : m_decoderStates) {
            if (state.status == QuarantineStatus::Quarantined || state.status == QuarantineStatus::PermanentlyDisabled) {
                count++;
            }
        }
        m_stats.decodersCurrentlyQuarantined = count;
    }

    mutable std::mutex m_mutex;
    uint32_t m_faultThreshold = 5;
    uint32_t m_maxQuarantines = 3;
    std::unordered_map<std::wstring, DecoderIsolationState> m_decoderStates;
    FaultIsolationStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
