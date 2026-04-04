// SemanticSearchOrchestrator.h — Top-level CLIP + HNSW search coordinator
// Copyright (c) 2026 ExplorerLens Project
//
// Single entry-point wiring CLIPQueryProcessor (text → embedding),
// HNSWIndexEngine (embedding → ANN results), and EmbeddingPersistenceEngine
// (load/store persisted on-disk index). Called from AISearchIntegration and
// Shell Explorer intelligent search integration.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

struct SemanticSearchRequest {
    std::wstring queryText{};
    uint32_t     topK         = 20;
    float        minRelevance = 0.15f;
    bool         reloadIndex  = false;
};

struct SemanticSearchResult {
    std::wstring filePath{};
    float        relevance = 0.0f;
    uint32_t     rank      = 0;
};

struct SemanticSearchStats {
    float    embedMs      = 0.0f;
    float    searchMs     = 0.0f;
    float    totalMs      = 0.0f;
    uint32_t indexedCount = 0;
    uint32_t resultsFound = 0;
};

class SemanticSearchOrchestrator {
public:
    static SemanticSearchOrchestrator& Instance();

    bool   Initialize(const std::wstring& indexPath);
    bool   IsReady()   const noexcept { return m_initialized; }
    bool   IndexFile(const std::wstring& filePath, const float embedding[512]);
    std::vector<SemanticSearchResult> Search(const SemanticSearchRequest& req);
    SemanticSearchStats LastStats()    const noexcept { return m_lastStats; }
    uint32_t            IndexedCount() const noexcept { return m_indexedCount; }

private:
    bool                m_initialized = false;
    uint32_t            m_indexedCount = 0;
    SemanticSearchStats m_lastStats{};
};

}} // namespace ExplorerLens::Engine
