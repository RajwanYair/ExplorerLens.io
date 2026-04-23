// SecurityBoundaryTests.cpp — Catch2 tests for input-validation security boundaries
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the security boundary constants from §15.1 (OWASP A1/A3/A5):
//   - Zip-bomb / archive-bomb threshold (max uncompressed ratio / size)
//   - Nested archive depth limit
//   - Pixel dimension overflow guard (SafeInt model)
//   - Stream read size ceiling (DoS prevention)
//   - Header probe byte limit before rejection
//   - Compression ratio red-flag threshold
//   - Path normalization and traversal rejection model
//   - Resource-exhaustion pixel-budget guard
//
// All tests are self-contained — no Windows headers, no Engine headers.
// Aligns with OWASP Top-10 A1 (Injection), A5 (Security Misconfiguration),
// and CWE-400 (Resource Exhaustion), CWE-23 (Path Traversal).
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <cstdint>
#include <optional>
#include <string_view>
#include <string>

// ---------------------------------------------------------------------------
// Security boundary constants (§15.1, OWASP, CWE)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::SecurityBoundary {

// ── Archive bomb limits ─────────────────────────────────────────────────────

/// Maximum uncompressed size of any single archive entry (1 GiB)
static constexpr uint64_t MAX_ARCHIVE_ENTRY_BYTES   = 1ULL * 1024 * 1024 * 1024;

/// Maximum total uncompressed content of an archive (4 GiB)
static constexpr uint64_t MAX_ARCHIVE_TOTAL_BYTES   = 4ULL * 1024 * 1024 * 1024;

/// Maximum nested archive depth (zip-in-zip recursion)
static constexpr uint32_t MAX_ARCHIVE_DEPTH         = 8;

/// Compression ratio that flags a potential zip-bomb (ratio > threshold)
/// A ratio of >100:1 is suspicious; >1000:1 is almost certainly malicious
static constexpr float    ZIPBOMB_RATIO_THRESHOLD   = 100.0f;

/// Maximum number of entries in a single archive before truncation
static constexpr uint32_t MAX_ARCHIVE_ENTRIES       = 100'000;

// ── Dimension overflow guards ───────────────────────────────────────────────

/// Maximum pixel width or height accepted from any format header (§15.1 SafeInt)
static constexpr uint32_t MAX_IMAGE_DIM_PX          = 65'535;

/// Maximum total pixel count (width * height) before decode is rejected
/// 65535 * 65535 exceeds uint32_t — use uint64_t; 500 MP soft-cap enforced
static constexpr uint64_t MAX_PIXEL_COUNT           = 500ULL * 1000 * 1000;

/// Maximum image memory footprint in bytes (4 bytes/px BGRA32)
static constexpr uint64_t MAX_IMAGE_MEMORY_BYTES    = 2ULL * 1024 * 1024 * 1024;

// ── Stream / I/O limits ─────────────────────────────────────────────────────

/// Maximum single stream read call size (256 MiB)
static constexpr uint64_t MAX_STREAM_READ_BYTES     = 256ULL * 1024 * 1024;

/// Maximum header probe window before rejection (1 MiB)
static constexpr uint64_t MAX_HEADER_PROBE_BYTES    = 1ULL * 1024 * 1024;

/// Maximum file size we will attempt to decode (2 GiB)
static constexpr uint64_t MAX_FILE_SIZE_BYTES       = 2ULL * 1024 * 1024 * 1024;

// ── Path traversal prevention ────────────────────────────────────────────────

/// Returns true if a path component contains a traversal sequence
inline bool ContainsTraversal(std::string_view path) {
    // Reject any path containing .. component
    if (path.find("..") != std::string_view::npos) return true;
    return false;
}

/// Returns true if a normalised path is safe (no traversal, no absolute prefix)
inline bool IsNormalisedPathSafe(std::string_view normPath) {
    if (normPath.empty()) return false;
    if (normPath.front() == '/') return false;   // no leading slash
    if (normPath.front() == '\\') return false;  // no leading backslash
    if (normPath.size() >= 2 && normPath[1] == ':') return false; // no drive letter
    if (ContainsTraversal(normPath)) return false;
    return true;
}

// ── Pixel budget guard ───────────────────────────────────────────────────────

/// Returns true when decoded pixel count would exceed the memory budget
inline bool ExceedsPixelBudget(uint64_t width, uint64_t height) {
    if (width == 0 || height == 0) return false;  // zero-dim is rejected elsewhere
    // Overflow-safe: use saturation arithmetic
    if (width > MAX_IMAGE_DIM_PX || height > MAX_IMAGE_DIM_PX) return true;
    uint64_t pixels = width * height;
    if (pixels > MAX_PIXEL_COUNT) return true;
    // Memory footprint: 4 bytes per BGRA32 pixel
    uint64_t mem = pixels * 4;
    return mem > MAX_IMAGE_MEMORY_BYTES;
}

