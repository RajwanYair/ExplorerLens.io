// InputValidationTests.cpp — Catch2 security & input-validation tests
// Copyright (c) 2026 ExplorerLens Project
//
// Tests OWASP-relevant input validation invariants at the ExplorerLens engine
// boundary:
//   - Oversized dimension defence (§15.1 integer-overflow / resource exhaustion)
//   - Magic byte probe on adversarial inputs (truncated, null-filled, random noise)
//   - Path traversal prevention in filename parsing helpers
//   - Safe integer arithmetic used throughout decode pipeline
//   - FormatId length bounds enforcement
//   - Empty / null buffer handling without crashing
//
// Uses only self-contained stubs — does NOT link against real decoder libs.
//
// ROADMAP: §10.4 (Catch2 migration), §15.1 (security hardening), OWASP A4/A8.
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <limits>
#include <stdexcept>
#include <algorithm>
#include <span>

// ============================================================================
// Inline helpers that mirror production code contracts
// (These are intentionally self-contained so tests never depend on build state)
// ============================================================================

namespace ExplorerLens::Engine::Validation {

// Maximum thumbnail dimension allowed (matches Engine/Core/BuildValidation.h).
inline constexpr uint32_t kMaxThumbDimension = 32768u;

// Maximum per-decode pixel budget (16 K × 16 K, 4 bytes/px = 1 GiB max).
inline constexpr uint64_t kMaxPixelBudget = static_cast<uint64_t>(kMaxThumbDimension) *
                                             kMaxThumbDimension;

/// Returns true iff the requested thumbnail dimensions are within safe limits.
inline bool AreDimensionsSafe(uint32_t width, uint32_t height) noexcept {
    if (width == 0 || height == 0)        return false;
    if (width > kMaxThumbDimension)       return false;
    if (height > kMaxThumbDimension)      return false;
    // Check pixel-count overflow separately (prevents 64-bit wrap at extremes).
    const uint64_t pixels = static_cast<uint64_t>(width) * height;
    return pixels <= kMaxPixelBudget;
}

/// Safe buffer-size computation: width * height * 4 bytes/pixel.
/// Returns 0 on overflow (caller must treat 0 as invalid).
inline uint64_t SafeBufferSize(uint32_t width, uint32_t height) noexcept {
    if (!AreDimensionsSafe(width, height)) return 0;
    return static_cast<uint64_t>(width) * height * 4u;
}

/// Strips leading/trailing path separators and rejects traversal sequences.
/// Returns the sanitised leaf name, or empty string on violation.
inline std::string SanitiseFilename(std::string_view name) {
    if (name.empty()) return {};
    // Reject traversal sequences
    if (name.find("..") != std::string_view::npos) return {};
    // Reject absolute Windows paths
    if (name.size() >= 2 && name[1] == ':')        return {};
    // Reject absolute UNIX paths
    if (name.front() == '/')                        return {};
    if (name.front() == '\\')                       return {};
    // Reject null bytes
    if (name.find('\0') != std::string_view::npos)  return {};
    return std::string(name);
}

/// Clamps a format identifier to the allowed maximum length (64 chars).
/// Returns empty string if the input contains non-printable ASCII.
inline std::string SanitiseFormatId(std::string_view raw) {
    constexpr std::size_t kMaxLen = 64;
    if (raw.empty()) return {};
    for (unsigned char c : raw) {
        if (c < 0x20 || c > 0x7E) return {};   // non-printable ASCII
    }
    return std::string(raw.substr(0, kMaxLen));
}

/// Minimal magic-byte probe: returns a format tag string or "UNKNOWN".
/// Mirrors the contract from Engine/Core/FormatDetector (D43).
inline std::string ProbeFormatTag(std::span<const uint8_t> bytes) noexcept {
    if (bytes.size() < 2) return "UNKNOWN";
    // JPEG
    if (bytes[0] == 0xFF && bytes[1] == 0xD8)              return "JPEG";
    // PNG
    if (bytes.size() >= 8 &&
        bytes[0] == 0x89 && bytes[1] == 0x50 &&
        bytes[2] == 0x4E && bytes[3] == 0x47)               return "PNG";
    // BMP
    if (bytes[0] == 0x42 && bytes[1] == 0x4D)              return "BMP";
    // GIF
    if (bytes.size() >= 6 &&
        bytes[0] == 'G' && bytes[1] == 'I' && bytes[2] == 'F') return "GIF";
    // WebP (RIFF....WEBP)
    if (bytes.size() >= 12 &&
        bytes[0] == 'R' && bytes[1] == 'I' &&
        bytes[8] == 'W' && bytes[9] == 'E' &&
        bytes[10] == 'B' && bytes[11] == 'P')               return "WebP";
    // ZIP/CBZ
    if (bytes[0] == 0x50 && bytes[1] == 0x4B &&
        bytes.size() >= 4 &&
        (bytes[2] == 0x03 || bytes[2] == 0x05 || bytes[2] == 0x07)) return "ZIP";
    // PDF
    if (bytes.size() >= 4 &&
        bytes[0] == '%' && bytes[1] == 'P' &&
        bytes[2] == 'D' && bytes[3] == 'F')                 return "PDF";
    return "UNKNOWN";
}

} // namespace ExplorerLens::Engine::Validation

