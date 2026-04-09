// AnimatedThumbnailCache.cpp — Per-Frame Thumbnail Cache for Animated Formats
// Copyright (c) 2026 ExplorerLens Project
//
#include "Cache/AnimatedThumbnailCache.h"
#include <algorithm>

namespace ExplorerLens { namespace Engine {

AnimatedThumbnailCache::AnimatedThumbnailCache(const AnimatedCacheConfig& cfg) noexcept
    : m_cfg(cfg)
{}

const AnimatedCacheEntry* AnimatedThumbnailCache::Get(const AnimatedCacheKey& key) noexcept
{
    std::unique_lock lock(m_mutex);
    auto it = m_cache.find(key);
    if (it == m_cache.end()) {
        ++m_stats.misses;
        return nullptr;
    }
    it->second.accessSeq = ++m_accessSeq;
    ++m_stats.hits;
    return &it->second;
}

void AnimatedThumbnailCache::Put(const AnimatedCacheKey& key, AnimatedCacheEntry entry) noexcept
{
    const uint64_t entryBytes = entry.pixelsBGRA.size();
    if (entryBytes > m_cfg.maxBytes) return;  // Single entry exceeds budget — skip

    std::unique_lock lock(m_mutex);

    // Remove existing entry if present (update path).
    auto it = m_cache.find(key);
    if (it != m_cache.end()) {
        m_bytesUsed -= it->second.pixelsBGRA.size();
        m_cache.erase(it);
        --m_stats.entryCount;
    }

    // Evict until we have room.
    while ((m_bytesUsed + entryBytes > m_cfg.maxBytes
            || m_stats.entryCount >= m_cfg.maxEntries)
           && !m_cache.empty()) {
        EvictLRU();
    }

    entry.accessSeq = ++m_accessSeq;
    m_bytesUsed += entryBytes;
    m_cache.emplace(key, std::move(entry));
    ++m_stats.entryCount;
    m_stats.bytesUsed = m_bytesUsed;
}

void AnimatedThumbnailCache::Invalidate(const std::wstring& path) noexcept
{
    std::unique_lock lock(m_mutex);
    auto it = m_cache.begin();
    while (it != m_cache.end()) {
        if (it->first.path == path) {
            m_bytesUsed -= it->second.pixelsBGRA.size();
            --m_stats.entryCount;
            it = m_cache.erase(it);
        } else {
            ++it;
        }
    }
    m_stats.bytesUsed = m_bytesUsed;
}

void AnimatedThumbnailCache::Clear() noexcept
{
    std::unique_lock lock(m_mutex);
    m_cache.clear();
    m_bytesUsed        = 0;
    m_stats.entryCount = 0;
    m_stats.bytesUsed  = 0;
}

void AnimatedThumbnailCache::EvictLRU() noexcept
{
    // Find entry with smallest accessSeq.
    auto victim = m_cache.begin();
    for (auto it = m_cache.begin(); it != m_cache.end(); ++it)
        if (it->second.accessSeq < victim->second.accessSeq) victim = it;

    if (victim != m_cache.end()) {
        m_bytesUsed -= victim->second.pixelsBGRA.size();
        m_cache.erase(victim);
        --m_stats.entryCount;
        ++m_stats.evictions;
    }
}

AnimatedThumbnailCacheStats AnimatedThumbnailCache::GetStats() const noexcept
{
    std::shared_lock lock(m_mutex);
    return m_stats;
}

}} // namespace ExplorerLens::Engine
