//==============================================================================
// DarkThumbs Engine — Lazy Loading Codec Manager
// Sprint 36+: Execution Optimization — Per-Format DLL Architecture
// Copyright (c) 2026 — DarkThumbs Project
//
// PURPOSE:
//   High-level manager that bridges the existing ThumbnailPipeline with the
//   new modular codec DLL system.  It intercepts decode requests, determines
//   whether to use the in-process WIC path or delegate to an on-demand
//   loaded codec DLL, and manages the lifecycle of loaded codecs.
//
// INTEGRATION POINT:
//   ThumbnailPipeline.cpp currently does:
//     decoder = m_decoderRegistry->FindDecoder(filePath);
//     decoder->Decode(request, result);
//
//   With LazyCodecManager, this becomes:
//     if (m_codecManager->ShouldUseModularCodec(filePath)) {
//         m_codecManager->DecodeThumbnail(filePath, width, height, flags, pixelResult);
//         // Convert pixelResult → HBITMAP
//     } else {
//         // Existing in-process decoder path (WIC)
//         decoder = m_decoderRegistry->FindDecoder(filePath);
//         decoder->Decode(request, result);
//     }
//
// FEATURES:
//   1. Directory-aware preloading — scans directory listing to predict
//      which codecs will be needed, batching LoadLibrary calls.
//   2. Memory pressure callbacks — hooks into system memory notifications
//      to proactively unload idle codecs before OOM.
//   3. Decode routing — transparently routes to the right codec DLL.
//   4. Statistics & diagnostics for CBXManager health display.
//==============================================================================

#pragma once

#include "CodecLoader.h"
#include "CodecModuleSpecs.h"
#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <thread>
#include <functional>

namespace DarkThumbs {
namespace Engine {
namespace Codec {

//==============================================================================
// Directory Format Census — snapshot of file types in a folder
//==============================================================================
struct DirectoryFormatCensus
{
    std::wstring                            directoryPath;
    std::unordered_map<std::wstring, uint32_t> extensionCounts;
    uint32_t                                totalFiles = 0;
    std::chrono::steady_clock::time_point   timestamp;

    /// Get the dominant extension (most common file type)
    std::wstring GetDominantExtension() const
    {
        std::wstring dominant;
        uint32_t maxCount = 0;
        for (auto& [ext, count] : extensionCounts) {
            if (count > maxCount) {
                maxCount = count;
                dominant = ext;
            }
        }
        return dominant;
    }

    /// Get unique extensions present
    std::vector<std::wstring> GetUniqueExtensions() const
    {
        std::vector<std::wstring> exts;
        exts.reserve(extensionCounts.size());
        for (auto& [ext, _] : extensionCounts) {
            exts.push_back(ext);
        }
        return exts;
    }

    /// Check if directory is single-format (or nearly so)
    bool IsSingleFormat(double threshold = 0.95) const
    {
        if (totalFiles == 0) return true;
        std::wstring dom = GetDominantExtension();
        auto it = extensionCounts.find(dom);
        if (it == extensionCounts.end()) return true;
        return (static_cast<double>(it->second) / totalFiles) >= threshold;
    }

    /// How many distinct format families (codecs) does this directory need?
    uint32_t GetDistinctCodecCount() const
    {
        // Maps extensions to codec IDs to count unique codec needs
        auto specs = GetAllCodecSpecs();
        std::unordered_set<std::string> codecs;
        for (auto& [ext, _] : extensionCounts) {
            for (auto& spec : specs) {
                for (auto& sExt : spec.extensions) {
                    if (sExt == ext) {
                        codecs.insert(spec.codecId);
                        break;
                    }
                }
            }
        }
        return static_cast<uint32_t>(codecs.size());
    }
};

//==============================================================================
// Preload Strategy — how aggressively to preload codecs
//==============================================================================
enum class PreloadStrategy : uint8_t
{
    None,           ///< Never preload — always lazy-load on first decode
    SingleFormat,   ///< Preload only if directory is 95%+ one format
    TopN,           ///< Preload top N most common formats in directory
    All,            ///< Preload all formats found in directory listing
};

//==============================================================================
// Lazy Codec Manager Configuration
//==============================================================================
struct LazyCodecManagerConfig
{
    /// Codec loader configuration
    CodecLoaderConfig   loaderConfig;

