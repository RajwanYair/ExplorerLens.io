// PathValidationTests.cpp — Catch2 tests for path traversal guards and
//                           file access security contracts
// Copyright (c) 2026 ExplorerLens Project
//
// Validates path sanitization logic used in the engine to prevent path
// traversal attacks, UNC path escapes, overlong paths, and null-byte
// injection (OWASP A1 — Injection, CWE-23 Path Traversal, §15.1).
//
// All logic is self-contained — no filesystem I/O is performed.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// ---------------------------------------------------------------------------
// Path validation logic mirroring engine conventions (§15.1, PathTraversalGuard.h)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::PathValidation {

/// Maximum path length ExplorerLens will process (avoids kernel limits on older Windows)
static constexpr size_t MAX_PATH_LEN = 32767;

/// Maximum extension length
static constexpr size_t MAX_EXT_LEN = 64;

enum class PathRejection {
    NONE,               // path is acceptable
    EMPTY,              // zero-length path
    TOO_LONG,           // exceeds MAX_PATH_LEN
    NULL_BYTE,          // embedded null character
    TRAVERSAL,          // contains ".." segments
    ABSOLUTE_FORBIDDEN, // path exits expected root
    UNC_FORBIDDEN,      // UNC or device namespace (\\.\, \\?\, \\\\ prefix)
    BAD_EXTENSION,      // extension too long or contains illegal chars
};

/// Returns the reason a path is rejected, or NONE if acceptable.
inline PathRejection ValidatePath(std::string_view path,
                                   std::string_view allowedRoot = "") {
    // 1. Empty
    if (path.empty()) return PathRejection::EMPTY;

    // 2. Null byte injection
    if (path.find('\0') != std::string_view::npos) return PathRejection::NULL_BYTE;

    // 3. Length
    if (path.size() > MAX_PATH_LEN) return PathRejection::TOO_LONG;

    // 4. UNC / device namespace: \\server\share, \\.\, \\?\
    if (path.size() >= 2 && path[0] == '\\' && path[1] == '\\')
        return PathRejection::UNC_FORBIDDEN;

    // 5. Path traversal: any ".." segment
    // Check for \..\ or /../ or path ending in \.. or /..
    // Simple: scan for "..": actual traversal is ".." preceded/followed by separator or end
    for (size_t i = 0; i + 1 < path.size(); ) {
        if (path[i] == '.' && path[i + 1] == '.') {
            bool pre  = (i == 0 || path[i - 1] == '\\' || path[i - 1] == '/');
            bool post = (i + 2 >= path.size() ||
                         path[i + 2] == '\\' || path[i + 2] == '/');
            if (pre && post) return PathRejection::TRAVERSAL;
            i += 2;
        } else {
            ++i;
        }
    }

    // 6. Allowed root check (if provided)
    if (!allowedRoot.empty()) {
        if (path.size() < allowedRoot.size() ||
            path.substr(0, allowedRoot.size()) != allowedRoot) {
            return PathRejection::ABSOLUTE_FORBIDDEN;
        }
    }

    return PathRejection::NONE;
}

/// Extracts the extension from a path (last '.' onwards, lowercased).
/// Returns empty string if no extension or extension too long.
inline std::string ExtractExtension(std::string_view path) {
    auto dot = path.rfind('.');
    if (dot == std::string_view::npos) return {};
    auto ext = path.substr(dot);
    if (ext.size() > MAX_EXT_LEN) return {};
    // Reject extension containing path separators (e.g. ".tar/gz")
    if (ext.find('\\') != std::string_view::npos ||
        ext.find('/')  != std::string_view::npos) return {};
    std::string result(ext);
    for (auto& c : result) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + 32);
    }
    return result;
}

/// Checks whether a path stays under an allowed root (case-insensitive on Windows).
/// Returns true if path begins with root.
inline bool IsUnderRoot(std::string_view path, std::string_view root) {
    if (path.size() < root.size()) return false;
    for (size_t i = 0; i < root.size(); ++i) {
        char p = path[i]; if (p >= 'A' && p <= 'Z') p = static_cast<char>(p + 32);
        char r = root[i]; if (r >= 'A' && r <= 'Z') r = static_cast<char>(r + 32);
        if (p != r) return false;
    }
    return true;
}

} // namespace ExplorerLens::Tests::PathValidation

using namespace ExplorerLens::Tests::PathValidation;

// ===========================================================================
// Basic accept/reject
// ===========================================================================

TEST_CASE("PathValidation — normal absolute path accepted",
          "[path][traversal][security]") {
    auto r = ValidatePath("C:\\Users\\test\\image.jpg");
    REQUIRE(r == PathRejection::NONE);
}

