// BatchCLI.h — CLI Batch Processing Driver
// Copyright (c) 2026 ExplorerLens Project
//
// Implements the 'lens batch' subcommand. Scans an input directory for
// supported files, dispatches to a thread pool via BatchDecodeScheduler,
// and writes thumbnails to the output directory. Supports glob filtering,
// recursive scan, skip-existing, and a live progress bar.
//
#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <atomic>
#include <chrono>
#include <functional>
#include <cstdint>
#include "LensCLI.h"
#include "../SDK/PublicAPI.h"

namespace ExplorerLens { namespace Engine {

// Per-file result entry
struct BatchFileResult {
    std::wstring inputPath;
    std::wstring outputPath;
    bool         success   = false;
    LENS_RESULT  code      = LENS_OK;
    double       elapsedMs = 0.0;
    std::wstring errorMsg;
};

// Summary statistics for a batch run
struct BatchRunStats {
    uint32_t totalFiles    = 0;
    uint32_t succeeded     = 0;
    uint32_t failed        = 0;
    uint32_t skipped       = 0;
    double   totalElapsedS = 0.0;
    double   avgMs         = 0.0;
    double   throughput    = 0.0;  // files/sec
};

class BatchCLI {
public:
    explicit BatchCLI(LENS_ENGINE_HANDLE hEngine) : m_hEngine(hEngine) {}

    // Execute batch processing with given options
    // Returns LENS_OK on full success, LENS_E_FAIL if any files failed
    LENS_RESULT Execute(const BatchOptions& opts,
                        std::vector<BatchFileResult>* pResults = nullptr,
                        BatchRunStats*                pStats   = nullptr);

    // Set progress callback (replaces default ANSI progress bar)
    void SetProgressCallback(ProgressCallback cb) { m_progressCb = cb; }

private:
    LENS_ENGINE_HANDLE m_hEngine;
    ProgressCallback   m_progressCb;

    // Collect matching files from opts.inputDir
    std::vector<std::wstring> ScanDirectory(const BatchOptions& opts) const;

    // Determine thumbnail output path for a given input
    std::wstring BuildOutputPath(const std::wstring& inputPath,
                                 const std::wstring& outputDir) const;

    // Filter: check if file matches opts.filter glob (empty = all supported)
    bool MatchesFilter(const std::wstring& path, const std::wstring& filter) const;

    // Write results summary to stdout
    static void PrintSummary(const BatchRunStats& stats,
                              const std::vector<BatchFileResult>& results);
};

inline std::wstring BatchCLI::BuildOutputPath(const std::wstring& inputPath,
                                               const std::wstring& outputDir) const {
    wchar_t* namePtr = PathFindFileNameW(inputPath.c_str());
    std::wstring stem(namePtr ? namePtr : inputPath.c_str());
    auto dot = stem.rfind(L'.');
    if (dot != std::wstring::npos) stem = stem.substr(0, dot);
    return outputDir + L"\\" + stem + L"_thumb.png";
}

inline bool BatchCLI::MatchesFilter(const std::wstring& path,
                                     const std::wstring& filter) const {
    if (filter.empty()) return true;
    // Simple extension filter (e.g. "*.psd" or ".psd")
    std::wstring ext;
    auto dot = path.rfind(L'.');
    if (dot != std::wstring::npos) ext = path.substr(dot);
    if (filter.size() > 1 && filter[0] == L'*')
        return _wcsicmp(ext.c_str(), filter.c_str() + 1) == 0;
    return _wcsicmp(ext.c_str(), filter.c_str()) == 0;
}

inline void BatchCLI::PrintSummary(const BatchRunStats& stats,
                                    const std::vector<BatchFileResult>& results) {
    wprintf(L"\n─────────────── Batch Complete ───────────────\n");
    wprintf(L"  Total   : %u files\n", stats.totalFiles);
    wprintf(L"  OK      : %u\n", stats.succeeded);
    wprintf(L"  Failed  : %u\n", stats.failed);
    wprintf(L"  Skipped : %u\n", stats.skipped);
    wprintf(L"  Elapsed : %.2fs  |  Avg: %.1fms  |  %.1f files/sec\n",
            stats.totalElapsedS, stats.avgMs, stats.throughput);
    if (stats.failed > 0) {
        wprintf(L"\nFailed files:\n");
        for (auto& r : results)
            if (!r.success)
                wprintf(L"  ✖ %s — %s\n", r.inputPath.c_str(), r.errorMsg.c_str());
    }
}

}} // namespace ExplorerLens::Engine
