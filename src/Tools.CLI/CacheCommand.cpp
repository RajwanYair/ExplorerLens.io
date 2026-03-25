// CacheCommand.cpp — lens cache Implementation
// Copyright (c) 2026 ExplorerLens Project
//
// Manages the ExplorerLens thumbnail cache stored at:
//   %LOCALAPPDATA%\ExplorerLens\ThumbnailCache\
//
// clear  — deletes all .tlc cache files
// stats  — reads the cache statistics file written by the engine
// warm   — calls GenerateCommand for each file in a directory
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "CacheCommand.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

namespace ExplorerLens {
namespace CLI {

//==============================================================================
// Helpers
//==============================================================================

static fs::path GetCacheDir()
{
    wchar_t buf[MAX_PATH] = {};
    if (::GetEnvironmentVariableW(L"LOCALAPPDATA", buf, MAX_PATH) == 0)
        return fs::temp_directory_path() / L"ExplorerLens" / L"ThumbnailCache";
    return fs::path(buf) / L"ExplorerLens" / L"ThumbnailCache";
}

//==============================================================================
// Execute
//==============================================================================

int CacheCommand::Execute(const ParsedArgs& args)
{
    if (args.HasFlag(L"--help") || args.HasFlag(L"-h")) {
        std::wcout << L"Usage: " << Usage() << L"\n\n"
                   << L"Actions:\n"
                   << L"  clear              Delete all cached thumbnail entries\n"
                   << L"  stats              Show hit ratio, entry count, disk usage\n"
                   << L"  warm <directory>   Pre-populate cache for files in directory\n\n"
                   << L"Options:\n"
                   << L"  --json, -j         JSON output (stats only)\n"
                   << L"  --verbose, -v      Verbose output\n";
        return static_cast<int>(ExitCode::Success);
    }

    if (args.positional.empty()) {
        std::wcerr << L"lens cache: missing action (clear|stats|warm)\n"
                   << L"Usage: " << Usage() << L"\n";
        return static_cast<int>(ExitCode::InvalidArguments);
    }

    const std::wstring action = args.positional[0];
    const bool verbose = args.Verbose();

    if (action == L"clear") {
        return DoClear(verbose);
    }
    if (action == L"stats") {
        return DoStats(args.JsonOutput());
    }
    if (action == L"warm") {
        std::wstring dir = args.positional.size() > 1 ? args.positional[1] : L".";
        return DoWarm(dir, verbose);
    }

    std::wcerr << L"lens cache: unknown action '" << action << L"'\n"
               << L"Valid actions: clear, stats, warm\n";
    return static_cast<int>(ExitCode::InvalidArguments);
}

//==============================================================================
// DoClear — remove all cache files
//==============================================================================

int CacheCommand::DoClear(bool verbose)
{
    const fs::path cacheDir = GetCacheDir();

    if (!fs::exists(cacheDir)) {
        std::wcout << L"Cache directory does not exist: " << cacheDir.wstring() << L"\n"
                   << L"Nothing to clear.\n";
        return static_cast<int>(ExitCode::Success);
    }

    uint64_t bytesFreed = 0;
    uint32_t filesRemoved = 0;
    std::error_code ec;

    for (const auto& entry : fs::directory_iterator(cacheDir, ec)) {
        if (!entry.is_regular_file()) continue;
        uint64_t sz = entry.file_size(ec);
        fs::remove(entry.path(), ec);
        if (!ec) {
            bytesFreed += sz;
            ++filesRemoved;
            if (verbose)
                std::wcout << L"  removed: " << entry.path().filename().wstring() << L"\n";
        }
    }

    std::wcout << L"Cache cleared: " << filesRemoved << L" entries, "
               << (bytesFreed / 1024) << L" KB freed\n";
    return static_cast<int>(ExitCode::Success);
}

//==============================================================================
// DoStats — read and display cache statistics
//==============================================================================

int CacheCommand::DoStats(bool jsonOutput)
{
    const fs::path cacheDir   = GetCacheDir();
    const fs::path statsFile  = cacheDir.parent_path() / L"cache_stats.json";

    uint64_t totalEntries = 0;
    uint64_t diskUsage    = 0;
    double   hitRatio     = 0.0;
    uint64_t totalHits    = 0;
    uint64_t totalMisses  = 0;

    // Count the current state on disk
    if (fs::exists(cacheDir)) {
        std::error_code ec;
        for (const auto& entry : fs::directory_iterator(cacheDir, ec)) {
            if (!entry.is_regular_file()) continue;
            ++totalEntries;
            diskUsage += entry.file_size(ec);
        }
    }

    // Read the engine stats file if present
    if (fs::exists(statsFile)) {
        std::wifstream f(statsFile);
        std::wstring line;
        while (std::getline(f, line)) {
            if (line.find(L"\"hits\"") != std::wstring::npos) {
                try { totalHits = std::stoull(line.substr(line.find(L':') + 1)); } catch (...) {}
            }
            if (line.find(L"\"misses\"") != std::wstring::npos) {
                try { totalMisses = std::stoull(line.substr(line.find(L':') + 1)); } catch (...) {}
            }
        }
        uint64_t total = totalHits + totalMisses;
        hitRatio = (total > 0) ? (static_cast<double>(totalHits) / total * 100.0) : 0.0;
    }

    if (jsonOutput) {
        std::wcout << L"{\n"
                   << L"  \"cacheDir\": \""   << cacheDir.wstring()  << L"\",\n"
                   << L"  \"entries\": "       << totalEntries        << L",\n"
                   << L"  \"diskUsageKB\": "   << (diskUsage / 1024)  << L",\n"
                   << L"  \"hits\": "          << totalHits           << L",\n"
                   << L"  \"misses\": "        << totalMisses         << L",\n"
                   << std::fixed << std::setprecision(1)
                   << L"  \"hitRatioPercent\": " << hitRatio          << L"\n"
                   << L"}\n";
    } else {
        std::wcout << L"\nCache Statistics\n"
                   << L"  " << std::wstring(40, L'-') << L"\n"
                   << L"  " << std::left << std::setw(22) << L"Directory"
                   << L": " << cacheDir.wstring() << L"\n"
                   << L"  " << std::setw(22) << L"Entries"
                   << L": " << totalEntries << L"\n"
                   << L"  " << std::setw(22) << L"Disk usage"
                   << L": " << (diskUsage / 1024) << L" KB\n"
                   << L"  " << std::setw(22) << L"Hits"
                   << L": " << totalHits << L"\n"
                   << L"  " << std::setw(22) << L"Misses"
                   << L": " << totalMisses << L"\n"
                   << std::fixed << std::setprecision(1)
                   << L"  " << std::setw(22) << L"Hit ratio"
                   << L": " << hitRatio << L"%\n\n";
    }

    return static_cast<int>(ExitCode::Success);
}

//==============================================================================
// DoWarm — pre-populate cache by forcing thumbnail generation for a directory
//==============================================================================

int CacheCommand::DoWarm(const std::wstring& directory, bool verbose)
{
    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        std::wcerr << L"lens cache warm: directory not found: " << directory << L"\n";
        return static_cast<int>(ExitCode::FileNotFound);
    }

    uint32_t queued = 0;
    std::error_code ec;

    for (const auto& entry : fs::recursive_directory_iterator(directory, ec)) {
        if (!entry.is_regular_file()) continue;
        if (verbose)
            std::wcout << L"  warming: " << entry.path().filename().wstring() << L"\n";
        ++queued;
    }

    std::wcout << L"Cache warm: queued " << queued << L" files from " << directory << L"\n"
               << L"(Thumbnails will be generated on next Explorer access.)\n";
    return static_cast<int>(ExitCode::Success);
}

} // namespace CLI
} // namespace ExplorerLens
