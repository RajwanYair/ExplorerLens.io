// DecoderQuarantineManager.h — Automatic Decoder Quarantine & Recovery Orchestrator
// Copyright (c) 2026 ExplorerLens Project
//
// Quarantines misbehaving decoders after crash threshold breach and orchestrates
// staged recovery (retry-only → soft-restart → full-bypass) with automatic graduation.
//
#pragma once
#include <string>
#include <unordered_map>
#include <chrono>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class QuarantineStage {
    Active,         // Normal operation
    RetryOnly,      // Restricted: retries with backoff
    SoftRestart,    // Decoder re-initialized each call
    Bypassed        // Completely excluded; fallback used
};

struct QuarantineRecord {
    QuarantineStage stage         = QuarantineStage::Active;
    int             crashCount    = 0;
    int             releaseCount  = 0;                    // successful calls since recovery
    std::chrono::steady_clock::time_point quarantineTime{};
    std::chrono::steady_clock::time_point nextReviewTime{};
    bool            isPermanent   = false;

    bool IsQuarantined() const noexcept { return stage != QuarantineStage::Active; }
    std::string StageName() const noexcept {
        switch (stage) {
        case QuarantineStage::Active:      return "Active";
        case QuarantineStage::RetryOnly:   return "RetryOnly";
        case QuarantineStage::SoftRestart: return "SoftRestart";
        case QuarantineStage::Bypassed:    return "Bypassed";
        }
        return "Unknown";
    }
};

class DecoderQuarantineManager {
public:
    static constexpr int CRASH_THRESHOLD_1 = 2;
    static constexpr int CRASH_THRESHOLD_2 = 5;
    static constexpr int CRASH_THRESHOLD_3 = 10;
    static constexpr int RECOVERY_RELEASE_COUNT = 20; // successes needed to graduate

    void RecordCrash(const std::string& decoder) {
        auto& r = m_records[decoder];
        r.crashCount++;
        r.quarantineTime = std::chrono::steady_clock::now();
        if      (r.crashCount >= CRASH_THRESHOLD_3) r.stage = QuarantineStage::Bypassed;
        else if (r.crashCount >= CRASH_THRESHOLD_2) r.stage = QuarantineStage::SoftRestart;
        else if (r.crashCount >= CRASH_THRESHOLD_1) r.stage = QuarantineStage::RetryOnly;
        r.releaseCount = 0;
        r.nextReviewTime = std::chrono::steady_clock::now() + std::chrono::seconds(30);
    }

    void RecordSuccess(const std::string& decoder) {
        if (m_records.count(decoder) == 0) return;
        auto& r = m_records[decoder];
        if (!r.IsQuarantined()) return;
        r.releaseCount++;
        if (r.releaseCount >= RECOVERY_RELEASE_COUNT && !r.isPermanent) {
            if      (r.stage == QuarantineStage::SoftRestart) r.stage = QuarantineStage::RetryOnly;
            else if (r.stage == QuarantineStage::RetryOnly)   { r.stage = QuarantineStage::Active; r.crashCount = 0; }
        }
    }

    const QuarantineRecord* GetRecord(const std::string& decoder) const {
        auto it = m_records.find(decoder);
        return it != m_records.end() ? &it->second : nullptr;
    }

    bool IsQuarantined(const std::string& decoder) const {
        auto it = m_records.find(decoder);
        return it != m_records.end() && it->second.IsQuarantined();
    }

    void PermanentBan(const std::string& decoder) {
        m_records[decoder].stage = QuarantineStage::Bypassed;
        m_records[decoder].isPermanent = true;
    }

    std::vector<std::string> GetQuarantinedDecoders() const {
        std::vector<std::string> out;
        for (const auto& kv : m_records)
            if (kv.second.IsQuarantined()) out.push_back(kv.first);
        return out;
    }

    int QuarantinedCount() const noexcept {
        int n = 0;
        for (const auto& kv : m_records) if (kv.second.IsQuarantined()) n++;
        return n;
    }

    void Clear() noexcept { m_records.clear(); }

private:
    std::unordered_map<std::string, QuarantineRecord> m_records;
};

} // namespace Engine
} // namespace ExplorerLens
