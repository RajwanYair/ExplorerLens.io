#pragma once
#include <string>
#include <vector>

namespace DarkThumbs::Manager::ViewModels {

    class DashboardViewModel {
    public:
        DashboardViewModel();

        // Properties (Bindable in real WinUI, simplified here)
        std::wstring GetEngineStatus() const;
        std::wstring GetCacheUsageText() const;
        int GetActivePluginsCount() const;
        bool IsGpuAccelerated() const;

        // Commands
        void RefreshStats();
        void ClearCache();

    private:
        std::wstring m_status;
        int m_cacheSizeBytes;
        int m_pluginCount;
    };
}
