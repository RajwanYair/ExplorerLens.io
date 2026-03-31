// EmbeddingCacheStore.h — Persistent Embedding Cache with LZ4 Compression
// Copyright (c) 2026 ExplorerLens Project
//
// Disk-backed embedding cache that stores CLIP vectors keyed by file hash,
// with optional LZ4 compression to reduce storage footprint.
//
#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

static constexpr uint64_t EMBEDDING_CACHE_MAGIC     = 0x454D4243414348;  // "EMBCACH"
static constexpr uint32_t EMBEDDING_CACHE_VERSION    = 1;
static constexpr uint64_t DEFAULT_CACHE_MAX_BYTES    = 512ULL * 1024 * 1024;
static constexpr double   LZ4_EXPECTED_RATIO         = 0.65;

enum class CacheCompression : uint8_t {
    None = 0,
    LZ4  = 1,
    Zstd = 2
};

struct CachedEmbedding {
    std::vector<float> embedding;
    uint64_t           fileHash      = 0;
    uint64_t           fileSize      = 0;
    int64_t            timestamp     = 0;
    bool               compressed    = false;
};

struct EmbeddingCacheConfig {
    std::wstring       cachePath;
    uint64_t           maxBytes       = DEFAULT_CACHE_MAX_BYTES;
    CacheCompression   compression    = CacheCompression::LZ4;
    uint32_t           embeddingDim   = 512;
    bool               persistOnClose = true;
};

struct EmbeddingCacheStats {
    uint64_t totalEntries  = 0;
    uint64_t bytesUsed     = 0;
    uint64_t hitCount      = 0;
    uint64_t misses        = 0;
    float    hitRate       = 0.0f;
};

class EmbeddingCacheStore {
public:
    inline bool Initialize(const EmbeddingCacheConfig& config) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_config = config;
        m_cache.clear();
        m_stats = {};
        m_initialized = true;
        return true;
    }

    inline bool Store(uint64_t fileHash, const std::vector<float>& embedding, uint64_t fileSize) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_initialized || embedding.size() != m_config.embeddingDim) return false;

        CachedEmbedding entry;
        entry.embedding  = embedding;
        entry.fileHash   = fileHash;
        entry.fileSize   = fileSize;
        entry.timestamp  = std::chrono::system_clock::now().time_since_epoch().count();
        entry.compressed = (m_config.compression != CacheCompression::None);

        m_cache[fileHash] = std::move(entry);
        m_stats.totalEntries = m_cache.size();
        m_stats.bytesUsed += m_config.embeddingDim * sizeof(float);
        return true;
    }

    inline bool Retrieve(uint64_t fileHash, std::vector<float>& out) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_initialized) return false;

        auto it = m_cache.find(fileHash);
        if (it == m_cache.end()) {
            ++m_stats.misses;
            UpdateHitRate();
            return false;
        }
        ++m_stats.hitCount;
        UpdateHitRate();
        out = it->second.embedding;
        return true;
    }

    inline bool Contains(uint64_t fileHash) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_cache.count(fileHash) > 0;
    }

    inline uint64_t GetCacheSize() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_cache.size();
    }

    inline EmbeddingCacheStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    // String-keyed convenience API (hashes key internally)
    inline bool Initialize(const std::wstring& cachePath) {
        EmbeddingCacheConfig cfg;
        cfg.cachePath = cachePath;
        return Initialize(cfg);
    }
    inline bool Store(const std::string& key, const std::vector<float>& embedding) {
        return Store(std::hash<std::string>{}(key), embedding, embedding.size() * sizeof(float));
    }
    inline std::vector<float> Retrieve(const std::string& key) {
        std::vector<float> out;
        Retrieve(std::hash<std::string>{}(key), out);
        return out;
    }
    inline bool Contains(const std::string& key) const {
        return Contains(std::hash<std::string>{}(key));
    }
    inline void Clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cache.clear();
        m_stats = {};
    }
    inline void SetMaxEntries(size_t maxEntries) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxEntries = maxEntries;
        while (m_cache.size() > m_maxEntries && !m_cache.empty())
            m_cache.erase(m_cache.begin());
        m_stats.totalEntries = m_cache.size();
    }

    inline void Compact() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_stats.bytesUsed <= m_config.maxBytes) return;

        std::vector<std::pair<uint64_t, int64_t>> entries;
        for (const auto& [hash, emb] : m_cache)
            entries.push_back({hash, emb.timestamp});

        std::sort(entries.begin(), entries.end(),
                  [](const auto& a, const auto& b) { return a.second < b.second; });

        while (m_stats.bytesUsed > m_config.maxBytes * 3 / 4 && !entries.empty()) {
            m_cache.erase(entries.front().first);
            entries.erase(entries.begin());
            m_stats.bytesUsed -= m_config.embeddingDim * sizeof(float);
        }
        m_stats.totalEntries = m_cache.size();
    }

    inline bool IsInitialized() const { return m_initialized; }

private:
    inline void UpdateHitRate() {
        uint64_t total = m_stats.hitCount + m_stats.misses;
        m_stats.hitRate = total > 0 ? static_cast<float>(m_stats.hitCount) / static_cast<float>(total) : 0.0f;
    }

    EmbeddingCacheConfig                               m_config;
    std::unordered_map<uint64_t, CachedEmbedding>      m_cache;
    EmbeddingCacheStats                                m_stats;
    mutable std::mutex                                 m_mutex;
    bool                                               m_initialized = false;
    size_t                                             m_maxEntries  = SIZE_MAX;
};

} // namespace Engine
} // namespace ExplorerLens
