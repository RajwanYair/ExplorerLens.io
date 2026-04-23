// MagicBytesDatabaseTests.cpp — Catch2 tests for MagicBytesDatabase
// Copyright (c) 2026 ExplorerLens Project
//
// Validates magic-byte detection, extension mapping, and MIME type lookup
// against the centralized MagicBytesDatabase registry.
//
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <array>
#include "../../Core/MagicBytesDatabase.h"

using namespace ExplorerLens::Engine;

// ---------------------------------------------------------------------------
// Magic-byte detection
// ---------------------------------------------------------------------------

TEST_CASE("MagicBytesDatabase detects JPEG", "[magic][jpeg]") {
    std::array<uint8_t, 4> hdr = { 0xFF, 0xD8, 0xFF, 0xE0 };
    auto r = MagicBytesDatabase::Detect(hdr);
    REQUIRE(r.formatId == "JPEG");
    CHECK(r.confidence >= 1.0f);
    CHECK(r.mimeType == "image/jpeg");
}

TEST_CASE("MagicBytesDatabase detects PNG", "[magic][png]") {
    std::array<uint8_t, 8> hdr = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    auto r = MagicBytesDatabase::Detect(hdr);
    REQUIRE(r.formatId == "PNG");
    CHECK(r.mimeType == "image/png");
}

TEST_CASE("MagicBytesDatabase detects GIF89a", "[magic][gif]") {
    std::array<uint8_t, 6> hdr = { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61 };
    auto r = MagicBytesDatabase::Detect(hdr);
    REQUIRE(r.formatId == "GIF");
}

TEST_CASE("MagicBytesDatabase detects WebP", "[magic][webp]") {
    std::array<uint8_t, 12> hdr = {
        0x52, 0x49, 0x46, 0x46,  // RIFF
        0x00, 0x00, 0x00, 0x00,  // size
        0x57, 0x45, 0x42, 0x50   // WEBP
    };
    auto r = MagicBytesDatabase::Detect(hdr);
    REQUIRE(r.formatId == "WEBP");
    CHECK(r.mimeType == "image/webp");
}

TEST_CASE("MagicBytesDatabase detects PDF", "[magic][pdf]") {
    std::array<uint8_t, 5> hdr = { 0x25, 0x50, 0x44, 0x46, 0x2D };
    auto r = MagicBytesDatabase::Detect(hdr);
    REQUIRE(r.formatId == "PDF");
    CHECK(r.mimeType == "application/pdf");
}

TEST_CASE("MagicBytesDatabase detects ZIP", "[magic][zip]") {
    std::array<uint8_t, 4> hdr = { 0x50, 0x4B, 0x03, 0x04 };
    auto r = MagicBytesDatabase::Detect(hdr, ".zip");
    REQUIRE(r.formatId == "ZIP");
}

TEST_CASE("MagicBytesDatabase detects CBZ from ZIP header + ext", "[magic][cbz]") {
    std::array<uint8_t, 4> hdr = { 0x50, 0x4B, 0x03, 0x04 };
    auto r = MagicBytesDatabase::Detect(hdr, ".cbz");
    REQUIRE(r.formatId == "CBZ");
}

TEST_CASE("MagicBytesDatabase detects 7-Zip", "[magic][7z]") {
    std::array<uint8_t, 6> hdr = { 0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C };
    auto r = MagicBytesDatabase::Detect(hdr);
    REQUIRE(r.formatId == "7Z");
}

TEST_CASE("MagicBytesDatabase detects TIFF LE", "[magic][tiff]") {
    std::array<uint8_t, 4> hdr = { 0x49, 0x49, 0x2A, 0x00 };
    auto r = MagicBytesDatabase::Detect(hdr, ".tiff");
    REQUIRE(r.formatId == "TIFF");
}

TEST_CASE("MagicBytesDatabase detects OpenEXR", "[magic][exr]") {
    std::array<uint8_t, 4> hdr = { 0x76, 0x2F, 0x31, 0x01 };
    auto r = MagicBytesDatabase::Detect(hdr);
    REQUIRE(r.formatId == "EXR");
    CHECK(r.mimeType == "image/x-exr");
}

TEST_CASE("MagicBytesDatabase detects Photoshop PSD", "[magic][psd]") {
    std::array<uint8_t, 4> hdr = { 0x38, 0x42, 0x50, 0x53 };
    auto r = MagicBytesDatabase::Detect(hdr);
    REQUIRE(r.formatId == "PSD");
}

TEST_CASE("MagicBytesDatabase detects DDS", "[magic][dds]") {
    std::array<uint8_t, 4> hdr = { 0x44, 0x44, 0x53, 0x20 };
    auto r = MagicBytesDatabase::Detect(hdr);
    REQUIRE(r.formatId == "DDS");
}

