#pragma once
//==============================================================================
// ShellContextMenuV2
// Windows Shell context menu integration for ExplorerLens operations.
// Supports right-click actions: regenerate, clear cache, export, info.
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

enum class ContextAction : uint8_t {
    Regenerate   = 0,
    ClearCache   = 1,
    ExportAs     = 2,
    ShowInfo     = 3,
    OpenWith     = 4,
    CopyThumbnail = 5,
    Settings     = 6,
    ActionCount  = 7
};

enum class MenuPosition : uint8_t {
    TopLevel  = 0,
    SubMenu   = 1,
    Cascading = 2
};

struct ContextMenuItem {
    ContextAction action = ContextAction::Regenerate;
    std::wstring label;
    std::wstring icon;
    bool enabled = true;
    bool separator = false;
    MenuPosition position = MenuPosition::SubMenu;
};

struct ContextMenuResult {
    bool success = false;
    ContextAction executedAction = ContextAction::Regenerate;
    std::wstring targetFile;
    double executionTimeMs = 0.0;
    std::wstring errorMessage;
};

class ShellContextMenuV2 {
public:
    ShellContextMenuV2();

    void AddMenuItem(const ContextMenuItem& item);
    std::vector<ContextMenuItem> GetMenuItems() const { return m_items; }
    ContextMenuResult ExecuteAction(ContextAction action, const std::wstring& filePath);

    void SetSubMenuLabel(const std::wstring& label) { m_subMenuLabel = label; }
    std::wstring GetSubMenuLabel() const { return m_subMenuLabel; }

    static std::vector<ContextMenuItem> GetDefaultMenu();
    static const wchar_t* GetActionName(ContextAction action);
    static const wchar_t* GetPositionName(MenuPosition position);
    static uint32_t GetActionCount() { return static_cast<uint32_t>(ContextAction::ActionCount); }

private:
    std::vector<ContextMenuItem> m_items;
    std::wstring m_subMenuLabel = L"ExplorerLens";
};

}} // namespace ExplorerLens::Engine

