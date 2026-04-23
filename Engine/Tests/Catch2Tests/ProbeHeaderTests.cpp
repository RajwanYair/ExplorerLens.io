// ProbeHeaderTests.cpp — Catch2 tests for format header probing
// Copyright (c) 2026 ExplorerLens Project
//
// Tests magic-byte recognition for the top 20 P0/P1 formats per §7.3 decoder
// priority tier table. Uses the ProbeResult/ProbeStatus types from
// IStreamingDecoder.h but drives a self-contained FormatProber that mirrors
// the real StatelessFormatDetector byte-pattern logic, making the test
// independent of decoder library linkage (§10.4 self-contained rule).
//
// Reference: TIFF 6.0 spec §2, JPEG ISO/IEC 10918-1 §B.1.1,
//            PNG Spec §5.2, WebP VP8 RFC 6386 §3.1.
//
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "../../Core/IStreamingDecoder.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <vector>

using namespace ExplorerLens::Engine;

// =============================================================================
// FormatProber — self-contained magic-byte probe helper
// =============================================================================

namespace {

/// Returns the formatId string for the leading bytes, or "UNKNOWN".
/// Mirrors the byte-pattern table harvested from Apache Tika / libmagic (H12).
std::string ProbeFormatId(std::span<const uint8_t> hdr) noexcept
{
    auto b = hdr;
    const size_t n = b.size();

    // JPEG — SOI marker FF D8
    if (n >= 2 && b[0] == 0xFF && b[1] == 0xD8)
        return "JPEG";

    // PNG — 8-byte signature
    if (n >= 8 && b[0] == 0x89 && b[1] == 0x50 && b[2] == 0x4E && b[3] == 0x47 &&
        b[4] == 0x0D && b[5] == 0x0A && b[6] == 0x1A && b[7] == 0x0A)
        return "PNG";

    // GIF87a / GIF89a
    if (n >= 6 && b[0] == 'G' && b[1] == 'I' && b[2] == 'F' && b[3] == '8' &&
        (b[4] == '7' || b[4] == '9') && b[5] == 'a')
        return "GIF";

    // BMP — 'BM' magic
    if (n >= 2 && b[0] == 'B' && b[1] == 'M')
        return "BMP";

    // TIFF little-endian (II) or big-endian (MM)
    if (n >= 4 &&
        ((b[0] == 'I' && b[1] == 'I' && b[2] == 0x2A && b[3] == 0x00) ||
         (b[0] == 'M' && b[1] == 'M' && b[2] == 0x00 && b[3] == 0x2A)))
        return "TIFF";

    // WebP — RIFF....WEBP
    if (n >= 12 && b[0] == 'R' && b[1] == 'I' && b[2] == 'F' && b[3] == 'F' &&
        b[8] == 'W' && b[9] == 'E' && b[10] == 'B' && b[11] == 'P')
        return "WEBP";

    // AVIF / HEIC / JXL using ISO Base Media File Format ftyp box
    // ftyp box: bytes 4-7 == "ftyp", bytes 8-11 == major brand
    if (n >= 12 && b[4] == 'f' && b[5] == 't' && b[6] == 'y' && b[7] == 'p') {
        // Major brand at offset 8
        char brand[5] = {0};
        memcpy(brand, &b[8], 4);
        std::string br(brand);
        if (br == "avif" || br == "avis") return "AVIF";
        if (br == "heic" || br == "heix" || br == "hevc" || br == "hevx")
            return "HEIC";
        if (br == "mif1" || br == "msf1") return "HEIF";
        return "ISOBMFF";  // generic ISO Base Media File Format
    }

    // JPEG XL — 0xFF 0x0A bare codestream, or ISO BMFF JXL container: 00 00 00 0C 4A 58 4C 20
    if (n >= 2 && b[0] == 0xFF && b[1] == 0x0A)
        return "JXL";
    if (n >= 12 && b[0] == 0x00 && b[1] == 0x00 && b[2] == 0x00 && b[3] == 0x0C &&
        b[4] == 'J' && b[5] == 'X' && b[6] == 'L' && b[7] == ' ')
        return "JXL";

    // PDF — %PDF-
    if (n >= 5 && b[0] == '%' && b[1] == 'P' && b[2] == 'D' && b[3] == 'F' && b[4] == '-')
        return "PDF";

    // ZIP / CBZ / EPUB / DOCX — PK local-file signature
    if (n >= 4 && b[0] == 'P' && b[1] == 'K' && b[2] == 0x03 && b[3] == 0x04)
        return "ZIP";

    // 7-Zip
    if (n >= 6 && b[0] == '7' && b[1] == 'z' && b[2] == 0xBC && b[3] == 0xAF &&
        b[4] == 0x27 && b[5] == 0x1C)
        return "7Z";

    // RAR 5.0 — 52 61 72 21 1A 07 01 00
    if (n >= 8 && b[0] == 0x52 && b[1] == 0x61 && b[2] == 0x72 && b[3] == 0x21 &&
        b[4] == 0x1A && b[5] == 0x07 && b[6] == 0x01 && b[7] == 0x00)
        return "RAR5";

    // RAR 4.x — 52 61 72 21 1A 07 00
    if (n >= 7 && b[0] == 0x52 && b[1] == 0x61 && b[2] == 0x72 && b[3] == 0x21 &&
        b[4] == 0x1A && b[5] == 0x07 && b[6] == 0x00)
        return "RAR4";

    // QOI — qoif
    if (n >= 4 && b[0] == 'q' && b[1] == 'o' && b[2] == 'i' && b[3] == 'f')
        return "QOI";

    // Radiance HDR — #?RADIANCE or #?RGBE
    if (n >= 10 && b[0] == '#' && b[1] == '?' &&
        ((b[2]=='R' && b[3]=='A' && b[4]=='D') ||
         (b[2]=='R' && b[3]=='G' && b[4]=='B')))
        return "HDR";

    // OpenEXR — 76 2F 31 01
    if (n >= 4 && b[0] == 0x76 && b[1] == 0x2F && b[2] == 0x31 && b[3] == 0x01)
        return "EXR";

    // DDS — 44 44 53 20 ("DDS ")
    if (n >= 4 && b[0] == 'D' && b[1] == 'D' && b[2] == 'S' && b[3] == ' ')
        return "DDS";

    // TrueType / OpenType — 00 01 00 00 00 or "OTTO"
    if (n >= 4 && b[0] == 0x00 && b[1] == 0x01 && b[2] == 0x00 && b[3] == 0x00)
        return "TTF";
    if (n >= 4 && b[0] == 'O' && b[1] == 'T' && b[2] == 'T' && b[3] == 'O')
        return "OTF";

    // WOFF — 77 4F 46 46
    if (n >= 4 && b[0] == 'w' && b[1] == 'O' && b[2] == 'F' && b[3] == 'F')
        return "WOFF";

    // WOFF2 — 77 4F 46 32
    if (n >= 4 && b[0] == 'w' && b[1] == 'O' && b[2] == 'F' && b[3] == '2')
        return "WOFF2";

    return "UNKNOWN";
}

/// Build a byte span padded to at least 16 bytes with zeros
template <size_t N>
std::vector<uint8_t> Hdr(const std::array<uint8_t, N>& raw) {
    std::vector<uint8_t> v(raw.begin(), raw.end());
    while (v.size() < 16) v.push_back(0x00);
    return v;
}

} // anonymous namespace

