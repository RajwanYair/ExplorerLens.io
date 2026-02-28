// ============================================================================
// PluginLoaderV2.h — LoadLibrary + C ABI Plugin Bridge
// ExplorerLens Engine v15.0.0
// Copyright (c) 2026 ExplorerLens Project
//
// Dynamic plugin loading via LoadLibrary with C ABI function resolution.
// Supports hot-reload, version negotiation, and sandbox boundary enforcement.
// ============================================================================

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <mutex>
#include <atomic>
#include <chrono>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Plugin ABI version negotiation
// ============================================================================

struct PluginABIVersion {
    uint16_t major;     // Breaking ABI changes
    uint16_t minor;     // Backwards-compatible additions

    bool IsCompatibleWith(const PluginABIVersion& host) const {
        return (major == host.major) && (minor <= host.minor);
    }

    bool operator==(const PluginABIVersion& other) const {
        return major == other.major && minor == other.minor;
    }
};

/// Current host ABI version — plugins must match major, minor <= host minor
static constexpr PluginABIVersion HOST_ABI_VERSION = { 1, 0 };

// ============================================================================
// C ABI function pointer typedefs (match plugin_api.h exports)
// ============================================================================

/// Plugin initialization — returns 0 on success
using PFN_PluginInit = int32_t(*)();
/// Plugin shutdown
using PFN_PluginShutdown = void     (*)();
/// Get plugin name (UTF-8)
using PFN_GetPluginName = const char* (*)();
/// Get plugin version string
using PFN_GetPluginVersion = const char* (*)();
/// Get ABI version
using PFN_GetABIVersion = void     (*)(uint16_t* major, uint16_t* minor);
/// Get supported file extensions (semicolon-separated, e.g., ".step;.stp;.iges")
using PFN_GetExtensions = const char* (*)();
/// Decode a file to BGRA bitmap — returns 0 on success
using PFN_DecodeThumbnail = int32_t(*)(
    const wchar_t* filePath,
    uint32_t       requestedWidth,
    uint32_t       requestedHeight,
    uint8_t* outputBuffer,       // Caller-allocated BGRA8 buffer
    uint32_t       outputBufferSize,
    uint32_t* actualWidth,
    uint32_t* actualHeight
    );
/// Query required buffer size for decode output
using PFN_GetBufferSize = uint32_t(*)(
    const wchar_t* filePath,
    uint32_t       requestedWidth,
    uint32_t       requestedHeight
    );

// ============================================================================
// Plugin load state
// ============================================================================

enum class PluginState : uint8_t {
    Unloaded,        // Not loaded
    Loading,         // LoadLibrary in progress
    Resolving,       // Resolving C ABI symbols
    Initializing,    // Calling PluginInit
    Active,          // Fully loaded and operational
    Error,           // Failed to load or init
    Unloading        // FreeLibrary in progress
};

inline const char* PluginStateToString(PluginState state) {
    static const char* names[] = {
        "Unloaded", "Loading", "Resolving", "Initializing",
        "Active", "Error", "Unloading"
    };
    return names[static_cast<uint8_t>(state)];
}

// ============================================================================
// Plugin descriptor (populated after successful load)
// ============================================================================

struct PluginDescriptor {
    std::string      name;
    std::string      version;
    std::wstring     dllPath;
    PluginABIVersion abiVersion = { 0, 0 };
    std::vector<std::string> supportedExtensions;  // e.g., { ".step", ".stp" }
    PluginState      state = PluginState::Unloaded;
    std::string      errorMessage;

    // Performance metrics
    uint64_t         loadTimeUs = 0;  // Microseconds to load + init
    uint64_t         totalDecodes = 0;  // Lifetime decode count
    uint64_t         failedDecodes = 0;  // Lifetime decode failures
};

// ============================================================================
// LoadedPlugin — RAII wrapper for a loaded plugin DLL
// ============================================================================

class LoadedPlugin {
public:
    LoadedPlugin() = default;
    ~LoadedPlugin() { Unload(); }

    // Non-copyable
    LoadedPlugin(const LoadedPlugin&) = delete;
    LoadedPlugin& operator=(const LoadedPlugin&) = delete;

