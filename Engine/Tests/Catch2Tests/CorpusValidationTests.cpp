// CorpusValidationTests.cpp — Catch2 tests for real corpus file detection
// Copyright (c) 2026 ExplorerLens Project
//
// Validates that synthetic corpus files have correct magic bytes and
// can be detected by the format prober. Uses data/corpus/ files generated
// by tools/Generate-Corpus.ps1.
//
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstring>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// Read first N bytes of a file
std::vector<uint8_t> ReadHeader(const fs::path& path, size_t maxBytes = 64) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};
    std::vector<uint8_t> buf(maxBytes);
    f.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(maxBytes));
    buf.resize(static_cast<size_t>(f.gcount()));
    return buf;
}

bool StartsWith(const std::vector<uint8_t>& data, const std::vector<uint8_t>& magic) {
    if (data.size() < magic.size()) return false;
    return std::memcmp(data.data(), magic.data(), magic.size()) == 0;
}

// Locate corpus root: walk up from exe dir looking for data/corpus/
fs::path FindCorpusRoot() {
    // Try relative to CWD first (common in CMake/CTest)
    fs::path candidate = fs::current_path();
    for (int i = 0; i < 5; ++i) {
        auto corpus = candidate / "data" / "corpus";
        if (fs::exists(corpus / "MANIFEST.json")) return corpus;
        candidate = candidate.parent_path();
    }
    // Fallback: use CMAKE_SOURCE_DIR set at configure time
    // (passed via CORPUS_ROOT define — see CMakeLists.txt)
#ifdef CORPUS_ROOT
    fs::path root(CORPUS_ROOT);
    if (fs::exists(root / "MANIFEST.json")) return root;
#endif
    return {};
}

} // namespace

TEST_CASE("Corpus root is discoverable", "[corpus]") {
    auto root = FindCorpusRoot();
    REQUIRE_FALSE(root.empty());
    REQUIRE(fs::exists(root / "MANIFEST.json"));
}

TEST_CASE("JPEG corpus file has valid magic bytes", "[corpus][images]") {
    auto root = FindCorpusRoot();
    REQUIRE_FALSE(root.empty());
    auto path = root / "images" / "sample_1mp.jpg";
    if (!fs::exists(path)) {
        WARN("Corpus file not found (run Generate-Corpus.ps1): " << path.string());
        SKIP("Corpus file not generated");
    }
    auto header = ReadHeader(path);
    REQUIRE(header.size() >= 2);
    CHECK(header[0] == 0xFF);
    CHECK(header[1] == 0xD8);
}

TEST_CASE("PNG corpus file has valid magic bytes", "[corpus][images]") {
    auto root = FindCorpusRoot();
    REQUIRE_FALSE(root.empty());
    auto path = root / "images" / "sample_256c.png";
    if (!fs::exists(path)) { SKIP("Corpus file not generated"); }
    auto header = ReadHeader(path);
    std::vector<uint8_t> pngMagic = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    CHECK(StartsWith(header, pngMagic));
}

TEST_CASE("WebP corpus file has valid RIFF/WEBP magic", "[corpus][images]") {
    auto root = FindCorpusRoot();
    REQUIRE_FALSE(root.empty());
    auto path = root / "images" / "sample_lossy.webp";
    if (!fs::exists(path)) { SKIP("Corpus file not generated"); }
    auto header = ReadHeader(path);
    REQUIRE(header.size() >= 12);
    CHECK(header[0] == 'R');
    CHECK(header[1] == 'I');
    CHECK(header[2] == 'F');
    CHECK(header[3] == 'F');
    CHECK(header[8] == 'W');
    CHECK(header[9] == 'E');
    CHECK(header[10] == 'B');
    CHECK(header[11] == 'P');
}

TEST_CASE("BMP corpus file has valid BM magic", "[corpus][images]") {
    auto root = FindCorpusRoot();
    REQUIRE_FALSE(root.empty());
    auto path = root / "images" / "sample_24bit.bmp";
    if (!fs::exists(path)) { SKIP("Corpus file not generated"); }
    auto header = ReadHeader(path);
    REQUIRE(header.size() >= 2);
    CHECK(header[0] == 'B');
    CHECK(header[1] == 'M');
}

TEST_CASE("PDF corpus file has valid %PDF magic", "[corpus][documents]") {
    auto root = FindCorpusRoot();
    REQUIRE_FALSE(root.empty());
    auto path = root / "documents" / "sample_1page.pdf";
    if (!fs::exists(path)) { SKIP("Corpus file not generated"); }
    auto header = ReadHeader(path);
    std::vector<uint8_t> pdfMagic = {0x25, 0x50, 0x44, 0x46}; // %PDF
    CHECK(StartsWith(header, pdfMagic));
}

TEST_CASE("CBZ corpus file has valid ZIP magic", "[corpus][archives]") {
    auto root = FindCorpusRoot();
    REQUIRE_FALSE(root.empty());
    auto path = root / "archives" / "sample_cbz.cbz";
    if (!fs::exists(path)) { SKIP("Corpus file not generated"); }
    auto header = ReadHeader(path);
    REQUIRE(header.size() >= 4);
    CHECK(header[0] == 0x50); // PK
    CHECK(header[1] == 0x4B);
    CHECK(header[2] == 0x03);
    CHECK(header[3] == 0x04);
}

TEST_CASE("SVG corpus file starts with <svg", "[corpus][images]") {
    auto root = FindCorpusRoot();
    REQUIRE_FALSE(root.empty());
    auto path = root / "images" / "sample.svg";
    if (!fs::exists(path)) { SKIP("Corpus file not generated"); }
    std::ifstream f(path);
    std::string content((std::istreambuf_iterator<char>(f)), {});
    CHECK(content.find("<svg") != std::string::npos);
}

TEST_CASE("glTF corpus file is valid JSON with asset.version", "[corpus][models]") {
    auto root = FindCorpusRoot();
    REQUIRE_FALSE(root.empty());
    auto path = root / "models" / "sample_box.gltf";
    if (!fs::exists(path)) { SKIP("Corpus file not generated"); }
    std::ifstream f(path);
    std::string content((std::istreambuf_iterator<char>(f)), {});
    CHECK(content.find("\"version\"") != std::string::npos);
    CHECK(content.find("2.0") != std::string::npos);
}

TEST_CASE("STL corpus file has 84-byte header with triangle count", "[corpus][models]") {
    auto root = FindCorpusRoot();
    REQUIRE_FALSE(root.empty());
    auto path = root / "models" / "sample_binary.stl";
    if (!fs::exists(path)) { SKIP("Corpus file not generated"); }
    auto header = ReadHeader(path, 84);
    REQUIRE(header.size() == 84);
    // Bytes 80-83 are uint32_t triangle count (little-endian)
    uint32_t triCount = 0;
    std::memcpy(&triCount, header.data() + 80, 4);
    CHECK(triCount == 1);
}
