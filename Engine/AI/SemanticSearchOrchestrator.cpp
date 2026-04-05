// SemanticSearchOrchestrator.cpp — Top-level CLIP + HNSW search coordinator
// Copyright (c) 2026 ExplorerLens Project

#include "SemanticSearchOrchestrator.h"
#include "HNSWIndexEngine.h"

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

SemanticSearchOrchestrator& SemanticSearchOrchestrator::Instance()
{
    static SemanticSearchOrchestrator instance;
    return instance;
}

bool SemanticSearchOrchestrator::Initialize(const std::wstring& indexPath)
{
    if (m_initialized) { return true; }
    // Attempt to load persisted HNSW index from disk
    const bool LOADED = HNSWIndexEngine::Instance().LoadFromFile(indexPath);
    if (LOADED) { m_indexedCount = HNSWIndexEngine::Instance().Count(); }
    m_initialized = true;
    return true;
}

bool SemanticSearchOrchestrator::IndexFile(const std::wstring& filePath, const float embedding[512])
{
    if (!m_initialized) { return false; }
    HNSWEntry e{};
    e.itemId   = ++m_indexedCount;
    e.filePath = filePath;
    for (int d = 0; d < 512; ++d) { e.vector[d] = embedding[d]; }
    return HNSWIndexEngine::Instance().Insert(e);
}

std::vector<SemanticSearchResult> SemanticSearchOrchestrator::Search(const SemanticSearchRequest& req)
{
    m_lastStats = {};
    if (!m_initialized || req.queryText.empty())
    {
        return {};
    }

    // Stub query vector (all-zero — real implementation uses CLIPQueryProcessor)
    float qv[512]{};
    const auto QUERY_RESULTS = HNSWIndexEngine::Instance().Query(qv, req.topK);

    std::vector<SemanticSearchResult> out;
    out.reserve(QUERY_RESULTS.size());
    uint32_t rank = 1;
    for (const auto& r : QUERY_RESULTS)
    {
        if (r.similarity < req.minRelevance) { continue; }
        SemanticSearchResult sr{};
        sr.filePath  = r.filePath;
        sr.relevance = r.similarity;
        sr.rank      = rank++;
        out.push_back(sr);
    }

    m_lastStats.searchMs     = HNSWIndexEngine::Instance().LastQueryMs();
    m_lastStats.indexedCount = m_indexedCount;
    m_lastStats.resultsFound = static_cast<uint32_t>(out.size());
    m_lastStats.totalMs      = m_lastStats.embedMs + m_lastStats.searchMs;
    return out;
}

}} // namespace ExplorerLens::Engine