using namespace ExplorerLens::Engine::Validation;

// ============================================================================
// §15.1 — Dimension overflow / resource-exhaustion defence (OWASP A4)
// ============================================================================

TEST_CASE("AreDimensionsSafe — normal thumbnail sizes are accepted",
          "[security][dimensions]") {
    REQUIRE(AreDimensionsSafe(256, 256));
    REQUIRE(AreDimensionsSafe(512, 512));
    REQUIRE(AreDimensionsSafe(1920, 1080));
    REQUIRE(AreDimensionsSafe(32768, 1));
    REQUIRE(AreDimensionsSafe(1, 32768));
}

TEST_CASE("AreDimensionsSafe — zero dimensions are rejected",
          "[security][dimensions][edge]") {
    REQUIRE_FALSE(AreDimensionsSafe(0, 256));
    REQUIRE_FALSE(AreDimensionsSafe(256, 0));
    REQUIRE_FALSE(AreDimensionsSafe(0, 0));
}

TEST_CASE("AreDimensionsSafe — dimensions exceeding kMaxThumbDimension are rejected",
          "[security][dimensions]") {
    REQUIRE_FALSE(AreDimensionsSafe(32769, 32768));
    REQUIRE_FALSE(AreDimensionsSafe(32768, 32769));
    REQUIRE_FALSE(AreDimensionsSafe(65535, 65535));
    REQUIRE_FALSE(AreDimensionsSafe(
        std::numeric_limits<uint32_t>::max(),
        std::numeric_limits<uint32_t>::max()));
}

TEST_CASE("AreDimensionsSafe — pixel-budget overflow (32768 * 32768 is boundary)",
          "[security][dimensions][overflow]") {
    // Exactly at the limit: 32768 * 32768 == kMaxPixelBudget
    REQUIRE(AreDimensionsSafe(32768, 32768));
    // One pixel over: use 32769 in one axis (> kMaxThumbDimension, rejected above)
    // Test through pixel-count path: 32768 wide, 32769 tall would overflow budget
    // but kMaxThumbDimension check fires first — confirm defence-in-depth still holds.
    REQUIRE_FALSE(AreDimensionsSafe(32769, 32769));
}

TEST_CASE("SafeBufferSize — normal dimensions return correct byte count",
          "[security][buffer-size]") {
    REQUIRE(SafeBufferSize(256, 256)  == 256ULL * 256 * 4);
    REQUIRE(SafeBufferSize(512, 512)  == 512ULL * 512 * 4);
    REQUIRE(SafeBufferSize(1920, 1080) == 1920ULL * 1080 * 4);
}

TEST_CASE("SafeBufferSize — invalid dimensions return 0",
          "[security][buffer-size][overflow]") {
    REQUIRE(SafeBufferSize(0, 512) == 0);
    REQUIRE(SafeBufferSize(512, 0) == 0);
    REQUIRE(SafeBufferSize(std::numeric_limits<uint32_t>::max(),
                           std::numeric_limits<uint32_t>::max()) == 0);
}

// ============================================================================
// §15.1 — Path traversal prevention in filename parsing (OWASP A5)
// ============================================================================

TEST_CASE("SanitiseFilename — normal leaf names pass through",
          "[security][path-traversal]") {
    REQUIRE(SanitiseFilename("photo.jpg") == "photo.jpg");
    REQUIRE(SanitiseFilename("archive.cbz") == "archive.cbz");
    REQUIRE(SanitiseFilename("model.gltf") == "model.gltf");
    REQUIRE(SanitiseFilename("readme.md") == "readme.md");
}

TEST_CASE("SanitiseFilename — path traversal sequences are rejected",
          "[security][path-traversal]") {
    REQUIRE(SanitiseFilename("../etc/passwd").empty());
    REQUIRE(SanitiseFilename("../../config").empty());
    REQUIRE(SanitiseFilename("subfolder/../secret").empty());
    REQUIRE(SanitiseFilename("..").empty());
}

TEST_CASE("SanitiseFilename — absolute paths are rejected",
          "[security][path-traversal]") {
    REQUIRE(SanitiseFilename("C:\\Windows\\System32\\cmd.exe").empty());
    REQUIRE(SanitiseFilename("/etc/passwd").empty());
    REQUIRE(SanitiseFilename("\\\\server\\share").empty());
}

TEST_CASE("SanitiseFilename — empty filename is rejected",
          "[security][path-traversal][edge]") {
    REQUIRE(SanitiseFilename("").empty());
}

TEST_CASE("SanitiseFilename — null byte injection is rejected",
          "[security][path-traversal][injection]") {
    std::string_view nullByte{"file\0.jpg", 9};
    REQUIRE(SanitiseFilename(nullByte).empty());
}