/// Returns true when compression ratio indicates archive bomb
inline bool IsZipBombRatio(uint64_t compressed, uint64_t uncompressed) {
    if (compressed == 0) return true;  // zero compressed = definitely suspicious
    float ratio = static_cast<float>(uncompressed) / static_cast<float>(compressed);
    return ratio > ZIPBOMB_RATIO_THRESHOLD;
}

} // namespace ExplorerLens::Tests::SecurityBoundary

using namespace ExplorerLens::Tests::SecurityBoundary;

// ===========================================================================
// Archive bomb limits
// ===========================================================================

TEST_CASE("SecurityBoundary — max archive entry size is 1 GiB",
          "[security][archive][bomb]") {
    REQUIRE(MAX_ARCHIVE_ENTRY_BYTES == 1ULL * 1024 * 1024 * 1024);
}

TEST_CASE("SecurityBoundary — max archive total size is 4 GiB",
          "[security][archive][bomb]") {
    REQUIRE(MAX_ARCHIVE_TOTAL_BYTES == 4ULL * 1024 * 1024 * 1024);
}

TEST_CASE("SecurityBoundary — total size limit is >= entry limit",
          "[security][archive][bomb]") {
    REQUIRE(MAX_ARCHIVE_TOTAL_BYTES >= MAX_ARCHIVE_ENTRY_BYTES);
}

TEST_CASE("SecurityBoundary — max nested archive depth is 8",
          "[security][archive][depth]") {
    REQUIRE(MAX_ARCHIVE_DEPTH == 8u);
}

TEST_CASE("SecurityBoundary — zip-bomb ratio threshold is 100:1",
          "[security][archive][bomb]") {
    REQUIRE(ZIPBOMB_RATIO_THRESHOLD == 100.0f);
}

