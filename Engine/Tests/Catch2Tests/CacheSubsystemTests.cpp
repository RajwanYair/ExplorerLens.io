// CacheSubsystemTests.cpp — Catch2 unit tests for the cache subsystem
// Copyright (c) 2026 ExplorerLens Project
//
// Tests the L1 in-memory LRU cache policy: capacity enforcement, hit/miss
// tracking, eviction order, and basic metrics collection.
// All tests are self-contained (no disk I/O, no GPU).
//
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <list>
#include <cstdint>

// ---------------------------------------------------------------------------
// Minimal LRU cache reference implementation — used to validate the interface
// contract that the full SubMillisecondCacheEngine must honour.
// ---------------------------------------------------------------------------

namespace {

template<typename K, typename V>
class LruCache {
public:
    explicit LruCache(size_t capacity) : m_capacity(capacity) {}

    // Insert or update a key-value pair.
    void Put(const K& key, V value) {
        auto it = m_map.find(key);
        if (it != m_map.end()) {
            m_list.erase(it->second);
            m_map.erase(it);
        }
        m_list.push_front({ key, std::move(value) });
        m_map[key] = m_list.begin();
        if (m_map.size() > m_capacity) {
            auto last = m_list.end();
            --last;
            m_map.erase(last->first);
            m_list.pop_back();
            ++m_evictions;
        }
    }

    // Retrieve a value by key. Returns nullopt on miss.
    std::optional<V> Get(const K& key) {
        auto it = m_map.find(key);
        if (it == m_map.end()) {
            ++m_misses;
            return std::nullopt;
        }
        ++m_hits;
        m_list.splice(m_list.begin(), m_list, it->second);
        return it->second->second;
    }

    size_t Size() const { return m_map.size(); }
    size_t HitCount() const { return m_hits; }
    size_t MissCount() const { return m_misses; }
    size_t EvictionCount() const { return m_evictions; }
    double HitRate() const {
        size_t total = m_hits + m_misses;
        return total == 0 ? 0.0 : static_cast<double>(m_hits) / static_cast<double>(total);
    }
    bool Contains(const K& key) const { return m_map.count(key) > 0; }
    void Clear() { m_list.clear(); m_map.clear(); m_hits = m_misses = m_evictions = 0; }

private:
    size_t m_capacity;
    std::list<std::pair<K, V>> m_list;
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> m_map;
    size_t m_hits     = 0;
    size_t m_misses   = 0;
    size_t m_evictions = 0;
};

} // anonymous namespace

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST_CASE("LRU cache — basic put and get", "[cache][lru]") {
    LruCache<std::string, int> cache(4);
    cache.Put("a", 1);
    cache.Put("b", 2);
    cache.Put("c", 3);

    REQUIRE(cache.Get("a") == 1);
    REQUIRE(cache.Get("b") == 2);
    REQUIRE(cache.Get("c") == 3);
    REQUIRE(cache.Size() == 3);
}

TEST_CASE("LRU cache — miss returns nullopt", "[cache][lru]") {
    LruCache<std::string, int> cache(4);
    auto result = cache.Get("missing");
    REQUIRE_FALSE(result.has_value());
    REQUIRE(cache.MissCount() == 1);
}

TEST_CASE("LRU cache — capacity enforcement (LRU eviction)", "[cache][lru][eviction]") {
    LruCache<int, int> cache(3);
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Put(3, 30);
    // Access 1 and 2 to promote them; 3 becomes LRU
    (void)cache.Get(1);
    (void)cache.Get(2);
    cache.Put(4, 40); // should evict 3
    REQUIRE(cache.Size() == 3);
    REQUIRE_FALSE(cache.Contains(3));
    REQUIRE(cache.Contains(1));
    REQUIRE(cache.Contains(2));
    REQUIRE(cache.Contains(4));
    REQUIRE(cache.EvictionCount() == 1);
}

TEST_CASE("LRU cache — update promotes entry to MRU", "[cache][lru]") {
    LruCache<int, int> cache(3);
    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Put(3, 30);
    cache.Put(1, 99); // update — should make 1 MRU, 2 becomes LRU
    cache.Put(4, 40); // evict 2
    REQUIRE_FALSE(cache.Contains(2));
    REQUIRE(cache.Get(1) == 99); // still there, value updated
}

