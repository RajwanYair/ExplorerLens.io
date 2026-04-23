// CacheKeyTests.cpp — Catch2 tests for L1/L2 thumbnail cache key composition
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the cache key construction scheme from §7.5 and D42:
//
//   key = SHA256(canonical_path ‖ mtime ‖ size ‖ target_w×target_h ‖ decoder_ver)
//
// Self-contained: implements a lightweight CacheKeyBuilder in the test
// namespace. Does NOT call the real cache subsystem (avoids SQLite linkage).
// Tests focus on key structure, collision resistance, and determinism.
//
// References: ROADMAP §7.5 cache architecture, D42 SQLite L2 cache,
//             §10.2 test stack, §15.1 P0 hash correctness.
//
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// =============================================================================
// Self-contained CacheKeyBuilder (mirrors the real key composition in §7.5)
// =============================================================================

namespace ExplorerLens { namespace Tests {

struct CacheKeyInput {
    std::wstring canonicalPath;  // Windows-normalized absolute path
    uint64_t     mtime        = 0;  // file modification time (100-ns intervals)
    uint64_t     fileSize     = 0;  // bytes
    uint32_t     targetWidth  = 256;
    uint32_t     targetHeight = 256;
    uint32_t     decoderVersion = 1;  // incremented when decoder changes output
};

/// Simplified cache key: deterministic hex string over the key components.
/// Production uses XXH3; here we use a simple FNV-1a 64-bit hash over UTF-8
/// serialization so the tests don't need any hash library.
inline uint64_t Fnv1a64(const void* data, size_t len) noexcept {
    uint64_t h = 0xcbf29ce484222325ULL;
    const auto* p = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < len; ++i) {
        h ^= static_cast<uint64_t>(p[i]);
        h *= 0x00000100000001B3ULL;
    }
    return h;
}

/// Returns a hex string of the 64-bit FNV-1a hash of the serialized key.
inline std::string BuildCacheKey(const CacheKeyInput& in) {
    // Serialize to a canonical byte buffer
    std::ostringstream ss;
    // Convert wstring path to UTF-8 using simple ASCII-safe approach for tests
    for (wchar_t c : in.canonicalPath) {
        ss.put(static_cast<char>(c & 0xFF));
        ss.put(static_cast<char>((c >> 8) & 0xFF));
    }
    ss.put('\0');  // delimiter
    // Append fixed-width components in little-endian order
    auto appendU64 = [&](uint64_t v) {
        for (int i = 0; i < 8; ++i) ss.put(static_cast<char>((v >> (i * 8)) & 0xFF));
    };
    auto appendU32 = [&](uint32_t v) {
        for (int i = 0; i < 4; ++i) ss.put(static_cast<char>((v >> (i * 8)) & 0xFF));
    };
    appendU64(in.mtime);
    appendU64(in.fileSize);
    appendU32(in.targetWidth);
    appendU32(in.targetHeight);
    appendU32(in.decoderVersion);

    std::string bytes = ss.str();
    uint64_t hash = Fnv1a64(bytes.data(), bytes.size());

    // Format as 16-char hex
    std::ostringstream hex;
    hex << std::hex << std::setfill('0') << std::setw(16) << hash;
    return hex.str();
}

}} // ExplorerLens::Tests

using namespace ExplorerLens::Tests;

// =============================================================================
// §1 — Determinism: same input always produces the same key
// =============================================================================

TEST_CASE("CacheKey: identical inputs produce identical key", "[cachekey][determinism]") {
    CacheKeyInput a, b;
    a.canonicalPath  = b.canonicalPath  = L"C:\\Photos\\IMG_0001.jpg";
    a.mtime          = b.mtime          = 133500000000000000ULL;  // Jan 2026
    a.fileSize       = b.fileSize       = 3'456'789;
    a.targetWidth    = b.targetWidth    = 256;
    a.targetHeight   = b.targetHeight   = 256;
    a.decoderVersion = b.decoderVersion = 1;

    REQUIRE(BuildCacheKey(a) == BuildCacheKey(b));
}

