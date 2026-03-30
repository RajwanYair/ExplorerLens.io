// PluginReputationScorer.h — Multi-Factor Plugin Reputation Scorer
// Copyright (c) 2026 ExplorerLens Project
//
// Computes weighted reputation scores from downloads, stars, CVE status and security scans.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <optional>
#include <chrono>

namespace ExplorerLens::Engine {

enum class ReputationFactor : uint8_t {
    Downloads       = 0,
    Stars           = 1,
    SecurityScan    = 2,
    CVEStatus       = 3,
    UpdateFrequency = 4,
    FACTOR_COUNT    = 5,
};

struct CVEEntry {
    std::string id;             // e.g. "CVE-2024-12345"
    std::string severity;       // "Critical", "High", "Medium", "Low"
    float       cvssScore   = 0.0f;
    std::string patchedVersion; // empty = unpatched

    [[nodiscard]] bool IsPatched() const noexcept { return !patchedVersion.empty(); }
    [[nodiscard]] bool IsCritical() const noexcept { return cvssScore >= 9.0f; }
};

struct ReputationScore {
    float overall      = 0.0f;
    std::array<float, static_cast<size_t>(ReputationFactor::FACTOR_COUNT)> factors{};
    bool  isBlacklisted = false;

    [[nodiscard]] float GetFactor(ReputationFactor f) const noexcept {
        return factors[static_cast<size_t>(f)];
    }
};

struct PluginReputationData {
    std::string              pluginId;
    uint64_t                 downloadCount    = 0;
    uint32_t                 starCount        = 0;
    uint32_t                 daysSinceUpdate  = 0;
    bool                     passedSecScan    = false;
    std::vector<CVEEntry>    cves;
};

class PluginReputationScorer {
public:
    using WeightArray = std::array<float, static_cast<size_t>(ReputationFactor::FACTOR_COUNT)>;

    PluginReputationScorer();
    ~PluginReputationScorer() noexcept;

    PluginReputationScorer(const PluginReputationScorer&)            = delete;
    PluginReputationScorer& operator=(const PluginReputationScorer&) = delete;
    PluginReputationScorer(PluginReputationScorer&&)                 = default;
    PluginReputationScorer& operator=(PluginReputationScorer&&)      = default;

    // Compute a full reputation score from plugin telemetry data.
    [[nodiscard]] ReputationScore ScorePlugin(const PluginReputationData& data) const noexcept;

    // Look up CVE entries for a specific plugin by ID from the local DB.
    [[nodiscard]] std::vector<CVEEntry> CheckCVE(const std::string& pluginId) const;

    // Return all blacklisted plugin IDs.
    [[nodiscard]] std::vector<std::string> GetBlacklist() const;

    // Adjust per-factor weights; values are normalised to sum to 1.0 internally.
    void SetWeights(const WeightArray& weights) noexcept;
    [[nodiscard]] WeightArray GetWeights() const noexcept { return m_weights; }

    // Returns true if the plugin appears on the blacklist.
    [[nodiscard]] bool IsBlacklisted(const std::string& pluginId) const;

    // Manually add a plugin to the blacklist.
    void Blacklist(const std::string& pluginId, const std::string& reason);

    // Reload the CVE database from a NVD-format JSON file path.
    bool LoadCVEDatabase(const std::string& jsonFilePath);

    // Minimum overall score required to be considered reputable.
    void SetReputationThreshold(float threshold) noexcept;

private:
    WeightArray m_weights;
    float       m_threshold = 0.4f;

    struct Impl;
    std::unique_ptr<Impl> m_impl;

    [[nodiscard]] float NormaliseDownloads(uint64_t count) const noexcept;
    [[nodiscard]] float NormaliseUpdateFrequency(uint32_t daysSince) const noexcept;
    [[nodiscard]] float ComputeCVEPenalty(const std::vector<CVEEntry>& cves) const noexcept;
};

} // namespace ExplorerLens::Engine
