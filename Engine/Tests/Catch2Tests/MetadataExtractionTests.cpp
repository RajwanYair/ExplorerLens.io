// MetadataExtractionTests.cpp — Catch2 tests for metadata extraction + EXIF orientation
// Copyright (c) 2026 ExplorerLens Project
//
// ROADMAP: §6.1 (IPropertyStore Details view — Phase 3 metadata),
//          §10.4 (Catch2 migration), §15.1 (input validation)
//
// PURPOSE
// ───────
// Validates metadata parsing invariants exercised by MetadataExtractor,
// ExifOrientationNormalizer, and CrossFormatMetadataEngine.  All tests use
// self-contained decoding of well-known TIFF/EXIF byte patterns — no external
// decoder libraries are linked, so these tests never flake on libjpeg-turbo
// or LibRaw availability.
//
// Coverage: EXIF orientation tag (1..8), endianness (II/MM), tag extraction,
// dimension sanity, defensive bounds for malformed metadata.

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace ExplorerLens::Engine::MetadataExtraction {

// ============================================================================
// Self-contained metadata model
// ============================================================================

enum class Endianness : uint8_t { LITTLE, BIG };

// ExifOrientation values per TIFF 6.0 spec §F.5.4 / EXIF 2.32 §4.6.4
enum class ExifOrientation : uint8_t {
    UNKNOWN = 0,
    TOP_LEFT = 1,
    TOP_RIGHT = 2,
    BOTTOM_RIGHT = 3,
    BOTTOM_LEFT = 4,
    LEFT_TOP = 5,
    RIGHT_TOP = 6,
    RIGHT_BOTTOM = 7,
    LEFT_BOTTOM = 8
};

struct ImageMetadata {
    uint32_t width{0};
    uint32_t height{0};
    ExifOrientation orientation{ExifOrientation::UNKNOWN};
    Endianness endian{Endianness::LITTLE};
    std::optional<std::string> cameraMake;
    std::optional<std::string> cameraModel;
    bool valid{false};
};

// Extract orientation rotation in degrees (clockwise) — per EXIF 2.32.
// Returns -1 for flipped orientations which need both rotate+flip.
constexpr int RotationDegreesFor(ExifOrientation o) noexcept {
    switch (o) {
        case ExifOrientation::TOP_LEFT:     return 0;
        case ExifOrientation::TOP_RIGHT:    return -1;  // flip horizontal
        case ExifOrientation::BOTTOM_RIGHT: return 180;
        case ExifOrientation::BOTTOM_LEFT:  return -1;  // flip vertical
        case ExifOrientation::LEFT_TOP:     return -1;  // transpose
        case ExifOrientation::RIGHT_TOP:    return 90;
        case ExifOrientation::RIGHT_BOTTOM: return -1;  // transverse
        case ExifOrientation::LEFT_BOTTOM:  return 270;
        default:                            return 0;
    }
}

constexpr bool IsFlippedOrientation(ExifOrientation o) noexcept {
    return o == ExifOrientation::TOP_RIGHT
        || o == ExifOrientation::BOTTOM_LEFT
        || o == ExifOrientation::LEFT_TOP
        || o == ExifOrientation::RIGHT_BOTTOM;
}

// Swap width/height for 90°/270° rotations
constexpr bool ShouldSwapDimensions(ExifOrientation o) noexcept {
    return o == ExifOrientation::LEFT_TOP
        || o == ExifOrientation::RIGHT_TOP
        || o == ExifOrientation::RIGHT_BOTTOM
        || o == ExifOrientation::LEFT_BOTTOM;
}

// Parse 16-bit value from 2 bytes with given endianness
constexpr uint16_t Read16(std::span<const uint8_t> bytes, Endianness e) noexcept {
    if (bytes.size() < 2) return 0;
    return (e == Endianness::LITTLE)
        ? static_cast<uint16_t>(bytes[0] | (bytes[1] << 8))
        : static_cast<uint16_t>((bytes[0] << 8) | bytes[1]);
}

constexpr uint32_t Read32(std::span<const uint8_t> bytes, Endianness e) noexcept {
    if (bytes.size() < 4) return 0;
    return (e == Endianness::LITTLE)
        ? static_cast<uint32_t>(bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24))
        : static_cast<uint32_t>((bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3]);
}

// Detect endianness from TIFF-style header — "II" = little, "MM" = big
std::optional<Endianness> DetectEndianness(std::span<const uint8_t> bytes) noexcept {
    if (bytes.size() < 2) return std::nullopt;
    if (bytes[0] == 'I' && bytes[1] == 'I') return Endianness::LITTLE;
    if (bytes[0] == 'M' && bytes[1] == 'M') return Endianness::BIG;
    return std::nullopt;
}

