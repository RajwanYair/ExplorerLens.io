//==============================================================================
// DarkThumbs Engine — Codec Loader (Demand-Loaded Codec DLL Registry)
// Sprint 36+: Execution Optimization — Per-Format DLL Architecture
// Copyright (c) 2026 — DarkThumbs Project
//
// PURPOSE:
//   Discover, load, and manage codec DLLs at runtime.  LoadLibrary is
//   DEFERRED until a file extension actually needs that codec, so browsing
//   a folder of PNGs never pages in libheif/libjxl/LibRaw.
//
// LOADING STRATEGY:
//   1. At startup, scan the codec directory for DarkThumbs_Codec_*.dll
//   2. Read the extension→codec mapping from a sidecar manifest
//      (codec-manifest.json) — NO LoadLibrary needed for mapping
//   3. On first decode request for an extension, LoadLibrary + resolve
//      function pointers once (double-checked locking)
//   4. Codec stays loaded until idle timeout or explicit unload
//
// MEMORY BUDGET:
//   The loader tracks per-codec memory and can evict idle codecs when the
//   total working-set exceeds a configurable budget (default 128 MB).
//   WIC-based codecs (JPEG/PNG/BMP/GIF/TIFF) are always in-process (no DLL)
//   because they use the OS imaging pipeline with negligible overhead.
//==============================================================================

#pragma once

#include "ICodecModule.h"
#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <memory>
#include <algorithm>
#include <functional>

namespace DarkThumbs {
namespace Engine {
namespace Codec {

//==============================================================================
// Codec Load State
//==============================================================================
enum class CodecState : uint8_t
{
    Discovered,     ///< Manifest entry known — DLL not loaded
    Loading,        ///< LoadLibrary in progress (transient)
    Ready,          ///< DLL loaded, functions resolved, Initialize() called
    Error,          ///< LoadLibrary or Initialize() failed
    Unloaded,       ///< Explicitly unloaded (Shutdown + FreeLibrary)
};

//==============================================================================
// Per-Codec DLL Handle + Resolved Functions
//==============================================================================
struct CodecHandle
{
    /// DLL path (absolute)
    std::wstring                dllPath;

    /// Codec identifier from manifest (e.g. "darkthumbs.codec.webp")
    std::string                 codecId;

    /// Module handle (nullptr if not loaded)
    HMODULE                     hModule = nullptr;

    /// Resolved function pointers (nullptr until loaded)
    PFN_DtCodec_Initialize      pfnInitialize      = nullptr;
    PFN_DtCodec_GetModuleInfo   pfnGetModuleInfo    = nullptr;
    PFN_DtCodec_DecodeThumbnail pfnDecodeThumbnail  = nullptr;
    PFN_DtCodec_FreeResult      pfnFreeResult       = nullptr;
    PFN_DtCodec_Shutdown        pfnShutdown         = nullptr;
    PFN_DtCodec_GetHealth       pfnGetHealth        = nullptr;  // optional

    /// Cached module info (after first GetModuleInfo call)
    DtCodecModuleInfo           info{};
    std::vector<std::wstring>   ownedExtensions;   // keeps extension strings alive

    /// Runtime state
    CodecState                  state = CodecState::Discovered;
    std::atomic<uint64_t>       decodeCount{0};
    std::atomic<uint64_t>       errorCount{0};
    std::atomic<uint64_t>       lastUseTimestamp{0};   // epoch ms
    uint64_t                    loadTimestamp = 0;
    uint64_t                    estimatedMemory = 0;

    /// Per-codec mutex for load/unload serialisation
    std::mutex                  loadMutex;

    bool IsLoaded() const { return state == CodecState::Ready && hModule != nullptr; }

    uint64_t IdleMs() const {
        auto now = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
        uint64_t last = lastUseTimestamp.load(std::memory_order_relaxed);
        return (last == 0) ? UINT64_MAX : (now > last ? now - last : 0);
    }