    /// Preload strategy
    PreloadStrategy     preloadStrategy = PreloadStrategy::SingleFormat;

    /// For TopN strategy: how many format codecs to preload
    uint32_t            topNPreload = 3;

    /// Directory census cache TTL (ms) — avoid re-scanning too often
    uint64_t            censusCacheTTlMs = 30000;  // 30 sec

    /// Maximum files to scan during census (avoid blocking on huge dirs)
    uint32_t            maxCensusScanFiles = 5000;

    /// Enable memory pressure monitoring via CreateMemoryResourceNotification
    bool                enableMemoryPressureMonitor = true;

    /// Low-memory threshold: if available physical RAM drops below this %,
    /// evict idle codecs proactively
    double              lowMemoryThresholdPercent = 15.0;

    /// Convert pixel data to HBITMAP automatically
    bool                autoConvertToHBitmap = true;
};

//==============================================================================
// Decode Result with HBITMAP conversion
//==============================================================================
struct ModularDecodeResult
{
    /// Decoded pixel data (owned by codec DLL, freed via FreeResult)
    DtDecodeResult  rawResult{};

    /// Codec that performed the decode (for FreeResult delegation)
    std::string     codecId;

    /// Converted HBITMAP (if autoConvertToHBitmap is enabled)
    /// Caller must DeleteObject() if non-null
    HBITMAP         hBitmap = nullptr;

    /// Actual dimensions
    uint32_t        width = 0;
    uint32_t        height = 0;

    /// Whether this used the modular path (true) or fell back to in-process
    bool            usedModularPath = false;

    /// Decode latency in microseconds
    uint64_t        decodeTimeUs = 0;
};

//==============================================================================
// LazyCodecManager — Main orchestrator
//==============================================================================
class LazyCodecManager
{
public:
    explicit LazyCodecManager(const LazyCodecManagerConfig& config = {})
        : m_config(config)
        , m_loader(config.loaderConfig)
    {
    }

    ~LazyCodecManager()
    {
        Shutdown();
    }

    //--------------------------------------------------------------------------
    // Lifecycle
    //--------------------------------------------------------------------------

    uint32_t Initialize()
    {
        uint32_t err = m_loader.Initialize();
        if (err != 0) return err;

        // Start memory pressure monitor
        if (m_config.enableMemoryPressureMonitor) {
            StartMemoryPressureMonitor();
        }

        m_initialized = true;
        return 0;
    }

    void Shutdown()
    {
        m_shutdownRequested = true;

        if (m_memoryMonitorThread.joinable()) {
            m_memoryMonitorThread.join();
        }

        m_loader.Shutdown();
        m_initialized = false;
    }

    //--------------------------------------------------------------------------
    // Core API
    //--------------------------------------------------------------------------

    /// Check if a file path should use the modular codec path
    bool ShouldUseModularCodec(const std::wstring& filePath) const
    {
        std::wstring ext = GetExtension(filePath);
        return m_loader.HasCodecForExtension(ext);
    }

