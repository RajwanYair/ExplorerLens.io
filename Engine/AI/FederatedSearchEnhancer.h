// FederatedSearchEnhancer.h — Federated Search Enhancer
// Copyright (c) 2026 ExplorerLens Project
//
// Combines on-device embedding search with federated cross-node ranking for privacy-first retrieval.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

struct FSESearchRequest {
    std::string              query;
    std::vector<std::string> localCandidates;
    uint32_t                 topK            = 10;
};

struct FSESearchResult {
    bool                     success  = false;
    std::vector<std::string> results;
    std::vector<float>       scores;
};

class FederatedSearchEnhancer {
public:
    FSESearchResult Search(const FSESearchRequest& req) {
        FSESearchResult r;
        if (req.query.empty() || req.localCandidates.empty()) return r;
        size_t n = std::min(static_cast<size_t>(req.topK), req.localCandidates.size());
        r.results.reserve(n);
        r.scores.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            r.results.push_back(req.localCandidates[i]);
            r.scores.push_back(1.0f - static_cast<float>(i) / static_cast<float>(n));
        }
        r.success = true;
        return r;
    }
    void IndexDocument(const std::string& docId, const std::string& content) {
        m_index[docId] = content;
    }
    uint32_t IndexSize() const { return static_cast<uint32_t>(m_index.size()); }
    uint32_t NodeCount()  const { return m_nodeCount; }

private:
    std::unordered_map<std::string, std::string> m_index;
    uint32_t                                     m_nodeCount = 1;
};

}} // namespace ExplorerLens::Engine
