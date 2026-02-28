//==============================================================================
// ShellContextMenuV2
//==============================================================================

#include "ShellContextMenuV2.h"
#include <chrono>

namespace ExplorerLens { namespace Engine {

ShellContextMenuV2::ShellContextMenuV2() {
 m_items = GetDefaultMenu();
}

void ShellContextMenuV2::AddMenuItem(const ContextMenuItem& item) {
 m_items.push_back(item);
}

ContextMenuResult ShellContextMenuV2::ExecuteAction(
 ContextAction action, const std::wstring& filePath)
{
 ContextMenuResult result;
 auto start = std::chrono::high_resolution_clock::now();

 result.targetFile = filePath;
 result.executedAction = action;

 switch (action) {
 case ContextAction::Regenerate:
 result.success = true; // Would trigger thumbnail regeneration
 break;
 case ContextAction::ClearCache:
 result.success = true; // Would clear cache for this file
 break;
 case ContextAction::ExportAs:
 result.success = !filePath.empty();
 break;
 case ContextAction::ShowInfo:
 result.success = true;
 break;
 case ContextAction::CopyThumbnail:
 result.success = true; // Would copy to clipboard
 break;
 case ContextAction::Settings:
 result.success = true; // Would open settings
 break;
 default:
 result.success = false;
 result.errorMessage = L"Unknown action";
 break;
 }

 auto end = std::chrono::high_resolution_clock::now();
 result.executionTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
 return result;
}

std::vector<ContextMenuItem> ShellContextMenuV2::GetDefaultMenu() {
 std::vector<ContextMenuItem> items;

 items.push_back({ContextAction::Regenerate, L"Regenerate Thumbnail", L"refresh", true, false, MenuPosition::SubMenu});
 items.push_back({ContextAction::ClearCache, L"Clear Cache", L"delete", true, false, MenuPosition::SubMenu});
 items.push_back({ContextAction::ExportAs, L"Export As...", L"export", true, true, MenuPosition::SubMenu});
 items.push_back({ContextAction::ShowInfo, L"Thumbnail Info", L"info", true, false, MenuPosition::SubMenu});
 items.push_back({ContextAction::CopyThumbnail, L"Copy Thumbnail", L"copy", true, false, MenuPosition::SubMenu});
 items.push_back({ContextAction::Settings, L"ExplorerLens Settings", L"settings", true, true, MenuPosition::SubMenu});

 return items;
}

const wchar_t* ShellContextMenuV2::GetActionName(ContextAction action) {
 switch (action) {
 case ContextAction::Regenerate: return L"Regenerate";
 case ContextAction::ClearCache: return L"Clear Cache";
 case ContextAction::ExportAs: return L"Export As";
 case ContextAction::ShowInfo: return L"Show Info";
 case ContextAction::OpenWith: return L"Open With";
 case ContextAction::CopyThumbnail: return L"Copy Thumbnail";
 case ContextAction::Settings: return L"Settings";
 default: return L"Unknown";
 }
}

const wchar_t* ShellContextMenuV2::GetPositionName(MenuPosition position) {
 switch (position) {
 case MenuPosition::TopLevel: return L"Top Level";
 case MenuPosition::SubMenu: return L"Sub Menu";
 case MenuPosition::Cascading: return L"Cascading";
 default: return L"Unknown";
 }
}

}} // namespace ExplorerLens::Engine

