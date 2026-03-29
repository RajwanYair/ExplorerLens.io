// TabbedExplorerSync.h — Tabbed Explorer Pane Synchronizer
// Copyright (c) 2026 ExplorerLens Project
//
// Synchronizes thumbnail state across multiple Explorer tabbed panes so that
// a zoom-level or sort-order change in one tab propagates consistently
// to sibling tabs showing the same folder path.
//
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace ExplorerLens {
namespace Engine {

enum class TabSyncPolicy { Independent, SyncSameFolder, SyncAll };

struct ExplorerTabState {
    uint64_t     tabId         = 0;
    std::wstring folderPath;
    int          zoomLevel     = 100;  // percent
    std::wstring sortColumn;
    bool         sortAscending = true;
    uint32_t     windowHandle  = 0;
};

struct SyncEvent {
    uint64_t         sourceTanId = 0;
    std::wstring     folderPath;
    ExplorerTabState newState;
};

using SyncCallback = std::function<void(const SyncEvent&)>;

class TabbedExplorerSync {
public:
    static TabbedExplorerSync& Instance() {
        static TabbedExplorerSync inst;
        return inst;
    }

    void RegisterTab(const ExplorerTabState& tab) { m_tabs[tab.tabId] = tab; }
    void UnregisterTab(uint64_t tabId) { m_tabs.erase(tabId); }

    void SetPolicy(TabSyncPolicy p) noexcept { m_policy = p; }
    TabSyncPolicy GetPolicy() const noexcept { return m_policy; }

    void OnTabStateChange(const ExplorerTabState& updated) {
        m_tabs[updated.tabId] = updated;
        if (m_policy == TabSyncPolicy::Independent) return;
        SyncEvent evt{ updated.tabId, updated.folderPath, updated };
        for (auto& [id, tab] : m_tabs) {
            if (id == updated.tabId) continue;
            bool shouldSync = (m_policy == TabSyncPolicy::SyncAll)
                           || (tab.folderPath == updated.folderPath);
            if (shouldSync) {
                tab.zoomLevel     = updated.zoomLevel;
                tab.sortColumn    = updated.sortColumn;
                tab.sortAscending = updated.sortAscending;
                for (auto& cb : m_callbacks) cb(evt);
            }
        }
    }

    void RegisterCallback(SyncCallback cb) { m_callbacks.push_back(std::move(cb)); }

    int  TabCount()       const noexcept { return (int)m_tabs.size(); }
    bool HasTab(uint64_t id) const noexcept { return m_tabs.count(id) > 0; }

    std::vector<ExplorerTabState> GetSiblingTabs(const std::wstring& folder) const {
        std::vector<ExplorerTabState> v;
        for (const auto& [id, tab] : m_tabs) if (tab.folderPath == folder) v.push_back(tab);
        return v;
    }

private:
    TabbedExplorerSync() = default;
    std::unordered_map<uint64_t, ExplorerTabState> m_tabs;
    std::vector<SyncCallback>                       m_callbacks;
    TabSyncPolicy                                   m_policy = TabSyncPolicy::SyncSameFolder;
};

} // namespace Engine
} // namespace ExplorerLens
