// CacheWarmupPreloader.cpp — Structured Cache Warm-Up Preloader
// Copyright (c) 2026 ExplorerLens Project
//
#include "CacheWarmupPreloader.h"
#include <fstream>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

bool CacheWarmupPreloader::Start(const std::wstring& mruLogPath, uint32_t maxPaths)
{
    m_complete = false;
    m_stats    = {};

    // Read paths from the MRU log (one path per line, UTF-8 encoded).
    std::vector<std::wstring> paths;
    paths.reserve(maxPaths);
    {
        std::ifstream f(mruLogPath);
        if (!f.is_open()) {
            m_complete = true;
            return false;
        }
        std::string line;
        while (std::getline(f, line) && paths.size() < maxPaths) {
            if (!line.empty()) {
                std::wstring wline(line.begin(), line.end());
                paths.push_back(wline);
            }
        }
    }

    const auto t0 = std::chrono::steady_clock::now();

    for (const auto& path : paths) {
        ++m_stats.attempted;
        if (m_decoder) {
            const bool ok = m_decoder(path);
            if (ok)
                ++m_stats.fulfilled;
            else
                ++m_stats.failed;
        } else {
            // No decoder injected — treat every path as a miss to generate.
            ++m_stats.generated;
        }
    }

    const auto t1 = std::chrono::steady_clock::now();
    m_stats.elapsedMs =
        std::chrono::duration<double, std::milli>(t1 - t0).count();

    m_complete = true;
    return true;
}

void CacheWarmupPreloader::Stop() noexcept
{
    // In the synchronous implementation warm-up completes before Stop() can
    // be observed.  Mark complete and reset in-flight counts.
    m_complete = true;
}

} // namespace Engine
} // namespace ExplorerLens
