// CompressionAlgorithmTests.cpp — Catch2 tests for compression algorithm contracts
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the compression algorithm identifier contracts for P1 archive
// formats from §7.3.
// Tests cover: 6 algorithm enum values (ZLIB/LZ4/ZSTD/LZMA/BZIP2/XZ),
// distinctness, algorithm name non-empty constraints, compression ratio
// ordering (XZ ≥ LZMA ≥ ZSTD > BZIP2 > ZLIB > LZ4), speed ordering
// (LZ4 fastest, XZ slowest), library version assignments, and the
// use-case mapping (archive formats to primary compression algorithm).
//
// All tests are self-contained — no Engine headers included.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstdint>
#include <string_view>

// ---------------------------------------------------------------------------
// Compression algorithm model (§7.3 P1 archives)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::CompressionAlgorithm {

// ── Algorithm identifiers ──────────────────────────────────────────────────

enum class CompressionAlgo : uint8_t {
    ZLIB  = 0,
    LZ4   = 1,
    ZSTD  = 2,
    LZMA  = 3,
    BZIP2 = 4,
    XZ    = 5,
};

static constexpr int ALGORITHM_COUNT = 6;

// ── Algorithm descriptor ───────────────────────────────────────────────────

struct AlgorithmInfo {
    CompressionAlgo  algo;
    std::string_view name;           // Lowercase short name
    std::string_view library;        // Backing library + version
    uint8_t          compressionRank; // 1=best, 6=worst (proxy for ratio)
    uint8_t          speedRank;       // 1=fastest, 6=slowest
};

// ── Algorithm registry ─────────────────────────────────────────────────────
// compressionRank: 1=XZ (best ratio), 6=LZ4 (worst ratio)
// speedRank:       1=LZ4 (fastest), 6=XZ (slowest)

static constexpr std::array<AlgorithmInfo, 6> ALGORITHM_REGISTRY = {{
    { CompressionAlgo::ZLIB,  "zlib",  "zlib-1.3.1",         5, 3 },
    { CompressionAlgo::LZ4,   "lz4",   "LZ4-1.10.0",         6, 1 },
    { CompressionAlgo::ZSTD,  "zstd",  "zstd-1.5.7",         3, 2 },
    { CompressionAlgo::LZMA,  "lzma",  "LZMA-SDK-26.00",     2, 5 },
    { CompressionAlgo::BZIP2, "bzip2", "bzip2-1.0.8",        4, 4 },
    { CompressionAlgo::XZ,    "xz",    "LZMA-SDK-26.00",     1, 6 },
}};

// ── Archive format → primary algorithm mapping ─────────────────────────────

struct FormatAlgoMapping {
    std::string_view archiveFormat;
    CompressionAlgo  primaryAlgo;
};

static constexpr std::array<FormatAlgoMapping, 6> FORMAT_ALGO_MAP = {{
    { "ZIP",  CompressionAlgo::ZLIB  },
    { "7Z",   CompressionAlgo::LZMA  },
    { "CBZ",  CompressionAlgo::ZLIB  },
    { "EPUB", CompressionAlgo::ZLIB  },
    { "RAR",  CompressionAlgo::BZIP2 },
    { "TAR",  CompressionAlgo::ZSTD  },
}};

} // namespace ExplorerLens::Tests::CompressionAlgorithm

using namespace ExplorerLens::Tests::CompressionAlgorithm;

// ===========================================================================
// Algorithm count and enum values
// ===========================================================================

TEST_CASE("CompressionAlgo — 6 algorithms defined",
          "[compression][count]") {
    REQUIRE(ALGORITHM_COUNT == 6);
    REQUIRE(ALGORITHM_REGISTRY.size() == 6u);
}

TEST_CASE("CompressionAlgo — ZLIB is 0",  "[compression][enum]") { REQUIRE(static_cast<uint8_t>(CompressionAlgo::ZLIB)  == 0); }
TEST_CASE("CompressionAlgo — LZ4 is 1",   "[compression][enum]") { REQUIRE(static_cast<uint8_t>(CompressionAlgo::LZ4)   == 1); }
TEST_CASE("CompressionAlgo — ZSTD is 2",  "[compression][enum]") { REQUIRE(static_cast<uint8_t>(CompressionAlgo::ZSTD)  == 2); }
TEST_CASE("CompressionAlgo — LZMA is 3",  "[compression][enum]") { REQUIRE(static_cast<uint8_t>(CompressionAlgo::LZMA)  == 3); }
TEST_CASE("CompressionAlgo — BZIP2 is 4", "[compression][enum]") { REQUIRE(static_cast<uint8_t>(CompressionAlgo::BZIP2) == 4); }
TEST_CASE("CompressionAlgo — XZ is 5",    "[compression][enum]") { REQUIRE(static_cast<uint8_t>(CompressionAlgo::XZ)    == 5); }

