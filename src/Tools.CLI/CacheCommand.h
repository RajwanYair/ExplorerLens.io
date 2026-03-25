// CacheCommand.h — lens cache: Cache Management Subcommand
// Copyright (c) 2026 ExplorerLens Project
//
// Sprint 20 (v15.4.0 "Zenith-U"): Implements 'lens cache' with three actions:
//   clear   — purge all thumbnail cache entries
//   stats   — print hit/miss ratio, size, entry count
//   warm    — pre-populate cache for a directory of files
//
#pragma once
#include "CommandRouter.h"

namespace ExplorerLens {
namespace CLI {

class CacheCommand final : public ISubCommand {
public:
    int Execute(const ParsedArgs& args) override;
    std::wstring_view Name()      const noexcept override { return L"cache"; }
    std::wstring_view ShortDesc() const noexcept override {
        return L"Manage the ExplorerLens thumbnail cache";
    }
    std::wstring_view Usage() const noexcept override {
        return L"lens cache <clear|stats|warm [directory]> [--verbose]";
    }

    // Returns the resolved thumbnail cache directory path.
    // Used by unit tests to verify path resolution without performing I/O.
    static std::wstring GetCachePath();

private:
    int DoClear(bool verbose);
    int DoStats(bool jsonOutput);
    int DoWarm(const std::wstring& directory, bool verbose);
};

} // namespace CLI
} // namespace ExplorerLens