TEST_CASE("PathValidation — empty path rejected",
          "[path][traversal][security]") {
    REQUIRE(ValidatePath("") == PathRejection::EMPTY);
}

TEST_CASE("PathValidation — path with null byte rejected",
          "[path][traversal][security][owasp-a1]") {
    std::string evil = "C:\\Users\\image";
    evil += '\0';
    evil += ".exe";
    REQUIRE(ValidatePath(evil) == PathRejection::NULL_BYTE);
}

TEST_CASE("PathValidation — path exceeding MAX_PATH_LEN rejected",
          "[path][traversal][limits]") {
    std::string huge(MAX_PATH_LEN + 1, 'a');
    REQUIRE(ValidatePath(huge) == PathRejection::TOO_LONG);
}

TEST_CASE("PathValidation — path exactly at MAX_PATH_LEN accepted",
          "[path][traversal][limits]") {
    std::string exact(MAX_PATH_LEN, 'a');
    REQUIRE(ValidatePath(exact) == PathRejection::NONE);
}

// ===========================================================================
// Path traversal
// ===========================================================================

TEST_CASE("PathValidation — double-dot traversal rejected (mid-path)",
          "[path][traversal][security][cwe-23]") {
    REQUIRE(ValidatePath("C:\\Users\\..\\Windows\\System32") == PathRejection::TRAVERSAL);
}

TEST_CASE("PathValidation — double-dot traversal rejected (at start)",
          "[path][traversal][security][cwe-23]") {
    REQUIRE(ValidatePath("..\\etc\\passwd") == PathRejection::TRAVERSAL);
}

TEST_CASE("PathValidation — double-dot traversal rejected (at end)",
          "[path][traversal][security][cwe-23]") {
    REQUIRE(ValidatePath("C:\\Users\\test\\..") == PathRejection::TRAVERSAL);
}

TEST_CASE("PathValidation — forward-slash traversal rejected",
          "[path][traversal][security][cwe-23]") {
    REQUIRE(ValidatePath("uploads/../../../etc/passwd") == PathRejection::TRAVERSAL);
}

TEST_CASE("PathValidation — three dots not a traversal",
          "[path][traversal]") {
    // "..." is not a standard traversal token
    auto r = ValidatePath("C:\\Users\\test\\...\\image.jpg");
    REQUIRE(r == PathRejection::NONE);
}

TEST_CASE("PathValidation — embedded filename with dots accepted",
          "[path][traversal]") {
    auto r = ValidatePath("C:\\Users\\test\\file.tar.gz");
    REQUIRE(r == PathRejection::NONE);
}

// ===========================================================================
// UNC paths
// ===========================================================================

TEST_CASE("PathValidation — UNC server share rejected (\\\\server\\share)",
          "[path][unc][security]") {
    REQUIRE(ValidatePath("\\\\server\\share\\image.jpg") == PathRejection::UNC_FORBIDDEN);
}

TEST_CASE("PathValidation — device namespace rejected (\\\\.\\PhysicalDrive0)",
          "[path][unc][security]") {
    REQUIRE(ValidatePath("\\\\.\\PhysicalDrive0") == PathRejection::UNC_FORBIDDEN);
}

TEST_CASE("PathValidation — extended-length path prefix rejected (\\\\?\\C:\\)",
          "[path][unc][security]") {
    REQUIRE(ValidatePath("\\\\?\\C:\\image.jpg") == PathRejection::UNC_FORBIDDEN);
}

TEST_CASE("PathValidation — single backslash root not treated as UNC",
          "[path][unc]") {
    auto r = ValidatePath("C:\\image.jpg");
    REQUIRE(r == PathRejection::NONE);
}

// ===========================================================================
// Allowed root enforcement
// ===========================================================================

TEST_CASE("PathValidation — path under allowed root accepted",
          "[path][root][security]") {
    auto r = ValidatePath("C:\\ProgramData\\ExplorerLens\\cache\\x.jpg",
                           "C:\\ProgramData\\ExplorerLens");
    REQUIRE(r == PathRejection::NONE);
}

TEST_CASE("PathValidation — path outside allowed root rejected",
          "[path][root][security][owasp-a1]") {
    auto r = ValidatePath("C:\\Windows\\System32\\cmd.exe",
                           "C:\\ProgramData\\ExplorerLens");
    REQUIRE(r == PathRejection::ABSOLUTE_FORBIDDEN);
}

