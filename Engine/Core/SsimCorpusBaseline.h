// Engine/Core/SsimCorpusBaseline.h
#pragma once

// SsimCorpusBaseline — SSIM corpus baseline record management (S365)
//
// Manages per-file SSIM baseline records stored in data/baselines/.
// Used to gate the Phase-3 quality target: SSIM threshold raised from 0.95 to 0.97.
//
// Relation to SSIMValidator.h (existing):
//   SSIMValidator.h — computes SSIM scores for a pair of bitmaps.
//   SsimCorpusBaseline.h — stores/retrieves reference scores per corpus file,
//   compares fresh scores against baselines, and generates regression reports.
//
// ROADMAP ref: Phase 3 exit criterion — "SSIM gate raised to 0.97" + "Corpus 300 files"
//
// Usage:
//   SsimCorpusBaseline& db = SsimCorpusBaseline::Global();
//   db.Configure(SsimBaselineConfig::ForPhase3());
//   db.RecordBaseline(L"test.heic", 0.983);
//   SsimBaselineResult r = db.Evaluate(L"test.heic", freshScore);

#ifndef EXPLORERLENS_ENGINE_SSIMCORPUSBASELINE_H
#define EXPLORERLENS_ENGINE_SSIMCORPUSBASELINE_H

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens::Engine {

/// Minimum SSIM score required for Phase 3 gate (raised from 0.95).
inline constexpr double kSsimPhase3Threshold = 0.97;
/// Minimum SSIM score for legacy Phase 1/2 gate.
inline constexpr double kSsimPhase2Threshold = 0.95;
/// Maximum acceptable regression delta vs stored baseline before CI fails.
inline constexpr double kSsimRegressionTolerance = 0.005;

// ---------------------------------------------------------------------------
// SsimBaselineStatus
// ---------------------------------------------------------------------------
enum class SsimBaselineStatus : std::uint8_t {
    OK                  = 0,
    NOT_FOUND           = 1,  ///< No baseline stored for this key
    BELOW_THRESHOLD     = 2,  ///< Score is below the configured gate threshold
    REGRESSION          = 3,  ///< Score dropped > kSsimRegressionTolerance vs baseline
    INVALID_SCORE       = 4,  ///< Score outside [0.0, 1.0]
    NULL_KEY            = 5,
    IO_ERROR            = 6,  ///< Could not read/write baselines JSON
};

// ---------------------------------------------------------------------------
// SsimBaselineEntry — one per corpus file
// ---------------------------------------------------------------------------
struct SsimBaselineEntry {
    std::wstring   fileKey;           ///< Corpus file path relative to data/corpus/
    double         baselineScore = 0.0;
    double         latestScore   = 0.0;
    double         threshold     = kSsimPhase3Threshold;
    std::uint32_t  sampleCount   = 0u; ///< Number of times score was recorded
    bool           pinned        = false; ///< Pinned entries bypass regression gate

    [[nodiscard]] bool MeetsThreshold() const noexcept { return latestScore >= threshold; }
    [[nodiscard]] double Delta()        const noexcept { return latestScore - baselineScore; }
    [[nodiscard]] bool HasRegressed()   const noexcept {
        return sampleCount > 0u && Delta() < -kSsimRegressionTolerance;
    }
};

// ---------------------------------------------------------------------------
// SsimBaselineReport — summary of a corpus evaluation run
// ---------------------------------------------------------------------------
struct SsimBaselineReport {
    std::uint32_t totalFiles       = 0u;
    std::uint32_t passedThreshold  = 0u;
    std::uint32_t failedThreshold  = 0u;
    std::uint32_t regressions      = 0u;
    std::uint32_t newBaselines     = 0u;   ///< Files with no prior baseline
    double        averageScore     = 0.0;
    double        minimumScore     = 1.0;

    [[nodiscard]] bool IsGreen() const noexcept {
        return failedThreshold == 0u && regressions == 0u;
    }
};

// ---------------------------------------------------------------------------
// SsimBaselineConfig
// ---------------------------------------------------------------------------
struct SsimBaselineConfig {
    double        threshold          = kSsimPhase3Threshold;
    double        regressionTolerance = kSsimRegressionTolerance;
    std::uint32_t maxEntries         = 1024u; ///< Maximum corpus entries tracked
    bool          autoUpdateBaseline = false; ///< Update stored baseline on each pass

    [[nodiscard]] static SsimBaselineConfig ForPhase3() noexcept {
        SsimBaselineConfig cfg{};
        cfg.threshold           = kSsimPhase3Threshold;
        cfg.autoUpdateBaseline  = false;
        return cfg;
    }

    [[nodiscard]] static SsimBaselineConfig ForPhase2() noexcept {
        SsimBaselineConfig cfg{};
        cfg.threshold           = kSsimPhase2Threshold;
        cfg.autoUpdateBaseline  = true;  // Phase 2: auto-update while threshold is loose
        return cfg;
    }
};

// ---------------------------------------------------------------------------
// SsimCorpusBaseline — singleton baseline store
// ---------------------------------------------------------------------------
class SsimCorpusBaseline final {
public:
    SsimCorpusBaseline(const SsimCorpusBaseline&)            = delete;
    SsimCorpusBaseline& operator=(const SsimCorpusBaseline&) = delete;

    /// Returns the process-wide singleton.
    [[nodiscard]] static SsimCorpusBaseline& Global() noexcept;

    /// Applies configuration before first use.
    void Configure(const SsimBaselineConfig& cfg) noexcept;

    /// Loads baseline records from a JSON file (data/baselines/ssim-baseline.json).
    [[nodiscard]] SsimBaselineStatus Load(const std::wstring& jsonPath) noexcept;

    /// Persists all records to the JSON baseline file.
    [[nodiscard]] SsimBaselineStatus Save(const std::wstring& jsonPath) const noexcept;

    /// Stores a baseline score for fileKey (overwrites existing).
    [[nodiscard]] SsimBaselineStatus RecordBaseline(
        const std::wstring& fileKey,
        double              score) noexcept;

    /// Compares freshScore against the stored baseline; updates latestScore.
    [[nodiscard]] SsimBaselineStatus Evaluate(
        const std::wstring& fileKey,
        double              freshScore,
        SsimBaselineEntry*  outEntry = nullptr) noexcept;

    /// Generates a summary report for all tracked entries.
    [[nodiscard]] SsimBaselineReport GenerateReport() const noexcept;

    /// Returns all entries (for iteration).
    [[nodiscard]] const std::vector<SsimBaselineEntry>& Entries() const noexcept;

    /// Removes all records.
    void Clear() noexcept;

    /// Returns the count of tracked corpus files.
    [[nodiscard]] std::uint32_t Count() const noexcept;

private:
    SsimCorpusBaseline() noexcept = default;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_SSIMCORPUSBASELINE_H