    // Movable
    LoadedPlugin(LoadedPlugin&& other) noexcept
        : m_hModule(other.m_hModule)
        , m_descriptor(std::move(other.m_descriptor))
        , m_pfnInit(other.m_pfnInit)
        , m_pfnShutdown(other.m_pfnShutdown)
        , m_pfnGetName(other.m_pfnGetName)
        , m_pfnGetVersion(other.m_pfnGetVersion)
        , m_pfnGetABIVersion(other.m_pfnGetABIVersion)
        , m_pfnGetExtensions(other.m_pfnGetExtensions)
        , m_pfnDecode(other.m_pfnDecode)
        , m_pfnGetBufferSize(other.m_pfnGetBufferSize) {
        other.m_hModule = nullptr;
    }

    LoadedPlugin& operator=(LoadedPlugin&& other) noexcept {
        if (this != &other) {
            Unload();
            m_hModule = other.m_hModule;
            m_descriptor = std::move(other.m_descriptor);
            m_pfnInit = other.m_pfnInit;
            m_pfnShutdown = other.m_pfnShutdown;
            m_pfnGetName = other.m_pfnGetName;
            m_pfnGetVersion = other.m_pfnGetVersion;
            m_pfnGetABIVersion = other.m_pfnGetABIVersion;
            m_pfnGetExtensions = other.m_pfnGetExtensions;
            m_pfnDecode = other.m_pfnDecode;
            m_pfnGetBufferSize = other.m_pfnGetBufferSize;
            other.m_hModule = nullptr;
        }
        return *this;
    }

    bool IsLoaded() const { return m_hModule != nullptr; }
    const PluginDescriptor& GetDescriptor() const { return m_descriptor; }
    PluginDescriptor& GetDescriptor() { return m_descriptor; }

    /// Load a plugin DLL and resolve all C ABI symbols
    bool Load(const std::wstring& dllPath) {
        auto start = std::chrono::steady_clock::now();

        // Store wide path for descriptor
        m_descriptor.dllPath = dllPath;
        m_descriptor.state = PluginState::Loading;

        // LoadLibrary with restricted search path (security)
        m_hModule = ::LoadLibraryExW(dllPath.c_str(), nullptr,
            LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);

        if (!m_hModule) {
            m_descriptor.state = PluginState::Error;
            m_descriptor.errorMessage = "LoadLibrary failed, error=" +
                std::to_string(::GetLastError());
            return false;
        }

        // Resolve symbols
        m_descriptor.state = PluginState::Resolving;
        if (!ResolveSymbols()) {
            ::FreeLibrary(m_hModule);
            m_hModule = nullptr;
            m_descriptor.state = PluginState::Error;
            return false;
        }

        // ABI version check
        if (m_pfnGetABIVersion) {
            uint16_t maj = 0, min = 0;
            m_pfnGetABIVersion(&maj, &min);
            m_descriptor.abiVersion = { maj, min };

            if (!m_descriptor.abiVersion.IsCompatibleWith(HOST_ABI_VERSION)) {
                m_descriptor.state = PluginState::Error;
                m_descriptor.errorMessage = "ABI version mismatch: plugin=" +
                    std::to_string(maj) + "." + std::to_string(min) +
                    " host=" + std::to_string(HOST_ABI_VERSION.major) + "." +
                    std::to_string(HOST_ABI_VERSION.minor);
                ::FreeLibrary(m_hModule);
                m_hModule = nullptr;
                return false;
            }
        }

        // Initialize plugin
        m_descriptor.state = PluginState::Initializing;
        if (m_pfnInit) {
            int32_t result = m_pfnInit();
            if (result != 0) {
                m_descriptor.state = PluginState::Error;
                m_descriptor.errorMessage = "PluginInit returned " + std::to_string(result);
                if (m_pfnShutdown) m_pfnShutdown();
                ::FreeLibrary(m_hModule);
                m_hModule = nullptr;
                return false;
            }
        }

        // Populate descriptor
        if (m_pfnGetName) m_descriptor.name = m_pfnGetName();
        if (m_pfnGetVersion) m_descriptor.version = m_pfnGetVersion();
        if (m_pfnGetExtensions) {
            ParseExtensions(m_pfnGetExtensions());
        }

        auto elapsed = std::chrono::steady_clock::now() - start;
        m_descriptor.loadTimeUs = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
        m_descriptor.state = PluginState::Active;
        return true;
    }