// ============================================================================
// FormatId length / content enforcement (OWASP A3 — injection)
// ============================================================================

TEST_CASE("SanitiseFormatId — normal format identifiers pass through",
          "[security][format-id]") {
    REQUIRE(SanitiseFormatId("JPEG")         == "JPEG");
    REQUIRE(SanitiseFormatId("PNG")          == "PNG");
    REQUIRE(SanitiseFormatId("JXL")          == "JXL");
    REQUIRE(SanitiseFormatId("video/mp4")    == "video/mp4");
    REQUIRE(SanitiseFormatId("glTF-2.0")     == "glTF-2.0");
}

TEST_CASE("SanitiseFormatId — non-printable bytes are rejected",
          "[security][format-id][injection]") {
    REQUIRE(SanitiseFormatId("\x01JPEG").empty());
    REQUIRE(SanitiseFormatId("JPEG\x7F").empty());
    std::string_view nullFormat{"JPEG\0EXTRA", 10};
    REQUIRE(SanitiseFormatId(nullFormat).empty());
}

TEST_CASE("SanitiseFormatId — overlong strings are truncated to 64 chars",
          "[security][format-id][length]") {
    std::string long_id(128, 'A');
    auto result = SanitiseFormatId(long_id);
    REQUIRE(result.size() == 64);
    REQUIRE(result == std::string(64, 'A'));
}

TEST_CASE("SanitiseFormatId — empty string is rejected",
          "[security][format-id][edge]") {
    REQUIRE(SanitiseFormatId("").empty());
}

// ============================================================================
// Magic byte probe on adversarial inputs (OWASP A8 — integrity)
// ============================================================================

TEST_CASE("ProbeFormatTag — zero-byte buffer returns UNKNOWN",
          "[security][probe][edge]") {
    std::vector<uint8_t> empty;
    REQUIRE(ProbeFormatTag(empty) == "UNKNOWN");
}

TEST_CASE("ProbeFormatTag — single-byte buffer returns UNKNOWN",
          "[security][probe][edge]") {
    std::vector<uint8_t> one{0xFF};
    REQUIRE(ProbeFormatTag(one) == "UNKNOWN");
}

TEST_CASE("ProbeFormatTag — all-null buffer returns UNKNOWN",
          "[security][probe][adversarial]") {
    std::vector<uint8_t> nullbuf(64, 0x00);
    REQUIRE(ProbeFormatTag(nullbuf) == "UNKNOWN");
}

TEST_CASE("ProbeFormatTag — all-0xFF buffer returns UNKNOWN",
          "[security][probe][adversarial]") {
    std::vector<uint8_t> ffbuf(64, 0xFF);
    REQUIRE(ProbeFormatTag(ffbuf) == "UNKNOWN");
}

TEST_CASE("ProbeFormatTag — JPEG truncated to exactly 2 bytes is recognised",
          "[security][probe][truncated]") {
    std::vector<uint8_t> truncatedJpeg{0xFF, 0xD8};
    REQUIRE(ProbeFormatTag(truncatedJpeg) == "JPEG");
}

TEST_CASE("ProbeFormatTag — PNG truncated to 4 bytes returns UNKNOWN (needs 8)",
          "[security][probe][truncated]") {
    std::vector<uint8_t> shortPng{0x89, 0x50, 0x4E, 0x47};
    // Fewer than 8 bytes — CanDecode must not crash; may return UNKNOWN
    REQUIRE(ProbeFormatTag(shortPng) == "UNKNOWN");
}

TEST_CASE("ProbeFormatTag — random noise does not crash (fuzz-like)",
          "[security][probe][fuzz]") {
    // 1000 pseudo-random bytes — any result is acceptable; must not throw/crash
    std::vector<uint8_t> noise(1000);
    for (std::size_t i = 0; i < noise.size(); ++i)
        noise[i] = static_cast<uint8_t>((i * 17 + 3) & 0xFF);
    REQUIRE_NOTHROW(ProbeFormatTag(noise));
}

TEST_CASE("ProbeFormatTag — valid JPEG magic with appended garbage is still JPEG",
          "[security][probe][composite]") {
    std::vector<uint8_t> buf{0xFF, 0xD8, 0x00, 0xDE, 0xAD, 0xBE, 0xEF};
    REQUIRE(ProbeFormatTag(buf) == "JPEG");
}

// ============================================================================
// Parametric: boundary values via GENERATE
// ============================================================================

TEST_CASE("AreDimensionsSafe — parametric boundary values",
          "[security][dimensions][parametric]") {
    auto [w, h, expected] = GENERATE(table<uint32_t, uint32_t, bool>({
        {1,     1,     true},
        {256,   256,   true},
        {4096,  4096,  true},
        {32768, 32768, true},   // kMaxPixelBudget exactly
        {32769, 32768, false},  // > kMaxThumbDimension
        {32768, 32769, false},  // > kMaxThumbDimension
        {0,     256,   false},
        {256,   0,     false},
    }));
    REQUIRE(AreDimensionsSafe(w, h) == expected);
}