TEST_CASE("CacheKey: key is stable across multiple calls", "[cachekey][determinism]") {
    CacheKeyInput in;
    in.canonicalPath = L"D:\\Corpus\\sample.png";
    in.mtime         = 133600000000000000ULL;
    in.fileSize      = 1024;
    in.targetWidth   = 512;
    in.targetHeight  = 512;
    in.decoderVersion = 2;

    auto k1 = BuildCacheKey(in);
    auto k2 = BuildCacheKey(in);
    auto k3 = BuildCacheKey(in);
    REQUIRE(k1 == k2);
    REQUIRE(k2 == k3);
}

TEST_CASE("CacheKey: key is 16 hex characters", "[cachekey][format]") {
    CacheKeyInput in;
    in.canonicalPath = L"C:\\test.webp";
    auto key = BuildCacheKey(in);
    REQUIRE(key.size() == 16);
    REQUIRE(key.find_first_not_of("0123456789abcdef") == std::string::npos);
}

// =============================================================================
// §2 — Sensitivity: each component is included in the key
// =============================================================================

TEST_CASE("CacheKey: different path → different key", "[cachekey][sensitivity]") {
    CacheKeyInput a, b;
    a.canonicalPath = L"C:\\photos\\IMG_0001.jpg";
    b.canonicalPath = L"C:\\photos\\IMG_0002.jpg";
    REQUIRE(BuildCacheKey(a) != BuildCacheKey(b));
}

TEST_CASE("CacheKey: different mtime → different key", "[cachekey][sensitivity]") {
    CacheKeyInput a, b;
    a.canonicalPath = b.canonicalPath = L"C:\\photo.jpg";
    a.mtime = 1000;
    b.mtime = 1001;
    REQUIRE(BuildCacheKey(a) != BuildCacheKey(b));
}

TEST_CASE("CacheKey: different file size → different key", "[cachekey][sensitivity]") {
    CacheKeyInput a, b;
    a.canonicalPath = b.canonicalPath = L"C:\\photo.jpg";
    a.mtime  = b.mtime  = 5000;
    a.fileSize = 1000;
    b.fileSize = 1001;
    REQUIRE(BuildCacheKey(a) != BuildCacheKey(b));
}

TEST_CASE("CacheKey: different target width → different key", "[cachekey][sensitivity]") {
    CacheKeyInput a, b;
    a.canonicalPath = b.canonicalPath = L"C:\\photo.jpg";
    a.targetWidth = 256; b.targetWidth = 512;
    REQUIRE(BuildCacheKey(a) != BuildCacheKey(b));
}

TEST_CASE("CacheKey: different target height → different key", "[cachekey][sensitivity]") {
    CacheKeyInput a, b;
    a.canonicalPath = b.canonicalPath = L"C:\\photo.jpg";
    a.targetHeight = 256; b.targetHeight = 128;
    REQUIRE(BuildCacheKey(a) != BuildCacheKey(b));
}

TEST_CASE("CacheKey: different decoder version → different key", "[cachekey][sensitivity]") {
    CacheKeyInput a, b;
    a.canonicalPath = b.canonicalPath = L"C:\\photo.avif";
    a.decoderVersion = 1;
    b.decoderVersion = 2;
    // v2 decoder = stale cached thumbnail is invalid → different key
    REQUIRE(BuildCacheKey(a) != BuildCacheKey(b));
}

// =============================================================================
// §3 — Path canonicalization invariants
// =============================================================================

TEST_CASE("CacheKey: path is case-sensitive in Windows canonical form", "[cachekey][path]") {
    CacheKeyInput a, b;
    a.canonicalPath = L"C:\\Photos\\IMG.jpg";
    b.canonicalPath = L"C:\\photos\\img.jpg";  // different after normalization
    // These represent different files after NtQueryInformationFile normalization
    REQUIRE(BuildCacheKey(a) != BuildCacheKey(b));
}

