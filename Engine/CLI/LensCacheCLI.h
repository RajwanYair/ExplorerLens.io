// LensCacheCLI.h — lens cache — Cache Inspect / Flush / Warm CLI
// Copyright (c) 2026 ExplorerLens Project
//
// CLI interface for lens cache — inspects cache stats, flushes entries, and pre-warms from a file list.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct CacheCLIStats { size_t entryCount; uint64_t totalBytes; double hitRate; double avgAgeMs; };
class LensCacheCLI {
public:
    CacheCLIStats Inspect()          const { return { 0, 0, 0.0, 0.0 }; }
    size_t        Flush(const std::wstring& pattern = L"*") { (void)pattern; return 0; }
    size_t        Warm(const std::vector<std::wstring>& paths, uint32_t thumbSz = 256) {
        (void)paths; (void)thumbSz; return 0;
    }
    bool          SetMaxSize(uint64_t bytes) { m_maxBytes = bytes; return true; }
    uint64_t      MaxSize() const { return m_maxBytes; }
private:
    uint64_t m_maxBytes = 1024 * 1024 * 1024;
};

} // namespace Engine
} // namespace ExplorerLens