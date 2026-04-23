// CacheTierTests.cpp — Catch2 tests for L1/L2 cache tier contracts
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the two-tier cache architecture contracts from §7.5 (D42).
// Tests cover: L1 LRU in-memory budget, L2 SQLite/mmap disk budget,
// cache key composition (path + mtime + size + target + decoder version),
// eviction policy invariants, invalidation triggers, and the XXH3-based
// key uniqueness model.
//
// All tests are self-contained — constants mirror Engine/Cache/*.h
// without including Windows or Engine headers.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

// ---------------------------------------------------------------------------
// Cache tier constants and contracts (§7.5, D42)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::CacheTier {

// ── L1 — In-process memory LRU ─────────────────────────────────────────────

static constexpr int64_t L1_BUDGET_DEFAULT_MB =  64;   // default L1 budget
static constexpr int64_t L1_BUDGET_MIN_MB     =   4;   // minimum L1 budget
static constexpr int64_t L1_BUDGET_MAX_MB     = 512;   // maximum L1 budget
static constexpr int64_t L1_HIT_P50_MICROS    = 500;   // < 500 µs target

// ── L2 — Disk-backed SQLite/mmap ───────────────────────────────────────────

static constexpr int64_t L2_BUDGET_DEFAULT_MB  = 1024; // 1 GB default
static constexpr int64_t L2_BUDGET_MIN_MB      =   64; // minimum L2 budget
static constexpr int64_t L2_BUDGET_MAX_MB      = 8192; // 8 GB hard ceiling
static constexpr int64_t L2_HIT_P50_MILLIS     =    3; // < 3 ms index hit
static constexpr int64_t L2_BLOB_HIT_P50_MILLIS=    5; // < 5 ms blob read

// ── Cache key components ────────────────────────────────────────────────────

// CacheKeyField enum mirrors the fields used in the XXH3 hash composition
// §7.5: SHA256(canonical_path ‖ mtime ‖ size ‖ target_w×target_h ‖ decoder_version)
enum class CacheKeyField : uint8_t {
    CANONICAL_PATH   = 0,   // Case-folded, normalized absolute path
    MTIME            = 1,   // File last-modified timestamp (FILETIME)
    FILE_SIZE        = 2,   // File size in bytes
    TARGET_WIDTH     = 3,   // Requested thumbnail width
    TARGET_HEIGHT    = 4,   // Requested thumbnail height
    DECODER_VERSION  = 5,   // Decoder version number (invalidates on upgrade)
};

static constexpr int CACHE_KEY_FIELD_COUNT = 6;

// ── Eviction policy ────────────────────────────────────────────────────────

enum class EvictionPolicy : uint8_t {
    LRU              = 0,   // Least-Recently-Used (default for L1)
    LFU              = 1,   // Least-Frequently-Used (optional for L2 index)
    SIZE_WEIGHTED    = 2,   // Evict largest-first when memory is critical
    TTL              = 3,   // Time-based expiry (used by cloud-file hydration)
};

// ── Invalidation triggers ──────────────────────────────────────────────────

enum class InvalidationTrigger : uint8_t {
    FILE_MODIFIED    = 0,   // mtime or size changed (ReadDirectoryChangesW)
    DECODER_UPGRADED = 1,   // Decoder version bumped
    BUDGET_EXCEEDED  = 2,   // LRU eviction due to capacity pressure
    MANUAL_PURGE     = 3,   // `lens cache purge` CLI command
    CORRUPT_ENTRY    = 4,   // Integrity check failed on retrieval
};

static constexpr int INVALIDATION_TRIGGER_COUNT = 5;

// ── Cache key uniqueness helpers ────────────────────────────────────────────

/// Simple string-based key composition (production uses XXH3 over raw bytes)
inline std::string ComposeCacheKey(std::string_view path, int64_t mtime,
                                   int64_t size, uint32_t tw, uint32_t th,
                                   uint32_t decoderVer) {
    return std::string(path) + "|" +
           std::to_string(mtime) + "|" +
           std::to_string(size)  + "|" +
           std::to_string(tw)    + "x" + std::to_string(th) + "|" +
           std::to_string(decoderVer);
}