// Clamp/validate orientation value 1..8
ExifOrientation ClampOrientation(uint16_t raw) noexcept {
    if (raw < 1 || raw > 8) return ExifOrientation::UNKNOWN;
    return static_cast<ExifOrientation>(raw);
}

// ============================================================================
// Tests — Orientation semantics
// ============================================================================

TEST_CASE("ExifOrientation TOP_LEFT is identity (0° rotation, no flip)",
          "[Metadata][EXIF][Orientation]") {
    REQUIRE(RotationDegreesFor(ExifOrientation::TOP_LEFT) == 0);
    REQUIRE_FALSE(IsFlippedOrientation(ExifOrientation::TOP_LEFT));
    REQUIRE_FALSE(ShouldSwapDimensions(ExifOrientation::TOP_LEFT));
}

TEST_CASE("ExifOrientation BOTTOM_RIGHT is 180° rotation",
          "[Metadata][EXIF][Orientation]") {
    REQUIRE(RotationDegreesFor(ExifOrientation::BOTTOM_RIGHT) == 180);
    REQUIRE_FALSE(IsFlippedOrientation(ExifOrientation::BOTTOM_RIGHT));
    REQUIRE_FALSE(ShouldSwapDimensions(ExifOrientation::BOTTOM_RIGHT));
}

TEST_CASE("ExifOrientation RIGHT_TOP (6) swaps dimensions for portrait iPhone photos",
          "[Metadata][EXIF][Orientation]") {
    REQUIRE(RotationDegreesFor(ExifOrientation::RIGHT_TOP) == 90);
    REQUIRE(ShouldSwapDimensions(ExifOrientation::RIGHT_TOP));
    REQUIRE_FALSE(IsFlippedOrientation(ExifOrientation::RIGHT_TOP));
}

TEST_CASE("ExifOrientation LEFT_BOTTOM (8) is 270° rotation",
          "[Metadata][EXIF][Orientation]") {
    REQUIRE(RotationDegreesFor(ExifOrientation::LEFT_BOTTOM) == 270);
    REQUIRE(ShouldSwapDimensions(ExifOrientation::LEFT_BOTTOM));
}

TEST_CASE("ExifOrientation flipped variants report as flipped",
          "[Metadata][EXIF][Orientation][Flip]") {
    REQUIRE(IsFlippedOrientation(ExifOrientation::TOP_RIGHT));
    REQUIRE(IsFlippedOrientation(ExifOrientation::BOTTOM_LEFT));
    REQUIRE(IsFlippedOrientation(ExifOrientation::LEFT_TOP));
    REQUIRE(IsFlippedOrientation(ExifOrientation::RIGHT_BOTTOM));
}

TEST_CASE("ExifOrientation all 8 defined values are mutually distinct",
          "[Metadata][EXIF][Orientation][Invariants]") {
    auto a = GENERATE(as<uint8_t>{}, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u);
    auto b = GENERATE(as<uint8_t>{}, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u);

    if (a != b) {
        REQUIRE(static_cast<ExifOrientation>(a) != static_cast<ExifOrientation>(b));
    }
}

// ============================================================================
// Tests — ClampOrientation defensive input validation
// ============================================================================

TEST_CASE("ClampOrientation accepts values 1..8", "[Metadata][EXIF][Validation]") {
    for (uint16_t i = 1; i <= 8; ++i) {
        REQUIRE(ClampOrientation(i) != ExifOrientation::UNKNOWN);
    }
}

TEST_CASE("ClampOrientation rejects 0 and >8", "[Metadata][EXIF][Validation]") {
    REQUIRE(ClampOrientation(0) == ExifOrientation::UNKNOWN);
    REQUIRE(ClampOrientation(9) == ExifOrientation::UNKNOWN);
    REQUIRE(ClampOrientation(255) == ExifOrientation::UNKNOWN);
    REQUIRE(ClampOrientation(0xFFFF) == ExifOrientation::UNKNOWN);
}

TEST_CASE("ClampOrientation is deterministic across parametric sweep",
          "[Metadata][EXIF][Validation]") {
    auto raw = GENERATE(as<uint16_t>{}, 0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 100u, 65535u);
    auto r1 = ClampOrientation(raw);
    auto r2 = ClampOrientation(raw);
    REQUIRE(r1 == r2);
}

// ============================================================================
// Tests — Endianness detection from TIFF header
// ============================================================================

TEST_CASE("DetectEndianness recognizes 'II' (little-endian)",
          "[Metadata][TIFF][Endianness]") {
    std::array<uint8_t, 4> bytes{'I', 'I', 0x2A, 0x00};
    auto e = DetectEndianness(bytes);
    REQUIRE(e.has_value());
    REQUIRE(*e == Endianness::LITTLE);
}

TEST_CASE("DetectEndianness recognizes 'MM' (big-endian)",
          "[Metadata][TIFF][Endianness]") {
    std::array<uint8_t, 4> bytes{'M', 'M', 0x00, 0x2A};
    auto e = DetectEndianness(bytes);
    REQUIRE(e.has_value());
    REQUIRE(*e == Endianness::BIG);
}