// =============================================================================
// §1 — P0 Image formats (§7.3 priority tier)
// =============================================================================

TEST_CASE("ProbeHeader: JPEG SOI recognized", "[probe][jpeg][p0]") {
    auto h = Hdr<2>({0xFF, 0xD8});
    REQUIRE(ProbeFormatId(h) == "JPEG");
}

TEST_CASE("ProbeHeader: JPEG JFIF full header", "[probe][jpeg][p0]") {
    // FF D8 FF E0 00 10 4A 46 49 46 00 01 ...
    std::vector<uint8_t> h = {0xFF,0xD8,0xFF,0xE0,0x00,0x10,0x4A,0x46,
                               0x49,0x46,0x00,0x01,0x01,0x00,0x00,0x01};
    REQUIRE(ProbeFormatId(h) == "JPEG");
}

TEST_CASE("ProbeHeader: PNG 8-byte signature", "[probe][png][p0]") {
    auto h = Hdr<8>({0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A});
    REQUIRE(ProbeFormatId(h) == "PNG");
}

TEST_CASE("ProbeHeader: PNG signature must be exact — corrupted byte rejected", "[probe][png][p0]") {
    auto h = Hdr<8>({0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x00}); // last byte wrong
    REQUIRE(ProbeFormatId(h) != "PNG");
}

TEST_CASE("ProbeHeader: WebP RIFF header", "[probe][webp][p0]") {
    std::vector<uint8_t> h = {'R','I','F','F',
                               0x24,0x00,0x00,0x00,
                               'W','E','B','P',
                               'V','P','8',' '};
    REQUIRE(ProbeFormatId(h) == "WEBP");
}

TEST_CASE("ProbeHeader: AVIF ftyp brand", "[probe][avif][p0]") {
    std::vector<uint8_t> h(16, 0);
    h[4]='f'; h[5]='t'; h[6]='y'; h[7]='p';
    h[8]='a'; h[9]='v'; h[10]='i'; h[11]='f';
    REQUIRE(ProbeFormatId(h) == "AVIF");
}