TEST_CASE("CompressionAlgo — all 6 enum values are distinct",
          "[compression][enum][uniqueness]") {
    std::array<uint8_t, 6> vals = {
        static_cast<uint8_t>(CompressionAlgo::ZLIB),
        static_cast<uint8_t>(CompressionAlgo::LZ4),
        static_cast<uint8_t>(CompressionAlgo::ZSTD),
        static_cast<uint8_t>(CompressionAlgo::LZMA),
        static_cast<uint8_t>(CompressionAlgo::BZIP2),
        static_cast<uint8_t>(CompressionAlgo::XZ),
    };
    for (size_t i = 0; i < vals.size(); ++i) {
        for (size_t j = i + 1; j < vals.size(); ++j) {
            CHECK(vals[i] != vals[j]);
        }
    }
}

// ===========================================================================
// Algorithm names
// ===========================================================================

TEST_CASE("AlgorithmNames — all 6 names are non-empty",
          "[compression][names]") {
    for (const auto& a : ALGORITHM_REGISTRY) {
        REQUIRE_FALSE(a.name.empty());
    }
}

TEST_CASE("AlgorithmNames — all names are lowercase (no uppercase chars)",
          "[compression][names]") {
    for (const auto& a : ALGORITHM_REGISTRY) {
        for (char c : a.name) {
            INFO("Algo: " << a.name << " char: " << c);
            CHECK_FALSE((c >= 'A' && c <= 'Z'));
        }
    }
}

TEST_CASE("AlgorithmNames — all 6 names are unique",
          "[compression][names][uniqueness]") {
    for (size_t i = 0; i < ALGORITHM_REGISTRY.size(); ++i) {
        for (size_t j = i + 1; j < ALGORITHM_REGISTRY.size(); ++j) {
            CHECK(ALGORITHM_REGISTRY[i].name != ALGORITHM_REGISTRY[j].name);
        }
    }
}

// ===========================================================================
// Library assignments
// ===========================================================================

TEST_CASE("AlgorithmLibraries — ZLIB uses zlib-1.3.1",
          "[compression][library]") {
    for (const auto& a : ALGORITHM_REGISTRY) {
        if (a.algo == CompressionAlgo::ZLIB) {
            CHECK(a.library.find("zlib-1.3.1") != std::string_view::npos);
        }
    }
}

TEST_CASE("AlgorithmLibraries — LZ4 uses LZ4-1.10.0",
          "[compression][library]") {
    for (const auto& a : ALGORITHM_REGISTRY) {
        if (a.algo == CompressionAlgo::LZ4) {
            CHECK(a.library.find("LZ4-1.10.0") != std::string_view::npos);
        }
    }
}

TEST_CASE("AlgorithmLibraries — ZSTD uses zstd-1.5.7",
          "[compression][library]") {
    for (const auto& a : ALGORITHM_REGISTRY) {
        if (a.algo == CompressionAlgo::ZSTD) {
            CHECK(a.library.find("zstd-1.5.7") != std::string_view::npos);
        }
    }
}

TEST_CASE("AlgorithmLibraries — LZMA uses LZMA-SDK-26.00",
          "[compression][library]") {
    for (const auto& a : ALGORITHM_REGISTRY) {
        if (a.algo == CompressionAlgo::LZMA) {
            CHECK(a.library.find("LZMA-SDK-26.00") != std::string_view::npos);
        }
    }
}

TEST_CASE("AlgorithmLibraries — XZ uses LZMA-SDK-26.00 (same SDK as LZMA)",
          "[compression][library]") {
    for (const auto& a : ALGORITHM_REGISTRY) {
        if (a.algo == CompressionAlgo::XZ) {
            CHECK(a.library.find("LZMA-SDK-26.00") != std::string_view::npos);
        }
    }
}

TEST_CASE("AlgorithmLibraries — all 6 library strings are non-empty",
          "[compression][library]") {
    for (const auto& a : ALGORITHM_REGISTRY) {
        REQUIRE_FALSE(a.library.empty());
    }
}

// ===========================================================================
// Compression ratio ordering (rank 1=best, 6=worst)
// ===========================================================================

TEST_CASE("CompressionRatio — XZ has best ratio (rank 1)",
          "[compression][ratio]") {
    for (const auto& a : ALGORITHM_REGISTRY) {
        if (a.algo == CompressionAlgo::XZ) {
            REQUIRE(a.compressionRank == 1);
        }
    }
}

TEST_CASE("CompressionRatio — LZ4 has worst ratio (rank 6)",
          "[compression][ratio]") {
    for (const auto& a : ALGORITHM_REGISTRY) {
        if (a.algo == CompressionAlgo::LZ4) {
            REQUIRE(a.compressionRank == 6);
        }
    }
}

TEST_CASE("CompressionRatio — XZ rank <= LZMA rank (XZ ≥ LZMA ratio)",
          "[compression][ratio][ordering]") {
    uint8_t xzRank = 0, lzmaRank = 0;
    for (const auto& a : ALGORITHM_REGISTRY) {
        if (a.algo == CompressionAlgo::XZ)   xzRank   = a.compressionRank;
        if (a.algo == CompressionAlgo::LZMA) lzmaRank = a.compressionRank;
    }
    REQUIRE(xzRank <= lzmaRank);
}

