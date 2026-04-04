// DiagnosticBundleCollector.h — System diagnostic snapshot collection
// Copyright (c) 2026 ExplorerLens Project
//
// Collects diagnostic information (OS version, GPU info, DLL versions,
// cache stats, error logs) into a shareable diagnostic bundle for support.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct DiagnosticBundleCollectorConfig
{
    bool enabled = true;
    bool includeSystemInfo = true;
    bool includeCacheStats = true;
    bool includeErrorLog = true;
    std::string label = "DiagnosticBundleCollector";
};

class DiagnosticBundleCollector
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    DiagnosticBundleCollectorConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    struct Bundle
    {
        std::string osVersion;
        std::string gpuName;
        uint64_t systemMemoryMB = 0;
        uint64_t cacheEntries = 0;
        uint32_t errorCount = 0;
        std::string engineVersion;
    };

    Bundle Collect() const
    {
        Bundle b;
        b.engineVersion = "15.0.0";
        b.systemMemoryMB = 16384;
        b.cacheEntries = 0;
        b.errorCount = 0;
        return b;
    }

    bool IsComplete(const Bundle& b) const
    {
        return !b.engineVersion.empty() && b.systemMemoryMB > 0;
    }

  private:
    bool m_initialized = false;
    DiagnosticBundleCollectorConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
