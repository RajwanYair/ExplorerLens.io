#pragma once
#include <string>
#include <vector>
#include <future>
#include "../../SDK/include/ExplorerLensPlugin.h" // Reuse SDK definitions

namespace ExplorerLens::Manager::Services {

    struct MarketplacePlugin {
        std::wstring Id;
        std::wstring Name;
        std::wstring Description;
        std::wstring Version;
        std::wstring Author;
        std::wstring IconUrl;
        bool IsInstalled;
        bool IsVerified;
        uint32_t Capabilities; // DT_Capability flags
    };

    class IPluginService {
    public:
        virtual ~IPluginService() = default;

        // Fetch from remote registry
        virtual std::future<std::vector<MarketplacePlugin>> GetAvailablePluginsAsync() = 0;
        
        // Operations
        virtual std::future<bool> InstallPluginAsync(const std::wstring& pluginId) = 0;
        virtual std::future<bool> UninstallPluginAsync(const std::wstring& pluginId) = 0;
        virtual std::future<bool> UpdatePluginAsync(const std::wstring& pluginId) = 0;
    };
}