    void TouchTimestamp() {
        lastUseTimestamp.store(
            static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count()),
            std::memory_order_relaxed);
    }
};

//==============================================================================
// Codec Manifest Entry — parsed from codec-manifest.json (sidecar file)
//
// The manifest provides extension→codec mapping WITHOUT loading the DLL.
// Format (JSON):
//   {
//     "codecs": [
//       {
//         "id": "darkthumbs.codec.webp",
//         "dll": "DarkThumbs_Codec_WebP.dll",
//         "extensions": [".webp"],
//         "estimatedMemoryMB": 4,
//         "priority": 100,
//         "dependencies": ["libwebp.dll", "libsharpyuv.dll"]
//       },
//       ...
//     ]
//   }
//==============================================================================
struct CodecManifestEntry
{
    std::string                 id;
    std::wstring                dllName;
    std::vector<std::wstring>   extensions;
    uint64_t                    estimatedMemoryBytes = 0;
    int                         priority = 100;    // lower = higher priority
    std::vector<std::wstring>   dependencies;      // prerequisite DLLs
};

//==============================================================================
// Loader Configuration
//==============================================================================
struct CodecLoaderConfig
{
    /// Directory containing codec DLLs (default: same dir as CBXShell.dll)
    std::wstring    codecDirectory;

    /// Path to codec-manifest.json (default: codecDirectory / codec-manifest.json)
    std::wstring    manifestPath;

    /// Maximum total memory budget for loaded codecs (bytes, default 128 MB)
    uint64_t        memoryBudgetBytes = 128ULL * 1024 * 1024;

    /// Idle timeout before a codec is eligible for unloading (ms, default 5 min)
    uint64_t        idleTimeoutMs = 5 * 60 * 1000;

    /// Auto-evict idle codecs when budget exceeded
    bool            autoEvict = true;

    /// Preload WIC-based codecs (JPEG/PNG/BMP/GIF/TIFF) — these are in-process
    bool            preloadWIC = true;

    /// Enable parallel DLL loading for startup preload
    bool            parallelPreload = false;

    /// Maximum concurrent codec loads
    uint32_t        maxConcurrentLoads = 2;
};

//==============================================================================
// Load / Decode Statistics
//==============================================================================
struct CodecLoaderStats
{
    uint64_t    totalLoads = 0;          ///< Number of LoadLibrary calls
    uint64_t    totalUnloads = 0;        ///< Number of FreeLibrary calls
    uint64_t    totalDecodes = 0;        ///< Total decode requests dispatched
    uint64_t    totalErrors = 0;         ///< Failed decodes
    uint64_t    loadFailures = 0;        ///< LoadLibrary failures
    uint64_t    evictions = 0;           ///< Budget-triggered unloads
    uint64_t    currentLoadedCodecs = 0; ///< Currently loaded codec DLLs
    uint64_t    currentMemoryBytes = 0;  ///< Estimated memory of loaded codecs
    uint64_t    peakMemoryBytes = 0;     ///< Peak estimated memory
    uint64_t    peakLoadedCodecs = 0;    ///< Peak simultaneously loaded codecs
    double      avgLoadTimeMs = 0.0;     ///< Moving average of LoadLibrary time
};

//==============================================================================
// CodecLoader — Main class
//==============================================================================
class CodecLoader
{
public:
    //--------------------------------------------------------------------------
    // Construction / Lifecycle
    //--------------------------------------------------------------------------
    explicit CodecLoader(const CodecLoaderConfig& config = {})
        : m_config(config)
    {
    }

    ~CodecLoader()
    {
        Shutdown();
    }

    /// Initialize: parse manifest, build extension map.  Does NOT load DLLs.
    uint32_t Initialize()
    {
        std::lock_guard<std::mutex> lk(m_initMutex);
        if (m_initialized) return 0;

        if (m_config.codecDirectory.empty()) {
            m_config.codecDirectory = GetModuleDirectory();
        }
        if (m_config.manifestPath.empty()) {
            m_config.manifestPath = m_config.codecDirectory + L"\\codec-manifest.json";
        }

        // Parse manifest and build extension→codecId map
        auto entries = ParseManifest(m_config.manifestPath);
        {
            std::unique_lock<std::shared_mutex> wl(m_rwMutex);
            for (auto& entry : entries) {
                auto handle = std::make_shared<CodecHandle>();
                handle->dllPath = m_config.codecDirectory + L"\\" + entry.dllName;
                handle->codecId = entry.id;
                handle->estimatedMemory = entry.estimatedMemoryBytes;
                handle->state = CodecState::Discovered;

                for (auto& ext : entry.extensions) {
                    std::wstring lower = ToLower(ext);
                    m_extensionMap[lower] = entry.id;
                }

                m_codecs[entry.id] = std::move(handle);
            }
        }

        // Register built-in WIC extensions (no DLL needed)
        RegisterBuiltInExtensions();

        m_initialized = true;
        return 0;
    }

