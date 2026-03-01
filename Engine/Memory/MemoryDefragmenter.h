#pragma once
// MemoryDefragmenter.h — Defragment memory pool to reduce fragmentation
// Sprint 415 · Batch 6 · ExplorerLens v15.0.0

#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// ── Constants ────────────────────────────────────────────────────────────────

static constexpr float DEFRAG_THRESHOLD_PERCENT = 30.0f;

// ── Enums ────────────────────────────────────────────────────────────────────

enum class FragmentationLevel : uint8_t {
    None = 0,
    Low = 1,
    Moderate = 2,
    High = 3,
    Critical = 4
};

inline const char* FragmentationLevelName(FragmentationLevel l) {
    switch (l) {
    case FragmentationLevel::None:     return "None";
    case FragmentationLevel::Low:      return "Low";
    case FragmentationLevel::Moderate: return "Moderate";
    case FragmentationLevel::High:     return "High";
    case FragmentationLevel::Critical: return "Critical";
    default:                           return "Unknown";
    }
}

enum class DefragStrategy : uint8_t {
    Compact = 0,
    Coalesce = 1,
    Relocate = 2,
    FullSweep = 3,
    Incremental = 4
};

inline const char* DefragStrategyName(DefragStrategy s) {
    switch (s) {
    case DefragStrategy::Compact:     return "Compact";
    case DefragStrategy::Coalesce:    return "Coalesce";
    case DefragStrategy::Relocate:    return "Relocate";
    case DefragStrategy::FullSweep:   return "FullSweep";
    case DefragStrategy::Incremental: return "Incremental";
    default:                          return "Unknown";
    }
}

// ── Structs ──────────────────────────────────────────────────────────────────

struct DefragResult {
    DefragStrategy strategy = DefragStrategy::Compact;
    float          beforeFragPercent = 0.0f;
    float          afterFragPercent = 0.0f;
    uint64_t       bytesReclaimed = 0;
    double         durationMs = 0.0;
};

// ── Class ────────────────────────────────────────────────────────────────────

class MemoryDefragmenter {
public:
    MemoryDefragmenter() = default;
    ~MemoryDefragmenter() = default;

    // Analyze current memory fragmentation level
    FragmentationLevel Analyze(float fragmentationPercent) {
        m_currentFragPercent = fragmentationPercent;
        if (fragmentationPercent <= 5.0f)
            m_level = FragmentationLevel::None;
        else if (fragmentationPercent <= 15.0f)
            m_level = FragmentationLevel::Low;
        else if (fragmentationPercent <= DEFRAG_THRESHOLD_PERCENT)
            m_level = FragmentationLevel::Moderate;
        else if (fragmentationPercent <= 60.0f)
            m_level = FragmentationLevel::High;
        else
            m_level = FragmentationLevel::Critical;
        m_analysisCount++;
        return m_level;
    }

    // Perform defragmentation with the specified strategy
    DefragResult Defragment(DefragStrategy strategy = DefragStrategy::Compact) {
        DefragResult result;
        result.strategy = strategy;
        result.beforeFragPercent = m_currentFragPercent;

        // Simulate defrag effectiveness by strategy
        float reduction = 0.0f;
        switch (strategy) {
        case DefragStrategy::Compact:     reduction = 0.6f;  break;
        case DefragStrategy::Coalesce:    reduction = 0.4f;  break;
        case DefragStrategy::Relocate:    reduction = 0.5f;  break;
        case DefragStrategy::FullSweep:   reduction = 0.85f; break;
        case DefragStrategy::Incremental: reduction = 0.2f;  break;
        }
        result.afterFragPercent = m_currentFragPercent * (1.0f - reduction);
        result.bytesReclaimed = static_cast<uint64_t>(
            m_currentFragPercent * reduction * 1024.0f * 1024.0f);
        result.durationMs = (strategy == DefragStrategy::FullSweep) ? 50.0 : 10.0;

        m_currentFragPercent = result.afterFragPercent;
        m_totalBytesReclaimed += result.bytesReclaimed;
        m_defragCount++;

        // Re-analyze after defrag
        Analyze(m_currentFragPercent);
        m_lastResult = result;
        return result;
    }

    FragmentationLevel GetFragmentationLevel() const { return m_level; }
    float GetCurrentFragPercent() const { return m_currentFragPercent; }
    uint64_t GetTotalBytesReclaimed() const { return m_totalBytesReclaimed; }
    uint32_t GetDefragCount() const { return m_defragCount; }
    uint32_t GetAnalysisCount() const { return m_analysisCount; }

    bool NeedsDefrag() const {
        return m_currentFragPercent > DEFRAG_THRESHOLD_PERCENT;
    }

    const DefragResult& GetLastResult() const { return m_lastResult; }

    void Reset() {
        m_level = FragmentationLevel::None;
        m_currentFragPercent = 0.0f;
        m_totalBytesReclaimed = 0;
        m_defragCount = 0;
        m_analysisCount = 0;
        m_lastResult = DefragResult{};
    }

private:
    FragmentationLevel m_level = FragmentationLevel::None;
    float              m_currentFragPercent = 0.0f;
    uint64_t           m_totalBytesReclaimed = 0;
    uint32_t           m_defragCount = 0;
    uint32_t           m_analysisCount = 0;
    DefragResult       m_lastResult;
};

} // namespace Engine
} // namespace ExplorerLens
