// CrashReportBundler.h — Bundles crash reports with diagnostic context
// Copyright (c) 2026 ExplorerLens Project
//
// Collects minidumps, log excerpts, system info, and recent operations
// into a single compressed bundle for crash report submission.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct CrashReportBundlerConfig {
    bool enabled = true;
    uint32_t maxLogLines = 1000;
    uint32_t maxBundleSizeMB = 10;
    std::string label = "CrashReportBundler";
};

class CrashReportBundler {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    CrashReportBundlerConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct BundleManifest {
        std::string dumpPath;
        std::string logPath;
        std::string systemInfo;
        uint64_t timestamp = 0;
        uint32_t exitCode = 0;
    };

    void SetManifest(const BundleManifest& m) { m_manifest = m; }
    BundleManifest GetManifest() const { return m_manifest; }
    bool HasDump() const { return !m_manifest.dumpPath.empty(); }

private:
    bool m_initialized = false;
    CrashReportBundlerConfig m_config;
    BundleManifest m_manifest;
};

}
} // namespace ExplorerLens::Engine
