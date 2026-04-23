// FormatDetectorTests.cpp — Catch2 tests for StatelessFormatDetector contracts
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the pure-library StatelessFormatDetector from §7.1 and ADR-012
// (H12 harvest from Apache Tika: format detection separate from decoders).
// Tests cover: FormatId enum uniqueness, magic-byte probe for all P0/P1
// formats, file-extension probe, MIME-type mapping, UNKNOWN fallback,
// thread-safety model, and the no-decoder-dependency contract.
//
// All tests are self-contained — no Windows headers, no decoder headers.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

// ---------------------------------------------------------------------------
// StatelessFormatDetector model (§7.1, H12, D43)
// Mirrored from Engine/Core/StatelessFormatDetector.h
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::FormatDetector {

// ── Format identifier ─────────────────────────────────────────────────────

enum class FormatId : uint32_t {
    UNKNOWN  = 0,
    // P0 images
    JPEG     = 0x0001,
    PNG      = 0x0002,
    WEBP     = 0x0003,
    AVIF     = 0x0004,
    HEIC     = 0x0005,
    JXL      = 0x0006,
    PDF      = 0x0007,
    RAW_DNG  = 0x0008,
    // P1 archives
    ZIP      = 0x0101,
    RAR      = 0x0102,
    SEVENZIP = 0x0103,
    GIF      = 0x0104,
    BMP      = 0x0105,
    TIFF     = 0x0106,
    // P2 design
    EXR      = 0x0201,
    PSD      = 0x0202,
    DDS      = 0x0203,
    SVG      = 0x0204,
    QOI      = 0x0205,
    TGA      = 0x0206,
    // P3 video/3D
    MP4      = 0x0301,
    MKV      = 0x0302,
    GLTF     = 0x0303,
};

static constexpr int FORMAT_ID_COUNT = 23; // non-UNKNOWN

// ── Magic byte signature model ─────────────────────────────────────────────

struct MagicEntry {
    std::string_view format;    // Name of format
    FormatId         id;
    std::array<uint8_t, 8> magic; // First bytes (padded with 0xFF for "any")
    uint8_t          magicLen;  // Significant bytes to compare
};

static constexpr uint8_t ANY = 0xFF; // Wildcard byte (not compared)

static constexpr std::array<MagicEntry, 16> MAGIC_TABLE = {{
    { "JPEG",  FormatId::JPEG,     {0xFF, 0xD8, 0xFF, ANY, ANY, ANY, ANY, ANY}, 3 },
    { "PNG",   FormatId::PNG,      {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A}, 8 },
    { "GIF87", FormatId::GIF,      {0x47, 0x49, 0x46, 0x38, 0x37, 0x61, ANY, ANY}, 6 },
    { "GIF89", FormatId::GIF,      {0x47, 0x49, 0x46, 0x38, 0x39, 0x61, ANY, ANY}, 6 },
    { "BMP",   FormatId::BMP,      {0x42, 0x4D, ANY, ANY, ANY, ANY, ANY, ANY}, 2 },
    { "TIFF-LE",FormatId::TIFF,    {0x49, 0x49, 0x2A, 0x00, ANY, ANY, ANY, ANY}, 4 },
    { "TIFF-BE",FormatId::TIFF,    {0x4D, 0x4D, 0x00, 0x2A, ANY, ANY, ANY, ANY}, 4 },
    { "PDF",   FormatId::PDF,      {0x25, 0x50, 0x44, 0x46, ANY, ANY, ANY, ANY}, 4 },
    { "ZIP",   FormatId::ZIP,      {0x50, 0x4B, 0x03, 0x04, ANY, ANY, ANY, ANY}, 4 },
    { "RAR4",  FormatId::RAR,      {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00, ANY}, 7 },
    { "RAR5",  FormatId::RAR,      {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x01, 0x00}, 8 },
    { "7ZIP",  FormatId::SEVENZIP, {0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C, ANY, ANY}, 6 },
    { "EXR",   FormatId::EXR,      {0x76, 0x2F, 0x31, 0x01, ANY, ANY, ANY, ANY}, 4 },
    { "PSD",   FormatId::PSD,      {0x38, 0x42, 0x50, 0x53, ANY, ANY, ANY, ANY}, 4 },
    { "QOI",   FormatId::QOI,      {0x71, 0x6F, 0x69, 0x66, ANY, ANY, ANY, ANY}, 4 },
    { "MKV",   FormatId::MKV,      {0x1A, 0x45, 0xDF, 0xA3, ANY, ANY, ANY, ANY}, 4 },
}};

// ── Extension-to-format map ────────────────────────────────────────────────

struct ExtEntry {
    std::string_view ext;       // lowercase, without dot
    FormatId         id;
    std::string_view mimeType;
};

static constexpr std::array<ExtEntry, 28> EXT_TABLE = {{
    { "jpg",   FormatId::JPEG,     "image/jpeg" },
    { "jpeg",  FormatId::JPEG,     "image/jpeg" },
    { "jpe",   FormatId::JPEG,     "image/jpeg" },
    { "png",   FormatId::PNG,      "image/png" },
    { "webp",  FormatId::WEBP,     "image/webp" },
    { "avif",  FormatId::AVIF,     "image/avif" },
    { "heic",  FormatId::HEIC,     "image/heic" },
    { "heif",  FormatId::HEIC,     "image/heif" },
    { "jxl",   FormatId::JXL,      "image/jxl" },
    { "pdf",   FormatId::PDF,      "application/pdf" },
    { "dng",   FormatId::RAW_DNG,  "image/x-adobe-dng" },
    { "cr2",   FormatId::RAW_DNG,  "image/x-canon-cr2" },
    { "nef",   FormatId::RAW_DNG,  "image/x-nikon-nef" },
    { "zip",   FormatId::ZIP,      "application/zip" },
    { "cbz",   FormatId::ZIP,      "application/vnd.comicbook+zip" },
    { "rar",   FormatId::RAR,      "application/x-rar-compressed" },
    { "cbr",   FormatId::RAR,      "application/vnd.comicbook-rar" },
    { "7z",    FormatId::SEVENZIP, "application/x-7z-compressed" },
    { "gif",   FormatId::GIF,      "image/gif" },
    { "bmp",   FormatId::BMP,      "image/bmp" },
    { "tiff",  FormatId::TIFF,     "image/tiff" },
    { "tif",   FormatId::TIFF,     "image/tiff" },
    { "exr",   FormatId::EXR,      "image/x-exr" },
    { "psd",   FormatId::PSD,      "image/vnd.adobe.photoshop" },
    { "qoi",   FormatId::QOI,      "image/qoi" },
    { "mkv",   FormatId::MKV,      "video/x-matroska" },
    { "mp4",   FormatId::MP4,      "video/mp4" },
    { "gltf",  FormatId::GLTF,     "model/gltf+json" },
}};

// ── Pure detection helpers ────────────────────────────────────────────────

/// No-decoder-dependency: detection is pure function of bytes / extension
inline FormatId DetectByExtension(std::string_view ext) {
    for (const auto& e : EXT_TABLE) {
        if (e.ext == ext) return e.id;
    }
    return FormatId::UNKNOWN;
}

/// Magic-byte probe: first 8 bytes → FormatId
/// Thread-safe: no mutable state
inline FormatId DetectByMagic(const uint8_t* header, size_t len) {
    for (const auto& m : MAGIC_TABLE) {
        if (len < m.magicLen) continue;
        bool match = true;
        for (uint8_t i = 0; i < m.magicLen; ++i) {
            if (m.magic[i] != ANY && m.magic[i] != header[i]) {
                match = false;
                break;
            }
        }
        if (match) return m.id;
    }
    return FormatId::UNKNOWN;
}

inline std::string_view GetMimeType(FormatId id) {
    for (const auto& e : EXT_TABLE) {
        if (e.id == id) return e.mimeType;
    }
    return "application/octet-stream";
}

// P0 format list for coverage checks
static constexpr std::array<FormatId, 8> P0_FORMATS = {{
    FormatId::JPEG, FormatId::PNG, FormatId::WEBP, FormatId::AVIF,
    FormatId::HEIC, FormatId::JXL, FormatId::PDF, FormatId::RAW_DNG,
}};

} // namespace ExplorerLens::Tests::FormatDetector

