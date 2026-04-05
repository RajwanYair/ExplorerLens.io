// MarketplaceSearchIndex.h — Fast inverted search index for plugin catalog
// Copyright (c) 2026 ExplorerLens Project
//
// Provides sub-millisecond full-text search over the plugin marketplace catalog
// using an in-memory inverted index. Supports keyword, tag, and author queries
// with relevance scoring based on install count and rating.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

struct MarketplaceIndexEntry
{
    std::string pluginId;
    float       relevanceScore = 0.0f;
};

struct MarketplaceIndexStats
{
    uint32_t  totalDocuments = 0;
    uint32_t  totalTokens    = 0;
    float     indexSizeKB    = 0.0f;
};

class MarketplaceSearchIndex
{
public:
    MarketplaceSearchIndex();
    ~MarketplaceSearchIndex();

    MarketplaceSearchIndex(const MarketplaceSearchIndex&)            = delete;
    MarketplaceSearchIndex& operator=(const MarketplaceSearchIndex&) = delete;

    bool                       Initialize();
    void                       Clear();
    bool                       IndexPlugin(const std::string& pluginId,
                                            const std::string& name,
                                            const std::string& tags);
    std::vector<MarketplaceIndexEntry> Query(const std::string& terms, uint32_t maxResults = 20) const;
    MarketplaceIndexStats              GetStats() const noexcept;
    uint32_t                      DocumentCount() const noexcept { return m_documentCount; }

    static MarketplaceSearchIndex& Instance() noexcept;

private:
    std::unordered_map<std::string, std::vector<std::string>> m_index;
    uint32_t  m_documentCount = 0;
    static MarketplaceSearchIndex s_instance;
};

}} // namespace ExplorerLens::Engine
