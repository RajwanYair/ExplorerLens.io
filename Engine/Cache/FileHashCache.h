// FileHashCache.h — Content-Addressable File Fingerprinting Cache
// Copyright (c) 2026 ExplorerLens Project
//
// Maintains a fast lookup of file content hashes (XXH3) to detect file
// changes without re-decoding, enabling efficient cache invalidation.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

struct FileFingerprint {
    uint64_t hashValue = 0;
    uint64_t fileSize = 0;
    uint64_t lastModifiedMs = 0;
    uint64_t computeTimeUs = 0;
    bool valid = false;
};

struct FileHashConfig {
    uint64_t maxCachedHashes = 100000;
    uint64_t headerBytesToHash = 64 * 1024; // Hash first 64KB for fast comparison
    bool fullHashOnMismatch = true;
    uint32_t evictionBatchSize = 1000;
};

struct FileHashStats {
    uint64_t totalLookups = 0;
    uint64_t cacheHits = 0;
    uint64_t cacheMisses = 0;
    uint64_t staleEntries = 0;
    uint64_t hashComputations = 0;
    double avgHashTimeUs = 0.0;
    double hitRate() const { return totalLookups > 0 ? 100.0 * cacheHits / totalLookups : 0.0; }
};

class FileHashCache {
public:
    void Configure(const FileHashConfig& config) { m_config = config; }

    bool HasFreshEntry(const std::wstring& path, uint64_t fileSize,
        uint64_t lastModifiedMs) const {
        std::lock_guard lock(m_mutex);
        auto it = m_cache.find(path);
        if (it == m_cache.end()) return false;
        return it->second.valid &&
            it->second.fileSize == fileSize &&
            it->second.lastModifiedMs == lastModifiedMs;
    }

    FileFingerprint Lookup(const std::wstring& path) const {
        std::lock_guard lock(m_mutex);
        auto it = m_cache.find(path);
        return it != m_cache.end() ? it->second : FileFingerprint{};
    }

    void Store(const std::wstring& path, const FileFingerprint& fp) {
        std::lock_guard lock(m_mutex);
        if (m_cache.size() >= m_config.maxCachedHashes) {
            EvictOldest();
        }
        m_cache[path] = fp;
    }

    void Invalidate(const std::wstring& path) {
        std::lock_guard lock(m_mutex);
        m_cache.erase(path);
    }

    size_t Size() const {
        std::lock_guard lock(m_mutex);
        return m_cache.size();
    }

    FileHashStats GetStats() const { return m_stats; }

private:
    void EvictOldest() {
        // Simple eviction: remove entries with oldest lastModifiedMs
        if (m_cache.empty()) return;
        uint32_t count = 0;
        uint64_t oldestTime = UINT64_MAX;
        for (const auto& [k, v] : m_cache) {
            if (v.lastModifiedMs < oldestTime) oldestTime = v.lastModifiedMs;
        }
        for (auto it = m_cache.begin(); it != m_cache.end() && count < m_config.evictionBatchSize;) {
            if (it->second.lastModifiedMs == oldestTime) {
                it = m_cache.erase(it);
                count++;
            }
            else {
                ++it;
            }
        }
    }

    mutable std::mutex m_mutex;
    FileHashConfig m_config;
    FileHashStats m_stats;
    std::unordered_map<std::wstring, FileFingerprint> m_cache;
};

} // namespace Engine
} // namespace ExplorerLens
