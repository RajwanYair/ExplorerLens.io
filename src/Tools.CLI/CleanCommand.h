// CleanCommand.h — lens clean: Prune stale/oversized cache entries
// Copyright (c) 2026 ExplorerLens Project
//
// Implements 'lens clean' which removes thumbnail cache files that:
//   - Are older than N days (--older-than <days>)
//   - Cause the cache to exceed a size limit (--size-limit <MB>)
//   - Correspond to files that no longer exist on disk (--orphans)
//
// Usage:
//   lens clean [--older-than <days>] [--size-limit <MB>] [--orphans] [--dry-run] [--yes]
//
#pragma once
#include "CommandRouter.h"

namespace ExplorerLens {
namespace CLI {

class CleanCommand final : public ISubCommand {
public:
    int Execute(const ParsedArgs& args) override;
    std::wstring_view Name()      const noexcept override { return L"clean"; }
    std::wstring_view ShortDesc() const noexcept override {
        return L"Remove stale, orphaned, or oversized thumbnail cache entries";
    }
    std::wstring_view Usage() const noexcept override {
        return L"lens clean [--older-than <days>] [--size-limit <MB>]\n"
               L"           [--orphans] [--dry-run] [--yes] [--verbose]";
    }

private:
    // Remove entries older than maxAgeDays (0 = no age filter)
    int DoCleanByAge(const std::wstring& cacheDir, int maxAgeDays,
                     bool dryRun, bool verbose, int& outRemoved, uint64_t& outFreed);

    // Remove entries until totalSize <= limitBytes (LRU order)
    int DoCleanBySize(const std::wstring& cacheDir, uint64_t limitBytes,
                      bool dryRun, bool verbose, int& outRemoved, uint64_t& outFreed);

    // Remove cache entries whose source file no longer exists
    int DoCleanOrphans(const std::wstring& cacheDir,
                       bool dryRun, bool verbose, int& outRemoved, uint64_t& outFreed);
};

} // namespace CLI
} // namespace ExplorerLens
