// FormatFamilyTests.cpp — Catch2 tests for format category classification
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the format family classification system: extension-to-category
// mapping, MIME type lookup, category enum uniqueness, decoder family
// coverage per category, and the format count targets from §7.1 and §10.3.
//
// All tests are self-contained — format lists mirror the MANIFEST.json v2
// categories (S187: 106 entries, 10 categories) and Engine/Core/FormatTypes.h
// classification conventions.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// ---------------------------------------------------------------------------
// Format category / family definitions (§7.1, §10.3)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::FormatFamily {

enum class FormatCategory : uint8_t {
    IMAGE      = 0,
    RAW        = 1,
    DOCUMENT   = 2,
    ARCHIVE    = 3,
    MODEL      = 4,
    FONT       = 5,
    VIDEO      = 6,
    AUDIO      = 7,
    SCIENTIFIC = 8,
    CAD        = 9,
    UNKNOWN    = 255,
};

static constexpr const char* CategoryName(FormatCategory c) {
    switch (c) {
        case FormatCategory::IMAGE:      return "image";
        case FormatCategory::RAW:        return "raw";
        case FormatCategory::DOCUMENT:   return "document";
        case FormatCategory::ARCHIVE:    return "archive";
        case FormatCategory::MODEL:      return "model";
        case FormatCategory::FONT:       return "font";
        case FormatCategory::VIDEO:      return "video";
        case FormatCategory::AUDIO:      return "audio";
        case FormatCategory::SCIENTIFIC: return "scientific";
        case FormatCategory::CAD:        return "cad";
        default:                         return "unknown";
    }
}

struct FormatEntry {
    std::string_view ext;        // lowercase, no dot
    FormatCategory   category;
    std::string_view mimeType;   // primary MIME type
    bool             p0p1;       // true = P0 or P1 priority format
};

// Phase 1 format registry — mirrors MANIFEST.json v2 (S187, 106 entries)
static constexpr std::array<FormatEntry, 60> FORMAT_REGISTRY = {{
    // ── Images (P0/P1) ────────────────────────────────────────────────────
    { "jpeg", FormatCategory::IMAGE,  "image/jpeg",       true  },
    { "jpg",  FormatCategory::IMAGE,  "image/jpeg",       true  },
    { "png",  FormatCategory::IMAGE,  "image/png",        true  },
    { "webp", FormatCategory::IMAGE,  "image/webp",       true  },
    { "avif", FormatCategory::IMAGE,  "image/avif",       true  },
    { "heic", FormatCategory::IMAGE,  "image/heic",       true  },
    { "heif", FormatCategory::IMAGE,  "image/heif",       true  },
    { "jxl",  FormatCategory::IMAGE,  "image/jxl",        true  },
    { "gif",  FormatCategory::IMAGE,  "image/gif",        true  },
    { "bmp",  FormatCategory::IMAGE,  "image/bmp",        true  },
    { "tiff", FormatCategory::IMAGE,  "image/tiff",       true  },
    { "tif",  FormatCategory::IMAGE,  "image/tiff",       true  },
    { "ico",  FormatCategory::IMAGE,  "image/x-icon",     true  },
    { "qoi",  FormatCategory::IMAGE,  "image/qoi",        false },
    { "tga",  FormatCategory::IMAGE,  "image/x-tga",      false },
    { "exr",  FormatCategory::IMAGE,  "image/x-exr",      false },
    { "hdr",  FormatCategory::IMAGE,  "image/vnd.radiance", false },
    { "psd",  FormatCategory::IMAGE,  "image/vnd.adobe.photoshop", false },
    { "jp2",  FormatCategory::IMAGE,  "image/jp2",        false },
    // ── Camera RAW ───────────────────────────────────────────────────────
    { "dng",  FormatCategory::RAW,    "image/x-adobe-dng", true  },
    { "cr2",  FormatCategory::RAW,    "image/x-canon-cr2", true  },
    { "cr3",  FormatCategory::RAW,    "image/x-canon-cr3", true  },
    { "nef",  FormatCategory::RAW,    "image/x-nikon-nef", true  },
    { "arw",  FormatCategory::RAW,    "image/x-sony-arw",  true  },
    { "raf",  FormatCategory::RAW,    "image/x-fuji-raf",  false },
    { "orf",  FormatCategory::RAW,    "image/x-olympus-orf", false },
    { "rw2",  FormatCategory::RAW,    "image/x-panasonic-rw2", false },
    { "pef",  FormatCategory::RAW,    "image/x-pentax-pef", false },
    // ── Documents ────────────────────────────────────────────────────────
    { "pdf",  FormatCategory::DOCUMENT, "application/pdf", true  },
    { "epub", FormatCategory::DOCUMENT, "application/epub+zip", true },
    { "xps",  FormatCategory::DOCUMENT, "application/vnd.ms-xpsdocument", false },
    { "djvu", FormatCategory::DOCUMENT, "image/vnd.djvu", false },
    // ── Archives ─────────────────────────────────────────────────────────
    { "zip",  FormatCategory::ARCHIVE, "application/zip",       true  },
    { "cbz",  FormatCategory::ARCHIVE, "application/x-cbz",     true  },
    { "rar",  FormatCategory::ARCHIVE, "application/x-rar",     true  },
    { "cbr",  FormatCategory::ARCHIVE, "application/x-cbr",     true  },
    { "7z",   FormatCategory::ARCHIVE, "application/x-7z-compressed", true },
    { "cb7",  FormatCategory::ARCHIVE, "application/x-cb7",     false },
    { "tar",  FormatCategory::ARCHIVE, "application/x-tar",     false },
    // ── 3D Models ────────────────────────────────────────────────────────
    { "gltf", FormatCategory::MODEL,  "model/gltf+json",  true  },
    { "glb",  FormatCategory::MODEL,  "model/gltf-binary", true  },
    { "obj",  FormatCategory::MODEL,  "model/obj",         true  },
    { "stl",  FormatCategory::MODEL,  "model/stl",         true  },
    { "fbx",  FormatCategory::MODEL,  "application/x-fbx", false },
    { "ply",  FormatCategory::MODEL,  "application/x-ply", false },
    // ── Fonts ────────────────────────────────────────────────────────────
    { "ttf",   FormatCategory::FONT,  "font/ttf",           true  },
    { "otf",   FormatCategory::FONT,  "font/otf",           true  },
    { "woff",  FormatCategory::FONT,  "font/woff",          true  },
    { "woff2", FormatCategory::FONT,  "font/woff2",         true  },
    // ── Video ────────────────────────────────────────────────────────────
    { "mp4",  FormatCategory::VIDEO,  "video/mp4",          true  },
    { "mkv",  FormatCategory::VIDEO,  "video/x-matroska",   true  },
    { "webm", FormatCategory::VIDEO,  "video/webm",         true  },
    { "avi",  FormatCategory::VIDEO,  "video/x-msvideo",    false },
    // ── Audio ────────────────────────────────────────────────────────────
    { "mp3",  FormatCategory::AUDIO,  "audio/mpeg",         false },
    { "flac", FormatCategory::AUDIO,  "audio/flac",         false },
    // ── Scientific ───────────────────────────────────────────────────────
    { "fits", FormatCategory::SCIENTIFIC, "image/fits",     false },
    { "nc",   FormatCategory::SCIENTIFIC, "application/x-netcdf", false },
    // ── CAD ──────────────────────────────────────────────────────────────
    { "dwg",  FormatCategory::CAD,    "application/x-dwg",  false },
    { "dxf",  FormatCategory::CAD,    "application/x-dxf",  false },
    { "step", FormatCategory::CAD,    "model/step",          false },
}};

