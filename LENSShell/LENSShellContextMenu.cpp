//==============================================================================
// LENSShellContextMenu.cpp — Windows 11 IExplorerCommand Implementation
// Provides ExplorerLens context menu in the modern Win11 Explorer right-click.
//==============================================================================

#include "StdAfx.h"
#include "LENSShellContextMenu.h"
#include <shlwapi.h>
#include <strsafe.h>
#include <shellapi.h>

#pragma comment(lib, "shlwapi.lib")

// Context menu CLSID: {A7B3F4E1-9C2D-4E8F-B6A1-3D5E7F9C2B4A}
// Generated uniquely for the context menu handler
// (separate from the thumbnail provider CLSID)
static const CLSID CLSID_LENSShellContextMenu_Internal =
{ 0xA7B3F4E1, 0x9C2D, 0x4E8F,
  { 0xB6, 0xA1, 0x3D, 0x5E, 0x7F, 0x9C, 0x2B, 0x4A } };

const CLSID CLSID_LENSShellContextMenu = CLSID_LENSShellContextMenu_Internal;

// SHStrDupW is provided by shlwapi.h — no local definition needed.

//==============================================================================
// CLENSExplorerSubCommand Implementation
//==============================================================================

void CLENSExplorerSubCommand::Initialize(LENSContextAction action,
    const wchar_t* title, const wchar_t* icon, const wchar_t* tooltip) {
    m_action = action;
    m_title = title ? title : L"";
    m_icon = icon ? icon : L"";
    m_tooltip = tooltip ? tooltip : L"";
}

IFACEMETHODIMP CLENSExplorerSubCommand::GetTitle(
    IShellItemArray* /*pItems*/, LPWSTR* ppszName) {
    return SHStrDupW(m_title.c_str(), ppszName);
}

IFACEMETHODIMP CLENSExplorerSubCommand::GetIcon(
    IShellItemArray* /*pItems*/, LPWSTR* ppszIcon) {
    if (m_icon.empty()) {
        *ppszIcon = nullptr;
        return E_NOTIMPL;
    }
    return SHStrDupW(m_icon.c_str(), ppszIcon);
}

IFACEMETHODIMP CLENSExplorerSubCommand::GetToolTip(
    IShellItemArray* /*pItems*/, LPWSTR* ppszInfotip) {
    return SHStrDupW(m_tooltip.c_str(), ppszInfotip);
}

IFACEMETHODIMP CLENSExplorerSubCommand::GetCanonicalName(GUID* pguidCommandName) {
    *pguidCommandName = GUID_NULL;
    return S_OK;
}

IFACEMETHODIMP CLENSExplorerSubCommand::GetState(
    IShellItemArray* /*pItems*/, BOOL /*fOkToBeSlow*/,
    EXPCMDSTATE* pCmdState) {
    *pCmdState = ECS_ENABLED;
    return S_OK;
}

IFACEMETHODIMP CLENSExplorerSubCommand::GetFlags(EXPCMDFLAGS* pFlags) {
    *pFlags = ECF_DEFAULT;
    return S_OK;
}

IFACEMETHODIMP CLENSExplorerSubCommand::EnumSubCommands(
    IEnumExplorerCommand** ppEnum) {
    *ppEnum = nullptr;
    return E_NOTIMPL; // Leaf command — no sub-commands
}

std::wstring CLENSExplorerSubCommand::GetSelectedFilePath(
    IShellItemArray* pItems) {
    if (!pItems) return L"";
    DWORD count = 0;
    pItems->GetCount(&count);
    if (count == 0) return L"";

    CComPtr<IShellItem> pItem;
    if (FAILED(pItems->GetItemAt(0, &pItem))) return L"";

    LPWSTR pszPath = nullptr;
    if (FAILED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)))
        return L"";

    std::wstring path(pszPath);
    CoTaskMemFree(pszPath);
    return path;
}

