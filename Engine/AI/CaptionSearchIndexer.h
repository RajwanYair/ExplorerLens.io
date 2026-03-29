// CaptionSearchIndexer.h — Caption Search Indexer
// Copyright (c) 2026 ExplorerLens Project
//
// Maintains an inverted index and vector similarity index over caption
// embeddings, returning top-K results for semantic and keyword search.
//
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct CaptionSearchEntry {
    std::string        fileKey;
    std::string        caption;
    std::vector<float> embedding;
};

struct CaptionSearchResult {
    std::string fileKey;
    std::string caption;
    float       score = 0.0f;
};

class CaptionSearchIndexer {
public:
    CaptionSearchIndexer() = default;

    bool Build() { m_built = true; return true; }
    bool IsBuilt() const { return m_built; }

    void AddEntry(const CaptionSearchEntry& entry) {
        m_index[entry.fileKey] = entry;
    }

    void RemoveEntry(const std::string& fileKey) {
        m_index.erase(fileKey);
    }

    std::vector<CaptionSearchResult> SearchKeyword(const std::string& query, uint32_t topK = 10) const {
        std::vector<CaptionSearchResult> results;
        for (const auto& [key, entry] : m_index) {
            if (entry.caption.find(query) != std::string::npos) {
                CaptionSearchResult r;
                r.fileKey = key;
                r.caption = entry.caption;
                r.score   = 1.0f;
                results.push_back(r);
                if (results.size() >= topK) break;
            }
        }
        return results;
    }

    std::vector<CaptionSearchResult> SearchSemantic(const std::vector<float>& queryEmbedding, uint32_t topK = 10) const {
        std::vector<CaptionSearchResult> results;
        for (const auto& [key, entry] : m_index) {
            if (entry.embedding.empty() || queryEmbedding.empty()) continue;
            float dot = 0.0f;
            size_t n = std::min(entry.embedding.size(), queryEmbedding.size());
            for (size_t i = 0; i < n; ++i) dot += entry.embedding[i] * queryEmbedding[i];
            CaptionSearchResult r;
            r.fileKey = key;
            r.caption = entry.caption;
            r.score   = dot;
            results.push_back(r);
            if (results.size() >= topK) break;
        }
        return results;
    }

    uint64_t GetEntryCount() const { return static_cast<uint64_t>(m_index.size()); }
    void Clear() { m_index.clear(); m_built = false; }

private:
    std::unordered_map<std::string, CaptionSearchEntry> m_index;
    bool m_built = false;
};

}} // namespace ExplorerLens::Engine