    /// Shutdown: unload all codec DLLs.
    void Shutdown()
    {
        std::lock_guard<std::mutex> lk(m_initMutex);
        if (!m_initialized) return;

        std::unique_lock<std::shared_mutex> wl(m_rwMutex);
        for (auto& [id, handle] : m_codecs) {
            UnloadCodecInternal(handle.get());
        }
        m_codecs.clear();
        m_extensionMap.clear();
        m_initialized = false;
    }

    //--------------------------------------------------------------------------
    // Core API — Extension-based Lookup & Decode
    //--------------------------------------------------------------------------

    /// Find the codec responsible for a file extension (e.g. L".webp")
    /// Returns the codec ID, or empty string if no codec registered.
    std::string FindCodecForExtension(const std::wstring& extension) const
    {
        std::shared_lock<std::shared_mutex> rl(m_rwMutex);
        std::wstring lower = ToLower(extension);
        auto it = m_extensionMap.find(lower);
        return (it != m_extensionMap.end()) ? it->second : std::string{};
    }

    /// Ensure a codec is loaded (lazy load on first use).
    /// Returns 0 on success, Win32 error on failure.
    uint32_t EnsureLoaded(const std::string& codecId)
    {
        auto handle = GetHandle(codecId);
        if (!handle) return ERROR_NOT_FOUND;
        return LoadCodecIfNeeded(handle.get());
    }

    /// Decode a thumbnail using the appropriate codec.
    /// Automatically loads the codec DLL if not yet loaded.
    uint32_t DecodeThumbnail(const std::wstring& filePath,
                             uint32_t maxWidth, uint32_t maxHeight,
                             uint32_t flags,
                             DtDecodeResult& result)
    {
        // Extract extension
        std::wstring ext = GetExtension(filePath);
        if (ext.empty()) return ERROR_BAD_FORMAT;

        // Find codec
        std::string codecId = FindCodecForExtension(ext);
        if (codecId.empty()) {
            // Check if it's a built-in WIC extension
            if (IsBuiltInExtension(ext)) {
                return UINT32_MAX; // sentinel: use built-in WIC path
            }
            return ERROR_NOT_SUPPORTED;
        }

        // Get handle, load if needed
        auto handle = GetHandle(codecId);
        if (!handle) return ERROR_NOT_FOUND;

        uint32_t err = LoadCodecIfNeeded(handle.get());
        if (err != 0) return err;

        // Budget check — evict idle codecs if over budget
        if (m_config.autoEvict) {
            EnforceMemoryBudget();
        }

        // Build request
        DtDecodeRequest request{};
        request.structSize = sizeof(DtDecodeRequest);
        request.filePath = filePath.c_str();
        request.maxWidth = maxWidth;
        request.maxHeight = maxHeight;
        request.flags = flags;

        result = {};
        result.structSize = sizeof(DtDecodeResult);

        // Decode
        err = handle->pfnDecodeThumbnail(&request, &result);
        handle->TouchTimestamp();
        handle->decodeCount.fetch_add(1, std::memory_order_relaxed);
        m_stats.totalDecodes++;

        if (err != 0 || result.errorCode != 0) {
            handle->errorCount.fetch_add(1, std::memory_order_relaxed);
            m_stats.totalErrors++;
        }

        return err;
    }

    /// Free a decode result (delegates to the owning codec's FreeResult)
    void FreeResult(const std::string& codecId, DtDecodeResult& result)
    {
        auto handle = GetHandle(codecId);
        if (handle && handle->IsLoaded() && handle->pfnFreeResult) {
            handle->pfnFreeResult(&result);
        }
        result.pixelData = nullptr;
    }

    //--------------------------------------------------------------------------
    // Codec Management
    //--------------------------------------------------------------------------