IFACEMETHODIMP CLENSExplorerSubCommand::Invoke(
    IShellItemArray* pItems, IBindCtx* /*pBindCtx*/) {
    std::wstring filePath = GetSelectedFilePath(pItems);
    if (filePath.empty()) return S_OK;

    switch (m_action) {
    case LENSContextAction::Regenerate:
    {
        // Delete cached thumbnail to force regeneration
        // Shell will re-query IThumbnailProvider on next display
        // Future: Wire to EngineAdapter::InvalidateCache(filePath)
        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATHW,
            filePath.c_str(), nullptr);
        break;
    }
    case LENSContextAction::ClearCache:
    {
        // Notify shell to clear thumbnail cache for this item
        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATHW,
            filePath.c_str(), nullptr);
        break;
    }
    case LENSContextAction::ShowInfo:
    {
        // Open file properties dialog
        SHELLEXECUTEINFOW sei = { sizeof(sei) };
        sei.fMask = SEE_MASK_INVOKEIDLIST;
        sei.lpVerb = L"properties";
        sei.lpFile = filePath.c_str();
        sei.nShow = SW_SHOW;
        ShellExecuteExW(&sei);
        break;
    }
    case LENSContextAction::CopyThumbnail:
    {
        // Copy file path to clipboard (thumbnail copy requires
        // GDI+ rendering — deferred to EngineAdapter integration)
        if (OpenClipboard(nullptr)) {
            EmptyClipboard();
            size_t len = (filePath.size() + 1) * sizeof(wchar_t);
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
            if (hMem) {
                wchar_t* dest = static_cast<wchar_t*>(GlobalLock(hMem));
                if (dest) {
                    memcpy(dest, filePath.c_str(), len);
                    GlobalUnlock(hMem);
                    SetClipboardData(CF_UNICODETEXT, hMem);
                }
            }
            CloseClipboard();
        }
        break;
    }
    case LENSContextAction::Settings:
    {
        // Launch LENSManager.exe (settings utility)
        wchar_t modulePath[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
        PathRemoveFileSpecW(modulePath);
        std::wstring managerPath =
            std::wstring(modulePath) + L"\\LENSManager.exe";
        ShellExecuteW(nullptr, L"open", managerPath.c_str(),
            nullptr, nullptr, SW_SHOWNORMAL);
        break;
    }
    case LENSContextAction::ExportAs:
    {
        // Future: Export thumbnail as PNG/JPEG
        // For now, open containing folder
        std::wstring param = L"/select,\"" + filePath + L"\"";
        ShellExecuteW(nullptr, L"open", L"explorer.exe",
            param.c_str(), nullptr, SW_SHOWNORMAL);
        break;
    }
    default:
        break;
    }

    return S_OK;
}

//==============================================================================
// CLENSEnumExplorerCommand Implementation
//==============================================================================

void CLENSEnumExplorerCommand::Initialize(
    const std::vector<CComPtr<IExplorerCommand>>& commands) {
    m_commands = commands;
    m_current = 0;
}

IFACEMETHODIMP CLENSEnumExplorerCommand::Next(
    ULONG celt, IExplorerCommand** pUICommand, ULONG* pceltFetched) {
    ULONG fetched = 0;
    while (fetched < celt &&
        m_current < static_cast<ULONG>(m_commands.size())) {
        m_commands[m_current].CopyTo(&pUICommand[fetched]);
        ++m_current;
        ++fetched;
    }
    if (pceltFetched) *pceltFetched = fetched;
    return (fetched == celt) ? S_OK : S_FALSE;
}

IFACEMETHODIMP CLENSEnumExplorerCommand::Skip(ULONG celt) {
    m_current += celt;
    return (m_current <= static_cast<ULONG>(m_commands.size()))
        ? S_OK
        : S_FALSE;
}

IFACEMETHODIMP CLENSEnumExplorerCommand::Reset() {
    m_current = 0;
    return S_OK;
}

IFACEMETHODIMP CLENSEnumExplorerCommand::Clone(
    IEnumExplorerCommand** ppEnum) {
    *ppEnum = nullptr;
    return E_NOTIMPL;
}

//==============================================================================
// CLENSShellContextMenu (Root) Implementation
//==============================================================================

HRESULT CLENSShellContextMenu::FinalConstruct() {
    BuildSubCommands();
    return S_OK;
}

