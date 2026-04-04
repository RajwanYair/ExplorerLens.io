// VisualSimilarityGraph.h — Per-Folder kNN Visual Similarity Graph
// Copyright (c) 2026 ExplorerLens Project
//
// Builds and maintains a k-nearest-neighbor graph over image embeddings within
// a folder scope, enabling "find similar" and visual clustering workflows.
//
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

static constexpr uint32_t KNN_DEFAULT_K = 8;
static constexpr uint32_t SIMILARITY_GRAPH_MAX_NODES = 500000;
static constexpr float SIMILARITY_EDGE_THRESHOLD = 0.3f;

struct GraphNode
{
    uint64_t fileId;
    std::vector<float> embedding;
};

struct GraphEdge
{
    uint64_t targetId;
    float similarity;
};

struct SimilarityResult
{
    uint64_t fileId;
    float score;
};

struct GraphConfig
{
    uint32_t k = KNN_DEFAULT_K;
    float edgeThreshold = SIMILARITY_EDGE_THRESHOLD;
    uint32_t maxNodes = SIMILARITY_GRAPH_MAX_NODES;
    uint32_t embeddingDim = 512;
};

class VisualSimilarityGraph
{
  public:
    inline bool BuildGraph(const GraphConfig& config, const std::vector<GraphNode>& nodes)
    {
        m_config = config;
        m_nodes.clear();
        m_adjacency.clear();

        for (const auto& node : nodes) {
            if (m_nodes.size() >= config.maxNodes)
                break;
            m_nodes[node.fileId] = node;
        }

        for (const auto& [id, node] : m_nodes) {
            auto neighbors = ComputeNeighbors(id, node.embedding);
            m_adjacency[id] = std::move(neighbors);
        }

        m_built = true;
        return true;
    }

    inline std::vector<SimilarityResult> FindSimilar(uint64_t fileId, uint32_t topK) const
    {
        auto it = m_adjacency.find(fileId);
        if (it == m_adjacency.end())
            return {};

        std::vector<SimilarityResult> results = it->second;
        std::sort(results.begin(), results.end(),
                  [](const SimilarityResult& a, const SimilarityResult& b) { return a.score > b.score; });

        if (results.size() > topK)
            results.resize(topK);
        return results;
    }

    inline bool UpdateIncremental(uint64_t fileId, const std::vector<float>& embedding)
    {
        if (!m_built || m_nodes.size() >= m_config.maxNodes)
            return false;

        GraphNode node{fileId, embedding};
        m_nodes[fileId] = node;
        m_adjacency[fileId] = ComputeNeighbors(fileId, embedding);
        return true;
    }

    inline uint64_t GetNodeCount() const
    {
        return m_nodes.size();
    }

    inline uint64_t GetEdgeCount() const
    {
        uint64_t count = 0;
        for (const auto& [_, edges] : m_adjacency)
            count += edges.size();
        return count;
    }

    inline bool IsBuilt() const
    {
        return m_built;
    }

  private:
    inline std::vector<SimilarityResult> ComputeNeighbors(uint64_t sourceId, const std::vector<float>& emb) const
    {
        std::vector<SimilarityResult> candidates;
        for (const auto& [id, node] : m_nodes) {
            if (id == sourceId)
                continue;
            float sim = CosineSim(emb, node.embedding);
            if (sim >= m_config.edgeThreshold)
                candidates.push_back({id, sim});
        }
        std::partial_sort(
            candidates.begin(), candidates.begin() + std::min(static_cast<size_t>(m_config.k), candidates.size()),
            candidates.end(), [](const SimilarityResult& a, const SimilarityResult& b) { return a.score > b.score; });
        if (candidates.size() > m_config.k)
            candidates.resize(m_config.k);
        return candidates;
    }

    inline float CosineSim(const std::vector<float>& a, const std::vector<float>& b) const
    {
        float dot = 0.0f, na = 0.0f, nb = 0.0f;
        for (size_t i = 0; i < a.size(); ++i) {
            dot += a[i] * b[i];
            na += a[i] * a[i];
            nb += b[i] * b[i];
        }
        float d = std::sqrt(na) * std::sqrt(nb);
        return d > 1e-8f ? dot / d : 0.0f;
    }

    GraphConfig m_config;
    std::unordered_map<uint64_t, GraphNode> m_nodes;
    std::unordered_map<uint64_t, std::vector<SimilarityResult>> m_adjacency;
    bool m_built = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