    /// Decode a thumbnail using the modular codec system.
    /// Returns 0 on success.  Fills result with pixel data and optional HBITMAP.
    uint32_t DecodeThumbnail(const std::wstring& filePath,
                             uint32_t maxWidth, uint32_t maxHeight,
                             uint32_t flags,
                             ModularDecodeResult& result)
    {
        if (!m_initialized) return ERROR_NOT_READY;

        auto startTime = std::chrono::steady_clock::now();

        // Find codec for this extension
        std::wstring ext = GetExtension(filePath);
        std::string codecId = m_loader.FindCodecForExtension(ext);
        if (codecId.empty()) return ERROR_NOT_SUPPORTED;

        result.codecId = codecId;
        result.usedModularPath = true;

        // Decode through CodecLoader
        uint32_t err = m_loader.DecodeThumbnail(
            filePath, maxWidth, maxHeight, flags, result.rawResult);

        if (err != 0) return err;
        if (result.rawResult.errorCode != 0) return result.rawResult.errorCode;

        result.width = result.rawResult.width;
        result.height = result.rawResult.height;

        // Convert to HBITMAP if requested
        if (m_config.autoConvertToHBitmap && result.rawResult.pixelData) {
            result.hBitmap = ConvertToHBitmap(
                result.rawResult.pixelData,
                result.rawResult.width,
                result.rawResult.height,
                result.rawResult.stride,
                result.rawResult.pixelFormat);
        }

        auto elapsed = std::chrono::steady_clock::now() - startTime;
        result.decodeTimeUs = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());

        m_totalDecodes.fetch_add(1, std::memory_order_relaxed);
        return 0;
    }

    /// Free the decode result (pixel data + HBITMAP)
    void FreeResult(ModularDecodeResult& result)
    {
        if (!result.codecId.empty()) {
            m_loader.FreeResult(result.codecId, result.rawResult);
        }
        if (result.hBitmap) {
            ::DeleteObject(result.hBitmap);
            result.hBitmap = nullptr;
        }
    }

    //--------------------------------------------------------------------------
    // Directory-Aware Preloading
    //--------------------------------------------------------------------------

    /// Scan a directory and preload codecs based on the configured strategy.
    /// Call this when the user navigates to a new folder in Explorer.
    void PreloadForDirectory(const std::wstring& directoryPath)
    {
        if (!m_initialized) return;

        // Check census cache
        {
            std::lock_guard<std::mutex> lk(m_censusMutex);
            auto it = m_censusCache.find(directoryPath);
            if (it != m_censusCache.end()) {
                auto age = std::chrono::steady_clock::now() - it->second.timestamp;
                if (std::chrono::duration_cast<std::chrono::milliseconds>(age).count()
                    < static_cast<int64_t>(m_config.censusCacheTTlMs)) {
                    // Cache hit — preload based on cached census
                    ApplyPreloadStrategy(it->second);
                    return;
                }
            }
        }

        // Take census of directory
        DirectoryFormatCensus census = ScanDirectory(directoryPath);

        // Cache it
        {
            std::lock_guard<std::mutex> lk(m_censusMutex);
            m_censusCache[directoryPath] = census;
        }

        // Apply preload strategy
        ApplyPreloadStrategy(census);
    }

    /// Get the last census for a directory (if cached)
    bool GetCachedCensus(const std::wstring& dirPath, DirectoryFormatCensus& out) const
    {
        std::lock_guard<std::mutex> lk(m_censusMutex);
        auto it = m_censusCache.find(dirPath);
        if (it == m_censusCache.end()) return false;
        out = it->second;
        return true;
    }

    /// Calculate memory impact for a directory's file types
    MemoryImpactReport GetMemoryImpact(const std::wstring& directoryPath)
    {
        DirectoryFormatCensus census;
        bool cached = GetCachedCensus(directoryPath, census);
        if (!cached) {
            census = ScanDirectory(directoryPath);
        }
        return AnalyzeMemoryImpact(census.GetUniqueExtensions());
    }

    //--------------------------------------------------------------------------
    // Statistics & Diagnostics
    //--------------------------------------------------------------------------

    CodecLoaderStats GetLoaderStats() const { return m_loader.GetStats(); }

    uint32_t GetLoadedCodecCount() const { return m_loader.GetLoadedCodecCount(); }

    uint64_t GetCurrentMemoryEstimate() const { return m_loader.GetCurrentMemoryEstimate(); }

    uint64_t GetTotalDecodes() const {
        return m_totalDecodes.load(std::memory_order_relaxed);
    }

    std::vector<std::string> GetLoadedCodecs() const { return m_loader.GetLoadedCodecs(); }

