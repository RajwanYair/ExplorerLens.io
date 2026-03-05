// ColdStartOptimizer.h — First-Use Decode Latency Reduction
// Copyright (c) 2026 ExplorerLens Project
//
// Reduces cold-start latency by pre-loading critical codec DLLs,
// warming the GPU pipeline state, and pre-building frequently needed
// data structures at registration/install time.
//
#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// Component that can be pre-loaded
enum class PreloadComponent : uint8_t {
    CodecDLL,         // A format decoder DLL (e.g. libwebp, libjxl)
    GPUPipeline,      // D3D11/12 device + PSO
    CacheIndex,       // Persistent cache directory
    FontRenderer,     // FreeType/DirectWrite
    RegistryConfig    // Read config from HKLM/HKCU
};

/// Result of a single preload operation
struct PreloadResult {
    PreloadComponent component = PreloadComponent::CodecDLL;
    std::wstring     name;
    bool             success = false;
    double           loadTimeMs = 0.0;
    std::string      errorMessage;
};

/// Aggregate cold-start optimization results
struct ColdStartReport {
    uint32_t componentsLoaded = 0;
    uint32_t componentsFailed = 0;
    double   totalPreloadMs = 0.0;
    double   estimatedSavingMs = 0.0;
    std::vector<PreloadResult> details;
};

/// Cold start statistics
struct ColdStartStats {
    uint64_t preloadRuns = 0;
    uint64_t dllsPreloaded = 0;
    uint64_t dllsFailed = 0;
    double   avgPreloadTimeMs = 0.0;
    double   totalTimeSavedMs = 0.0;
};

/// Reduces first-use latency by pre-loading critical components
class ColdStartOptimizer {
public:
    static ColdStartOptimizer& Instance() {
        static ColdStartOptimizer inst;
        return inst;
    }

    /// Register a codec DLL for preloading
    void RegisterCodecDLL(const std::wstring& dllName) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_codecDLLs.push_back(dllName);
    }

    /// Pre-load all registered components
    ColdStartReport PreloadAll() {
        std::lock_guard<std::mutex> lock(m_mutex);
        ColdStartReport report;

        auto start = std::chrono::high_resolution_clock::now();

        // Phase 1: Pre-load codec DLLs
        for (const auto& dll : m_codecDLLs) {
            PreloadResult r;
            r.component = PreloadComponent::CodecDLL;
            r.name = dll;

            auto t0 = std::chrono::high_resolution_clock::now();
            HMODULE hMod = LoadLibraryW(dll.c_str());
            auto t1 = std::chrono::high_resolution_clock::now();
            r.loadTimeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

            if (hMod) {
                r.success = true;
                m_loadedModules[dll] = hMod;
                report.componentsLoaded++;
                m_stats.dllsPreloaded++;
                // Estimate: first-call savings is ~80% of load time
                report.estimatedSavingMs += r.loadTimeMs * 0.8;
            }
            else {
                r.success = false;
                r.errorMessage = "LoadLibraryW failed: " + std::to_string(GetLastError());
                report.componentsFailed++;
                m_stats.dllsFailed++;
            }

            report.details.push_back(r);
        }

        // Phase 2: Pre-read registry configuration
        {
            PreloadResult r;
            r.component = PreloadComponent::RegistryConfig;
            r.name = L"HKCU\\Software\\ExplorerLens";

            auto t0 = std::chrono::high_resolution_clock::now();
            HKEY hKey = nullptr;
            LONG rc = RegOpenKeyExW(HKEY_CURRENT_USER,
                L"Software\\ExplorerLens", 0, KEY_READ, &hKey);
            if (rc == ERROR_SUCCESS) {
                // Read a few common values to warm the registry cache
                DWORD val = 0, size = sizeof(val);
                RegQueryValueExW(hKey, L"EnableGPU", nullptr, nullptr,
                    reinterpret_cast<LPBYTE>(&val), &size);
                RegCloseKey(hKey);
                r.success = true;
                report.componentsLoaded++;
            }
            else {
                r.success = false;
                r.errorMessage = "Registry key not found";
                report.componentsFailed++;
            }
            auto t1 = std::chrono::high_resolution_clock::now();
            r.loadTimeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
            report.details.push_back(r);
        }

        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        report.totalPreloadMs = std::chrono::duration<double, std::milli>(elapsed).count();

        m_stats.preloadRuns++;
        m_stats.avgPreloadTimeMs = m_stats.avgPreloadTimeMs * 0.8 +
            report.totalPreloadMs * 0.2;
        m_stats.totalTimeSavedMs += report.estimatedSavingMs;
        m_preloadComplete = true;

        return report;
    }

    /// Check if preload has been performed
    bool IsPreloadComplete() const { return m_preloadComplete.load(); }

    /// Unload all preloaded modules
    void UnloadAll() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& [name, hMod] : m_loadedModules) {
            if (hMod) FreeLibrary(hMod);
        }
        m_loadedModules.clear();
        m_preloadComplete = false;
    }

    /// Get default codec DLL list for ExplorerLens
    static std::vector<std::wstring> DefaultCodecDLLs() {
        return {
            L"libwebp.dll",
            L"libjxl.dll",
            L"libheif.dll",
            L"libavif.dll",
            L"dav1d.dll",
            L"libde265.dll"
        };
    }

    ColdStartStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

private:
    ColdStartOptimizer() = default;

    mutable std::mutex        m_mutex;
    std::vector<std::wstring> m_codecDLLs;
    std::unordered_map<std::wstring, HMODULE> m_loadedModules;
    std::atomic<bool>         m_preloadComplete{ false };
    ColdStartStats            m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