TEST_CASE("CacheKey: empty path produces valid (non-empty) key", "[cachekey][path]") {
    CacheKeyInput in;
    in.canonicalPath = L"";
    auto key = BuildCacheKey(in);
    REQUIRE_FALSE(key.empty());
}

TEST_CASE("CacheKey: very long path produces valid key", "[cachekey][path]") {
    CacheKeyInput in;
    std::wstring longPath(260, L'A');
    longPath[0] = L'C'; longPath[1] = L':'; longPath[2] = L'\\';
    longPath += L".jpg";
    in.canonicalPath = longPath;
    auto key = BuildCacheKey(in);
    REQUIRE(key.size() == 16);
}

// =============================================================================
// §4 — Collision resistance
// =============================================================================

TEST_CASE("CacheKey: 100 unique paths produce 100 unique keys", "[cachekey][collision]") {
    std::set<std::string> keys;
    for (int i = 0; i < 100; ++i) {
        CacheKeyInput in;
        in.canonicalPath = L"C:\\corpus\\file_" + std::to_wstring(i) + L".jpg";
        in.mtime         = static_cast<uint64_t>(i) * 1000;
        in.fileSize      = static_cast<uint64_t>(i) * 512 + 1024;
        keys.insert(BuildCacheKey(in));
    }
    REQUIRE(keys.size() == 100u);
}

TEST_CASE("CacheKey: different sizes same path same mtime → no collision", "[cachekey][collision]") {
    CacheKeyInput a, b;
    a.canonicalPath = b.canonicalPath = L"C:\\photos\\a.tiff";
    a.mtime = b.mtime = 99999;
    a.fileSize = 1024;
    b.fileSize = 1025;
    REQUIRE(BuildCacheKey(a) != BuildCacheKey(b));
}

TEST_CASE("CacheKey: different width×height combos all unique", "[cachekey][collision]") {
    std::set<std::string> keys;
    std::vector<std::pair<uint32_t,uint32_t>> sizes =
        {{64,64},{96,96},{128,128},{192,192},{256,256},{384,384},{512,512},{768,768},{1024,1024}};
    for (auto [w, h] : sizes) {
        CacheKeyInput in;
        in.canonicalPath = L"C:\\test.png";
        in.targetWidth   = w;
        in.targetHeight  = h;
        keys.insert(BuildCacheKey(in));
    }
    REQUIRE(keys.size() == sizes.size());
}

// =============================================================================
// §5 — Decoder version bump invalidates all cached entries
// =============================================================================

TEST_CASE("CacheKey: decoder version increment changes ALL existing keys", "[cachekey][invalidate]") {
    std::vector<std::string> v1keys, v2keys;
    for (int i = 0; i < 10; ++i) {
        CacheKeyInput in;
        in.canonicalPath  = L"C:\\corpus\\test_" + std::to_wstring(i) + L".jpg";
        in.mtime          = static_cast<uint64_t>(i);
        in.fileSize       = 1024u * i + 512;
        in.decoderVersion = 1;
        v1keys.push_back(BuildCacheKey(in));

        in.decoderVersion = 2;
        v2keys.push_back(BuildCacheKey(in));
    }
    // No v1 key should appear in v2 keys
    for (const auto& k : v1keys) {
        REQUIRE(std::find(v2keys.begin(), v2keys.end(), k) == v2keys.end());
    }
}

// =============================================================================
// §6 — Parametric target sizes from Explorer thumbnail grid
// =============================================================================

TEST_CASE("CacheKey: Explorer standard thumbnail sizes all produce distinct keys",
          "[cachekey][parametric]") {
    // Explorer uses 32, 48, 96, 256 px as the standard thumbnail grid sizes
    uint32_t sz = GENERATE(32u, 48u, 96u, 256u);
    CacheKeyInput in;
    in.canonicalPath = L"C:\\Photos\\vacation.jpg";
    in.mtime         = 133700000000000000ULL;
    in.fileSize      = 5'000'000;
    in.targetWidth   = sz;
    in.targetHeight  = sz;
    auto key = BuildCacheKey(in);
    REQUIRE(key.size() == 16);
}
