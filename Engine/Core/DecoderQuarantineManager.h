// DecoderQuarantineManager.h — Decoder Quarantine State Machine
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks cumulative crash counts per decoder and advances the quarantine stage
// ladder: Active → RetryOnly → SoftRestart → Bypassed. Sufficient successful
// decodes demote a decoder back to Active.
//
#pragma once
#include <string>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class QuarantineStage : uint8_t {
    Active = 0,       // Normal operation
    RetryOnly = 1,    // Second-chance retries only
    SoftRestart = 2,  // Restart decoder between requests
    Bypassed = 3,     // Completely bypassed
};

struct QuarantineRecord
{
    QuarantineStage stage = QuarantineStage::Active;
    int crashCount = 0;
    int successRun = 0;

    std::string StageName() const noexcept
    {
        switch (stage) {
            case QuarantineStage::Active:
                return "Active";
            case QuarantineStage::RetryOnly:
                return "RetryOnly";
            case QuarantineStage::SoftRestart:
                return "SoftRestart";
            case QuarantineStage::Bypassed:
                return "Bypassed";
        }
        return "Unknown";
    }
};

class DecoderQuarantineManager
{
  public:
    static constexpr int RECOVERY_RELEASE_COUNT = 5;

    bool IsQuarantined(const std::string& decoder) const
    {
        auto it = m_records.find(decoder);
        if (it == m_records.end())
            return false;
        return it->second.stage != QuarantineStage::Active;
    }

    void RecordCrash(const std::string& decoder)
    {
        auto& r = m_records[decoder];
        r.crashCount++;
        r.successRun = 0;

        if (r.crashCount >= 10) {
            r.stage = QuarantineStage::Bypassed;
        } else if (r.crashCount >= 5) {
            r.stage = QuarantineStage::SoftRestart;
        } else if (r.crashCount >= 2) {
            r.stage = QuarantineStage::RetryOnly;
        }
    }

    void RecordSuccess(const std::string& decoder)
    {
        auto it = m_records.find(decoder);
        if (it == m_records.end())
            return;
        auto& r = it->second;
        if (r.stage == QuarantineStage::Active)
            return;
        r.successRun++;
        if (r.successRun >= RECOVERY_RELEASE_COUNT) {
            r.stage = QuarantineStage::Active;
            r.successRun = 0;
        }
    }

    const QuarantineRecord* GetRecord(const std::string& decoder) const
    {
        auto it = m_records.find(decoder);
        if (it == m_records.end())
            return nullptr;
        return &it->second;
    }

  private:
    std::unordered_map<std::string, QuarantineRecord> m_records;
};

}  // namespace Engine
}  // namespace ExplorerLens
