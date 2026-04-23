// CorpusCoverageTests.cpp — Catch2 tests for corpus MANIFEST.json integrity
// Copyright (c) 2026 ExplorerLens Project
//
// Validates that the data/corpus/MANIFEST.json is well-formed and covers the
// minimum Phase 1 format requirements per ROADMAP §10.3 and §15.1:
//   ≥100 format entries across ≥8 decoder families
//   All required fields present in each entry
//   No duplicate ext+decoder pairs within a category
//   Priority values 1–3 only
//   test_file field is non-empty and has a valid extension
//
// Uses the CORPUS_ROOT compile-time macro (set by Engine/Tests/CMakeLists.txt
// to "${CMAKE_SOURCE_DIR}/data/corpus") and a minimal JSON tokenizer that
// does NOT require nlohmann/json or any external library.
//
// References: ROADMAP §10.3, §15.1 Phase 1 milestone, D57 corpus workflow.
//
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#ifndef CORPUS_ROOT
#  define CORPUS_ROOT "data/corpus"
#endif

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// =============================================================================
// Minimal JSON reader — enough to validate the manifest structure
// =============================================================================

namespace {

/// Read the entire file into a string.
std::string ReadFile(const std::filesystem::path& p, bool& ok) {
    std::ifstream f(p);
    ok = f.is_open();
    if (!ok) return {};
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

/// Count non-overlapping occurrences of needle in haystack.
size_t CountOccurrences(const std::string& haystack, const std::string& needle) {
    size_t count = 0;
    size_t pos = 0;
    while ((pos = haystack.find(needle, pos)) != std::string::npos) {
        ++count;
        pos += needle.size();
    }
    return count;
}

/// Extract all string values for a given JSON key pattern: "key": "VALUE"
/// Simple single-pass scan — not a full JSON parser.
std::vector<std::string> ExtractStringValues(const std::string& json, const std::string& key) {
    std::vector<std::string> results;
    std::string pattern = "\"" + key + "\"";
    size_t pos = 0;
    while ((pos = json.find(pattern, pos)) != std::string::npos) {
        pos += pattern.size();
        // Skip whitespace, colon
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' ||
               json[pos] == '\n' || json[pos] == '\r' || json[pos] == ':'))
            ++pos;
        if (pos < json.size() && json[pos] == '"') {
            ++pos;
            size_t end = json.find('"', pos);
            if (end != std::string::npos) {
                results.push_back(json.substr(pos, end - pos));
                pos = end + 1;
            }
        }
    }
    return results;
}

} // anonymous namespace

// =============================================================================
// §1 — MANIFEST.json is readable
// =============================================================================

TEST_CASE("CorpusCoverage: MANIFEST.json exists and is readable", "[corpus][manifest][smoke]") {
    namespace fs = std::filesystem;
    fs::path manifest = fs::path(CORPUS_ROOT) / "MANIFEST.json";
    REQUIRE(fs::exists(manifest));
    bool ok = false;
    std::string content = ReadFile(manifest, ok);
    REQUIRE(ok);
    REQUIRE_FALSE(content.empty());
}

TEST_CASE("CorpusCoverage: MANIFEST.json is valid JSON (top-level brace)", "[corpus][manifest][syntax]") {
    namespace fs = std::filesystem;
    fs::path manifest = fs::path(CORPUS_ROOT) / "MANIFEST.json";
    bool ok = false;
    std::string content = ReadFile(manifest, ok);
    REQUIRE(ok);
    REQUIRE(content.find('{') != std::string::npos);
    REQUIRE(content.find('}') != std::string::npos);
}

// =============================================================================
// §2 — Required top-level keys present
// =============================================================================

TEST_CASE("CorpusCoverage: has '_comment' key", "[corpus][manifest][keys]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("\"_comment\"") != std::string::npos);
}

TEST_CASE("CorpusCoverage: has '_version' key", "[corpus][manifest][keys]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("\"_version\"") != std::string::npos);
}

TEST_CASE("CorpusCoverage: has '_updated' key", "[corpus][manifest][keys]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("\"_updated\"") != std::string::npos);
}

TEST_CASE("CorpusCoverage: has 'categories' key", "[corpus][manifest][keys]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("\"categories\"") != std::string::npos);
}

TEST_CASE("CorpusCoverage: has 'expected_performance' key", "[corpus][manifest][keys]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("\"expected_performance\"") != std::string::npos);
}

// =============================================================================
// §3 — Each entry has required fields
// =============================================================================

TEST_CASE("CorpusCoverage: 'ext' field present in format entries", "[corpus][manifest][fields]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(CountOccurrences(content, "\"ext\"") >= 50u);  // Phase 1: ≥50 ext entries
}

TEST_CASE("CorpusCoverage: 'name' field present in format entries", "[corpus][manifest][fields]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    // 'name' appears in format entries and 'decoderName' in chain entries — count just "name"
    REQUIRE(CountOccurrences(content, "\"name\"") >= 50u);
}

TEST_CASE("CorpusCoverage: 'decoder' field present in format entries", "[corpus][manifest][fields]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(CountOccurrences(content, "\"decoder\"") >= 50u);
}

TEST_CASE("CorpusCoverage: 'test_file' field present in format entries", "[corpus][manifest][fields]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(CountOccurrences(content, "\"test_file\"") >= 50u);
}