TEST_CASE("ProbeHeader: HEIC ftyp brand", "[probe][heic][p0]") {
    std::vector<uint8_t> h(16, 0);
    h[4]='f'; h[5]='t'; h[6]='y'; h[7]='p';
    h[8]='h'; h[9]='e'; h[10]='i'; h[11]='c';
    REQUIRE(ProbeFormatId(h) == "HEIC");
}

TEST_CASE("ProbeHeader: JXL bare codestream FF 0A", "[probe][jxl][p0]") {
    auto h = Hdr<2>({0xFF, 0x0A});
    REQUIRE(ProbeFormatId(h) == "JXL");
}

TEST_CASE("ProbeHeader: JXL ISO BMFF container", "[probe][jxl][p0]") {
    std::vector<uint8_t> h = {0x00,0x00,0x00,0x0C,'J','X','L',' ',
                               0x0D,0x0A,0x87,0x0A,0x00,0x00,0x00,0x00};
    REQUIRE(ProbeFormatId(h) == "JXL");
}

TEST_CASE("ProbeHeader: PDF magic", "[probe][pdf][p0]") {
    std::vector<uint8_t> h = {'%','P','D','F','-','1','.','4',
                               0x0A,0x25,0xE2,0xE3,0xCF,0xD3,0x0A,0x0A};
    REQUIRE(ProbeFormatId(h) == "PDF");
}

// =============================================================================
// §2 — P1 Image formats
// =============================================================================

TEST_CASE("ProbeHeader: TIFF little-endian (II)", "[probe][tiff][p1]") {
    auto h = Hdr<4>({'I','I',0x2A,0x00});
    REQUIRE(ProbeFormatId(h) == "TIFF");
}

TEST_CASE("ProbeHeader: TIFF big-endian (MM)", "[probe][tiff][p1]") {
    auto h = Hdr<4>({'M','M',0x00,0x2A});
    REQUIRE(ProbeFormatId(h) == "TIFF");
}

TEST_CASE("ProbeHeader: GIF87a", "[probe][gif][p1]") {
    auto h = Hdr<6>({'G','I','F','8','7','a'});
    REQUIRE(ProbeFormatId(h) == "GIF");
}

TEST_CASE("ProbeHeader: GIF89a", "[probe][gif][p1]") {
    auto h = Hdr<6>({'G','I','F','8','9','a'});
    REQUIRE(ProbeFormatId(h) == "GIF");
}

TEST_CASE("ProbeHeader: GIF88a (non-standard) not GIF", "[probe][gif][p1]") {
    auto h = Hdr<6>({'G','I','F','8','8','a'});
    REQUIRE(ProbeFormatId(h) != "GIF");
}

TEST_CASE("ProbeHeader: BMP magic", "[probe][bmp][p1]") {
    auto h = Hdr<2>({'B','M'});
    REQUIRE(ProbeFormatId(h) == "BMP");
}

// =============================================================================
// §3 — Archive formats
// =============================================================================

TEST_CASE("ProbeHeader: ZIP PK signature", "[probe][zip]") {
    auto h = Hdr<4>({'P','K',0x03,0x04});
    REQUIRE(ProbeFormatId(h) == "ZIP");
}

TEST_CASE("ProbeHeader: 7-Zip magic", "[probe][7z]") {
    auto h = Hdr<6>({'7','z',0xBC,0xAF,0x27,0x1C});
    REQUIRE(ProbeFormatId(h) == "7Z");
}

TEST_CASE("ProbeHeader: RAR 5.0 magic", "[probe][rar]") {
    auto h = Hdr<8>({0x52,0x61,0x72,0x21,0x1A,0x07,0x01,0x00});
    REQUIRE(ProbeFormatId(h) == "RAR5");
}

TEST_CASE("ProbeHeader: RAR 4.x magic", "[probe][rar]") {
    auto h = Hdr<7>({0x52,0x61,0x72,0x21,0x1A,0x07,0x00});
    REQUIRE(ProbeFormatId(h) == "RAR4");
}

// =============================================================================
// §4 — P2 image formats (HDR/EXR/DDS/QOI)
// =============================================================================

TEST_CASE("ProbeHeader: OpenEXR magic", "[probe][exr][p2]") {
    auto h = Hdr<4>({0x76,0x2F,0x31,0x01});
    REQUIRE(ProbeFormatId(h) == "EXR");
}

TEST_CASE("ProbeHeader: Radiance HDR #?RADIANCE", "[probe][hdr][p2]") {
    std::vector<uint8_t> h = {'#','?','R','A','D','I','A','N','C','E','\n',0,0,0,0,0};
    REQUIRE(ProbeFormatId(h) == "HDR");
}