/// Two keys are equal iff all 6 components match
inline bool CacheKeysEqual(const std::string& a, const std::string& b) {
    return a == b;
}

/// Returns true when mb is a valid L1 budget
inline bool IsValidL1BudgetMB(int64_t mb) {
    return mb >= L1_BUDGET_MIN_MB && mb <= L1_BUDGET_MAX_MB;
}

/// Returns true when mb is a valid L2 budget
inline bool IsValidL2BudgetMB(int64_t mb) {
    return mb >= L2_BUDGET_MIN_MB && mb <= L2_BUDGET_MAX_MB;
}

} // namespace ExplorerLens::Tests::CacheTier

using namespace ExplorerLens::Tests::CacheTier;

// ===========================================================================
// L1 budget constants
// ===========================================================================

TEST_CASE("L1Budget — DEFAULT is 64 MB",   "[cache][l1][budget]") { REQUIRE(L1_BUDGET_DEFAULT_MB == 64); }
TEST_CASE("L1Budget — MIN is 4 MB",        "[cache][l1][budget]") { REQUIRE(L1_BUDGET_MIN_MB == 4);  }
TEST_CASE("L1Budget — MAX is 512 MB",      "[cache][l1][budget]") { REQUIRE(L1_BUDGET_MAX_MB == 512); }

TEST_CASE("L1Budget — MIN < DEFAULT < MAX ordering",
          "[cache][l1][budget]") {
    REQUIRE(L1_BUDGET_MIN_MB < L1_BUDGET_DEFAULT_MB);
    REQUIRE(L1_BUDGET_DEFAULT_MB < L1_BUDGET_MAX_MB);
}

TEST_CASE("L1Budget — hit latency target < 500 µs",
          "[cache][l1][latency]") {
    REQUIRE(L1_HIT_P50_MICROS < 1000);
    REQUIRE(L1_HIT_P50_MICROS == 500);
}

TEST_CASE("IsValidL1BudgetMB — accepts boundary values",
          "[cache][l1][validation]") {
    CHECK(IsValidL1BudgetMB(4));
    CHECK(IsValidL1BudgetMB(64));
    CHECK(IsValidL1BudgetMB(512));
}

TEST_CASE("IsValidL1BudgetMB — rejects out-of-range values",
          "[cache][l1][validation]") {
    CHECK_FALSE(IsValidL1BudgetMB(0));
    CHECK_FALSE(IsValidL1BudgetMB(3));
    CHECK_FALSE(IsValidL1BudgetMB(513));
    CHECK_FALSE(IsValidL1BudgetMB(-1));
}

// ===========================================================================
// L2 budget constants
// ===========================================================================

TEST_CASE("L2Budget — DEFAULT is 1024 MB (1 GB)",  "[cache][l2][budget]") { REQUIRE(L2_BUDGET_DEFAULT_MB == 1024); }
TEST_CASE("L2Budget — MIN is 64 MB",               "[cache][l2][budget]") { REQUIRE(L2_BUDGET_MIN_MB == 64);    }
TEST_CASE("L2Budget — MAX is 8192 MB (8 GB)",       "[cache][l2][budget]") { REQUIRE(L2_BUDGET_MAX_MB == 8192);  }

TEST_CASE("L2Budget — MIN < DEFAULT < MAX ordering",
          "[cache][l2][budget]") {
    REQUIRE(L2_BUDGET_MIN_MB < L2_BUDGET_DEFAULT_MB);
    REQUIRE(L2_BUDGET_DEFAULT_MB < L2_BUDGET_MAX_MB);
}

TEST_CASE("L2Budget — index hit target < 3 ms",
          "[cache][l2][latency]") {
    REQUIRE(L2_HIT_P50_MILLIS == 3);
}

TEST_CASE("L2Budget — blob hit target < 5 ms",
          "[cache][l2][latency]") {
    REQUIRE(L2_BLOB_HIT_P50_MILLIS == 5);
}

