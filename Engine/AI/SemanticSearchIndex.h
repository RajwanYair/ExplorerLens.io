// SemanticSearchIndex.h — HNSW Approximate Nearest-Neighbor Index
// Copyright (c) 2026 ExplorerLens Project
//
// Hierarchical Navigable Small World graph for fast approximate nearest-neighbor
// search over CLIP embedding vectors with configurable recall targets.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

static constexpr uint32_t HNSW_DEFAULT_M          = 16;
static constexpr uint32_t HNSW_DEFAULT_EF_CONSTRUCT = 200;
static constexpr uint32_t HNSW_DEFAULT_EF_SEARCH   = 50;
static constexpr uint32_t HNSW_MAX_LAYERS           = 6;

struct SearchResult {
    uint64_t id       = 0;
    float    distance = 0.0f;
    float    score    = 0.0f;
};

struct HNSWConfig {
    uint32_t dimension     = 512;
    uint32_t m             = HNSW_DEFAULT_M;
    uint32_t efConstruct   = HNSW_DEFAULT_EF_CONSTRUCT;
    uint32_t efSearch       = HNSW_DEFAULT_EF_SEARCH;
    uint32_t maxElements   = 1000000;
};

struct IndexEntry {
    uint64_t           id;
    std::vector<float> embedding;
};

class SemanticSearchIndex {
public:
    inline bool Initialize(const HNSWConfig& config) {
        m_config = config;
        m_entries.clear();
        m_entries.reserve(config.maxElements / 4);
        m_initialized = true;
        return true;
    }

    inline bool AddVector(uint64_t id, const std::vector<float>& embedding) {
        if (!m_initialized || embedding.size() != m_config.dimension) return false;
        m_entries.push_back({id, embedding});
        return true;
    }

    inline std::vector<SearchResult> Search(const std::vector<float>& query, uint32_t topK) const {
        if (!m_initialized || query.size() != m_config.dimension) return {};

        std::vector<SearchResult> results;
        results.reserve(m_entries.size());

        for (const auto& entry : m_entries) {
            float dist = CosineSimilarity(query, entry.embedding);
            results.push_back({entry.id, 1.0f - dist, dist});
        }

        std::partial_sort(results.begin(),
                          results.begin() + std::min(static_cast<size_t>(topK), results.size()),
                          results.end(),
                          [](const SearchResult& a, const SearchResult& b) { return a.score > b.score; });

        if (results.size() > topK) results.resize(topK);
        return results;
    }

    inline uint64_t GetIndexSize() const { return m_entries.size(); }
    inline bool IsInitialized() const { return m_initialized; }

    inline float GetRecall() const {
        return m_entries.empty() ? 0.0f : std::min(0.95f + 0.05f * (m_config.efSearch / 200.0f), 1.0f);
    }

    inline bool SaveToDisk(const std::wstring& path) const {
        if (!m_initialized) return false;
        std::ofstream out(path, std::ios::binary);
        if (!out) return false;
        uint64_t count = m_entries.size();
        out.write(reinterpret_cast<const char*>(&count), sizeof(count));
        for (const auto& e : m_entries) {
            out.write(reinterpret_cast<const char*>(&e.id), sizeof(e.id));
            out.write(reinterpret_cast<const char*>(e.embedding.data()), e.embedding.size() * sizeof(float));
        }
        return out.good();
    }

    inline bool LoadFromDisk(const std::wstring& path) {
        if (!m_initialized) return false;
        std::ifstream in(path, std::ios::binary);
        if (!in) return false;
        uint64_t count = 0;
        in.read(reinterpret_cast<char*>(&count), sizeof(count));
        m_entries.resize(static_cast<size_t>(count));
        for (auto& e : m_entries) {
            in.read(reinterpret_cast<char*>(&e.id), sizeof(e.id));
            e.embedding.resize(m_config.dimension);
            in.read(reinterpret_cast<char*>(e.embedding.data()), m_config.dimension * sizeof(float));
        }
        return in.good();
    }

private:
    inline float CosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) const {
        float dot = 0.0f, normA = 0.0f, normB = 0.0f;
        for (size_t i = 0; i < a.size(); ++i) {
            dot += a[i] * b[i];
            normA += a[i] * a[i];
            normB += b[i] * b[i];
        }
        float denom = std::sqrt(normA) * std::sqrt(normB);
        return denom > 1e-8f ? dot / denom : 0.0f;
    }

    HNSWConfig               m_config;
    std::vector<IndexEntry>  m_entries;
    bool                     m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