void CLENSShellContextMenu::BuildSubCommands() {
    struct SubCmdDef {
        LENSContextAction action;
        const wchar_t* title;
        const wchar_t* tooltip;
    };

    static const SubCmdDef defs[] = {
        { LENSContextAction::Regenerate,
          L"Regenerate Thumbnail",
          L"Force regeneration of the cached thumbnail" },
        { LENSContextAction::ClearCache,
          L"Clear Thumbnail Cache",
          L"Remove cached thumbnail for this file" },
        { LENSContextAction::ShowInfo,
          L"File Properties",
          L"Show file details and metadata" },
        { LENSContextAction::CopyThumbnail,
          L"Copy File Path",
          L"Copy the file path to clipboard" },
        { LENSContextAction::ExportAs,
          L"Show in Explorer",
          L"Open containing folder and select file" },
        { LENSContextAction::Settings,
          L"ExplorerLens Settings...",
          L"Open the ExplorerLens configuration utility" },
    };

    for (const auto& def : defs) {
        CComObject<CLENSExplorerSubCommand>* pSub = nullptr;
        if (SUCCEEDED(
            CComObject<CLENSExplorerSubCommand>::CreateInstance(&pSub))) {
            pSub->AddRef();
            pSub->Initialize(def.action, def.title, L"", def.tooltip);
            CComPtr<IExplorerCommand> spCmd;
            pSub->QueryInterface(IID_PPV_ARGS(&spCmd));
            m_subCommands.push_back(spCmd);
            pSub->Release();
        }
    }
}

IFACEMETHODIMP CLENSShellContextMenu::GetTitle(
    IShellItemArray* /*pItems*/, LPWSTR* ppszName) {
    return SHStrDupW(L"ExplorerLens", ppszName);
}

IFACEMETHODIMP CLENSShellContextMenu::GetIcon(
    IShellItemArray* /*pItems*/, LPWSTR* ppszIcon) {
    // Use the ExplorerLens icon from resources
    wchar_t modulePath[MAX_PATH] = {};
    GetModuleFileNameW(_AtlBaseModule.GetModuleInstance(),
        modulePath, MAX_PATH);
    // Icon resource index 0 (IDI_ICON1)
    std::wstring iconRef = std::wstring(modulePath) + L",0";
    return SHStrDupW(iconRef.c_str(), ppszIcon);
}

IFACEMETHODIMP CLENSShellContextMenu::GetToolTip(
    IShellItemArray* /*pItems*/, LPWSTR* ppszInfotip) {
    return SHStrDupW(L"ExplorerLens thumbnail operations", ppszInfotip);
}

IFACEMETHODIMP CLENSShellContextMenu::GetCanonicalName(
    GUID* pguidCommandName) {
    // Use the context menu CLSID as canonical name
    *pguidCommandName = CLSID_LENSShellContextMenu;
    return S_OK;
}

IFACEMETHODIMP CLENSShellContextMenu::GetState(
    IShellItemArray* pItems, BOOL /*fOkToBeSlow*/,
    EXPCMDSTATE* pCmdState) {
    // Show the menu if at least one file is selected
    *pCmdState = ECS_ENABLED;
    if (pItems) {
        DWORD count = 0;
        pItems->GetCount(&count);
        if (count == 0) *pCmdState = ECS_HIDDEN;
    }
    return S_OK;
}

IFACEMETHODIMP CLENSShellContextMenu::GetFlags(EXPCMDFLAGS* pFlags) {
    // ECF_HASSUBCOMMANDS: This is a cascading menu with sub-items
    *pFlags = ECF_HASSUBCOMMANDS;
    return S_OK;
}

IFACEMETHODIMP CLENSShellContextMenu::EnumSubCommands(
    IEnumExplorerCommand** ppEnum) {
    if (!ppEnum) return E_POINTER;
    *ppEnum = nullptr;

    CComObject<CLENSEnumExplorerCommand>* pEnum = nullptr;
    HRESULT hr =
        CComObject<CLENSEnumExplorerCommand>::CreateInstance(&pEnum);
    if (FAILED(hr)) return hr;

    pEnum->AddRef();
    pEnum->Initialize(m_subCommands);
    hr = pEnum->QueryInterface(IID_PPV_ARGS(ppEnum));
    pEnum->Release();
    return hr;
}

IFACEMETHODIMP CLENSShellContextMenu::Invoke(
    IShellItemArray* /*pItems*/, IBindCtx* /*pBindCtx*/) {
    // Root menu — no direct action (sub-commands handle it)
    return S_OK;
}
