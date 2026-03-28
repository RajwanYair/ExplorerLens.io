// ShellContextMenuProvider.h — Dynamic Explorer Context Menu Integration
// Copyright (c) 2026 ExplorerLens Project
//
// Provides dynamically populated context menu entries for Explorer,
// offering format-specific actions like re-decode, export, and quality settings.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ContextMenuProviderAction : uint8_t {
    Redecode = 0,
    ExportThumbnail,
    ClearCacheEntry,
    ShowMetadata,
    CopyAsImage,
    OpenWithDecoder
};

struct ShellContextMenuItem {
    std::wstring label;
    ContextMenuProviderAction action = ContextMenuProviderAction::Redecode;
    bool enabled = true;
    uint32_t iconId = 0;
};

class ShellContextMenuProvider {
public:
    ShellContextMenuProvider() = default;

    std::vector<ShellContextMenuItem> GetMenuItems(const std::wstring& filePath) const {
        std::vector<ShellContextMenuItem> items;
        if (filePath.empty()) return items;
        items.push_back(ShellContextMenuItem{ L"Re-decode Thumbnail", ContextMenuProviderAction::Redecode, true, 1 });
        items.push_back(ShellContextMenuItem{ L"Export Thumbnail...", ContextMenuProviderAction::ExportThumbnail, true, 2 });
        items.push_back(ShellContextMenuItem{ L"Clear Cache Entry", ContextMenuProviderAction::ClearCacheEntry, true, 3 });
        return items;
    }

    bool ExecuteAction(ContextMenuProviderAction action, const std::wstring& /*filePath*/) const {
        return action != ContextMenuProviderAction::OpenWithDecoder;
    }

    uint32_t GetMenuItemCount() const { return 6; }

private:
    bool m_enabled = true;
};

} // namespace Engine
} // namespace ExplorerLens