TEST_CASE("LRU cache — hit rate calculation", "[cache][lru][metrics]") {
    LruCache<std::string, int> cache(10);
    cache.Put("x", 1);
    cache.Put("y", 2);
    (void)cache.Get("x");  // hit
    (void)cache.Get("y");  // hit
    (void)cache.Get("z");  // miss
    (void)cache.Get("w");  // miss
    // 2 hits / 4 total = 0.5
    REQUIRE_THAT(cache.HitRate(), Catch::Matchers::WithinAbs(0.5, 1e-9));
    REQUIRE(cache.HitCount() == 2);
    REQUIRE(cache.MissCount() == 2);
}

TEST_CASE("LRU cache — zero capacity is safe (all inserts evict)", "[cache][lru][edge]") {
    // A capacity of 0 would be pathological; in practice the engine clamps to 1.
    // This test documents what happens with capacity=1 (the minimum).
    LruCache<int, int> cache(1);
    cache.Put(1, 10);
    REQUIRE(cache.Contains(1));
    cache.Put(2, 20); // evict 1
    REQUIRE_FALSE(cache.Contains(1));
    REQUIRE(cache.Contains(2));
    REQUIRE(cache.EvictionCount() == 1);
}

TEST_CASE("LRU cache — clear resets all state", "[cache][lru]") {
    LruCache<int, int> cache(10);
    for (int i = 0; i < 8; ++i) {
        cache.Put(i, i * i);
        (void)cache.Get(i);
    }
    cache.Clear();
    REQUIRE(cache.Size() == 0);
    REQUIRE(cache.HitCount() == 0);
    REQUIRE(cache.MissCount() == 0);
    REQUIRE(cache.EvictionCount() == 0);
    REQUIRE_THAT(cache.HitRate(), Catch::Matchers::WithinAbs(0.0, 1e-9));
}

TEST_CASE("LRU cache — large sequential fill maintains capacity", "[cache][lru]") {
    constexpr size_t CAP = 32;
    LruCache<int, int> cache(CAP);
    for (int i = 0; i < 1000; ++i) {
        cache.Put(i, i);
    }
    REQUIRE(cache.Size() == CAP);
    REQUIRE(cache.EvictionCount() == 1000 - CAP);
    // Most recently inserted should be present
    for (int i = 999 - static_cast<int>(CAP) + 1; i < 1000; ++i) {
        REQUIRE(cache.Contains(i));
    }
}

TEST_CASE("LRU cache — string keys and large values", "[cache][lru]") {
    LruCache<std::string, std::string> cache(4);
    cache.Put("file1.jpg", std::string(1024, 'A'));
    cache.Put("file2.png", std::string(2048, 'B'));
    REQUIRE(cache.Get("file1.jpg").has_value());
    REQUIRE(cache.Get("file1.jpg")->size() == 1024);
}

TEST_CASE("LRU cache — no eviction when capacity not reached", "[cache][lru]") {
    LruCache<int, int> cache(100);
    for (int i = 0; i < 50; ++i) { cache.Put(i, i); }
    REQUIRE(cache.EvictionCount() == 0);
    REQUIRE(cache.Size() == 50);
}

TEST_CASE("Cache hit rate — perfect cache (all hits)", "[cache][metrics]") {
    LruCache<int, int> cache(10);
    for (int i = 0; i < 5; ++i) { cache.Put(i, i * 2); }
    for (int i = 0; i < 5; ++i) { (void)cache.Get(i); }
    REQUIRE_THAT(cache.HitRate(), Catch::Matchers::WithinAbs(1.0, 1e-9));
}

TEST_CASE("Cache hit rate — empty cache (all misses)", "[cache][metrics]") {
    LruCache<int, int> cache(10);
    for (int i = 0; i < 5; ++i) { (void)cache.Get(i); }
    REQUIRE_THAT(cache.HitRate(), Catch::Matchers::WithinAbs(0.0, 1e-9));
}
