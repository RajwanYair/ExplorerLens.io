#ifndef _ABOUT_0D23B3C4_9FA8_49E8_880D_5B596CC1EB28_
#define _ABOUT_0D23B3C4_9FA8_49E8_880D_5B596CC1EB28_
#pragma once
#include <sstream>
#include <string>

#include "DarkModeController.h"
#include "resource.h"

namespace ExplorerLens {
namespace Engine {
class HardwareCapabilities;
}
}  // namespace ExplorerLens

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
  public:
    enum {
        IDD = IDD_ABOUTBOX
    };

    BEGIN_MSG_MAP(CAboutDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnClick)
    MESSAGE_HANDLER(WM_CTLCOLORDLG, OnDlgColor)
    MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnDlgColor)
    MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnDlgColor)
    MESSAGE_HANDLER(WM_CTLCOLORBTN, OnDlgColor)
    COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
    COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    COMMAND_ID_HANDLER(IDC_BTN_COPY_INFO, OnCopyInfo)
    END_MSG_MAP()

    /*
     * Handle WM_CTLCOLOR* messages to apply correct text/background colors
     * based on the current system theme (dark or light mode).
     */
    LRESULT OnDlgColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
    {
        auto& darkCtrl = ExplorerLens::DarkModeController::Instance();
        return reinterpret_cast<LRESULT>(
            darkCtrl.OnCtlColor(reinterpret_cast<HDC>(wParam), reinterpret_cast<HWND>(lParam)));
    }

    /*
     * Initialize the About dialog: center on parent, apply dark mode
     * theming, and populate hardware/decoder information.
     */
    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        CenterWindow(GetParent());
        SetWindowLongW(GWL_STYLE, WS_BORDER);

        auto& darkCtrl = ExplorerLens::DarkModeController::Instance();
        darkCtrl.ApplyToWindow(m_hWnd);

        PopulateAboutInfo();

        return TRUE;
    }

    LRESULT OnClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        EndDialog(IDOK);
        return 0;
    }
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        EndDialog(wID);
        return 0;
    }

    LRESULT OnCopyInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        HWND hEdit = GetDlgItem(IDC_ABOUT_SYSINFO);
        if (!hEdit)
            return 0;
        int len = ::GetWindowTextLengthW(hEdit);
        if (len <= 0)
            return 0;
        std::wstring text(len + 1, L'\0');
        ::GetWindowTextW(hEdit, &text[0], len + 1);
        text.resize(len);
        if (OpenClipboard()) {
            EmptyClipboard();
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * sizeof(wchar_t));
            if (hMem) {
                wchar_t* pMem = static_cast<wchar_t*>(GlobalLock(hMem));
                if (pMem) {
                    wcscpy_s(pMem, text.size() + 1, text.c_str());
                    GlobalUnlock(hMem);
                    SetClipboardData(CF_UNICODETEXT, hMem);
                }
            }
            CloseClipboard();
            ::MessageBoxW(m_hWnd, L"System information copied to clipboard.", L"ExplorerLens",
                          MB_OK | MB_ICONINFORMATION);
        }
        return 0;
    }

  private:
    HBRUSH m_bgBrush = nullptr;

    void PopulateAboutInfo();
};
#endif  //_ABOUT_0D23B3C4_9FA8_49E8_880D_5B596CC1EB28_
