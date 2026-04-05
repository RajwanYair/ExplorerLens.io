// ScrubberCacheEngine.cpp — LRU frame cache implementation
// Copyright (c) 2026 ExplorerLens Project
//
#include "ScrubberCacheEngine.h"
#include <cstdint>

namespace ExplorerLens { namespace Engine {

ScrubberCacheEngine& ScrubberCacheEngine::Instance() noexcept
{
    static ScrubberCacheEngine instance;
    return instance;
}

bool ScrubberCacheEngine::Put(const ScrubberCacheKey& key, const ScrubberCacheEntry& entry) noexcept
{
    if (key.filePath.empty())
    {
        return false;
    }
    m_map[key] = entry;
    return true;
}

bool ScrubberCacheEngine::Get(const ScrubberCacheKey& key, ScrubberCacheEntry& out) noexcept
{
    const auto IT = m_map.find(key);
    if (IT == m_map.end())
    {
        ++m_misses;
        return false;
    }
    out = IT->second;
    ++m_hits;
    return true;
}

void ScrubberCacheEngine::Evict(const ScrubberCacheKey& key) noexcept
{
    m_map.erase(key);
}

void ScrubberCacheEngine::Clear() noexcept
{
    m_map.clear();
    m_hits   = 0U;
    m_misses = 0U;
}

ScrubberCacheStats ScrubberCacheEngine::GetStats() const noexcept
{
    ScrubberCacheStats st{};
    st.entries = Size();
    st.hits    = m_hits;
    st.misses  = m_misses;
    const uint32_t TOTAL = m_hits + m_misses;
    st.hitRate = (TOTAL > 0U)
                 ? (static_cast<float>(m_hits) / static_cast<float>(TOTAL))
                 : 0.0f;
    return st;
}

}} // namespace ExplorerLens::Engine