TEST_CASE("CompressionRatio — LZMA rank < ZSTD rank (LZMA > ZSTD ratio)",
          "[compression][ratio][ordering]") {
    uint8_t lzmaRank = 0, zstdRank = 0;
    for (const auto& a : ALGORITHM_REGISTRY) {
        if (a.algo == CompressionAlgo::LZMA) lzmaRank = a.compressionRank;
        if (a.algo == CompressionAlgo::ZSTD) zstdRank = a.compressionRank;
    }
    REQUIRE(lzmaRank < zstdRank);
}

TEST_CASE("CompressionRatio — ZSTD rank < BZIP2 rank (ZSTD > BZIP2 ratio)",
          "[compression][ratio][ordering]") {
    uint8_t zstdRank = 0, bzip2Rank = 0;
    for (const auto& a : ALGORITHM_REGISTRY) {
        if (a.algo == CompressionAlgo::ZSTD)  zstdRank  = a.compressionRank;
        if (a.algo == CompressionAlgo::BZIP2) bzip2Rank = a.compressionRank;
    }
    REQUIRE(zstdRank < bzip2Rank);
}

TEST_CASE("CompressionRatio — BZIP2 rank < ZLIB rank (BZIP2 > ZLIB ratio)",
          "[compression][ratio][ordering]") {
    uint8_t bzip2Rank = 0, zlibRank = 0;
    for (const auto& a : ALGORITHM_REGISTRY) {
        if (a.algo == CompressionAlgo::BZIP2) bzip2Rank = a.compressionRank;
        if (a.algo == CompressionAlgo::ZLIB)  zlibRank  = a.compressionRank;
    }
    REQUIRE(bzip2Rank < zlibRank);
}

TEST_CASE("CompressionRatio — ZLIB rank < LZ4 rank (ZLIB > LZ4 ratio)",
          "[compression][ratio][ordering]") {
    uint8_t zlibRank = 0, lz4Rank = 0;
    for (const auto& a : ALGORITHM_REGISTRY) {
        if (a.algo == CompressionAlgo::ZLIB) zlibRank = a.compressionRank;
        if (a.algo == CompressionAlgo::LZ4)  lz4Rank  = a.compressionRank;
    }
    REQUIRE(zlibRank < lz4Rank);
}

// ===========================================================================
// Speed ordering (rank 1=fastest, 6=slowest)
// ===========================================================================

TEST_CASE("CompressionSpeed — LZ4 is fastest (rank 1)",
          "[compression][speed]") {
    for (const auto& a : ALGORITHM_REGISTRY) {
        if (a.algo == CompressionAlgo::LZ4) {
            REQUIRE(a.speedRank == 1);
        }
    }
}

TEST_CASE("CompressionSpeed — XZ is slowest (rank 6)",
          "[compression][speed]") {
    for (const auto& a : ALGORITHM_REGISTRY) {
        if (a.algo == CompressionAlgo::XZ) {
            REQUIRE(a.speedRank == 6);
        }
    }
}

TEST_CASE("CompressionSpeed — LZ4 faster than ZSTD (rank 1 < 2)",
          "[compression][speed][ordering]") {
    uint8_t lz4Speed = 0, zstdSpeed = 0;
    for (const auto& a : ALGORITHM_REGISTRY) {
        if (a.algo == CompressionAlgo::LZ4)  lz4Speed  = a.speedRank;
        if (a.algo == CompressionAlgo::ZSTD) zstdSpeed = a.speedRank;
    }
    REQUIRE(lz4Speed < zstdSpeed);
}

// ===========================================================================
// Archive format → algorithm mapping
// ===========================================================================

TEST_CASE("FormatAlgoMap — ZIP uses ZLIB",
          "[compression][mapping]") {
    for (const auto& m : FORMAT_ALGO_MAP) {
        if (m.archiveFormat == "ZIP") {
            CHECK(m.primaryAlgo == CompressionAlgo::ZLIB);
        }
    }
}

TEST_CASE("FormatAlgoMap — 7Z uses LZMA",
          "[compression][mapping]") {
    for (const auto& m : FORMAT_ALGO_MAP) {
        if (m.archiveFormat == "7Z") {
            CHECK(m.primaryAlgo == CompressionAlgo::LZMA);
        }
    }
}

TEST_CASE("FormatAlgoMap — 6 format mappings defined",
          "[compression][mapping]") {
    REQUIRE(FORMAT_ALGO_MAP.size() == 6u);
}

// ===========================================================================
// Parametric: each algorithm rank pair is valid (rank 1–6)
// ===========================================================================

TEST_CASE("AlgorithmRanks — all compressionRank values are in 1–6",
          "[compression][rank][property-based]") {
    for (const auto& a : ALGORITHM_REGISTRY) {
        INFO("Algo: " << a.name);
        CHECK(a.compressionRank >= 1u);
        CHECK(a.compressionRank <= 6u);
    }
}

TEST_CASE("AlgorithmRanks — all speedRank values are in 1–6",
          "[compression][rank][property-based]") {
    for (const auto& a : ALGORITHM_REGISTRY) {
        INFO("Algo: " << a.name);
        CHECK(a.speedRank >= 1u);
        CHECK(a.speedRank <= 6u);
    }
}