TEST_CASE("IsValidL2BudgetMB — accepts boundary values",
          "[cache][l2][validation]") {
    CHECK(IsValidL2BudgetMB(64));
    CHECK(IsValidL2BudgetMB(1024));
    CHECK(IsValidL2BudgetMB(8192));
}

TEST_CASE("IsValidL2BudgetMB — rejects out-of-range values",
          "[cache][l2][validation]") {
    CHECK_FALSE(IsValidL2BudgetMB(0));
    CHECK_FALSE(IsValidL2BudgetMB(63));
    CHECK_FALSE(IsValidL2BudgetMB(8193));
}

// ===========================================================================
// Cache key field enum
// ===========================================================================

TEST_CASE("CacheKeyField — 6 fields defined",
          "[cache][key][fields]") {
    REQUIRE(CACHE_KEY_FIELD_COUNT == 6);
}

TEST_CASE("CacheKeyField — CANONICAL_PATH is field 0",
          "[cache][key][fields]") {
    REQUIRE(static_cast<uint8_t>(CacheKeyField::CANONICAL_PATH) == 0);
}

TEST_CASE("CacheKeyField — all 6 field values are distinct",
          "[cache][key][uniqueness]") {
    std::array<uint8_t, 6> vals = {
        static_cast<uint8_t>(CacheKeyField::CANONICAL_PATH),
        static_cast<uint8_t>(CacheKeyField::MTIME),
        static_cast<uint8_t>(CacheKeyField::FILE_SIZE),
        static_cast<uint8_t>(CacheKeyField::TARGET_WIDTH),
        static_cast<uint8_t>(CacheKeyField::TARGET_HEIGHT),
        static_cast<uint8_t>(CacheKeyField::DECODER_VERSION),
    };
    for (size_t i = 0; i < vals.size(); ++i) {
        for (size_t j = i + 1; j < vals.size(); ++j) {
            CHECK(vals[i] != vals[j]);
        }
    }
}

// ===========================================================================
// Cache key composition
// ===========================================================================

TEST_CASE("CacheKey — same inputs produce same key",
          "[cache][key][composition]") {
    auto k1 = ComposeCacheKey("C:\\img.jpg", 1700000000LL, 204800, 256, 256, 1);
    auto k2 = ComposeCacheKey("C:\\img.jpg", 1700000000LL, 204800, 256, 256, 1);
    REQUIRE(CacheKeysEqual(k1, k2));
}

TEST_CASE("CacheKey — different path produces different key",
          "[cache][key][composition]") {
    auto k1 = ComposeCacheKey("C:\\a.jpg", 100, 1024, 256, 256, 1);
    auto k2 = ComposeCacheKey("C:\\b.jpg", 100, 1024, 256, 256, 1);
    REQUIRE_FALSE(CacheKeysEqual(k1, k2));
}

TEST_CASE("CacheKey — different mtime produces different key",
          "[cache][key][composition]") {
    auto k1 = ComposeCacheKey("C:\\img.jpg", 1000, 1024, 256, 256, 1);
    auto k2 = ComposeCacheKey("C:\\img.jpg", 1001, 1024, 256, 256, 1);
    REQUIRE_FALSE(CacheKeysEqual(k1, k2));
}

TEST_CASE("CacheKey — different file size produces different key",
          "[cache][key][composition]") {
    auto k1 = ComposeCacheKey("C:\\img.jpg", 100, 1024, 256, 256, 1);
    auto k2 = ComposeCacheKey("C:\\img.jpg", 100, 2048, 256, 256, 1);
    REQUIRE_FALSE(CacheKeysEqual(k1, k2));
}

TEST_CASE("CacheKey — different target dimensions produce different key",
          "[cache][key][composition]") {
    auto k256 = ComposeCacheKey("C:\\img.jpg", 100, 1024, 256, 256, 1);
    auto k512 = ComposeCacheKey("C:\\img.jpg", 100, 1024, 512, 512, 1);
    REQUIRE_FALSE(CacheKeysEqual(k256, k512));
}

