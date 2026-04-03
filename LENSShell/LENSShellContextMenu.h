#pragma once
//==============================================================================
// LENSShellContextMenu — Windows 11 IExplorerCommand Context Menu Handler
// Implements IExplorerCommand for the modern Win11 right-click context menu.
// Provides sub-commands: Regenerate Thumbnail, Clear Cache, Export As,
// Show Info, Copy Thumbnail, Settings.
//==============================================================================

#include <atlbase.h>
#include <atlcom.h>
#include <shlobj.h>
#include <shobjidl.h>

#include <windows.h>

#include <memory>
#include <string>
#include <vector>

// Forward-declare from LENSShell.h (MIDL-generated)
extern const CLSID CLSID_LENSShellContextMenu;

//==============================================================================
// Sub-command IDs matching ContextAction from ShellContextMenuV2
//==============================================================================
enum class LENSContextAction : UINT {
    Regenerate = 0,
    ClearCache = 1,
    ExportAs = 2,
    ShowInfo = 3,
    CopyThumbnail = 4,
    Settings = 5,
    Count = 6
};

//==============================================================================
// CLENSExplorerSubCommand — Individual sub-command (implements IExplorerCommand)
//==============================================================================
class ATL_NO_VTABLE CLENSExplorerSubCommand
    : public CComObjectRootEx<CComSingleThreadModel>
    , public IExplorerCommand
{
  public:
    CLENSExplorerSubCommand() = default;

    void Initialize(LENSContextAction action, const wchar_t* title, const wchar_t* icon, const wchar_t* tooltip);

    BEGIN_COM_MAP(CLENSExplorerSubCommand)
    COM_INTERFACE_ENTRY(IExplorerCommand)
    END_COM_MAP()

    // IExplorerCommand
    IFACEMETHODIMP GetTitle(IShellItemArray* pItems, LPWSTR* ppszName) override;
    IFACEMETHODIMP GetIcon(IShellItemArray* pItems, LPWSTR* ppszIcon) override;
    IFACEMETHODIMP GetToolTip(IShellItemArray* pItems, LPWSTR* ppszInfotip) override;
    IFACEMETHODIMP GetCanonicalName(GUID* pguidCommandName) override;
    IFACEMETHODIMP GetState(IShellItemArray* pItems, BOOL fOkToBeSlow, EXPCMDSTATE* pCmdState) override;
    IFACEMETHODIMP GetFlags(EXPCMDFLAGS* pFlags) override;
    IFACEMETHODIMP EnumSubCommands(IEnumExplorerCommand** ppEnum) override;
    IFACEMETHODIMP Invoke(IShellItemArray* pItems, IBindCtx* pBindCtx) override;

  private:
    LENSContextAction m_action = LENSContextAction::Regenerate;
    std::wstring m_title;
    std::wstring m_icon;
    std::wstring m_tooltip;

    std::wstring GetSelectedFilePath(IShellItemArray* pItems);
};

//==============================================================================
// CLENSEnumExplorerCommand — Enumerator for sub-commands
//==============================================================================
class ATL_NO_VTABLE CLENSEnumExplorerCommand
    : public CComObjectRootEx<CComSingleThreadModel>
    , public IEnumExplorerCommand
{
  public:
    CLENSEnumExplorerCommand() = default;

    void Initialize(const std::vector<CComPtr<IExplorerCommand>>& commands);

    BEGIN_COM_MAP(CLENSEnumExplorerCommand)
    COM_INTERFACE_ENTRY(IEnumExplorerCommand)
    END_COM_MAP()

    // IEnumExplorerCommand
    IFACEMETHODIMP Next(ULONG celt, IExplorerCommand** pUICommand, ULONG* pceltFetched) override;
    IFACEMETHODIMP Skip(ULONG celt) override;
    IFACEMETHODIMP Reset() override;
    IFACEMETHODIMP Clone(IEnumExplorerCommand** ppEnum) override;

  private:
    std::vector<CComPtr<IExplorerCommand>> m_commands;
    ULONG m_current = 0;
};

//==============================================================================
// CLENSShellContextMenu — Root IExplorerCommand (cascading parent)
// This is the top-level "ExplorerLens" menu item in the Win11 context menu.
// It returns sub-commands via EnumSubCommands().
//==============================================================================
class ATL_NO_VTABLE CLENSShellContextMenu
    : public CComObjectRootEx<CComSingleThreadModel>
    , public CComCoClass<CLENSShellContextMenu, &CLSID_LENSShellContextMenu>
    , public IExplorerCommand
{
  public:
    CLENSShellContextMenu() = default;

    HRESULT FinalConstruct();

    BEGIN_COM_MAP(CLENSShellContextMenu)
    COM_INTERFACE_ENTRY(IExplorerCommand)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(CLENSShellContextMenu)
    DECLARE_NO_REGISTRY()

    // IExplorerCommand
    IFACEMETHODIMP GetTitle(IShellItemArray* pItems, LPWSTR* ppszName) override;
    IFACEMETHODIMP GetIcon(IShellItemArray* pItems, LPWSTR* ppszIcon) override;
    IFACEMETHODIMP GetToolTip(IShellItemArray* pItems, LPWSTR* ppszInfotip) override;
    IFACEMETHODIMP GetCanonicalName(GUID* pguidCommandName) override;
    IFACEMETHODIMP GetState(IShellItemArray* pItems, BOOL fOkToBeSlow, EXPCMDSTATE* pCmdState) override;
    IFACEMETHODIMP GetFlags(EXPCMDFLAGS* pFlags) override;
    IFACEMETHODIMP EnumSubCommands(IEnumExplorerCommand** ppEnum) override;
    IFACEMETHODIMP Invoke(IShellItemArray* pItems, IBindCtx* pBindCtx) override;

  private:
    std::vector<CComPtr<IExplorerCommand>> m_subCommands;
    void BuildSubCommands();
};