    /// Explicitly unload a codec to reclaim memory.
    void UnloadCodec(const std::string& codecId)
    {
        auto handle = GetHandle(codecId);
        if (handle) {
            std::lock_guard<std::mutex> lk(handle->loadMutex);
            UnloadCodecInternal(handle.get());
            m_stats.totalUnloads++;
            m_stats.evictions++;
        }
    }

    /// Unload all codecs that have been idle longer than the timeout.
    uint32_t EvictIdleCodecs()
    {
        uint32_t evicted = 0;
        std::shared_lock<std::shared_mutex> rl(m_rwMutex);
        for (auto& [id, handle] : m_codecs) {
            if (handle->IsLoaded() && handle->IdleMs() > m_config.idleTimeoutMs) {
                std::lock_guard<std::mutex> lk(handle->loadMutex);
                if (handle->IsLoaded() && handle->IdleMs() > m_config.idleTimeoutMs) {
                    UnloadCodecInternal(handle.get());
                    m_stats.totalUnloads++;
                    m_stats.evictions++;
                    evicted++;
                }
            }
        }
        return evicted;
    }

    /// Get module info for a loaded codec.
    bool GetCodecInfo(const std::string& codecId, DtCodecModuleInfo& out) const
    {
        auto handle = GetHandle(codecId);
        if (!handle || !handle->IsLoaded()) return false;
        out = handle->info;
        return true;
    }

    /// Get health stats for a loaded codec.
    bool GetCodecHealth(const std::string& codecId, DtCodecHealth& out) const
    {
        auto handle = GetHandle(codecId);
        if (!handle || !handle->IsLoaded() || !handle->pfnGetHealth) return false;
        out.structSize = sizeof(DtCodecHealth);
        return (handle->pfnGetHealth(&out) == 0);
    }

    /// Get overall loader statistics.
    CodecLoaderStats GetStats() const { return m_stats; }

    /// Get list of all registered codec IDs.
    std::vector<std::string> GetRegisteredCodecs() const
    {
        std::shared_lock<std::shared_mutex> rl(m_rwMutex);
        std::vector<std::string> ids;
        ids.reserve(m_codecs.size());
        for (auto& [id, _] : m_codecs) {
            ids.push_back(id);
        }
        return ids;
    }

    /// Get list of currently loaded codec IDs.
    std::vector<std::string> GetLoadedCodecs() const
    {
        std::shared_lock<std::shared_mutex> rl(m_rwMutex);
        std::vector<std::string> ids;
        for (auto& [id, handle] : m_codecs) {
            if (handle->IsLoaded()) {
                ids.push_back(id);
            }
        }
        return ids;
    }

    /// Check if a specific file extension is handled by a modular codec
    bool HasCodecForExtension(const std::wstring& extension) const
    {
        return !FindCodecForExtension(extension).empty();
    }

    /// Get the number of currently loaded codec DLLs
    uint32_t GetLoadedCodecCount() const
    {
        std::shared_lock<std::shared_mutex> rl(m_rwMutex);
        uint32_t count = 0;
        for (auto& [_, handle] : m_codecs) {
            if (handle->IsLoaded()) count++;
        }
        return count;
    }

    /// Get estimated current memory used by loaded codecs
    uint64_t GetCurrentMemoryEstimate() const
    {
        std::shared_lock<std::shared_mutex> rl(m_rwMutex);
        uint64_t total = 0;
        for (auto& [_, handle] : m_codecs) {
            if (handle->IsLoaded()) {
                total += handle->estimatedMemory;
            }
        }
        return total;
    }

private:
    //--------------------------------------------------------------------------
    // Internal Helpers
    //--------------------------------------------------------------------------

    std::shared_ptr<CodecHandle> GetHandle(const std::string& codecId) const
    {
        std::shared_lock<std::shared_mutex> rl(m_rwMutex);
        auto it = m_codecs.find(codecId);
        return (it != m_codecs.end()) ? it->second : nullptr;
    }

