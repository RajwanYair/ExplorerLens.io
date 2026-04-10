// TokenBoundCacheEntry.h — Tenant-Token-Bound Cache Entry
// Copyright (c) 2026 ExplorerLens Project
//
// Wraps a cached thumbnail blob and binds it to a specific user/tenant token.
// Cache lookups enforce that the requesting token matches the stored token,
// preventing cross-tenant cache reads (zero-trust cache isolation).
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

struct TenantToken {
    uint32_t    userId   = 0;
    uint32_t    tenantId = 0;
    std::string scope;    // "read", "read-write", etc.
    bool Matches(const TenantToken& other) const {
        return userId == other.userId && tenantId == other.tenantId;
    }
};

struct TokenBoundEntry {
    TenantToken           boundToken;
    std::vector<uint8_t>  thumbnailBGRA;
    uint32_t              width      = 0;
    uint32_t              height     = 0;
    uint64_t              storedAtMs = 0;
};

class TokenBoundCacheEntry {
public:
    struct Config {
        uint32_t maxEntries = 1024;
    };

    explicit TokenBoundCacheEntry(const Config& cfg = {}) : m_cfg(cfg) {}

    void Store(const std::wstring& path, const TenantToken& token,
               const TokenBoundEntry& entry);
    bool Lookup(const std::wstring& path, const TenantToken& requestor,
                TokenBoundEntry& out) const;
    void Evict(const std::wstring& path);
    void Clear();

    uint32_t EntryCount()            const { return static_cast<uint32_t>(m_cache.size()); }
    uint32_t AuthorizedHitCount()    const { return m_authorizedHits; }
    uint32_t UnauthorizedMissCount() const { return m_unauthorizedMisses; }

    const Config& GetConfig() const { return m_cfg; }

private:
    Config m_cfg;
    std::unordered_map<std::wstring, TokenBoundEntry> m_cache;
    mutable uint32_t m_authorizedHits    = 0;
    mutable uint32_t m_unauthorizedMisses = 0;
};

}} // namespace ExplorerLens::Engine