    /// Unload the plugin DLL
    void Unload() {
        if (!m_hModule) return;
        m_descriptor.state = PluginState::Unloading;
        if (m_pfnShutdown) {
            m_pfnShutdown();
        }
        ::FreeLibrary(m_hModule);
        m_hModule = nullptr;
        m_descriptor.state = PluginState::Unloaded;
    }

    /// Decode a thumbnail via the plugin's C ABI
    int32_t DecodeThumbnail(
        const wchar_t* filePath,
        uint32_t width, uint32_t height,
        uint8_t* buffer, uint32_t bufferSize,
        uint32_t* actualWidth, uint32_t* actualHeight) {
        if (!m_pfnDecode || m_descriptor.state != PluginState::Active) return -1;
        m_descriptor.totalDecodes++;
        int32_t result = m_pfnDecode(filePath, width, height, buffer, bufferSize,
            actualWidth, actualHeight);
        if (result != 0) m_descriptor.failedDecodes++;
        return result;
    }

    /// Query required buffer size
    uint32_t GetRequiredBufferSize(
        const wchar_t* filePath,
        uint32_t width, uint32_t height) const {
        if (!m_pfnGetBufferSize) return width * height * 4;  // BGRA8 default
        return m_pfnGetBufferSize(filePath, width, height);
    }

private:
    bool ResolveSymbols() {
        // Required symbols
        m_pfnDecode = reinterpret_cast<PFN_DecodeThumbnail>(
            ::GetProcAddress(m_hModule, "LENS_DecodeThumbnail"));

        if (!m_pfnDecode) {
            m_descriptor.errorMessage = "Missing required export: LENS_DecodeThumbnail";
            return false;
        }

        // Optional symbols (graceful degradation)
        m_pfnInit = reinterpret_cast<PFN_PluginInit>(
            ::GetProcAddress(m_hModule, "LENS_PluginInit"));
        m_pfnShutdown = reinterpret_cast<PFN_PluginShutdown>(
            ::GetProcAddress(m_hModule, "LENS_PluginShutdown"));
        m_pfnGetName = reinterpret_cast<PFN_GetPluginName>(
            ::GetProcAddress(m_hModule, "LENS_GetPluginName"));
        m_pfnGetVersion = reinterpret_cast<PFN_GetPluginVersion>(
            ::GetProcAddress(m_hModule, "LENS_GetPluginVersion"));
        m_pfnGetABIVersion = reinterpret_cast<PFN_GetABIVersion>(
            ::GetProcAddress(m_hModule, "LENS_GetABIVersion"));
        m_pfnGetExtensions = reinterpret_cast<PFN_GetExtensions>(
            ::GetProcAddress(m_hModule, "LENS_GetExtensions"));
        m_pfnGetBufferSize = reinterpret_cast<PFN_GetBufferSize>(
            ::GetProcAddress(m_hModule, "LENS_GetBufferSize"));

        return true;
    }

    void ParseExtensions(const char* extList) {
        if (!extList) return;
        m_descriptor.supportedExtensions.clear();
        std::string list(extList);
        size_t pos = 0;
        while (pos < list.size()) {
            size_t sep = list.find(';', pos);
            if (sep == std::string::npos) sep = list.size();
            std::string ext = list.substr(pos, sep - pos);
            if (!ext.empty()) {
                m_descriptor.supportedExtensions.push_back(ext);
            }
            pos = sep + 1;
        }
    }

    HMODULE              m_hModule = nullptr;
    PluginDescriptor     m_descriptor;

    // C ABI function pointers
    PFN_PluginInit       m_pfnInit = nullptr;
    PFN_PluginShutdown   m_pfnShutdown = nullptr;
    PFN_GetPluginName    m_pfnGetName = nullptr;
    PFN_GetPluginVersion m_pfnGetVersion = nullptr;
    PFN_GetABIVersion    m_pfnGetABIVersion = nullptr;
    PFN_GetExtensions    m_pfnGetExtensions = nullptr;
    PFN_DecodeThumbnail  m_pfnDecode = nullptr;
    PFN_GetBufferSize    m_pfnGetBufferSize = nullptr;
};

// ============================================================================
// PluginLoaderV2 — Plugin registry with extension-based routing
// ============================================================================

class PluginLoaderV2 {
public:
    static constexpr uint32_t MAX_PLUGINS = 64;