TEST_CASE("ProbeHeader: Radiance HDR #?RGBE variant", "[probe][hdr][p2]") {
    std::vector<uint8_t> h = {'#','?','R','G','B','E','\n',0,0,0,0,0,0,0,0,0};
    REQUIRE(ProbeFormatId(h) == "HDR");
}

TEST_CASE("ProbeHeader: DDS magic", "[probe][dds][p2]") {
    auto h = Hdr<4>({'D','D','S',' '});
    REQUIRE(ProbeFormatId(h) == "DDS");
}

TEST_CASE("ProbeHeader: QOI magic qoif", "[probe][qoi][p2]") {
    auto h = Hdr<4>({'q','o','i','f'});
    REQUIRE(ProbeFormatId(h) == "QOI");
}

// =============================================================================
// §5 — Font formats
// =============================================================================

TEST_CASE("ProbeHeader: TrueType 00 01 00 00", "[probe][font]") {
    auto h = Hdr<4>({0x00,0x01,0x00,0x00});
    REQUIRE(ProbeFormatId(h) == "TTF");
}

TEST_CASE("ProbeHeader: OpenType OTTO", "[probe][font]") {
    auto h = Hdr<4>({'O','T','T','O'});
    REQUIRE(ProbeFormatId(h) == "OTF");
}

TEST_CASE("ProbeHeader: WOFF wOFF", "[probe][font]") {
    auto h = Hdr<4>({'w','O','F','F'});
    REQUIRE(ProbeFormatId(h) == "WOFF");
}

TEST_CASE("ProbeHeader: WOFF2 wOF2", "[probe][font]") {
    auto h = Hdr<4>({'w','O','F','2'});
    REQUIRE(ProbeFormatId(h) == "WOFF2");
}

// =============================================================================
// §6 — Truncated / empty / garbage input
// =============================================================================

TEST_CASE("ProbeHeader: empty span returns UNKNOWN", "[probe][edge]") {
    std::vector<uint8_t> empty;
    REQUIRE(ProbeFormatId(empty) == "UNKNOWN");
}

TEST_CASE("ProbeHeader: single null byte returns UNKNOWN", "[probe][edge]") {
    std::vector<uint8_t> h = {0x00};
    REQUIRE(ProbeFormatId(h) == "UNKNOWN");
}

TEST_CASE("ProbeHeader: 16 zero bytes returns UNKNOWN", "[probe][edge]") {
    std::vector<uint8_t> h(16, 0x00);
    REQUIRE(ProbeFormatId(h) == "UNKNOWN");
}

TEST_CASE("ProbeHeader: all-0xFF bytes not recognized as any format", "[probe][edge]") {
    std::vector<uint8_t> h(16, 0xFF);
    REQUIRE(ProbeFormatId(h) == "UNKNOWN");
}

TEST_CASE("ProbeHeader: only 1 byte — JPEG SOI incomplete returns UNKNOWN", "[probe][edge]") {
    std::vector<uint8_t> h = {0xFF};  // Only first byte of JPEG SOI
    REQUIRE(ProbeFormatId(h) == "UNKNOWN");
}

// =============================================================================
// §7 — Parametric: ensure each P0/P1 format is distinct (no magic collisions)
// =============================================================================

TEST_CASE("ProbeHeader: top 6 formats are mutually distinct", "[probe][collision]") {
    // Each format's magic bytes should NOT match any other format's probe
    struct FormatSig { const char* name; std::vector<uint8_t> hdr; };

    std::vector<FormatSig> sigs = {
        { "JPEG",  {0xFF,0xD8,0xFF,0xE0,0,0,0,0,0,0,0,0,0,0,0,0} },
        { "PNG",   {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0,0,0,0,0,0,0,0} },
        { "TIFF_LE",{'I','I',0x2A,0x00,0,0,0,0,0,0,0,0,0,0,0,0} },
        { "TIFF_BE",{'M','M',0x00,0x2A,0,0,0,0,0,0,0,0,0,0,0,0} },
        { "PDF",   {'%','P','D','F','-','1','.','4',0,0,0,0,0,0,0,0} },
        { "ZIP",   {'P','K',0x03,0x04,0,0,0,0,0,0,0,0,0,0,0,0} },
    };

    for (size_t i = 0; i < sigs.size(); ++i) {
        for (size_t j = 0; j < sigs.size(); ++j) {
            if (i == j) continue;
            // A format's magic should not be misidentified as another's
            // (We just check JPEG/PNG/PDF/ZIP don't match each other)
            auto id = ProbeFormatId(sigs[i].hdr);
            REQUIRE(id != sigs[j].name);
        }
    }
}
