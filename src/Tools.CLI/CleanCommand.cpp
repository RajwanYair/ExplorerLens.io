// CleanCommand.cpp — Implementation
// Copyright (c) 2026 ExplorerLens Project
//
#include "CleanCommand.h"
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace CLI {

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

int CleanCommand::Execute(const ParsedArgs& args) {
    bool   dryRun      = args.HasFlag(L"dry-run");
    bool   verbose     = args.HasFlag(L"verbose") || args.HasFlag(L"v");
    bool   confirmed   = args.HasFlag(L"yes") || args.HasFlag(L"y");
    bool   doOrphans   = args.HasFlag(L"orphans");
    int    maxAge      = 0;
    int64_t limitMB    = 0;

    if (auto v = args.GetOption(L"older-than")) {
        try { maxAge = std::stoi(*v); } catch (...) {
            std::wcerr << L"lens clean: --older-than requires an integer (days)\n";
            return 1;
        }
    }
    if (auto v = args.GetOption(L"size-limit")) {
        try { limitMB = std::stoll(*v); } catch (...) {
            std::wcerr << L"lens clean: --size-limit requires an integer (MB)\n";
            return 1;
        }
    }

    // Default: if nothing specified, run all clean passes
    if (maxAge == 0 && limitMB == 0 && !doOrphans) {
        maxAge = 30;
        doOrphans = true;
    }

    // Resolve cache directory
    wchar_t local[MAX_PATH];
    HRESULT hr = SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, local);
    std::wstring cacheDir = (SUCCEEDED(hr) ? std::wstring(local) : L"%LOCALAPPDATA%")
                          + L"\\ExplorerLens\\Cache";

    if (!fs::exists(cacheDir)) {
        std::wcout << L"Cache directory does not exist: " << cacheDir << L"\n";
        return 0;
    }

    if (!confirmed && !dryRun) {
        std::wcout << L"This will delete thumbnail cache files in:\n  " << cacheDir << L"\n";
        std::wcout << L"Continue? [y/N] ";
        std::wstring answer;
        std::getline(std::wcin, answer);
        if (answer != L"y" && answer != L"Y") {
            std::wcout << L"Aborted.\n";
            return 0;
        }
    }
    if (dryRun) std::wcout << L"[dry-run] No files will be deleted.\n";

    int      totalRemoved = 0;
    uint64_t totalFreed   = 0;

    if (maxAge > 0) {
        int n = 0; uint64_t freed = 0;
        DoCleanByAge(cacheDir, maxAge, dryRun, verbose, n, freed);
        totalRemoved += n; totalFreed += freed;
        std::wcout << L"Age filter (>" << maxAge << L" days): removed " << n
                   << L" files, freed " << (freed / 1024) << L" KB\n";
    }
    if (limitMB > 0) {
        int n = 0; uint64_t freed = 0;
        DoCleanBySize(cacheDir, static_cast<uint64_t>(limitMB) * 1024 * 1024,
                      dryRun, verbose, n, freed);
        totalRemoved += n; totalFreed += freed;
        std::wcout << L"Size limit (" << limitMB << L" MB): removed " << n
                   << L" files, freed " << (freed / 1024) << L" KB\n";
    }
    if (doOrphans) {
        int n = 0; uint64_t freed = 0;
        DoCleanOrphans(cacheDir, dryRun, verbose, n, freed);
        totalRemoved += n; totalFreed += freed;
        std::wcout << L"Orphan scan: removed " << n
                   << L" files, freed " << (freed / 1024) << L" KB\n";
    }

    std::wcout << L"Total: removed " << totalRemoved
               << L" files, freed " << (totalFreed / 1024) << L" KB\n";
    return 0;
}

// ---------------------------------------------------------------------------
// DoCleanByAge
// ---------------------------------------------------------------------------

int CleanCommand::DoCleanByAge(const std::wstring& cacheDir, int maxAgeDays,
                                bool dryRun, bool verbose,
                                int& outRemoved, uint64_t& outFreed) {
    outRemoved = 0; outFreed = 0;
    auto cutoff = fs::file_time_type::clock::now() -
                  std::chrono::hours(maxAgeDays * 24);

    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(cacheDir, ec)) {
        if (!entry.is_regular_file()) continue;
        auto mtime = entry.last_write_time(ec);
        if (ec) continue;
        if (mtime < cutoff) {
            uint64_t sz = entry.file_size(ec);
            if (verbose)
                std::wcout << L"  [age] " << entry.path().filename().wstring() << L"\n";
            if (!dryRun) fs::remove(entry.path(), ec);
            ++outRemoved;
            outFreed += sz;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// DoCleanBySize
// ---------------------------------------------------------------------------

int CleanCommand::DoCleanBySize(const std::wstring& cacheDir, uint64_t limitBytes,
                                 bool dryRun, bool verbose,
                                 int& outRemoved, uint64_t& outFreed) {
    outRemoved = 0; outFreed = 0;

    // Collect all cache files sorted oldest-first
    using FEntry = std::pair<fs::file_time_type, fs::directory_entry>;
    std::vector<FEntry> files;
    uint64_t totalSize = 0;
    std::error_code ec;

    for (const auto& e : fs::directory_iterator(cacheDir, ec)) {
        if (!e.is_regular_file()) continue;
        files.emplace_back(e.last_write_time(ec), e);
        totalSize += e.file_size(ec);
    }

    if (totalSize <= limitBytes) return 0; // Already within limit

    std::sort(files.begin(), files.end(),
              [](const FEntry& a, const FEntry& b){ return a.first < b.first; });

    for (const auto& [mtime, entry] : files) {
        if (totalSize <= limitBytes) break;
        uint64_t sz = entry.file_size(ec);
        if (verbose)
            std::wcout << L"  [size] " << entry.path().filename().wstring() << L"\n";
        if (!dryRun) fs::remove(entry.path(), ec);
        totalSize -= sz;
        outFreed  += sz;
        ++outRemoved;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// DoCleanOrphans
// ---------------------------------------------------------------------------

int CleanCommand::DoCleanOrphans(const std::wstring& cacheDir,
                                  bool dryRun, bool verbose,
                                  int& outRemoved, uint64_t& outFreed) {
    outRemoved = 0; outFreed = 0;
    std::error_code ec;

    for (const auto& entry : fs::directory_iterator(cacheDir, ec)) {
        if (!entry.is_regular_file()) continue;

        // Cache file name format: <sha256_of_path>_<size>_<mtime>_<targetpx>.thumb
        // We decode the original source path from a companion .meta sidecar if present.
        // If no sidecar exists, we skip (cannot determine origin).
        fs::path meta = entry.path();
        meta.replace_extension(L".meta");
        if (!fs::exists(meta)) continue;

        std::ifstream f(meta.string());
        std::string srcPath;
        std::getline(f, srcPath);
        if (srcPath.empty() || fs::exists(srcPath)) continue;

        uint64_t sz = entry.file_size(ec);
        if (verbose)
            std::wcout << L"  [orphan] " << entry.path().filename().wstring()
                       << L" (src: " << std::wstring(srcPath.begin(), srcPath.end()) << L")\n";
        if (!dryRun) {
            fs::remove(entry.path(), ec);
            fs::remove(meta, ec);
        }
        ++outRemoved;
        outFreed += sz;
    }
    return 0;
}

} // namespace CLI
} // namespace ExplorerLens