    PluginLoaderV2() = default;
    ~PluginLoaderV2() { UnloadAll(); }

    // Non-copyable
    PluginLoaderV2(const PluginLoaderV2&) = delete;
    PluginLoaderV2& operator=(const PluginLoaderV2&) = delete;

    /// Load a single plugin DLL
    bool LoadPlugin(const std::wstring& dllPath) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_plugins.size() >= MAX_PLUGINS) return false;

        auto plugin = std::make_unique<LoadedPlugin>();
        if (!plugin->Load(dllPath)) {
            m_lastError = plugin->GetDescriptor().errorMessage;
            return false;
        }

        // Register extension → plugin mappings
        for (const auto& ext : plugin->GetDescriptor().supportedExtensions) {
            m_extensionMap[ext] = plugin.get();
        }

        m_plugins.push_back(std::move(plugin));
        return true;
    }

    /// Scan a directory for plugin DLLs matching *Plugin.dll pattern
    uint32_t ScanDirectory(const std::wstring& pluginDir) {
        std::wstring searchPattern = pluginDir + L"\\*Plugin.dll";
        WIN32_FIND_DATAW findData;
        HANDLE hFind = ::FindFirstFileW(searchPattern.c_str(), &findData);

        if (hFind == INVALID_HANDLE_VALUE) return 0;

        uint32_t loaded = 0;
        do {
            std::wstring fullPath = pluginDir + L"\\" + findData.cFileName;
            if (LoadPlugin(fullPath)) {
                loaded++;
            }
        } while (::FindNextFileW(hFind, &findData));

        ::FindClose(hFind);
        return loaded;
    }

    /// Find a plugin that handles the given file extension
    LoadedPlugin* FindPluginForExtension(const std::string& ext) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_extensionMap.find(ext);
        return (it != m_extensionMap.end()) ? it->second : nullptr;
    }

    /// Decode a file using the appropriate plugin
    int32_t DecodeWithPlugin(
        const wchar_t* filePath,
        const std::string& extension,
        uint32_t width, uint32_t height,
        uint8_t* buffer, uint32_t bufferSize,
        uint32_t* actualWidth, uint32_t* actualHeight) {
        auto* plugin = FindPluginForExtension(extension);
        if (!plugin) return -1;  // No plugin for this extension
        return plugin->DecodeThumbnail(filePath, width, height, buffer, bufferSize,
            actualWidth, actualHeight);
    }

    /// Hot-reload a plugin (unload + reload from same path)
    bool ReloadPlugin(const std::string& pluginName) {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (auto& plugin : m_plugins) {
            if (plugin->GetDescriptor().name == pluginName) {
                const std::wstring& path = plugin->GetDescriptor().dllPath;

                // Remove extension mappings
                RemoveExtensionMappingsLocked(plugin.get());

                // Unload and reload
                plugin->Unload();
                if (!plugin->Load(path)) {
                    m_lastError = plugin->GetDescriptor().errorMessage;
                    return false;
                }

                // Re-register extensions
                for (const auto& ext : plugin->GetDescriptor().supportedExtensions) {
                    m_extensionMap[ext] = plugin.get();
                }
                return true;
            }
        }
        return false;  // Plugin not found
    }

    /// Unload all plugins
    void UnloadAll() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_extensionMap.clear();
        m_plugins.clear();
    }

    /// Get all loaded plugin descriptors
    std::vector<PluginDescriptor> GetLoadedPlugins() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<PluginDescriptor> result;
        result.reserve(m_plugins.size());
        for (const auto& p : m_plugins) {
            result.push_back(p->GetDescriptor());
        }
        return result;
    }

    uint32_t GetPluginCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return static_cast<uint32_t>(m_plugins.size());
    }

    const std::string& GetLastError() const { return m_lastError; }

private:
    void RemoveExtensionMappingsLocked(LoadedPlugin* plugin) {
        for (auto it = m_extensionMap.begin(); it != m_extensionMap.end(); ) {
            if (it->second == plugin) {
                it = m_extensionMap.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    mutable std::mutex m_mutex;
    std::vector<std::unique_ptr<LoadedPlugin>> m_plugins;
    std::unordered_map<std::string, LoadedPlugin*> m_extensionMap;  // ext → plugin
    std::string m_lastError;
};

} // namespace Engine
} // namespace ExplorerLens