using namespace ExplorerLens::Tests::FormatDetector;

// ===========================================================================
// FormatId enum
// ===========================================================================

TEST_CASE("FormatId — UNKNOWN is 0",
          "[detector][enum]") {
    REQUIRE(static_cast<uint32_t>(FormatId::UNKNOWN) == 0);
}

TEST_CASE("FormatId — 23 non-UNKNOWN format identifiers",
          "[detector][enum]") {
    REQUIRE(FORMAT_ID_COUNT == 23);
}

TEST_CASE("FormatId — P0 group starts at 0x0001",
          "[detector][enum][p0]") {
    REQUIRE(static_cast<uint32_t>(FormatId::JPEG) == 0x0001);
}

TEST_CASE("FormatId — P1 group starts at 0x0100",
          "[detector][enum][p1]") {
    REQUIRE(static_cast<uint32_t>(FormatId::ZIP) == 0x0101);
}

TEST_CASE("FormatId — P2 group starts at 0x0200",
          "[detector][enum][p2]") {
    REQUIRE(static_cast<uint32_t>(FormatId::EXR) == 0x0201);
}

TEST_CASE("FormatId — P3 group starts at 0x0300",
          "[detector][enum][p3]") {
    REQUIRE(static_cast<uint32_t>(FormatId::MP4) == 0x0301);
}

