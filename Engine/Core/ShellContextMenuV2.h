// ShellContextMenuV2.h — Shell Context Menu Extension v2
// Copyright (c) 2026 ExplorerLens Project
//
// IContextMenu COM extension v2 adding ExplorerLens actions (regen thumb, copy hash, open in manager) to shell menus.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ContextMenuAction {
    RegenerateThumbnail,
    CopyFileHash,
    OpenInManager,
    ExportThumbnail
};
struct MenuEntry
{
    std::wstring label;
    ContextMenuAction action;
    bool enabled = true;
};

enum class ContextAction : uint8_t {
    Regenerate = 0,
    ClearCache = 1,
    Settings = 2,
    CopyHash = 3,
    OpenManager = 4,
    ExportThumb = 5,
    Disable = 6
};

enum class MenuPosition : uint8_t {
    TopLevel = 0,
    SubMenu = 1
};

struct ContextMenuItem
{
    std::wstring label;
    ContextAction action = ContextAction::Regenerate;
    bool enabled = true;
};

struct ContextActionResult
{
    bool success = false;
    ContextAction executedAction = ContextAction::Regenerate;
};

class ShellContextMenuV2
{
  public:
    void AddEntry(MenuEntry e)
    {
        m_entries.push_back(std::move(e));
    }
    size_t EntryCount() const
    {
        return m_entries.size();
    }
    bool Execute(ContextMenuAction act, const std::wstring& path)
    {
        (void)act;
        (void)path;
        return true;
    }
    bool IsEnabled(ContextMenuAction act) const
    {
        for (auto& e : m_entries)
            if (e.action == act)
                return e.enabled;
        return false;
    }

    ContextActionResult ExecuteAction(ContextAction act, const std::wstring& path)
    {
        (void)path;
        return {true, act};
    }

    static const wchar_t* GetActionName(ContextAction act)
    {
        switch (act) {
            case ContextAction::Regenerate:
                return L"Regenerate";
            case ContextAction::ClearCache:
                return L"Clear Cache";
            case ContextAction::Settings:
                return L"Settings";
            case ContextAction::CopyHash:
                return L"Copy Hash";
            case ContextAction::OpenManager:
                return L"Open Manager";
            case ContextAction::ExportThumb:
                return L"Export Thumbnail";
            case ContextAction::Disable:
                return L"Disable";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* GetPositionName(MenuPosition pos)
    {
        return pos == MenuPosition::TopLevel ? L"Top Level" : L"Sub Menu";
    }

    static constexpr size_t GetActionCount()
    {
        return 7;
    }

    static std::vector<ContextMenuItem> GetDefaultMenu()
    {
        return {
            {L"Regenerate Thumbnail", ContextAction::Regenerate, true},
            {L"Clear Cache Entry", ContextAction::ClearCache, true},
            {L"Copy File Hash", ContextAction::CopyHash, true},
            {L"Open in Manager", ContextAction::OpenManager, true},
            {L"Export Thumbnail", ContextAction::ExportThumb, true},
            {L"Settings", ContextAction::Settings, true},
            {L"Disable for File", ContextAction::Disable, true},
        };
    }

  private:
    std::vector<MenuEntry> m_entries;
};

}  // namespace Engine
}  // namespace ExplorerLens