    /// Get a compact diagnostic string for CBXManager health display
    std::string GetDiagnosticsSummary() const
    {
        auto stats = m_loader.GetStats();
        std::string summary;
        summary += "Modular Codec System\n";
        summary += "  Loaded codecs: " + std::to_string(stats.currentLoadedCodecs) + "\n";
        summary += "  Memory (est.): " + std::to_string(stats.currentMemoryBytes / (1024*1024)) + " MB\n";
        summary += "  Total decodes: " + std::to_string(stats.totalDecodes) + "\n";
        summary += "  Evictions:     " + std::to_string(stats.evictions) + "\n";
        summary += "  Avg load time: " + std::to_string(static_cast<int>(stats.avgLoadTimeMs)) + " ms\n";
        return summary;
    }

private:
    //--------------------------------------------------------------------------
    // Directory Scanning
    //--------------------------------------------------------------------------
    DirectoryFormatCensus ScanDirectory(const std::wstring& dirPath)
    {
        DirectoryFormatCensus census;
        census.directoryPath = dirPath;
        census.timestamp = std::chrono::steady_clock::now();

        std::wstring searchPath = dirPath;
        if (!searchPath.empty() && searchPath.back() != L'\\')
            searchPath += L'\\';
        searchPath += L"*";

        WIN32_FIND_DATAW fd{};
        HANDLE hFind = ::FindFirstFileW(searchPath.c_str(), &fd);
        if (hFind == INVALID_HANDLE_VALUE) return census;

        uint32_t scanned = 0;
        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

            std::wstring fileName(fd.cFileName);
            std::wstring ext = GetExtension(fileName);
            if (!ext.empty()) {
                census.extensionCounts[ext]++;
                census.totalFiles++;
            }

            scanned++;
        } while (::FindNextFileW(hFind, &fd) && scanned < m_config.maxCensusScanFiles);

        ::FindClose(hFind);
        return census;
    }

    //--------------------------------------------------------------------------
    // Preload Strategy Application
    //--------------------------------------------------------------------------
    void ApplyPreloadStrategy(const DirectoryFormatCensus& census)
    {
        if (m_config.preloadStrategy == PreloadStrategy::None) return;
        if (census.totalFiles == 0) return;

        std::vector<std::wstring> extensionsToPreload;

        switch (m_config.preloadStrategy) {
        case PreloadStrategy::SingleFormat:
            // Only preload if directory is dominated by one format
            if (census.IsSingleFormat(0.95)) {
                std::wstring dom = census.GetDominantExtension();
                if (!dom.empty()) extensionsToPreload.push_back(dom);
            }
            break;

        case PreloadStrategy::TopN:
        {
            // Sort extensions by count, preload top N
            std::vector<std::pair<std::wstring, uint32_t>> sorted(
                census.extensionCounts.begin(), census.extensionCounts.end());
            std::sort(sorted.begin(), sorted.end(),
                      [](const auto& a, const auto& b) { return a.second > b.second; });

            uint32_t n = std::min(m_config.topNPreload,
                                  static_cast<uint32_t>(sorted.size()));
            for (uint32_t i = 0; i < n; i++) {
                extensionsToPreload.push_back(sorted[i].first);
            }
            break;
        }

        case PreloadStrategy::All:
            extensionsToPreload = census.GetUniqueExtensions();
            break;

        default:
            break;
        }

        // Preload codec DLLs for the selected extensions
        for (auto& ext : extensionsToPreload) {
            std::string codecId = m_loader.FindCodecForExtension(ext);
            if (!codecId.empty()) {
                m_loader.EnsureLoaded(codecId);
            }
        }
    }