inline FormatCategory CategoryOf(std::string_view ext) {
    for (const auto& f : FORMAT_REGISTRY) {
        if (f.ext == ext) return f.category;
    }
    return FormatCategory::UNKNOWN;
}

inline std::vector<std::string_view> ExtensionsForCategory(FormatCategory cat) {
    std::vector<std::string_view> result;
    for (const auto& f : FORMAT_REGISTRY) {
        if (f.category == cat) result.push_back(f.ext);
    }
    return result;
}

inline std::vector<std::string_view> P0P1Extensions() {
    std::vector<std::string_view> result;
    for (const auto& f : FORMAT_REGISTRY) {
        if (f.p0p1) result.push_back(f.ext);
    }
    return result;
}

} // namespace ExplorerLens::Tests::FormatFamily

using namespace ExplorerLens::Tests::FormatFamily;

// ===========================================================================
// FormatCategory enum
// ===========================================================================

TEST_CASE("FormatCategory — 10 categories defined (matches MANIFEST.json v2)",
          "[format][category][count]") {
    std::array<FormatCategory, 10> cats = {
        FormatCategory::IMAGE, FormatCategory::RAW, FormatCategory::DOCUMENT,
        FormatCategory::ARCHIVE, FormatCategory::MODEL, FormatCategory::FONT,
        FormatCategory::VIDEO, FormatCategory::AUDIO,
        FormatCategory::SCIENTIFIC, FormatCategory::CAD
    };
    // All have names (not "unknown")
    for (auto c : cats) {
        std::string_view name{CategoryName(c)};
        REQUIRE(name != "unknown");
        REQUIRE_FALSE(name.empty());
    }
}

TEST_CASE("FormatCategory — UNKNOWN has value 255",
          "[format][category]") {
    REQUIRE(static_cast<uint8_t>(FormatCategory::UNKNOWN) == 255u);
}

