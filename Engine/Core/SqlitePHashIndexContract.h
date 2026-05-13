// SqlitePHashIndexContract.h -- S292 / ROADMAP v6.0 H14 Phase 4
// pHash (perceptual hash) SQLite index policy for the L2 thumbnail cache.
//
// Enables deduplication: thumbnails with Hamming distance <= threshold
// are considered visually identical and the cached version is reused.
// Uses the DCT-based pHash column in the SQLite schema (§12.2).
//
// Rule: this is a contract header — no implementation, no Win32 includes.
// All types are in namespace ExplorerLens::Engine.

#pragma once
#include <cstdint>
#include <cstddef>

namespace ExplorerLens {
namespace Engine {

// ── Hamming distance threshold ───────────────────────────────────────────────
// 0  = exact match only (conservative)
// 10 = ~84% similar (recommended default)
// 20 = ~69% similar (aggressive dedup, more false positives)

static constexpr uint32_t kSqlitePHashHammingThreshold     = 10;
static constexpr uint32_t kSqlitePHashHardMaxThreshold      = 32;  // Never exceed
static constexpr uint32_t kSqlitePHashBitDepth              = 64;  // 64-bit DCT hash

// ── Algorithm selection ──────────────────────────────────────────────────────

enum class SqlitePHashAlgorithm : uint8_t {
    DCT_PHASH       = 0,  // Discrete Cosine Transform — most robust (default)
    PHASH_DIFFERENCE = 1, // dHash — fast, good for near-duplicates
    AVERAGE          = 2, // aHash — simplest, most false positives
};

// ── Index entry (as stored in SQLite column) ─────────────────────────────────

struct SqlitePHashEntry {
    uint64_t          hash;           // 64-bit perceptual hash value
    SqlitePHashAlgorithm algorithm;
    uint8_t           confidence;     // 0–100; <50 = skip dedup
};

// ── Policy ───────────────────────────────────────────────────────────────────

struct SqlitePHashIndexPolicy {
    SqlitePHashAlgorithm algorithm       = SqlitePHashAlgorithm::DCT_PHASH;
    uint32_t             hammingThreshold = kSqlitePHashHammingThreshold;
    uint8_t              minConfidence    = 70;   // Skip if confidence < 70
    bool                 enableDedup      = true;
    bool                 indexEnabled     = true; // CREATE INDEX on phash column
};

// ── Probe (for diagnostics / LENSManager panel) ──────────────────────────────

struct SqlitePHashIndexProbe {
    uint64_t totalHashed      = 0;
    uint64_t dedupHits        = 0;
    uint64_t dedupMisses      = 0;
    uint64_t indexSizeBytes   = 0;
    double   avgHammingDist   = 0.0;
};

} // namespace Engine
} // namespace ExplorerLens