TEST_CASE("MagicBytesDatabase detects FLAC", "[magic][flac]") {
    std::array<uint8_t, 4> hdr = { 0x66, 0x4C, 0x61, 0x43 };
    auto r = MagicBytesDatabase::Detect(hdr);
    REQUIRE(r.formatId == "FLAC");
}

TEST_CASE("MagicBytesDatabase detects OGG", "[magic][ogg]") {
    std::array<uint8_t, 4> hdr = { 0x4F, 0x67, 0x67, 0x53 };
    auto r = MagicBytesDatabase::Detect(hdr);
    REQUIRE(r.formatId == "OGG");
}

TEST_CASE("MagicBytesDatabase falls back to extension for unknown magic", "[magic][fallback]") {
    std::array<uint8_t, 4> hdr = { 0x00, 0x00, 0x00, 0x00 };
    auto r = MagicBytesDatabase::Detect(hdr, ".jpg");
    REQUIRE(r.formatId == "JPEG");
    CHECK(r.confidence < 1.0f);  // Extension-only confidence
}

TEST_CASE("MagicBytesDatabase returns empty formatId for unrecognized content", "[magic][unknown]") {
    std::array<uint8_t, 4> hdr = { 0xDE, 0xAD, 0xBE, 0xEF };
    auto r = MagicBytesDatabase::Detect(hdr, "");
    CHECK(r.formatId.empty());
}

// ---------------------------------------------------------------------------
// ExtensionToFormatId
// ---------------------------------------------------------------------------

TEST_CASE("MagicBytesDatabase ExtensionToFormatId maps common extensions", "[ext]") {
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".jpg")  == "JPEG");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".jpeg") == "JPEG");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".png")  == "PNG");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".webp") == "WEBP");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".avif") == "AVIF");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".heic") == "HEIC");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".jxl")  == "JXL");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".pdf")  == "PDF");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".zip")  == "ZIP");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".cbz")  == "CBZ");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".rar")  == "RAR");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".cbr")  == "CBR");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".7z")   == "7Z");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".mkv")  == "MKV");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".glb")  == "GLB");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".exr")  == "EXR");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".dng")  == "DNG");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".fits") == "FITS");
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".dcm")  == "DICOM");
}

TEST_CASE("MagicBytesDatabase ExtensionToFormatId returns empty for unknown ext", "[ext]") {
    CHECK(MagicBytesDatabase::ExtensionToFormatId(".xyz123").empty());
}

// ---------------------------------------------------------------------------
// FormatIdToMimeType
// ---------------------------------------------------------------------------

TEST_CASE("MagicBytesDatabase FormatIdToMimeType returns correct MIME types", "[mime]") {
    CHECK(MagicBytesDatabase::FormatIdToMimeType("JPEG") == "image/jpeg");
    CHECK(MagicBytesDatabase::FormatIdToMimeType("PNG")  == "image/png");
    CHECK(MagicBytesDatabase::FormatIdToMimeType("PDF")  == "application/pdf");
    CHECK(MagicBytesDatabase::FormatIdToMimeType("ZIP")  == "application/zip");
    CHECK(MagicBytesDatabase::FormatIdToMimeType("MP4")  == "video/mp4");
    CHECK(MagicBytesDatabase::FormatIdToMimeType("FLAC") == "audio/flac");
    CHECK(MagicBytesDatabase::FormatIdToMimeType("GLB")  == "model/gltf-binary");
    CHECK(MagicBytesDatabase::FormatIdToMimeType("SVG")  == "image/svg+xml");
}

TEST_CASE("MagicBytesDatabase FormatIdToMimeType returns fallback for unknown formatId", "[mime]") {
    CHECK(MagicBytesDatabase::FormatIdToMimeType("NONEXISTENT") == "application/octet-stream");
}

// ---------------------------------------------------------------------------
// ExtensionsForFormat
// ---------------------------------------------------------------------------

TEST_CASE("MagicBytesDatabase ExtensionsForFormat returns all JPEG extensions", "[ext-list]") {
    auto exts = MagicBytesDatabase::ExtensionsForFormat("JPEG");
    REQUIRE_FALSE(exts.empty());
    CHECK(std::find(exts.begin(), exts.end(), ".jpg") != exts.end());
    CHECK(std::find(exts.begin(), exts.end(), ".jpeg") != exts.end());
}

TEST_CASE("MagicBytesDatabase FormatCount returns > 0", "[count]") {
    CHECK(MagicBytesDatabase::FormatCount() > 0);
}