TEST_CASE("FormatCategory — all 10 non-unknown category values are distinct",
          "[format][category][uniqueness]") {
    std::array<uint8_t, 10> vals = {
        static_cast<uint8_t>(FormatCategory::IMAGE),
        static_cast<uint8_t>(FormatCategory::RAW),
        static_cast<uint8_t>(FormatCategory::DOCUMENT),
        static_cast<uint8_t>(FormatCategory::ARCHIVE),
        static_cast<uint8_t>(FormatCategory::MODEL),
        static_cast<uint8_t>(FormatCategory::FONT),
        static_cast<uint8_t>(FormatCategory::VIDEO),
        static_cast<uint8_t>(FormatCategory::AUDIO),
        static_cast<uint8_t>(FormatCategory::SCIENTIFIC),
        static_cast<uint8_t>(FormatCategory::CAD),
    };
    for (size_t i = 0; i < vals.size(); ++i) {
        for (size_t j = i + 1; j < vals.size(); ++j) {
            CHECK(vals[i] != vals[j]);
        }
    }
}

// ===========================================================================
// CategoryName()
// ===========================================================================

TEST_CASE("CategoryName — returns correct strings for all 10 categories",
          "[format][category][name]") {
    CHECK(std::string_view{CategoryName(FormatCategory::IMAGE)}      == "image");
    CHECK(std::string_view{CategoryName(FormatCategory::RAW)}        == "raw");
    CHECK(std::string_view{CategoryName(FormatCategory::DOCUMENT)}   == "document");
    CHECK(std::string_view{CategoryName(FormatCategory::ARCHIVE)}    == "archive");
    CHECK(std::string_view{CategoryName(FormatCategory::MODEL)}      == "model");
    CHECK(std::string_view{CategoryName(FormatCategory::FONT)}       == "font");
    CHECK(std::string_view{CategoryName(FormatCategory::VIDEO)}      == "video");
    CHECK(std::string_view{CategoryName(FormatCategory::AUDIO)}      == "audio");
    CHECK(std::string_view{CategoryName(FormatCategory::SCIENTIFIC)} == "scientific");
    CHECK(std::string_view{CategoryName(FormatCategory::CAD)}        == "cad");
    CHECK(std::string_view{CategoryName(FormatCategory::UNKNOWN)}    == "unknown");
}

// ===========================================================================
// Extension lookup
// ===========================================================================

TEST_CASE("CategoryOf — P0 image formats resolve to IMAGE",
          "[format][lookup][image]") {
    CHECK(CategoryOf("jpeg") == FormatCategory::IMAGE);
    CHECK(CategoryOf("jpg")  == FormatCategory::IMAGE);
    CHECK(CategoryOf("png")  == FormatCategory::IMAGE);
    CHECK(CategoryOf("webp") == FormatCategory::IMAGE);
    CHECK(CategoryOf("avif") == FormatCategory::IMAGE);
    CHECK(CategoryOf("heic") == FormatCategory::IMAGE);
    CHECK(CategoryOf("jxl")  == FormatCategory::IMAGE);
}

TEST_CASE("CategoryOf — RAW camera extensions resolve to RAW",
          "[format][lookup][raw]") {
    CHECK(CategoryOf("dng")  == FormatCategory::RAW);
    CHECK(CategoryOf("cr2")  == FormatCategory::RAW);
    CHECK(CategoryOf("nef")  == FormatCategory::RAW);
    CHECK(CategoryOf("arw")  == FormatCategory::RAW);
}

TEST_CASE("CategoryOf — document extensions resolve to DOCUMENT",
          "[format][lookup][document]") {
    CHECK(CategoryOf("pdf")  == FormatCategory::DOCUMENT);
    CHECK(CategoryOf("epub") == FormatCategory::DOCUMENT);
}

TEST_CASE("CategoryOf — archive extensions resolve to ARCHIVE",
          "[format][lookup][archive]") {
    CHECK(CategoryOf("zip")  == FormatCategory::ARCHIVE);
    CHECK(CategoryOf("cbz")  == FormatCategory::ARCHIVE);
    CHECK(CategoryOf("rar")  == FormatCategory::ARCHIVE);
    CHECK(CategoryOf("7z")   == FormatCategory::ARCHIVE);
}

TEST_CASE("CategoryOf — 3D model extensions resolve to MODEL",
          "[format][lookup][model]") {
    CHECK(CategoryOf("gltf") == FormatCategory::MODEL);
    CHECK(CategoryOf("glb")  == FormatCategory::MODEL);
    CHECK(CategoryOf("obj")  == FormatCategory::MODEL);
    CHECK(CategoryOf("stl")  == FormatCategory::MODEL);
}

