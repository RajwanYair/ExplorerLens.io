//==============================================================================
// DarkThumbs Engine — Sprint 332: Multi-Tenant Cache Manager
// Isolated per-tenant cache namespaces with quota enforcement, cross-tenant
// eviction fairness, tenant activity scoring, and namespace migration.
//==============================================================================
#pragma once
#include <string>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

enum class TenantCacheTier   : uint8_t { Hot=0,Warm,Cold,Evicted,COUNT };
enum class TenantIsolation   : uint8_t { Strict=0,Soft,Shared,COUNT };
enum class TenantEvictPolicy : uint8_t { LRU=0,LFU,ActivityScore,QuotaFirst,COUNT };

struct TenantCacheConfig {
    std::wstring    tenantId;
    TenantIsolation isolation       = TenantIsolation::Strict;
    TenantEvictPolicy evictPolicy   = TenantEvictPolicy::ActivityScore;
    uint64_t        quotaBytes      = 128ULL * 1024 * 1024; // 128 MB default
    float           activityScore   = 1.0f;
    bool            allowMigration  = true;
};

struct TenantCacheStats {
    std::wstring tenantId;
    uint64_t     usedBytes      = 0;
    uint64_t     quotaBytes     = 0;
    uint32_t     hitCount       = 0;
    uint32_t     missCount      = 0;
    float        hitRate        = 0.0f;
    TenantCacheTier activeTier  = TenantCacheTier::Cold;
};

class MultiTenantCacheManager {
public:
    static const wchar_t* TierName(TenantCacheTier t) {
        switch(t) {
            case TenantCacheTier::Hot:     return L"Hot";
            case TenantCacheTier::Warm:    return L"Warm";
            case TenantCacheTier::Cold:    return L"Cold";
            case TenantCacheTier::Evicted: return L"Evicted";
            default: return L"Unknown";
        }
    }
    static const wchar_t* IsolationName(TenantIsolation i) {
        switch(i) {
            case TenantIsolation::Strict: return L"Strict";
            case TenantIsolation::Soft:   return L"Soft";
            case TenantIsolation::Shared: return L"Shared";
            default: return L"Unknown";
        }
    }
    static const wchar_t* EvictPolicyName(TenantEvictPolicy p) {
        switch(p) {
            case TenantEvictPolicy::LRU:           return L"LRU";
            case TenantEvictPolicy::LFU:           return L"LFU";
            case TenantEvictPolicy::ActivityScore: return L"Activity Score";
            case TenantEvictPolicy::QuotaFirst:    return L"Quota First";
            default: return L"Unknown";
        }
    }
    static constexpr size_t TierCount()        { return static_cast<size_t>(TenantCacheTier::COUNT); }
    static constexpr size_t IsolationCount()   { return static_cast<size_t>(TenantIsolation::COUNT); }
    static constexpr size_t EvictPolicyCount() { return static_cast<size_t>(TenantEvictPolicy::COUNT); }
    static bool IsQuotaExceeded(const TenantCacheStats& s) { return s.usedBytes > s.quotaBytes; }
};

}} // namespace DarkThumbs::Engine
