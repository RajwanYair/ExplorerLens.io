#pragma once

#include <vector>
#include <string>
#include <memory>
#include <windows.h>
#include "../SDK/include/DarkThumbsPlugin.h"

namespace DarkThumbs::Worker {

    enum class PluginStatus {
        Unloaded,
        Loaded,
        Error,
        Disabled
    };

    struct PluginRuntimeStats {
        uint64_t requestCount;
        uint64_t failureCount;
        uint64_t totalLatencyUs;
        uint64_t lastCrashTimestamp;
    };

    struct LoadedPlugin {
        std::wstring id;
        HMODULE hModule;
        const DT_PluginInfo* info;
        PluginStatus status;
        PluginRuntimeStats stats;
        
        // Function Pointers
        HRESULT (*Initialize)();
        HRESULT (*Shutdown)();
        HRESULT (*GenerateThumbnail)(const wchar_t*, IStream*, uint32_t, HBITMAP*);
    };

    class IPluginManager {
    public:
        virtual ~IPluginManager() = default;

        // Discovers plugins in the standard directories (ProgramData, UserProfile)
        virtual HRESULT DiscoverPlugins() = 0;

        // Loads a specific plugin by ID or Path
        virtual HRESULT LoadPlugin(const std::wstring& id) = 0;

        // Unloads a plugin safely
        virtual HRESULT UnloadPlugin(const std::wstring& id) = 0;

        // Finds the best plugin for a given extension
        virtual std::shared_ptr<LoadedPlugin> ResolvePluginForExtension(const std::wstring& ext) = 0;

        // Returns a snapshot of all known plugins and their status
        virtual std::vector<LoadedPlugin> GetPluginList() const = 0;

        // Administrative: Force disable a plugin (e.g., after repeated crashes)
        virtual void BlockPlugin(const std::wstring& id, const std::wstring& reason) = 0;
    };

    // Factory
    std::shared_ptr<IPluginManager> CreatePluginManager();

}