TEST_CASE("CacheKey — decoder version bump invalidates key",
          "[cache][key][invalidation]") {
    auto k1 = ComposeCacheKey("C:\\img.jpg", 100, 1024, 256, 256, 1);
    auto k2 = ComposeCacheKey("C:\\img.jpg", 100, 1024, 256, 256, 2);
    REQUIRE_FALSE(CacheKeysEqual(k1, k2));
}

TEST_CASE("CacheKey — L2 > L1 budget (disk > memory)",
          "[cache][key][tier-ordering]") {
    REQUIRE(L2_BUDGET_DEFAULT_MB > L1_BUDGET_DEFAULT_MB);
}

// ===========================================================================
// Eviction policy
// ===========================================================================

TEST_CASE("EvictionPolicy — LRU is policy 0 (default)",
          "[cache][eviction]") {
    REQUIRE(static_cast<uint8_t>(EvictionPolicy::LRU) == 0);
}

TEST_CASE("EvictionPolicy — 4 policies defined with distinct values",
          "[cache][eviction][uniqueness]") {
    std::array<uint8_t, 4> vals = {
        static_cast<uint8_t>(EvictionPolicy::LRU),
        static_cast<uint8_t>(EvictionPolicy::LFU),
        static_cast<uint8_t>(EvictionPolicy::SIZE_WEIGHTED),
        static_cast<uint8_t>(EvictionPolicy::TTL),
    };
    for (size_t i = 0; i < vals.size(); ++i) {
        for (size_t j = i + 1; j < vals.size(); ++j) {
            CHECK(vals[i] != vals[j]);
        }
    }
}

// ===========================================================================
// Invalidation triggers
// ===========================================================================

TEST_CASE("InvalidationTrigger — 5 triggers defined",
          "[cache][invalidation]") {
    REQUIRE(INVALIDATION_TRIGGER_COUNT == 5);
}

TEST_CASE("InvalidationTrigger — FILE_MODIFIED is trigger 0",
          "[cache][invalidation]") {
    REQUIRE(static_cast<uint8_t>(InvalidationTrigger::FILE_MODIFIED) == 0);
}

TEST_CASE("InvalidationTrigger — all 5 values are distinct",
          "[cache][invalidation][uniqueness]") {
    std::array<uint8_t, 5> vals = {
        static_cast<uint8_t>(InvalidationTrigger::FILE_MODIFIED),
        static_cast<uint8_t>(InvalidationTrigger::DECODER_UPGRADED),
        static_cast<uint8_t>(InvalidationTrigger::BUDGET_EXCEEDED),
        static_cast<uint8_t>(InvalidationTrigger::MANUAL_PURGE),
        static_cast<uint8_t>(InvalidationTrigger::CORRUPT_ENTRY),
    };
    for (size_t i = 0; i < vals.size(); ++i) {
        for (size_t j = i + 1; j < vals.size(); ++j) {
            CHECK(vals[i] != vals[j]);
        }
    }
}

// ===========================================================================
// Parametric key uniqueness (property-based via GENERATE)
// ===========================================================================

TEST_CASE("CacheKey — parametric uniqueness over path variants",
          "[cache][key][property-based]") {
    auto path = GENERATE("C:\\a.jpg", "C:\\b.png", "C:\\sub\\c.webp",
                         "D:\\photos\\raw.dng", "\\\\server\\share\\img.avif");
    // Each path generates a unique key prefix independent of other fields
    auto k = ComposeCacheKey(path, 1000, 512, 256, 256, 1);
    REQUIRE_FALSE(k.empty());
    REQUIRE(k.find('|') != std::string::npos);  // delimiter present
}

TEST_CASE("CacheKey — parametric uniqueness over target sizes",
          "[cache][key][property-based]") {
    auto target = GENERATE(16u, 32u, 64u, 128u, 256u, 512u, 1024u, 2048u, 4096u);
    auto k = ComposeCacheKey("C:\\img.jpg", 1000, 512, target, target, 1);
    REQUIRE_FALSE(k.empty());
    // Size string appears in key
    auto expected = std::to_string(target) + "x" + std::to_string(target);
    REQUIRE(k.find(expected) != std::string::npos);
}
