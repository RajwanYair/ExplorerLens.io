// RemoteFileManifestCache.cpp — Remote Directory Manifest Cache
// Copyright (c) 2026 ExplorerLens Project
//
#include "RemoteFileManifestCache.h"

namespace ExplorerLens { namespace Engine {

void RemoteFileManifestCache::Store(const DirectoryManifest& manifest)
{
    // Evict LRU entry if at capacity
    if (m_cache.size() >= m_cfg.maxDirectories &&
        m_cache.find(manifest.directoryPath) == m_cache.end()) {
        m_cache.erase(m_cache.begin());
        ++m_evictions;
    }
    m_cache[manifest.directoryPath] = manifest;
}

bool RemoteFileManifestCache::Lookup(const std::wstring& dirPath, uint64_t nowMs,
                                     DirectoryManifest& out) const
{
    const auto it = m_cache.find(dirPath);
    if (it == m_cache.end() || it->second.IsStale(nowMs)) {
        ++m_misses;
        return false;
    }
    out = it->second;
    ++m_hits;
    return true;
}

void RemoteFileManifestCache::Invalidate(const std::wstring& dirPath)
{
    m_cache.erase(dirPath);
}

void RemoteFileManifestCache::PurgeStale(uint64_t nowMs)
{
    for (auto it = m_cache.begin(); it != m_cache.end(); ) {
        if (it->second.IsStale(nowMs))
            it = m_cache.erase(it);
        else
            ++it;
    }
}

void RemoteFileManifestCache::Clear()
{
    m_cache.clear();
    m_hits    = 0;
    m_misses  = 0;
    m_evictions = 0;
}

}} // namespace ExplorerLens::Engine
