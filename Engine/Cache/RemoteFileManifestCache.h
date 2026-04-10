// RemoteFileManifestCache.h — Remote Directory Manifest Cache
// Copyright (c) 2026 ExplorerLens Project
//
// Caches remote directory listings (manifests) to avoid redundant LIST RPC calls
// when enumerating cloud or network share directories for thumbnail generation.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

struct RemoteFileEntry {
    std::wstring path;
    uint64_t     mtimeMs     = 0;
    uint64_t     sizeBytes   = 0;
    std::string  etag;
};

struct DirectoryManifest {
    std::wstring               directoryPath;
    std::vector<RemoteFileEntry> entries;
    uint64_t                   fetchedAtMs  = 0;
    uint64_t                   ttlMs        = 60'000;
    bool                       IsStale(uint64_t nowMs) const {
        return (nowMs - fetchedAtMs) > ttlMs;
    }
};

class RemoteFileManifestCache {
public:
    struct Config {
        uint32_t maxDirectories  = 128;
        uint64_t defaultTtlMs    = 60'000;
    };

    explicit RemoteFileManifestCache(const Config& cfg = {}) : m_cfg(cfg) {}

    void Store(const DirectoryManifest& manifest);
    bool Lookup(const std::wstring& dirPath, uint64_t nowMs, DirectoryManifest& out) const;
    void Invalidate(const std::wstring& dirPath);
    void PurgeStale(uint64_t nowMs);
    void Clear();

    uint32_t EntryCount()    const { return static_cast<uint32_t>(m_cache.size()); }
    uint32_t HitCount()      const { return m_hits; }
    uint32_t MissCount()     const { return m_misses; }
    uint32_t EvictionCount() const { return m_evictions; }

    const Config& GetConfig() const { return m_cfg; }

private:
    Config                                            m_cfg;
    std::unordered_map<std::wstring, DirectoryManifest> m_cache;
    mutable uint32_t m_hits      = 0;
    mutable uint32_t m_misses    = 0;
    uint32_t         m_evictions = 0;
};

}} // namespace ExplorerLens::Engine