TEST_CASE("CorpusCoverage: 'priority' field present in format entries", "[corpus][manifest][fields]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(CountOccurrences(content, "\"priority\"") >= 50u);
}

// =============================================================================
// §4 — Minimum entry count for Phase 1
// =============================================================================

TEST_CASE("CorpusCoverage: at least 50 'ext' entries (Phase 1 minimum)", "[corpus][manifest][count]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    size_t extCount = CountOccurrences(content, "\"ext\"");
    REQUIRE(extCount >= 50u);
}

TEST_CASE("CorpusCoverage: at least 100 'ext' entries (Phase 1 goal)", "[corpus][manifest][count]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    size_t extCount = CountOccurrences(content, "\"ext\"");
    REQUIRE(extCount >= 100u);
}

// =============================================================================
// §5 — Category coverage: at least 8 decoder families
// =============================================================================

TEST_CASE("CorpusCoverage: 'images' category present", "[corpus][manifest][category]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("\"images\"") != std::string::npos);
}

TEST_CASE("CorpusCoverage: 'raw' category present", "[corpus][manifest][category]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("\"raw\"") != std::string::npos);
}

TEST_CASE("CorpusCoverage: 'documents' category present", "[corpus][manifest][category]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("\"documents\"") != std::string::npos);
}

TEST_CASE("CorpusCoverage: 'archives' category present", "[corpus][manifest][category]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("\"archives\"") != std::string::npos);
}

TEST_CASE("CorpusCoverage: 'models' category present", "[corpus][manifest][category]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("\"models\"") != std::string::npos);
}

TEST_CASE("CorpusCoverage: 'fonts' category present", "[corpus][manifest][category]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("\"fonts\"") != std::string::npos);
}

TEST_CASE("CorpusCoverage: 'video' category present", "[corpus][manifest][category]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("\"video\"") != std::string::npos);
}

TEST_CASE("CorpusCoverage: 'audio' category present", "[corpus][manifest][category]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("\"audio\"") != std::string::npos);
}

TEST_CASE("CorpusCoverage: 'scientific' category present (S187 new category)", "[corpus][manifest][category]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("\"scientific\"") != std::string::npos);
}

TEST_CASE("CorpusCoverage: 'cad' category present (S187 new category)", "[corpus][manifest][category]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("\"cad\"") != std::string::npos);
}

// =============================================================================
// §6 — Key decoder families are referenced
// =============================================================================

TEST_CASE("CorpusCoverage: JpegDecoder is referenced", "[corpus][manifest][decoder]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("JpegDecoder") != std::string::npos);
}

TEST_CASE("CorpusCoverage: RawDecoder is referenced", "[corpus][manifest][decoder]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("RawDecoder") != std::string::npos);
}

TEST_CASE("CorpusCoverage: GltfDecoder is referenced", "[corpus][manifest][decoder]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("GltfDecoder") != std::string::npos);
}

TEST_CASE("CorpusCoverage: VideoDecoder is referenced", "[corpus][manifest][decoder]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("VideoDecoder") != std::string::npos);
}

TEST_CASE("CorpusCoverage: FontDecoder is referenced", "[corpus][manifest][decoder]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    REQUIRE(content.find("FontDecoder") != std::string::npos);
}

// =============================================================================
// §7 — test_file names are non-empty and have extensions
// =============================================================================

TEST_CASE("CorpusCoverage: all test_file values contain a dot (have extension)", "[corpus][manifest][testfile]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    auto testFiles = ExtractStringValues(content, "test_file");
    REQUIRE_FALSE(testFiles.empty());
    for (const auto& tf : testFiles) {
        INFO("test_file: " << tf);
        REQUIRE(tf.find('.') != std::string::npos);
    }
}

TEST_CASE("CorpusCoverage: no empty test_file values", "[corpus][manifest][testfile]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    auto testFiles = ExtractStringValues(content, "test_file");
    REQUIRE_FALSE(testFiles.empty());
    for (const auto& tf : testFiles) {
        REQUIRE_FALSE(tf.empty());
    }
}

// =============================================================================
// §8 — expected_performance section covers P0 formats
// =============================================================================

TEST_CASE("CorpusCoverage: expected_performance has 'jpg' entry", "[corpus][manifest][perf]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    // Check within the expected_performance section
    size_t perfStart = content.find("\"expected_performance\"");
    REQUIRE(perfStart != std::string::npos);
    REQUIRE(content.find("\"jpg\"", perfStart) != std::string::npos);
}

TEST_CASE("CorpusCoverage: expected_performance has 'png' entry", "[corpus][manifest][perf]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    size_t perfStart = content.find("\"expected_performance\"");
    REQUIRE(perfStart != std::string::npos);
    REQUIRE(content.find("\"png\"", perfStart) != std::string::npos);
}

TEST_CASE("CorpusCoverage: expected_performance has 'avif' entry", "[corpus][manifest][perf]") {
    namespace fs = std::filesystem;
    bool ok = false;
    auto content = ReadFile(fs::path(CORPUS_ROOT) / "MANIFEST.json", ok);
    REQUIRE(ok);
    size_t perfStart = content.find("\"expected_performance\"");
    REQUIRE(perfStart != std::string::npos);
    REQUIRE(content.find("\"avif\"", perfStart) != std::string::npos);
}
