// PluginRatingEngine.h — Plugin Rating Aggregator with Bayesian Scoring
// Copyright (c) 2026 ExplorerLens Project
//
// Aggregates multi-source ratings and applies Bayesian smoothing for fair ranking.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <optional>

namespace ExplorerLens::Engine {

enum class RatingSource : uint8_t {
    UserReview   = 0,
    AutoScan     = 1,
    CompatTest   = 2,
    SecurityScan = 3,
};

struct RatingEntry {
    std::string  pluginId;
    float        score          = 0.0f;
    RatingSource source         = RatingSource::UserReview;
    uint32_t     reviewCount    = 0;
    float        bayesianScore  = 0.0f;

    [[nodiscard]] bool IsValid() const noexcept {
        return !pluginId.empty() && score >= 0.0f && score <= 5.0f;
    }
};

struct RatingThreshold {
    float trusted    = 3.5f;
    float suspicious = 2.0f;
    float blocked    = 1.0f;
};

struct ReviewSubmission {
    std::string  pluginId;
    std::string  reviewerToken;
    float        score     = 0.0f;
    std::string  comment;
    RatingSource source    = RatingSource::UserReview;
};

class PluginRatingEngine {
public:
    explicit PluginRatingEngine(const RatingThreshold& thresholds = {});
    ~PluginRatingEngine() noexcept;

    PluginRatingEngine(const PluginRatingEngine&)            = delete;
    PluginRatingEngine& operator=(const PluginRatingEngine&) = delete;
    PluginRatingEngine(PluginRatingEngine&&)                 = default;
    PluginRatingEngine& operator=(PluginRatingEngine&&)      = default;

    // Retrieve aggregated rating for a plugin, or nullopt if unknown.
    [[nodiscard]] std::optional<RatingEntry> GetRating(const std::string& pluginId) const;

    // Submit a new review; returns false if the token has already voted.
    bool SubmitReview(const ReviewSubmission& submission);

    // Recompute Bayesian score: score = (C * m + n * x) / (C + n)
    // where C = prior count, m = global mean, n = review count, x = raw score.
    [[nodiscard]] float ComputeBayesian(float rawScore, uint32_t reviewCount) const noexcept;

    // Returns true when the plugin's Bayesian score meets the trusted threshold.
    [[nodiscard]] bool IsTrusted(const std::string& pluginId) const;

    // Returns the top-N plugins by Bayesian score.
    [[nodiscard]] std::vector<RatingEntry> GetLeaderboard(uint32_t topN = 20) const;

    // Bulk-load ratings (e.g. from a local DB snapshot).
    void LoadRatings(std::vector<RatingEntry> entries);

    void SetThresholds(const RatingThreshold& thresholds) noexcept;
    [[nodiscard]] const RatingThreshold& GetThresholds() const noexcept { return m_thresholds; }

    // Number of globally tracked ratings.
    [[nodiscard]] uint64_t TotalRatings() const noexcept;

private:
    RatingThreshold             m_thresholds;
    float                       m_globalMean  = 3.0f;
    uint32_t                    m_priorCount  = 10;

    struct Impl {};
    std::unique_ptr<Impl>       m_impl;

    void RecalculateGlobalMean() noexcept;
    bool IsTokenUsed(const std::string& pluginId, const std::string& token) const;
};

} // namespace ExplorerLens::Engine
