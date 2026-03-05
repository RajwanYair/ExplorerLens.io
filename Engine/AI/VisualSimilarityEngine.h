// VisualSimilarityEngine.h — Visual Similarity Search via Embeddings
// Copyright (c) 2026 ExplorerLens Project
//
// Computes image embeddings and performs nearest-neighbor similarity search
// using configurable distance metrics (Cosine, Euclidean, Manhattan).
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class VSimilarityMetric : uint32_t {
    Cosine = 0,
    Euclidean = 1,
    Manhattan = 2
};

struct VSimilarityResult {
    uint64_t    itemId = 0;
    std::string label;
    double      distance = 0.0;
    double      score = 0.0;  // 0.0 - 1.0, higher = more similar

    bool operator<(const VSimilarityResult& other) const {
        return score > other.score;
    }
};

class VisualSimilarityEngine {
public:
    static VisualSimilarityEngine& Instance() {
        static VisualSimilarityEngine s;
        return s;
    }

    std::vector<float> ComputeEmbedding(const uint8_t* pixels, uint32_t width,
        uint32_t height, uint32_t channels) {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<float> embedding(m_embeddingDim, 0.0f);

        if (!pixels || width == 0 || height == 0) return embedding;

        size_t pixelCount = static_cast<size_t>(width) * height;
        size_t step = (std::max)(pixelCount / 1024, static_cast<size_t>(1));

        // Color histogram-based embedding (simple but effective for similarity)
        uint32_t bins = m_embeddingDim / 3;
        for (size_t i = 0; i < pixelCount; i += step) {
            size_t idx = i * channels;
            uint32_t rBin = static_cast<uint32_t>(pixels[idx]) * bins / 256;
            rBin = (std::min)(rBin, bins - 1);
            embedding[rBin] += 1.0f;

            if (channels > 1) {
                uint32_t gBin = static_cast<uint32_t>(pixels[idx + 1]) * bins / 256;
                gBin = (std::min)(gBin, bins - 1);
                embedding[bins + gBin] += 1.0f;
            }
            if (channels > 2) {
                uint32_t bBin = static_cast<uint32_t>(pixels[idx + 2]) * bins / 256;
                bBin = (std::min)(bBin, bins - 1);
                embedding[2 * bins + bBin] += 1.0f;
            }
        }

        // L2 normalize
        float norm = 0.0f;
        for (float v : embedding) norm += v * v;
        norm = std::sqrt(norm);
        if (norm > 0.0f) {
            for (float& v : embedding) v /= norm;
        }

        return embedding;
    }

    uint64_t AddToIndex(const std::vector<float>& embedding, const std::string& label) {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t id = m_nextItemId++;
        IndexEntry entry;
        entry.itemId = id;
        entry.label = label;
        entry.embedding = embedding;
        m_index.push_back(entry);
        return id;
    }

    std::vector<VSimilarityResult> FindSimilar(const std::vector<float>& query,
        size_t topK = 5) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<VSimilarityResult> results;

        for (const auto& entry : m_index) {
            VSimilarityResult r;
            r.itemId = entry.itemId;
            r.label = entry.label;
            r.distance = ComputeDistance(query, entry.embedding, m_metric);
            r.score = DistanceToScore(r.distance, m_metric);
            results.push_back(r);
        }

        std::sort(results.begin(), results.end());
        if (results.size() > topK) results.resize(topK);
        return results;
    }

    void BuildIndex() {
        // Index is maintained incrementally; this is a placeholder for
        // future spatial indexing (e.g., VP-tree, HNSW)
        std::lock_guard<std::mutex> lock(m_mutex);
        m_indexBuilt = true;
    }

    void SetMetric(VSimilarityMetric metric) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_metric = metric;
    }

    size_t GetIndexSize() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_index.size();
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_index.clear();
        m_nextItemId = 1;
        m_indexBuilt = false;
    }

    bool Validate() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& entry : m_index) {
            if (entry.itemId == 0) return false;
            if (entry.embedding.size() != m_embeddingDim) return false;
        }
        return static_cast<uint32_t>(m_metric) <= 2;
    }

private:
    VisualSimilarityEngine() = default;
    ~VisualSimilarityEngine() = default;
    VisualSimilarityEngine(const VisualSimilarityEngine&) = delete;
    VisualSimilarityEngine& operator=(const VisualSimilarityEngine&) = delete;

    struct IndexEntry {
        uint64_t          itemId = 0;
        std::string       label;
        std::vector<float> embedding;
    };

    double ComputeDistance(const std::vector<float>& a, const std::vector<float>& b,
        VSimilarityMetric metric) const {
        size_t dim = (std::min)(a.size(), b.size());
        if (dim == 0) return 1e9;

        double result = 0.0;
        switch (metric) {
        case VSimilarityMetric::Cosine: {
            double dot = 0, normA = 0, normB = 0;
            for (size_t i = 0; i < dim; ++i) {
                dot += a[i] * b[i];
                normA += a[i] * a[i];
                normB += b[i] * b[i];
            }
            double denom = std::sqrt(normA) * std::sqrt(normB);
            result = denom > 0 ? 1.0 - (dot / denom) : 1.0;
            break;
        }
        case VSimilarityMetric::Euclidean: {
            for (size_t i = 0; i < dim; ++i) {
                double d = a[i] - b[i];
                result += d * d;
            }
            result = std::sqrt(result);
            break;
        }
        case VSimilarityMetric::Manhattan: {
            for (size_t i = 0; i < dim; ++i)
                result += std::abs(static_cast<double>(a[i] - b[i]));
            break;
        }
        }
        return result;
    }

    double DistanceToScore(double distance, VSimilarityMetric metric) const {
        switch (metric) {
        case VSimilarityMetric::Cosine:
            return (std::max)(0.0, 1.0 - distance);
        case VSimilarityMetric::Euclidean:
            return 1.0 / (1.0 + distance);
        case VSimilarityMetric::Manhattan:
            return 1.0 / (1.0 + distance);
        }
        return 0.0;
    }

    mutable std::mutex m_mutex;
    std::vector<IndexEntry> m_index;
    uint64_t m_nextItemId = 1;
    uint32_t m_embeddingDim = 128;
    VSimilarityMetric m_metric = VSimilarityMetric::Cosine;
    bool m_indexBuilt = false;
};

}
} // namespace ExplorerLens::Engine