TEST_CASE("SecurityBoundary — max archive entries is 100,000",
          "[security][archive]") {
    REQUIRE(MAX_ARCHIVE_ENTRIES == 100'000u);
}

// ===========================================================================
// IsZipBombRatio
// ===========================================================================

TEST_CASE("IsZipBombRatio — 1:1 ratio is safe",
          "[security][zipbomb]") {
    REQUIRE_FALSE(IsZipBombRatio(1000, 1000));
}

TEST_CASE("IsZipBombRatio — 50:1 ratio is safe (below threshold)",
          "[security][zipbomb]") {
    REQUIRE_FALSE(IsZipBombRatio(1000, 50'000));
}

TEST_CASE("IsZipBombRatio — exactly 100:1 is NOT flagged (threshold is >100)",
          "[security][zipbomb]") {
    // 100 * 1000 = 100000 uncompressed, ratio == 100.0 exactly
    REQUIRE_FALSE(IsZipBombRatio(1000, 100'000));
}

TEST_CASE("IsZipBombRatio — 101:1 ratio is flagged",
          "[security][zipbomb]") {
    REQUIRE(IsZipBombRatio(1000, 101'000));
}

TEST_CASE("IsZipBombRatio — 1000:1 ratio (classic zip bomb) is flagged",
          "[security][zipbomb]") {
    // zip-bomb test: 1 byte → 1 MB
    REQUIRE(IsZipBombRatio(1, 1'000'000));
}

TEST_CASE("IsZipBombRatio — zero compressed size is flagged",
          "[security][zipbomb]") {
    REQUIRE(IsZipBombRatio(0, 0));
    REQUIRE(IsZipBombRatio(0, 1000));
}

// ===========================================================================
// Dimension overflow guards
// ===========================================================================

TEST_CASE("SecurityBoundary — max image dimension is 65535 px",
          "[security][dimension]") {
    REQUIRE(MAX_IMAGE_DIM_PX == 65'535u);
}

TEST_CASE("SecurityBoundary — max pixel count is 500 megapixels",
          "[security][dimension]") {
    REQUIRE(MAX_PIXEL_COUNT == 500ULL * 1'000'000);
}

TEST_CASE("ExceedsPixelBudget — normal image (1920×1080) is safe",
          "[security][dimension][budget]") {
    REQUIRE_FALSE(ExceedsPixelBudget(1920, 1080));
}

TEST_CASE("ExceedsPixelBudget — 4K (3840×2160) is safe",
          "[security][dimension][budget]") {
    REQUIRE_FALSE(ExceedsPixelBudget(3840, 2160));
}

TEST_CASE("ExceedsPixelBudget — 16K (15360×8640) is safe under 500 MP",
          "[security][dimension][budget]") {
    // 15360 * 8640 = 132,710,400 — under 500 MP
    REQUIRE_FALSE(ExceedsPixelBudget(15360, 8640));
}

TEST_CASE("ExceedsPixelBudget — dimension > 65535 is rejected",
          "[security][dimension][budget]") {
    REQUIRE(ExceedsPixelBudget(65'536, 100));
    REQUIRE(ExceedsPixelBudget(100, 65'536));
    REQUIRE(ExceedsPixelBudget(65'536, 65'536));
}

TEST_CASE("ExceedsPixelBudget — 30000×30000 (900 MP) exceeds limit",
          "[security][dimension][budget]") {
    // 30000 * 30000 = 900,000,000 > 500,000,000
    REQUIRE(ExceedsPixelBudget(30'000, 30'000));
}

TEST_CASE("ExceedsPixelBudget — zero dimension does not trigger budget",
          "[security][dimension][budget]") {
    // Zero dimensions are handled elsewhere; budget guard returns false
    REQUIRE_FALSE(ExceedsPixelBudget(0, 1000));
    REQUIRE_FALSE(ExceedsPixelBudget(1000, 0));
}

// ===========================================================================
// Stream / file size limits
// ===========================================================================

TEST_CASE("SecurityBoundary — max single stream read is 256 MiB",
          "[security][stream]") {
    REQUIRE(MAX_STREAM_READ_BYTES == 256ULL * 1024 * 1024);
}

TEST_CASE("SecurityBoundary — max header probe is 1 MiB",
          "[security][stream]") {
    REQUIRE(MAX_HEADER_PROBE_BYTES == 1ULL * 1024 * 1024);
}

TEST_CASE("SecurityBoundary — max file size is 2 GiB",
          "[security][stream]") {
    REQUIRE(MAX_FILE_SIZE_BYTES == 2ULL * 1024 * 1024 * 1024);
}

TEST_CASE("SecurityBoundary — header probe limit < stream read limit",
          "[security][stream]") {
    REQUIRE(MAX_HEADER_PROBE_BYTES < MAX_STREAM_READ_BYTES);
}

TEST_CASE("SecurityBoundary — stream read limit < file size limit",
          "[security][stream]") {
    REQUIRE(MAX_STREAM_READ_BYTES < MAX_FILE_SIZE_BYTES);
}

// ===========================================================================
// Path traversal prevention (CWE-23)
// ===========================================================================

TEST_CASE("ContainsTraversal — normal relative path is safe",
          "[security][path][traversal]") {
    REQUIRE_FALSE(ContainsTraversal("images/photo.jpg"));
    REQUIRE_FALSE(ContainsTraversal("folder/subfolder/file.png"));
}

TEST_CASE("ContainsTraversal — double-dot traversal is detected",
          "[security][path][traversal]") {
    REQUIRE(ContainsTraversal("../etc/passwd"));
    REQUIRE(ContainsTraversal("folder/../../../secret"));
    REQUIRE(ContainsTraversal(".."));
}

TEST_CASE("IsNormalisedPathSafe — normal archive entry path is safe",
          "[security][path]") {
    REQUIRE(IsNormalisedPathSafe("comics/issue01/cover.jpg"));
    REQUIRE(IsNormalisedPathSafe("file.txt"));
}

TEST_CASE("IsNormalisedPathSafe — absolute paths are rejected",
          "[security][path]") {
    REQUIRE_FALSE(IsNormalisedPathSafe("/etc/passwd"));
    REQUIRE_FALSE(IsNormalisedPathSafe("\\Windows\\system32\\cmd.exe"));
}

TEST_CASE("IsNormalisedPathSafe — drive-letter paths are rejected",
          "[security][path]") {
    REQUIRE_FALSE(IsNormalisedPathSafe("C:\\Windows\\system32"));
    REQUIRE_FALSE(IsNormalisedPathSafe("D:/secret.txt"));
}

TEST_CASE("IsNormalisedPathSafe — traversal paths are rejected",
          "[security][path]") {
    REQUIRE_FALSE(IsNormalisedPathSafe("../escape.txt"));
    REQUIRE_FALSE(IsNormalisedPathSafe("sub/../../etc/passwd"));
}

TEST_CASE("IsNormalisedPathSafe — empty path is rejected",
          "[security][path]") {
    REQUIRE_FALSE(IsNormalisedPathSafe(""));
}

// ===========================================================================
// Boundary relationships
// ===========================================================================

TEST_CASE("SecurityBoundary — max archive total >= max image memory",
          "[security][consistency]") {
    // A decoded image must fit within the archive total budget
    REQUIRE(MAX_ARCHIVE_TOTAL_BYTES >= MAX_IMAGE_MEMORY_BYTES);
}

TEST_CASE("SecurityBoundary — max image memory is 2 GiB",
          "[security][consistency]") {
    REQUIRE(MAX_IMAGE_MEMORY_BYTES == 2ULL * 1024 * 1024 * 1024);
}
