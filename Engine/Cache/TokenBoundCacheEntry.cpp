// TokenBoundCacheEntry.cpp — Tenant-Token-Bound Cache Entry
// Copyright (c) 2026 ExplorerLens Project
//
#include "TokenBoundCacheEntry.h"

namespace ExplorerLens { namespace Engine {

void TokenBoundCacheEntry::Store(const std::wstring& path, const TenantToken& token,
                                  const TokenBoundEntry& entry)
{
    if (m_cache.size() >= m_cfg.maxEntries &&
        m_cache.find(path) == m_cache.end()) {
        m_cache.erase(m_cache.begin()); // Simple FIFO eviction
    }
    TokenBoundEntry bound = entry;
    bound.boundToken = token;
    m_cache[path] = std::move(bound);
}

bool TokenBoundCacheEntry::Lookup(const std::wstring& path, const TenantToken& requestor,
                                   TokenBoundEntry& out) const
{
    const auto it = m_cache.find(path);
    if (it == m_cache.end()) return false;

    if (!it->second.boundToken.Matches(requestor)) {
        ++m_unauthorizedMisses;
        return false;
    }
    out = it->second;
    ++m_authorizedHits;
    return true;
}

void TokenBoundCacheEntry::Evict(const std::wstring& path)
{
    m_cache.erase(path);
}

void TokenBoundCacheEntry::Clear()
{
    m_cache.clear();
    m_authorizedHits     = 0;
    m_unauthorizedMisses = 0;
}

}} // namespace ExplorerLens::Engine
