// AdjacencyPredictor.h — Sibling Directory Adjacency Predictor
// Copyright (c) 2026 ExplorerLens Project
//
// Predicts which folder the user is likely to navigate to next based on
// the most-recently-used folder history and sibling directory enumeration.
// Feeds predicted directories into DirectoryPreScanQueue for pre-warming
// the thumbnail cache before the user arrives. Improves perceived
// navigation latency from ~50 ms cold to < 1 ms (cache hit).
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct AdjacencyPrediction {
    std::wstring directoryPath;
    float        confidence    = 0.0f;  // [0.0, 1.0] likelihood score
    uint32_t     estimatedFiles = 0;    // Files in predicted directory
    bool         isSibling      = false; // True if sibling of current dir
};

struct AdjacencyPredictorConfig {
    uint32_t mruHistoryDepth  = 16;   // MRU navigation entries to consider
    uint32_t siblingLookAhead = 2;    // Number of sibling dirs to pre-scan
    float    minConfidence    = 0.3f; // Minimum confidence to trigger pre-scan
};

class AdjacencyPredictor {
public:
    explicit AdjacencyPredictor(
        const AdjacencyPredictorConfig& config = {}) noexcept;
    ~AdjacencyPredictor() = default;

    // Record a navigation event to update MRU and prediction model.
    void RecordNavigation(const std::wstring& dirPath) noexcept;

    // Predict up to `maxResults` adjacent directories to pre-scan.
    std::vector<AdjacencyPrediction> Predict(
        const std::wstring& currentDir, uint32_t maxResults = 4) const noexcept;

    // Enumerate immediate sibling directories of `dirPath`.
    static std::vector<std::wstring> GetSiblings(
        const std::wstring& dirPath) noexcept;

    // Clear the MRU history.
    void Reset() noexcept;

    uint32_t HistoryDepth() const noexcept { return static_cast<uint32_t>(m_mru.size()); }

private:
    AdjacencyPredictorConfig m_config;
    std::vector<std::wstring> m_mru;   // Most-recently-used directories (latest first)
};

}} // namespace ExplorerLens::Engine
