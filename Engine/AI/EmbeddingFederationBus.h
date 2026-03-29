// EmbeddingFederationBus.h — Embedding Federation Bus
// Copyright (c) 2026 ExplorerLens Project
//
// Buses visual and semantic embeddings across nodes without centralizing raw data.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

struct EFBQueryRequest {
    std::vector<float> queryEmbedding;
    uint32_t           topK            = 5;
    float              minSimilarity   = 0.5f;
};

struct EFBMatch {
    std::string id;
    float       similarity = 0.0f;
};

struct EFBQueryResult {
    bool                   success = false;
    std::vector<EFBMatch>  matches;
};

class EmbeddingFederationBus {
public:
    void RegisterEmbedding(const std::string& id, const std::vector<float>& embedding) {
        m_embeddings[id] = embedding;
    }
    EFBQueryResult Query(const EFBQueryRequest& req) {
        EFBQueryResult r;
        if (req.queryEmbedding.empty()) return r;
        std::vector<EFBMatch> candidates;
        for (const auto& [id, emb] : m_embeddings) {
            float sim = CosineSimilarity(req.queryEmbedding, emb);
            if (sim >= req.minSimilarity) candidates.push_back({ id, sim });
        }
        std::sort(candidates.begin(), candidates.end(),
            [](const EFBMatch& a, const EFBMatch& b){ return a.similarity > b.similarity; });
        if (candidates.size() > req.topK) candidates.resize(req.topK);
        r.matches = std::move(candidates);
        r.success = true;
        return r;
    }
    uint32_t EmbeddingCount() const { return static_cast<uint32_t>(m_embeddings.size()); }

private:
    std::unordered_map<std::string, std::vector<float>> m_embeddings;

    static float CosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) {
        size_t n = std::min(a.size(), b.size());
        float dot = 0.0f, na = 0.0f, nb = 0.0f;
        for (size_t i = 0; i < n; ++i) { dot += a[i]*b[i]; na += a[i]*a[i]; nb += b[i]*b[i]; }
        float denom = std::sqrt(na) * std::sqrt(nb);
        return (denom > 1e-6f) ? dot / denom : 0.0f;
    }
};

}} // namespace ExplorerLens::Engine
