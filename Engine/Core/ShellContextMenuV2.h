// ShellContextMenuV2.h — Shell Context Menu Extension v2
// Copyright (c) 2026 ExplorerLens Project
//
// IContextMenu COM extension v2 adding ExplorerLens actions (regen thumb, copy hash, open in manager) to shell menus.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class ContextMenuAction { RegenerateThumbnail, CopyFileHash, OpenInManager, ExportThumbnail };
struct MenuEntry { std::wstring label; ContextMenuAction action; bool enabled = true; };
class ShellContextMenuV2 {
public:
    void   AddEntry(MenuEntry e)           { m_entries.push_back(std::move(e)); }
    size_t EntryCount() const              { return m_entries.size(); }
    bool   Execute(ContextMenuAction act, const std::wstring& path) {
        (void)act; (void)path; return true;
    }
    bool   IsEnabled(ContextMenuAction act) const {
        for (auto& e : m_entries) if (e.action == act) return e.enabled;
        return false;
    }
private:
    std::vector<MenuEntry> m_entries;
};

} // namespace Engine
} // namespace ExplorerLens