    //--------------------------------------------------------------------------
    // HBITMAP Conversion (pixelData → GDI bitmap)
    //--------------------------------------------------------------------------
    static HBITMAP ConvertToHBitmap(const uint8_t* pixelData, uint32_t width,
                                    uint32_t height, uint32_t stride,
                                    DtPixelFormat format)
    {
        if (!pixelData || width == 0 || height == 0) return nullptr;

        // Create a 32-bit DIB section
        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = static_cast<LONG>(width);
        bmi.bmiHeader.biHeight = -static_cast<LONG>(height); // top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* bits = nullptr;
        HDC hdc = ::GetDC(nullptr);
        HBITMAP hBitmap = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
        ::ReleaseDC(nullptr, hdc);

        if (!hBitmap || !bits) return nullptr;

        uint32_t dstStride = width * 4;
        uint8_t* dst = static_cast<uint8_t*>(bits);

        for (uint32_t y = 0; y < height; y++) {
            const uint8_t* src = pixelData + y * stride;
            uint8_t* dstRow = dst + y * dstStride;

            switch (format) {
            case DT_PIXEL_BGRA32:
                // Direct copy — native GDI format
                memcpy(dstRow, src, width * 4);
                break;

            case DT_PIXEL_BGR24:
                // Expand 24-bit to 32-bit
                for (uint32_t x = 0; x < width; x++) {
                    dstRow[x * 4 + 0] = src[x * 3 + 0]; // B
                    dstRow[x * 4 + 1] = src[x * 3 + 1]; // G
                    dstRow[x * 4 + 2] = src[x * 3 + 2]; // R
                    dstRow[x * 4 + 3] = 255;             // A
                }
                break;

            case DT_PIXEL_RGBA32:
                // Swizzle RGBA → BGRA
                for (uint32_t x = 0; x < width; x++) {
                    dstRow[x * 4 + 0] = src[x * 4 + 2]; // B ← R
                    dstRow[x * 4 + 1] = src[x * 4 + 1]; // G
                    dstRow[x * 4 + 2] = src[x * 4 + 0]; // R ← B
                    dstRow[x * 4 + 3] = src[x * 4 + 3]; // A
                }
                break;
            }
        }

        return hBitmap;
    }

    //--------------------------------------------------------------------------
    // Memory Pressure Monitoring (Windows Low-Memory Notification)
    //--------------------------------------------------------------------------
    void StartMemoryPressureMonitor()
    {
        m_memoryMonitorThread = std::thread([this]() {
            HANDLE hLowMem = ::CreateMemoryResourceNotification(LowMemoryResourceNotification);
            if (!hLowMem) return;

            while (!m_shutdownRequested.load(std::memory_order_relaxed)) {
                DWORD waitResult = ::WaitForSingleObject(hLowMem, 5000); // 5s poll
                if (waitResult == WAIT_OBJECT_0) {
                    // Low memory — evict idle codecs
                    m_loader.EvictIdleCodecs();
                    m_memoryPressureEvents.fetch_add(1, std::memory_order_relaxed);
                }
            }

            ::CloseHandle(hLowMem);
        });
    }

    //--------------------------------------------------------------------------
    // Utility
    //--------------------------------------------------------------------------
    static std::wstring GetExtension(const std::wstring& path)
    {
        auto dot = path.rfind(L'.');
        if (dot == std::wstring::npos) return {};
        std::wstring ext = path.substr(dot);
        for (auto& c : ext) {
            if (c >= L'A' && c <= L'Z') c += 32;
        }
        return ext;
    }

    //--------------------------------------------------------------------------
    // Data Members
    //--------------------------------------------------------------------------
    LazyCodecManagerConfig              m_config;
    CodecLoader                         m_loader;
    bool                                m_initialized = false;
    std::atomic<bool>                   m_shutdownRequested{false};
    std::atomic<uint64_t>               m_totalDecodes{0};
    std::atomic<uint64_t>               m_memoryPressureEvents{0};
    std::thread                         m_memoryMonitorThread;

    /// Directory census cache
    mutable std::mutex                  m_censusMutex;
    std::unordered_map<std::wstring, DirectoryFormatCensus> m_censusCache;
};

} // namespace Codec
} // namespace Engine
} // namespace DarkThumbs
