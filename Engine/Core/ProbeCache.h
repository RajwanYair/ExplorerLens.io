//==============================================================================
// ExplorerLens Engine — ProbeCache (Sprint S234)
// Copyright (c) 2026 — ExplorerLens Project
// ROADMAP v6.0 §2.1 A19: memoize probe results in memory, avoid re-probe
//                       on Explorer refresh cycles
//==============================================================================
//
// Explorer re-asks IThumbnailProvider for the same file whenever the user
// scrolls past it or the folder is refreshed. Every probe re-reads magic
// bytes and re-runs format detection. ProbeCache holds the last N probe
// results in memory so repeat Explorer queries short-circuit to O(1).
//
// Thread safety: shared_mutex — reads are concurrent, inserts take the
// exclusive lock briefly. LRU eviction when the capacity is reached.
//==============================================================================
#pragma once

#include <cstdint>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <utility>

namespace ExplorerLens {
namespace Engine {

/// <summary>
/// Cached probe outcome: canonical format family id discovered during the
/// probe stage of the decode pipeline. Named `ProbeCacheEntry` to avoid
/// collision with `ProbeResult` in IStreamingDecoder.h.
/// </summary>
struct ProbeCacheEntry
{
    std::string formatFamilyId;   // e.g. "jpeg", "heif", "raw", "unknown"
    std::uint64_t fileSize = 0;
    std::uint64_t mtime100ns = 0;
    bool          conclusive = true;
};

/// <summary>
/// Bounded LRU cache keyed by normalized path hash for probe results.
/// Thread-safe. Capacity defaults to 4096 entries (~500 KB).
/// </summary>
class ProbeCache
{
  public:
    explicit ProbeCache(std::size_t capacity = 4096)
        : m_capacity(capacity ? capacity : 1)
    {}

    ProbeCache(const ProbeCache&)            = delete;
    ProbeCache& operator=(const ProbeCache&) = delete;

    std::size_t Capacity() const noexcept { return m_capacity; }

    std::size_t Size() const
    {
        std::shared_lock lk(m_mtx);
        return m_map.size();
    }

    /// <summary>
    /// Insert or update a probe result. If the key exists it is promoted
    /// to the front of the LRU list. Oldest entries are evicted if full.
    /// </summary>
    void Put(std::uint64_t pathHash, ProbeCacheEntry result)
    {
        std::unique_lock lk(m_mtx);
        auto it = m_map.find(pathHash);
        if (it != m_map.end()) {
            m_lru.erase(it->second.second);
            m_lru.push_front(pathHash);
            it->second = { std::move(result), m_lru.begin() };
            return;
        }
        if (m_map.size() >= m_capacity) {
            auto victim = m_lru.back();
            m_lru.pop_back();
            m_map.erase(victim);
        }
        m_lru.push_front(pathHash);
        m_map.emplace(pathHash, std::make_pair(std::move(result), m_lru.begin()));
    }

    /// <summary>
    /// Look up a cached probe. Returns true + fills `out` on hit; false on miss.
    /// Hit promotes the entry to the front of the LRU list.
    /// </summary>
    bool TryGet(std::uint64_t pathHash, ProbeCacheEntry& out)
    {
        std::unique_lock lk(m_mtx); // upgrade because LRU touches list
        auto it = m_map.find(pathHash);
        if (it == m_map.end()) {
            ++m_misses;
            return false;
        }
        ++m_hits;
        m_lru.erase(it->second.second);
        m_lru.push_front(pathHash);
        it->second.second = m_lru.begin();
        out               = it->second.first;
        return true;
    }

    void Clear()
    {
        std::unique_lock lk(m_mtx);
        m_map.clear();
        m_lru.clear();
        m_hits = m_misses = 0;
    }

    std::uint64_t Hits()   const noexcept { return m_hits; }
    std::uint64_t Misses() const noexcept { return m_misses; }

  private:
    using LruIter = std::list<std::uint64_t>::iterator;
    mutable std::shared_mutex m_mtx;
    std::list<std::uint64_t>  m_lru;
    std::unordered_map<std::uint64_t, std::pair<ProbeCacheEntry, LruIter>> m_map;
    std::size_t   m_capacity;
    std::uint64_t m_hits   = 0;
    std::uint64_t m_misses = 0;
};

} // namespace Engine
} // namespace ExplorerLens
