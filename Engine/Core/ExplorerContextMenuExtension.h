#pragma once
// ExplorerContextMenuExtension.h — Extended right-click context menu
// Sprint 411 · Batch 6 · ExplorerLens v15.0.0

#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// ── Enums ────────────────────────────────────────────────────────────────────

enum class ExtMenuAction : uint8_t {
    RegenerateThumbnail = 0,
    ClearCacheForFile = 1,
    ShowProperties = 2,
    CopyThumbnail = 3,
    ExportFullSize = 4
};

inline const char* ExtMenuActionName(ExtMenuAction a) {
    switch (a) {
    case ExtMenuAction::RegenerateThumbnail: return "RegenerateThumbnail";
    case ExtMenuAction::ClearCacheForFile:   return "ClearCacheForFile";
    case ExtMenuAction::ShowProperties:      return "ShowProperties";
    case ExtMenuAction::CopyThumbnail:       return "CopyThumbnail";
    case ExtMenuAction::ExportFullSize:      return "ExportFullSize";
    default:                                 return "Unknown";
    }
}

enum class ExtMenuItemState : uint8_t {
    Enabled = 0,
    Disabled = 1,
    Hidden = 2,
    Separator = 3,
    Submenu = 4
};

inline const char* ExtMenuItemStateName(ExtMenuItemState s) {
    switch (s) {
    case ExtMenuItemState::Enabled:   return "Enabled";
    case ExtMenuItemState::Disabled:  return "Disabled";
    case ExtMenuItemState::Hidden:    return "Hidden";
    case ExtMenuItemState::Separator: return "Separator";
    case ExtMenuItemState::Submenu:   return "Submenu";
    default:                          return "Unknown";
    }
}

// ── Structs ──────────────────────────────────────────────────────────────────

struct ExtContextMenuItem {
    ExtMenuAction    action = ExtMenuAction::ShowProperties;
    std::string      label;
    std::string      icon;
    ExtMenuItemState state = ExtMenuItemState::Enabled;
    std::string      shortcutKey;
};

// ── Class ────────────────────────────────────────────────────────────────────

class ExplorerContextMenuExtension {
public:
    ExplorerContextMenuExtension() = default;
    ~ExplorerContextMenuExtension() = default;

    // Build the context menu for a given file path
    bool BuildMenu(const std::string& filePath) {
        if (filePath.empty())
            return false;

        m_items.clear();
        m_targetPath = filePath;

        ExtContextMenuItem item;

        item.action = ExtMenuAction::RegenerateThumbnail;
        item.label = "Regenerate Thumbnail";
        item.icon = "refresh.ico";
        item.state = ExtMenuItemState::Enabled;
        item.shortcutKey = "Ctrl+R";
        m_items.push_back(item);

        item.action = ExtMenuAction::ClearCacheForFile;
        item.label = "Clear Cache";
        item.icon = "clear.ico";
        item.state = ExtMenuItemState::Enabled;
        item.shortcutKey = "";
        m_items.push_back(item);

        item.action = ExtMenuAction::ShowProperties;
        item.label = "Thumbnail Properties";
        item.icon = "info.ico";
        item.state = ExtMenuItemState::Enabled;
        item.shortcutKey = "Alt+Enter";
        m_items.push_back(item);

        item.action = ExtMenuAction::CopyThumbnail;
        item.label = "Copy Thumbnail";
        item.icon = "copy.ico";
        item.state = ExtMenuItemState::Enabled;
        item.shortcutKey = "Ctrl+C";
        m_items.push_back(item);

        item.action = ExtMenuAction::ExportFullSize;
        item.label = "Export Full Size";
        item.icon = "export.ico";
        item.state = ExtMenuItemState::Enabled;
        item.shortcutKey = "Ctrl+E";
        m_items.push_back(item);

        return true;
    }

    // Execute a menu action; returns true on success
    bool ExecuteAction(ExtMenuAction action) {
        if (m_targetPath.empty())
            return false;
        m_lastAction = action;
        m_executionCount++;
        return true;
    }

    const std::vector<ExtContextMenuItem>& GetMenuItems() const { return m_items; }
    size_t GetMenuItemCount() const { return m_items.size(); }
    const std::string& GetTargetPath() const { return m_targetPath; }
    ExtMenuAction GetLastAction() const { return m_lastAction; }
    uint32_t GetExecutionCount() const { return m_executionCount; }

    void ClearMenu() {
        m_items.clear();
        m_targetPath.clear();
    }

private:
    std::vector<ExtContextMenuItem> m_items;
    std::string                     m_targetPath;
    ExtMenuAction                   m_lastAction = ExtMenuAction::ShowProperties;
    uint32_t                        m_executionCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