    uint32_t LoadCodecIfNeeded(CodecHandle* handle)
    {
        if (handle->IsLoaded()) return 0;

        std::lock_guard<std::mutex> lk(handle->loadMutex);
        if (handle->IsLoaded()) return 0;  // double-check
        if (handle->state == CodecState::Error) return ERROR_MOD_NOT_FOUND;

        handle->state = CodecState::Loading;

        auto startTime = std::chrono::steady_clock::now();

        // LoadLibraryW
        handle->hModule = ::LoadLibraryW(handle->dllPath.c_str());
        if (!handle->hModule) {
            handle->state = CodecState::Error;
            m_stats.loadFailures++;
            return ::GetLastError();
        }

        // Resolve mandatory exports
        handle->pfnInitialize     = reinterpret_cast<PFN_DtCodec_Initialize>(
            ::GetProcAddress(handle->hModule, DTCODEC_FN_INITIALIZE));
        handle->pfnGetModuleInfo  = reinterpret_cast<PFN_DtCodec_GetModuleInfo>(
            ::GetProcAddress(handle->hModule, DTCODEC_FN_GETMODULEINFO));
        handle->pfnDecodeThumbnail = reinterpret_cast<PFN_DtCodec_DecodeThumbnail>(
            ::GetProcAddress(handle->hModule, DTCODEC_FN_DECODETHUMBNAIL));
        handle->pfnFreeResult     = reinterpret_cast<PFN_DtCodec_FreeResult>(
            ::GetProcAddress(handle->hModule, DTCODEC_FN_FREERESULT));
        handle->pfnShutdown       = reinterpret_cast<PFN_DtCodec_Shutdown>(
            ::GetProcAddress(handle->hModule, DTCODEC_FN_SHUTDOWN));

        // Optional health
        handle->pfnGetHealth = reinterpret_cast<PFN_DtCodec_GetHealth>(
            ::GetProcAddress(handle->hModule, DTCODEC_FN_GETHEALTH));

        // Check mandatory
        if (!handle->pfnInitialize || !handle->pfnGetModuleInfo ||
            !handle->pfnDecodeThumbnail || !handle->pfnFreeResult || !handle->pfnShutdown) {
            ::FreeLibrary(handle->hModule);
            handle->hModule = nullptr;
            handle->state = CodecState::Error;
            m_stats.loadFailures++;
            return ERROR_PROC_NOT_FOUND;
        }

        // Initialize
        uint32_t err = handle->pfnInitialize();
        if (err != 0) {
            ::FreeLibrary(handle->hModule);
            handle->hModule = nullptr;
            handle->state = CodecState::Error;
            m_stats.loadFailures++;
            return err;
        }

        // Get module info
        handle->info = {};
        handle->info.structSize = sizeof(DtCodecModuleInfo);
        handle->pfnGetModuleInfo(&handle->info);

        // Update memory estimate from codec info if available
        if (handle->info.estimatedMemoryBytes > 0) {
            handle->estimatedMemory = handle->info.estimatedMemoryBytes;
        }

        auto elapsed = std::chrono::steady_clock::now() - startTime;
        double loadMs = std::chrono::duration<double, std::milli>(elapsed).count();

        handle->loadTimestamp = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
        handle->TouchTimestamp();
        handle->state = CodecState::Ready;

        // Stats
        m_stats.totalLoads++;
        m_stats.currentLoadedCodecs++;
        m_stats.currentMemoryBytes += handle->estimatedMemory;
        if (m_stats.currentMemoryBytes > m_stats.peakMemoryBytes)
            m_stats.peakMemoryBytes = m_stats.currentMemoryBytes;
        if (m_stats.currentLoadedCodecs > m_stats.peakLoadedCodecs)
            m_stats.peakLoadedCodecs = m_stats.currentLoadedCodecs;

        // Rolling avg load time
        double prev = m_stats.avgLoadTimeMs;
        m_stats.avgLoadTimeMs = (prev * (m_stats.totalLoads - 1) + loadMs) / m_stats.totalLoads;

        return 0;
    }

    void UnloadCodecInternal(CodecHandle* handle)
    {
        if (!handle->hModule) return;

        if (handle->pfnShutdown) {
            handle->pfnShutdown();
        }

        ::FreeLibrary(handle->hModule);
        handle->hModule = nullptr;
        handle->pfnInitialize = nullptr;
        handle->pfnGetModuleInfo = nullptr;
        handle->pfnDecodeThumbnail = nullptr;
        handle->pfnFreeResult = nullptr;
        handle->pfnShutdown = nullptr;
        handle->pfnGetHealth = nullptr;
        handle->state = CodecState::Unloaded;

        if (m_stats.currentLoadedCodecs > 0)
            m_stats.currentLoadedCodecs--;
        if (m_stats.currentMemoryBytes >= handle->estimatedMemory)
            m_stats.currentMemoryBytes -= handle->estimatedMemory;
    }