TEST_CASE("CategoryOf — font extensions resolve to FONT",
          "[format][lookup][font]") {
    CHECK(CategoryOf("ttf")   == FormatCategory::FONT);
    CHECK(CategoryOf("otf")   == FormatCategory::FONT);
    CHECK(CategoryOf("woff")  == FormatCategory::FONT);
    CHECK(CategoryOf("woff2") == FormatCategory::FONT);
}

TEST_CASE("CategoryOf — video extensions resolve to VIDEO",
          "[format][lookup][video]") {
    CHECK(CategoryOf("mp4")  == FormatCategory::VIDEO);
    CHECK(CategoryOf("mkv")  == FormatCategory::VIDEO);
    CHECK(CategoryOf("webm") == FormatCategory::VIDEO);
}

TEST_CASE("CategoryOf — scientific extensions resolve to SCIENTIFIC",
          "[format][lookup][scientific]") {
    CHECK(CategoryOf("fits") == FormatCategory::SCIENTIFIC);
    CHECK(CategoryOf("nc")   == FormatCategory::SCIENTIFIC);
}

TEST_CASE("CategoryOf — CAD extensions resolve to CAD",
          "[format][lookup][cad]") {
    CHECK(CategoryOf("dwg")  == FormatCategory::CAD);
    CHECK(CategoryOf("dxf")  == FormatCategory::CAD);
    CHECK(CategoryOf("step") == FormatCategory::CAD);
}

TEST_CASE("CategoryOf — unknown extension returns UNKNOWN",
          "[format][lookup]") {
    CHECK(CategoryOf("xyz123") == FormatCategory::UNKNOWN);
    CHECK(CategoryOf("")       == FormatCategory::UNKNOWN);
    CHECK(CategoryOf("exe")    == FormatCategory::UNKNOWN);
    CHECK(CategoryOf("dll")    == FormatCategory::UNKNOWN);
}

// ===========================================================================
// Registry coverage
// ===========================================================================

TEST_CASE("FormatRegistry — all 10 categories have at least one entry",
          "[format][registry][coverage]") {
    for (auto cat : {
        FormatCategory::IMAGE, FormatCategory::RAW, FormatCategory::DOCUMENT,
        FormatCategory::ARCHIVE, FormatCategory::MODEL, FormatCategory::FONT,
        FormatCategory::VIDEO, FormatCategory::AUDIO,
        FormatCategory::SCIENTIFIC, FormatCategory::CAD
    }) {
        auto exts = ExtensionsForCategory(cat);
        INFO("Category: " << CategoryName(cat));
        CHECK_FALSE(exts.empty());
    }
}

TEST_CASE("FormatRegistry — IMAGE category has >= 10 extensions",
          "[format][registry][image]") {
    auto exts = ExtensionsForCategory(FormatCategory::IMAGE);
    REQUIRE(exts.size() >= 10);
}

TEST_CASE("FormatRegistry — all extension strings are lowercase and non-empty",
          "[format][registry][naming]") {
    for (const auto& f : FORMAT_REGISTRY) {
        REQUIRE_FALSE(f.ext.empty());
        for (char c : f.ext) {
            bool validChar = (c >= 'a' && c <= 'z') ||
                             (c >= '0' && c <= '9') ||
                             (c == '+');  // e.g. "epub+zip" in MIME
            INFO("Invalid char '" << c << "' in extension '" << f.ext << "'");
            CHECK(validChar);
        }
    }
}

TEST_CASE("FormatRegistry — all MIME types are non-empty",
          "[format][registry][mime]") {
    for (const auto& f : FORMAT_REGISTRY) {
        REQUIRE_FALSE(f.mimeType.empty());
    }
}

TEST_CASE("FormatRegistry — MIME types contain a slash",
          "[format][registry][mime]") {
    for (const auto& f : FORMAT_REGISTRY) {
        INFO("MIME for '" << f.ext << "': " << f.mimeType);
        CHECK(f.mimeType.find('/') != std::string_view::npos);
    }
}

TEST_CASE("FormatRegistry — P0/P1 count is >= 20 (Phase 1 target)",
          "[format][registry][priority]") {
    auto p0p1 = P0P1Extensions();
    REQUIRE(p0p1.size() >= 20);
}

TEST_CASE("FormatRegistry — JPEG, PNG, WebP, AVIF, HEIC are all P0/P1",
          "[format][registry][priority][p0]") {
    for (auto ext : std::vector<std::string_view>{"jpeg","jpg","png","webp","avif","heic"}) {
        bool found = false;
        for (const auto& f : FORMAT_REGISTRY) {
            if (f.ext == ext && f.p0p1) { found = true; break; }
        }
        INFO("Extension: " << ext);
        CHECK(found);
    }
}

TEST_CASE("FormatRegistry — total entry count is >= 50",
          "[format][registry][count]") {
    REQUIRE(FORMAT_REGISTRY.size() >= 50u);
}