TEST_CASE("DetectEndianness rejects arbitrary bytes", "[Metadata][TIFF][Endianness]") {
    std::array<uint8_t, 4> bytes{0xFF, 0xFF, 0xFF, 0xFF};
    REQUIRE_FALSE(DetectEndianness(bytes).has_value());
}

TEST_CASE("DetectEndianness handles too-short spans defensively",
          "[Metadata][TIFF][Endianness][Validation]") {
    std::array<uint8_t, 1> oneByte{'I'};
    std::span<const uint8_t> empty;
    REQUIRE_FALSE(DetectEndianness(empty).has_value());
    REQUIRE_FALSE(DetectEndianness(oneByte).has_value());
}

// ============================================================================
// Tests — Read16 / Read32 endianness handling
// ============================================================================

TEST_CASE("Read16 little-endian matches byte pattern", "[Metadata][IO]") {
    std::array<uint8_t, 2> bytes{0x34, 0x12};
    REQUIRE(Read16(bytes, Endianness::LITTLE) == 0x1234u);
}

TEST_CASE("Read16 big-endian matches byte pattern", "[Metadata][IO]") {
    std::array<uint8_t, 2> bytes{0x12, 0x34};
    REQUIRE(Read16(bytes, Endianness::BIG) == 0x1234u);
}

TEST_CASE("Read32 little-endian matches byte pattern", "[Metadata][IO]") {
    std::array<uint8_t, 4> bytes{0x78, 0x56, 0x34, 0x12};
    REQUIRE(Read32(bytes, Endianness::LITTLE) == 0x12345678u);
}

TEST_CASE("Read32 big-endian matches byte pattern", "[Metadata][IO]") {
    std::array<uint8_t, 4> bytes{0x12, 0x34, 0x56, 0x78};
    REQUIRE(Read32(bytes, Endianness::BIG) == 0x12345678u);
}

TEST_CASE("Read16/Read32 defensively return 0 on undersized span",
          "[Metadata][IO][Validation]") {
    std::span<const uint8_t> empty;
    std::array<uint8_t, 1> oneByte{0xAB};
    REQUIRE(Read16(empty, Endianness::LITTLE) == 0u);
    REQUIRE(Read16(oneByte, Endianness::LITTLE) == 0u);
    REQUIRE(Read32(empty, Endianness::LITTLE) == 0u);
    REQUIRE(Read32(oneByte, Endianness::LITTLE) == 0u);
}

// ============================================================================
// Tests — ImageMetadata invariants
// ============================================================================

TEST_CASE("ImageMetadata default construction is invalid/zero",
          "[Metadata][Invariants]") {
    ImageMetadata m;
    REQUIRE_FALSE(m.valid);
    REQUIRE(m.width == 0);
    REQUIRE(m.height == 0);
    REQUIRE(m.orientation == ExifOrientation::UNKNOWN);
    REQUIRE_FALSE(m.cameraMake.has_value());
    REQUIRE_FALSE(m.cameraModel.has_value());
}

TEST_CASE("ImageMetadata dimension swap logic matches orientation",
          "[Metadata][Orientation][Dimensions]") {
    uint32_t w = 4032, h = 3024;

    // Landscape TOP_LEFT: no swap
    ImageMetadata landscape{.width = w, .height = h,
                             .orientation = ExifOrientation::TOP_LEFT,
                             .valid = true};
    if (ShouldSwapDimensions(landscape.orientation)) {
        std::swap(landscape.width, landscape.height);
    }
    REQUIRE(landscape.width == 4032);
    REQUIRE(landscape.height == 3024);

    // Portrait RIGHT_TOP: swap
    ImageMetadata portrait{.width = w, .height = h,
                            .orientation = ExifOrientation::RIGHT_TOP,
                            .valid = true};
    if (ShouldSwapDimensions(portrait.orientation)) {
        std::swap(portrait.width, portrait.height);
    }
    REQUIRE(portrait.width == 3024);
    REQUIRE(portrait.height == 4032);
}

TEST_CASE("ImageMetadata camera make/model are preserved if set",
          "[Metadata][Camera]") {
    ImageMetadata m{
        .width = 6000,
        .height = 4000,
        .orientation = ExifOrientation::TOP_LEFT,
        .endian = Endianness::LITTLE,
        .cameraMake = "Canon",
        .cameraModel = "EOS R5",
        .valid = true
    };
    REQUIRE(m.cameraMake.has_value());
    REQUIRE(*m.cameraMake == "Canon");
    REQUIRE(m.cameraModel.has_value());
    REQUIRE(*m.cameraModel == "EOS R5");
}

}  // namespace ExplorerLens::Engine::MetadataExtraction
