// MarketplaceSearchIndex.cpp — Fast inverted search index for plugin catalog
// Copyright (c) 2026 ExplorerLens Project
//
#include "MarketplaceSearchIndex.h"
#include <sstream>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

MarketplaceSearchIndex MarketplaceSearchIndex::s_instance;

MarketplaceSearchIndex::MarketplaceSearchIndex()  = default;
MarketplaceSearchIndex::~MarketplaceSearchIndex() = default;

MarketplaceSearchIndex& MarketplaceSearchIndex::Instance() noexcept { return s_instance; }

bool MarketplaceSearchIndex::Initialize()
{
    m_index.clear();
    m_documentCount = 0;
    return true;
}

void MarketplaceSearchIndex::Clear()
{
    m_index.clear();
    m_documentCount = 0;
}

bool MarketplaceSearchIndex::IndexPlugin(const std::string& pluginId,
                                          const std::string& name,
                                          const std::string& tags)
{
    if (pluginId.empty())
        return false;
    std::istringstream ss(name + " " + tags);
    std::string token;
    while (ss >> token)
    {
        std::transform(token.begin(), token.end(), token.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        m_index[token].push_back(pluginId);
    }
    ++m_documentCount;
    return true;
}

std::vector<MarketplaceIndexEntry> MarketplaceSearchIndex::Query(const std::string& terms,
                                                              uint32_t maxResults) const
{
    std::vector<MarketplaceIndexEntry> results;
    if (terms.empty())
        return results;
    std::unordered_map<std::string, float> scores;
    std::istringstream ss(terms);
    std::string token;
    while (ss >> token)
    {
        std::transform(token.begin(), token.end(), token.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        auto it = m_index.find(token);
        if (it != m_index.end())
        {
            for (const auto& id : it->second)
                scores[id] += 1.0f;
        }
    }
    for (const auto& kv : scores)
        results.push_back({kv.first, kv.second});
    std::sort(results.begin(), results.end(),
              [](const MarketplaceIndexEntry& a, const MarketplaceIndexEntry& b){
                  return a.relevanceScore > b.relevanceScore;
              });
    if (results.size() > maxResults)
        results.resize(maxResults);
    return results;
}

MarketplaceIndexStats MarketplaceSearchIndex::GetStats() const noexcept
{
    MarketplaceIndexStats stats;
    stats.totalDocuments = m_documentCount;
    stats.totalTokens    = static_cast<uint32_t>(m_index.size());
    return stats;
}

}} // namespace ExplorerLens::Engine
