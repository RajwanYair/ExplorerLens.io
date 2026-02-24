#pragma once
#include <vector>
#include <memory>
#include "../Services/IPluginService.h"

namespace ExplorerLens::Manager::ViewModels {

    class PluginStoreViewModel {
    public:
        PluginStoreViewModel(std::shared_ptr<Services::IPluginService> pluginService);

        void LoadPlugins();
        void InstallSelected();
        void UninstallSelected();

        // Exposed Collection
        std::vector<Services::MarketplacePlugin> Plugins;
        Services::MarketplacePlugin SelectedPlugin;

        // Status
        bool IsLoading;
        std::wstring StatusMessage;

    private:
        std::shared_ptr<Services::IPluginService> m_service;
    };
}