TEST_CASE("PathValidation — root escape via traversal + root check rejected",
          "[path][root][traversal][security]") {
    // Traversal check fires first
    auto r = ValidatePath("C:\\ProgramData\\ExplorerLens\\..\\..\\Windows",
                           "C:\\ProgramData\\ExplorerLens");
    REQUIRE(r == PathRejection::TRAVERSAL);
}

// ===========================================================================
// IsUnderRoot
// ===========================================================================

TEST_CASE("IsUnderRoot — exact match returns true",
          "[path][root]") {
    REQUIRE(IsUnderRoot("C:\\Data\\ExplorerLens", "C:\\Data\\ExplorerLens"));
}

TEST_CASE("IsUnderRoot — subpath returns true",
          "[path][root]") {
    REQUIRE(IsUnderRoot("C:\\Data\\ExplorerLens\\cache\\a.jpg", "C:\\Data\\ExplorerLens"));
}

TEST_CASE("IsUnderRoot — unrelated path returns false",
          "[path][root]") {
    REQUIRE_FALSE(IsUnderRoot("C:\\Windows\\System32", "C:\\Data\\ExplorerLens"));
}

TEST_CASE("IsUnderRoot — case-insensitive match",
          "[path][root]") {
    REQUIRE(IsUnderRoot("c:\\data\\explorerlens\\a.jpg", "C:\\Data\\ExplorerLens"));
}

TEST_CASE("IsUnderRoot — shorter path than root returns false",
          "[path][root]") {
    REQUIRE_FALSE(IsUnderRoot("C:\\Data", "C:\\Data\\ExplorerLens"));
}

// ===========================================================================
// Extension extraction
// ===========================================================================

TEST_CASE("ExtractExtension — simple extension lowercased",
          "[path][extension]") {
    REQUIRE(ExtractExtension("photo.JPEG") == ".jpeg");
}

TEST_CASE("ExtractExtension — no extension returns empty",
          "[path][extension]") {
    REQUIRE(ExtractExtension("Makefile").empty());
}

TEST_CASE("ExtractExtension — dot in directory name, no final extension",
          "[path][extension]") {
    // "C:\\My.Dir\\Makefile" — last '.' is in directory part but no extension
    REQUIRE(ExtractExtension("C:\\My.Dir\\Makefile").empty());
}

TEST_CASE("ExtractExtension — extension too long returns empty",
          "[path][extension][limits]") {
    std::string fname = "file.";
    fname += std::string(MAX_EXT_LEN, 'x');
    REQUIRE(ExtractExtension(fname).empty());
}

TEST_CASE("ExtractExtension — compound extension returns last segment",
          "[path][extension]") {
    REQUIRE(ExtractExtension("archive.tar.gz") == ".gz");
}

TEST_CASE("ExtractExtension — extension with path separator returns empty",
          "[path][extension][security]") {
    // Malformed path where '.' appears before a separator
    REQUIRE(ExtractExtension("file.tar/extra").empty());
}

// ===========================================================================
// Parametric: known safe paths all pass
// ===========================================================================

TEST_CASE("PathValidation — known safe corpus paths accepted",
          "[path][parametric]") {
    const std::vector<std::string> safePaths = {
        "C:\\Users\\test\\photo.jpg",
        "C:\\ProgramData\\ExplorerLens\\cache\\thumb.png",
        "D:\\Downloads\\document.pdf",
        "C:\\Users\\Public\\Pictures\\image.avif",
        "C:\\Temp\\test.cr2",
        "C:\\archive.zip",
        "relative\\path\\to\\file.txt",
        "file.txt",
    };
    for (const auto& p : safePaths) {
        INFO("Testing path: " << p);
        CHECK(ValidatePath(p) == PathRejection::NONE);
    }
}

// ===========================================================================
// Parametric: known dangerous paths all rejected
// ===========================================================================

TEST_CASE("PathValidation — known dangerous paths rejected",
          "[path][security][owasp-a1][cwe-23]") {
    struct Case { std::string path; PathRejection expected; };
    const std::vector<Case> dangerous = {
        { "",                                              PathRejection::EMPTY },
        { "..\\etc\\passwd",                               PathRejection::TRAVERSAL },
        { "C:\\Users\\..\\Windows",                        PathRejection::TRAVERSAL },
        { "\\\\server\\share\\file.jpg",                   PathRejection::UNC_FORBIDDEN },
        { "\\\\.\\PhysicalDrive0",                         PathRejection::UNC_FORBIDDEN },
        { "\\\\?\\C:\\secret.txt",                         PathRejection::UNC_FORBIDDEN },
    };
    for (const auto& c : dangerous) {
        INFO("Testing path: " << c.path);
        CHECK(ValidatePath(c.path) == c.expected);
    }
}
