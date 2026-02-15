#ifndef _ABOUT_0D23B3C4_9FA8_49E8_880D_5B596CC1EB28_
#define _ABOUT_0D23B3C4_9FA8_49E8_880D_5B596CC1EB28_
#pragma once
#include "resource.h"

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnClick)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnDlgColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnDlgColor)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnDlgColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
	return (LRESULT)GetStockObject(WHITE_BRUSH);
	}
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CenterWindow(GetParent());
		SetWindowLongW(GWL_STYLE, WS_BORDER);
		
		// Populate decoder stats in About dialog
		PopulateDecoderStats();
		
	return TRUE;
	}
	LRESULT OnClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/){EndDialog(IDOK);return 0;}
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/){EndDialog(wID);return 0;}
	
private:
	void PopulateDecoderStats() {
		// Show decoder count and version info in a static control, if available
		HWND hEdit = GetDlgItem(IDC_SORT_DESC);
		if (hEdit) {
			// 21 Engine decoders, 55+ registered extensions
			TCHAR buf[256];
			_stprintf_s(buf, _T("DarkThumbs v6.2.0\r\n")
				_T("Engine Decoders: 21\r\n")
				_T("Registered Extensions: 55+"));
			::SetWindowText(hEdit, buf);
		}
	}
};
#endif//_ABOUT_0D23B3C4_9FA8_49E8_880D_5B596CC1EB28_