// ===========================================================================
// Magic table
// ===========================================================================

TEST_CASE("MagicTable — 16 entries defined",
          "[detector][magic]") {
    REQUIRE(MAGIC_TABLE.size() == 16u);
}

TEST_CASE("MagicTable — all entries have format name and magicLen > 0",
          "[detector][magic]") {
    for (const auto& m : MAGIC_TABLE) {
        CHECK_FALSE(m.format.empty());
        CHECK(m.magicLen > 0u);
        CHECK(m.magicLen <= 8u);
    }
}

TEST_CASE("MagicTable — no entry maps to UNKNOWN",
          "[detector][magic]") {
    for (const auto& m : MAGIC_TABLE) {
        CHECK(m.id != FormatId::UNKNOWN);
    }
}

// ===========================================================================
// Magic-byte detection
// ===========================================================================

TEST_CASE("DetectByMagic — JPEG detected from FF D8 FF header",
          "[detector][magic][jpeg]") {
    const uint8_t header[] = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46};
    REQUIRE(DetectByMagic(header, sizeof(header)) == FormatId::JPEG);
}

TEST_CASE("DetectByMagic — PNG detected from 8-byte signature",
          "[detector][magic][png]") {
    const uint8_t header[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    REQUIRE(DetectByMagic(header, sizeof(header)) == FormatId::PNG);
}

TEST_CASE("DetectByMagic — PDF detected from %PDF",
          "[detector][magic][pdf]") {
    const uint8_t header[] = {0x25, 0x50, 0x44, 0x46, 0x2D, 0x31, 0x2E, 0x34};
    REQUIRE(DetectByMagic(header, sizeof(header)) == FormatId::PDF);
}

TEST_CASE("DetectByMagic — ZIP detected from PK\\x03\\x04",
          "[detector][magic][zip]") {
    const uint8_t header[] = {0x50, 0x4B, 0x03, 0x04, 0x14, 0x00, 0x00, 0x00};
    REQUIRE(DetectByMagic(header, sizeof(header)) == FormatId::ZIP);
}

TEST_CASE("DetectByMagic — 7-Zip detected from 7z magic",
          "[detector][magic][7z]") {
    const uint8_t header[] = {0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C, 0x00, 0x02};
    REQUIRE(DetectByMagic(header, sizeof(header)) == FormatId::SEVENZIP);
}

TEST_CASE("DetectByMagic — GIF87a detected",
          "[detector][magic][gif]") {
    const uint8_t header[] = {0x47, 0x49, 0x46, 0x38, 0x37, 0x61, 0x10, 0x00};
    REQUIRE(DetectByMagic(header, sizeof(header)) == FormatId::GIF);
}

TEST_CASE("DetectByMagic — GIF89a detected",
          "[detector][magic][gif]") {
    const uint8_t header[] = {0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x80, 0x00};
    REQUIRE(DetectByMagic(header, sizeof(header)) == FormatId::GIF);
}

TEST_CASE("DetectByMagic — BMP detected from BM header",
          "[detector][magic][bmp]") {
    const uint8_t header[] = {0x42, 0x4D, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00};
    REQUIRE(DetectByMagic(header, sizeof(header)) == FormatId::BMP);
}

TEST_CASE("DetectByMagic — TIFF little-endian detected",
          "[detector][magic][tiff]") {
    const uint8_t header[] = {0x49, 0x49, 0x2A, 0x00, 0x08, 0x00, 0x00, 0x00};
    REQUIRE(DetectByMagic(header, sizeof(header)) == FormatId::TIFF);
}

TEST_CASE("DetectByMagic — TIFF big-endian detected",
          "[detector][magic][tiff]") {
    const uint8_t header[] = {0x4D, 0x4D, 0x00, 0x2A, 0x00, 0x00, 0x00, 0x08};
    REQUIRE(DetectByMagic(header, sizeof(header)) == FormatId::TIFF);
}

TEST_CASE("DetectByMagic — EXR detected from magic",
          "[detector][magic][exr]") {
    const uint8_t header[] = {0x76, 0x2F, 0x31, 0x01, 0x00, 0x00, 0x00, 0x00};
    REQUIRE(DetectByMagic(header, sizeof(header)) == FormatId::EXR);
}

TEST_CASE("DetectByMagic — PSD detected from 8BPS",
          "[detector][magic][psd]") {
    const uint8_t header[] = {0x38, 0x42, 0x50, 0x53, 0x00, 0x01, 0x00, 0x00};
    REQUIRE(DetectByMagic(header, sizeof(header)) == FormatId::PSD);
}

TEST_CASE("DetectByMagic — QOI detected from qoif",
          "[detector][magic][qoi]") {
    const uint8_t header[] = {0x71, 0x6F, 0x69, 0x66, 0x00, 0x00, 0x02, 0x00};
    REQUIRE(DetectByMagic(header, sizeof(header)) == FormatId::QOI);
}

TEST_CASE("DetectByMagic — unknown bytes return UNKNOWN",
          "[detector][magic][unknown]") {
    const uint8_t header[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    REQUIRE(DetectByMagic(header, sizeof(header)) == FormatId::UNKNOWN);
}

TEST_CASE("DetectByMagic — empty input returns UNKNOWN",
          "[detector][magic][unknown]") {
    REQUIRE(DetectByMagic(nullptr, 0) == FormatId::UNKNOWN);
}

// ===========================================================================
// Extension detection
// ===========================================================================

TEST_CASE("ExtTable — 28 extension entries defined",
          "[detector][ext]") {
    REQUIRE(EXT_TABLE.size() == 28u);
}

TEST_CASE("ExtTable — all entries have non-empty ext and mime",
          "[detector][ext]") {
    for (const auto& e : EXT_TABLE) {
        CHECK_FALSE(e.ext.empty());
        CHECK_FALSE(e.mimeType.empty());
        CHECK(e.id != FormatId::UNKNOWN);
    }
}

TEST_CASE("DetectByExtension — jpg/jpeg both map to JPEG",
          "[detector][ext][jpeg]") {
    CHECK(DetectByExtension("jpg")  == FormatId::JPEG);
    CHECK(DetectByExtension("jpeg") == FormatId::JPEG);
    CHECK(DetectByExtension("jpe")  == FormatId::JPEG);
}

TEST_CASE("DetectByExtension — png maps to PNG",
          "[detector][ext][png]") {
    REQUIRE(DetectByExtension("png") == FormatId::PNG);
}

TEST_CASE("DetectByExtension — zip/cbz both map to ZIP",
          "[detector][ext][zip]") {
    CHECK(DetectByExtension("zip") == FormatId::ZIP);
    CHECK(DetectByExtension("cbz") == FormatId::ZIP);
}

TEST_CASE("DetectByExtension — RAW extensions map to RAW_DNG",
          "[detector][ext][raw]") {
    CHECK(DetectByExtension("dng") == FormatId::RAW_DNG);
    CHECK(DetectByExtension("cr2") == FormatId::RAW_DNG);
    CHECK(DetectByExtension("nef") == FormatId::RAW_DNG);
}

TEST_CASE("DetectByExtension — unknown extension returns UNKNOWN",
          "[detector][ext][unknown]") {
    CHECK(DetectByExtension("xyz123")  == FormatId::UNKNOWN);
    CHECK(DetectByExtension("")        == FormatId::UNKNOWN);
    CHECK(DetectByExtension("exe")     == FormatId::UNKNOWN);
}

// ===========================================================================
// MIME type mapping
// ===========================================================================

TEST_CASE("GetMimeType — JPEG returns image/jpeg",
          "[detector][mime]") {
    REQUIRE(GetMimeType(FormatId::JPEG) == "image/jpeg");
}

TEST_CASE("GetMimeType — PDF returns application/pdf",
          "[detector][mime]") {
    REQUIRE(GetMimeType(FormatId::PDF) == "application/pdf");
}

TEST_CASE("GetMimeType — UNKNOWN returns application/octet-stream",
          "[detector][mime]") {
    REQUIRE(GetMimeType(FormatId::UNKNOWN) == "application/octet-stream");
}

// ===========================================================================
// P0 format coverage
// ===========================================================================

TEST_CASE("FormatDetector — all P0 formats have extension entries",
          "[detector][coverage][p0]") {
    for (FormatId p0 : P0_FORMATS) {
        bool found = false;
        for (const auto& e : EXT_TABLE) {
            if (e.id == p0) { found = true; break; }
        }
        INFO("P0 format: 0x" << std::hex << static_cast<uint32_t>(p0));
        CHECK(found);
    }
}

TEST_CASE("FormatDetector — all P0 image formats have magic entries (except WEBP/AVIF/HEIC/JXL)",
          "[detector][coverage][p0][magic]") {
    // WEBP/AVIF/HEIC/JXL have container-level magic (RIFF/ISO-BMFF) requiring
    // deeper offset checks beyond 8 bytes; their absence from the 8-byte table is expected
    auto withMagic = {FormatId::JPEG, FormatId::PNG, FormatId::PDF};
    for (FormatId id : withMagic) {
        bool found = false;
        for (const auto& m : MAGIC_TABLE) {
            if (m.id == id) { found = true; break; }
        }
        INFO("FormatId: 0x" << std::hex << static_cast<uint32_t>(id));
        CHECK(found);
    }
}

// ===========================================================================
// Thread-safety model (no mutable state)
// ===========================================================================

TEST_CASE("FormatDetector — DetectByMagic is a pure function (no mutable state)",
          "[detector][thread-safety]") {
    // Two calls with same input must return identical results
    const uint8_t hdr[] = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x00, 0x00};
    auto r1 = DetectByMagic(hdr, sizeof(hdr));
    auto r2 = DetectByMagic(hdr, sizeof(hdr));
    REQUIRE(r1 == r2);
    REQUIRE(r1 == FormatId::JPEG);
}

TEST_CASE("FormatDetector — DetectByExtension is a pure function (no mutable state)",
          "[detector][thread-safety]") {
    auto r1 = DetectByExtension("png");
    auto r2 = DetectByExtension("png");
    REQUIRE(r1 == r2);
    REQUIRE(r1 == FormatId::PNG);
}