    void EnforceMemoryBudget()
    {
        if (GetCurrentMemoryEstimate() <= m_config.memoryBudgetBytes) return;

        // Collect loaded codecs sorted by idle time (most idle first)
        struct Candidate {
            std::string id;
            uint64_t idleMs;
        };
        std::vector<Candidate> candidates;

        {
            std::shared_lock<std::shared_mutex> rl(m_rwMutex);
            for (auto& [id, handle] : m_codecs) {
                if (handle->IsLoaded()) {
                    candidates.push_back({id, handle->IdleMs()});
                }
            }
        }

        std::sort(candidates.begin(), candidates.end(),
                  [](const Candidate& a, const Candidate& b) {
                      return a.idleMs > b.idleMs;
                  });

        for (auto& c : candidates) {
            if (GetCurrentMemoryEstimate() <= m_config.memoryBudgetBytes) break;
            UnloadCodec(c.id);
        }
    }

    //--------------------------------------------------------------------------
    // Manifest Parser (lightweight JSON — no external dependency)
    //--------------------------------------------------------------------------
    std::vector<CodecManifestEntry> ParseManifest(const std::wstring& path)
    {
        std::vector<CodecManifestEntry> entries;

        // Open file
        HANDLE hFile = ::CreateFileW(path.c_str(), GENERIC_READ,
                                     FILE_SHARE_READ, nullptr,
                                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return entries;

        DWORD fileSize = ::GetFileSize(hFile, nullptr);
        if (fileSize == 0 || fileSize > 1024 * 1024) {
            ::CloseHandle(hFile);
            return entries;
        }

        std::string json(fileSize, '\0');
        DWORD bytesRead = 0;
        ::ReadFile(hFile, json.data(), fileSize, &bytesRead, nullptr);
        ::CloseHandle(hFile);

        if (bytesRead != fileSize) return entries;

        // Minimal JSON parser — find "codecs" array entries
        // Each entry: { "id", "dll", "extensions", "estimatedMemoryMB", "priority" }
        // This is a simplified parser for our known schema
        size_t pos = 0;
        while ((pos = json.find("\"id\"", pos)) != std::string::npos) {
            CodecManifestEntry entry;

            // Parse id
            entry.id = ExtractJsonString(json, pos, "id");
            if (entry.id.empty()) { pos++; continue; }

            // Parse dll
            std::string dll = ExtractJsonString(json, pos, "dll");
            if (!dll.empty()) {
                entry.dllName = std::wstring(dll.begin(), dll.end());
            }

            // Parse extensions array
            size_t extPos = json.find("\"extensions\"", pos);
            if (extPos != std::string::npos) {
                size_t arrStart = json.find('[', extPos);
                size_t arrEnd = json.find(']', arrStart);
                if (arrStart != std::string::npos && arrEnd != std::string::npos) {
                    std::string arr = json.substr(arrStart + 1, arrEnd - arrStart - 1);
                    size_t epos = 0;
                    while ((epos = arr.find('"', epos)) != std::string::npos) {
                        size_t eend = arr.find('"', epos + 1);
                        if (eend != std::string::npos) {
                            std::string ext = arr.substr(epos + 1, eend - epos - 1);
                            entry.extensions.push_back(std::wstring(ext.begin(), ext.end()));
                            epos = eend + 1;
                        } else break;
                    }
                }
            }

            // Parse estimatedMemoryMB
            std::string mem = ExtractJsonNumber(json, pos, "estimatedMemoryMB");
            if (!mem.empty()) {
                entry.estimatedMemoryBytes = static_cast<uint64_t>(std::stod(mem) * 1024 * 1024);
            }

            // Parse priority
            std::string pri = ExtractJsonNumber(json, pos, "priority");
            if (!pri.empty()) {
                entry.priority = std::stoi(pri);
            }

            if (!entry.id.empty() && !entry.dllName.empty()) {
                entries.push_back(std::move(entry));
            }

            pos++;
        }

        return entries;
    }

    std::string ExtractJsonString(const std::string& json, size_t searchFrom,
                                  const std::string& key)
    {
        std::string search = "\"" + key + "\"";
        size_t keyPos = json.find(search, searchFrom);
        if (keyPos == std::string::npos) return {};
        size_t colon = json.find(':', keyPos);
        if (colon == std::string::npos) return {};
        size_t valStart = json.find('"', colon + 1);
        if (valStart == std::string::npos) return {};
        size_t valEnd = json.find('"', valStart + 1);
        if (valEnd == std::string::npos) return {};
        return json.substr(valStart + 1, valEnd - valStart - 1);
    }

    std::string ExtractJsonNumber(const std::string& json, size_t searchFrom,
                                  const std::string& key)
    {
        std::string search = "\"" + key + "\"";
        size_t keyPos = json.find(search, searchFrom);
        if (keyPos == std::string::npos) return {};
        size_t colon = json.find(':', keyPos);
        if (colon == std::string::npos) return {};
        size_t numStart = colon + 1;
        while (numStart < json.size() && (json[numStart] == ' ' || json[numStart] == '\t'))
            numStart++;
        size_t numEnd = numStart;
        while (numEnd < json.size() && (std::isdigit(json[numEnd]) || json[numEnd] == '.'))
            numEnd++;
        if (numEnd == numStart) return {};
        return json.substr(numStart, numEnd - numStart);
    }

    //--------------------------------------------------------------------------
    // Built-in WIC extension registration
    //--------------------------------------------------------------------------
    void RegisterBuiltInExtensions()
    {
        // These extensions are handled by the in-process WIC path (ImageDecoder)
        // No external DLL needed — no memory overhead.
        static const wchar_t* wicExts[] = {
            L".jpg", L".jpeg", L".jpe", L".jfif",
            L".png",
            L".bmp", L".dib",
            L".gif",
            L".tif", L".tiff",
            L".ico", L".cur",         // ICODecoder (WIC-based)
            L".dds",                   // DDSDecoder (WIC + D3D11)
            L".exr",                   // EXRDecoder (WIC + MS codec)
            nullptr
        };

        for (int i = 0; wicExts[i]; i++) {
            m_builtInExtensions.insert(ToLower(wicExts[i]));
        }
    }

    bool IsBuiltInExtension(const std::wstring& ext) const
    {
        return m_builtInExtensions.count(ToLower(ext)) > 0;
    }

    //--------------------------------------------------------------------------
    // Utility
    //--------------------------------------------------------------------------
    static std::wstring ToLower(const std::wstring& s)
    {
        std::wstring result = s;
        for (auto& c : result) {
            if (c >= L'A' && c <= L'Z') c += 32;
        }
        return result;
    }

    static std::wstring GetExtension(const std::wstring& path)
    {
        auto dot = path.rfind(L'.');
        if (dot == std::wstring::npos) return {};
        return ToLower(path.substr(dot));
    }

    static std::wstring GetModuleDirectory()
    {
        wchar_t buf[MAX_PATH]{};
        HMODULE hMod = nullptr;
        ::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                             GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                             reinterpret_cast<LPCWSTR>(&GetModuleDirectory),
                             &hMod);
        ::GetModuleFileNameW(hMod, buf, MAX_PATH);
        std::wstring path(buf);
        auto slash = path.rfind(L'\\');
        return (slash != std::wstring::npos) ? path.substr(0, slash) : path;
    }

    //--------------------------------------------------------------------------
    // Data Members
    //--------------------------------------------------------------------------
    CodecLoaderConfig                                           m_config;
    CodecLoaderStats                                            m_stats;
    bool                                                        m_initialized = false;
    std::mutex                                                  m_initMutex;
    mutable std::shared_mutex                                   m_rwMutex;

    /// codecId → CodecHandle
    std::unordered_map<std::string, std::shared_ptr<CodecHandle>> m_codecs;

    /// extension → codecId (lowercase, with dot)
    std::unordered_map<std::wstring, std::string>               m_extensionMap;

    /// Built-in WIC extensions (always in-process, no DLL)
    std::unordered_set<std::wstring>                            m_builtInExtensions;
};

} // namespace Codec
} // namespace Engine
} // namespace DarkThumbs
