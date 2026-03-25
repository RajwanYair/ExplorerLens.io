// IntegrationTestRunner.h — Sprint 25 Integration Test Framework
// Copyright (c) 2026 ExplorerLens Project
//
// Standalone runner that decodes files from data/corpus/ and produces
// a per-format pass/fail report as both a console summary and an HTML file.
// Designed for use in CI pipelines and local developer validation.
//
#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>
#include <chrono>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {
namespace Tests {

//==============================================================================
// IntegrationTestRunner
//==============================================================================

class IntegrationTestRunner
{
public:
    //--------------------------------------------------------------------------
    // Per-file test outcome
    //--------------------------------------------------------------------------
    struct TestResult
    {
        std::wstring filePath;       // Absolute path to the test file
        std::wstring format;         // Detected format name (e.g. "JPEG", "ZIP")
        std::wstring extension;      // File extension (lowercase, no dot)
        bool         passed{false};  // True if decode returned non-null / non-error
        std::wstring errorMessage;   // Empty on success; populated on failure
        double       durationMs{0.0};
        uint64_t     fileSizeBytes{0};
    };

    //--------------------------------------------------------------------------
    // Aggregated report produced by Run()
    //--------------------------------------------------------------------------
    struct RunReport
    {
        std::vector<TestResult> results;
        int    totalFiles{0};
        int    passed{0};
        int    failed{0};
        int    skipped{0};         // Files skipped (unsupported extension, too large, etc.)
        double totalDurationMs{0.0};
        double averageDurationMs{0.0};
        std::string generatedAt;   // ISO-8601 timestamp
        std::string engineVersion; // From BuildValidation::VersionString
    };

    //--------------------------------------------------------------------------
    // Configuration API
    //--------------------------------------------------------------------------

    // Add a directory to scan for corpus files (recursive).
    void AddCorpusDirectory(const std::filesystem::path& dir);

    // Restrict the run to at most 'max' files (0 = unlimited).
    void SetMaxFiles(uint32_t max);

    // Optional callback to filter which files to include (true = include).
    void SetFileFilter(std::function<bool(const std::filesystem::path&)> filter);

    // Set maximum file size to attempt decoding (bytes; 0 = no limit).
    void SetMaxFileSizeBytes(uint64_t bytes);

    //--------------------------------------------------------------------------
    // Execution
    //--------------------------------------------------------------------------

    // Run all configured corpus directories. Thread-safe (read-only on state
    // after configuration is finalised).
    RunReport Run() const;

    // Attempt to decode a single file via the engine's format detector.
    // Returns true if the operation succeeded (non-null thumbnail produced).
    // Sets formatOut to the detected format string.
    static bool TryDecodeFile(const std::filesystem::path& path,
                              std::wstring& formatOut,
                              std::wstring& errorOut);

    //--------------------------------------------------------------------------
    // Report writers
    //--------------------------------------------------------------------------

    // Write results to an HTML file with per-format summary tables.
    static bool WriteHtmlReport(const std::filesystem::path& outputPath,
                                const RunReport& report);

    // Write results to a CSV file (one row per file).
    static bool WriteCsvReport(const std::filesystem::path& outputPath,
                               const RunReport& report);

    // Print a short summary table to std::wcout.
    static void PrintSummary(const RunReport& report);

private:
    std::vector<std::filesystem::path>                   m_corpusDirs;
    uint32_t                                             m_maxFiles{0};
    uint64_t                                             m_maxFileSizeBytes{0};
    std::function<bool(const std::filesystem::path&)>    m_filter;
};

} // namespace Tests
} // namespace Engine
} // namespace ExplorerLens
