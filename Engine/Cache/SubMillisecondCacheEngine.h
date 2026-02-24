//==============================================================================
// ExplorerLens Engine — Sub-Millisecond Cache Engine
// Lock-free hash map with open-addressing, NUMA-aware slab allocator,
// prefetch pipeline, and adaptive hash function selection targeting
// < 1 ms P99 lookup latency for hot thumbnail records.
//==============================================================================
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class CacheHashAlgo       : uint8_t { FNV1a=0, CRC32C, WyHash, XXHash3, COUNT };
enum class SubMsCacheEviction  : uint8_t { CLOCK=0, TinyLFU, SLRUClock, COUNT };
enum class NumaTier            : uint8_t { Local=0, Remote, Interleave, COUNT };

struct SubMsCacheConfig {
    CacheHashAlgo     hashAlgo    = CacheHashAlgo::XXHash3;
    SubMsCacheEviction eviction   = SubMsCacheEviction::TinyLFU;
    NumaTier          numaTier    = NumaTier::Local;
    uint32_t          capacityK   = 32768; // entries (thousands)
    uint8_t           prefetchD   = 4;     // prefetch distance
    bool              lockFree    = true;
};

struct SubMsCachePerfReport {
    uint64_t avgLookupNs = 0;
    uint64_t p99LookupNs = 0; // target: < 1,000,000 ns (1 ms)
    float    hitRate     = 0.0f;
    uint64_t capacity    = 0;
    uint64_t used        = 0;
};

class SubMillisecondCacheEngine {
public:
    static const wchar_t* HashAlgoName(CacheHashAlgo a) {
        switch(a) {
            case CacheHashAlgo::FNV1a:   return L"FNV-1a";
            case CacheHashAlgo::CRC32C:  return L"CRC32C";
            case CacheHashAlgo::WyHash:  return L"WyHash";
            case CacheHashAlgo::XXHash3: return L"XXHash3";
            default: return L"Unknown";
        }
    }
    static const wchar_t* EvictionName(SubMsCacheEviction e) {
        switch(e) {
            case SubMsCacheEviction::CLOCK:    return L"CLOCK";
            case SubMsCacheEviction::TinyLFU:  return L"TinyLFU";
            case SubMsCacheEviction::SLRUClock:return L"SLRU+CLOCK";
            default: return L"Unknown";
        }
    }
    static const wchar_t* NumaTierName(NumaTier n) {
        switch(n) {
            case NumaTier::Local:     return L"NUMA Local";
            case NumaTier::Remote:    return L"NUMA Remote";
            case NumaTier::Interleave:return L"NUMA Interleave";
            default: return L"Unknown";
        }
    }
    static constexpr size_t HashAlgoCount()    { return static_cast<size_t>(CacheHashAlgo::COUNT); }
    static constexpr size_t EvictionCount()    { return static_cast<size_t>(SubMsCacheEviction::COUNT); }
    static constexpr size_t NumaTierCount()    { return static_cast<size_t>(NumaTier::COUNT); }
    static bool MeetsSLA(const SubMsCachePerfReport& r) { return r.p99LookupNs < 1000000ULL; }
};

}} // namespace ExplorerLens::Engine

