#ifndef _MAINDLG_A8394D0D_EE2B_4A00_9FAC_AB8D3B03F078_
#define _MAINDLG_A8394D0D_EE2B_4A00_9FAC_AB8D3B03F078_

#include "tools.h"
#include "regmanager.h"
#include "DarkModeHelper.h"
#include "ChangeSummaryDlg.h"

#include <Htmlhelp.h>
#include <map>
#pragma comment(lib,"Htmlhelp.lib")

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>, public CDialogDrag<CMainDlg>,
		public CMessageFilter, public CIdleHandler, public CSnapWindow<CMainDlg>, public CDialogHelp<CMainDlg>
{
public:
	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorBtn)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDC_APPLY, OnApply)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_BTN_THEME, OnThemeToggle)
		COMMAND_ID_HANDLER(IDC_BTN_LOAD_CONFIG, OnLoadConfig)
		COMMAND_HANDLER(IDC_CB_CBZ, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_CBR, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_CB7, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_CBT, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_EPUB, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_MOBI, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_AZW, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_AZW3, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_ZIP, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_RAR, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_7Z, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_TAR, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_PHZ, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_FB2, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_WEBP, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_HEIF, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_AVIF, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_JXL, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_VIDEO, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_PDF, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_TIFF, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_SVG, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_RAW, BN_CLICKED, OnCheckboxClicked)
		CHAIN_MSG_MAP(CDialogDrag<CMainDlg>)
		CHAIN_MSG_MAP(CSnapWindow<CMainDlg>)
		CHAIN_MSG_MAP(CDialogHelp<CMainDlg>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCtlColorDlg(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnCtlColorBtn(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnApply(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnThemeToggle(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCheckboxClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLoadConfig(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	void OnSelectAll();
	void OnDeselectAll();
	void InitUI();
	void OnApplyImpl();
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void CloseDialog(int nVal);

	LRESULT OnAppHelp(LPHELPINFO lphi);

private:
	CRegManager m_reg;
	CToolTipCtrl m_tooltip;
	CStatusBarCtrl m_statusBar;
	
	bool m_isDarkMode;
	DarkMode::ThemeColors m_theme;
	HBRUSH m_hBrushBackground;
	HBRUSH m_hBrushGroupBox;
	HFONT m_hGroupBoxFont;  // Bold font for group box headers
	
	// Status icon mapping for owner-draw checkboxes (Sprint D3)
	std::map<int, HandlerStatus> m_checkboxStatus;
	
	// Helper methods
	void ApplyTheme();
	void UpdateThemeButton();
	
	void InitTooltips();
	void AddTooltipWithStatus(int ctrlID, int cbxType, LPCTSTR formatName);
	void InitDarkMode();
	void InitStatusIcons();
	void UpdateStatusBar();
	int GetEnabledFormatCount();
	void DrawStatusIcon(HDC hdc, int x, int y, HandlerStatus status);
	COLORREF GetStatusColor(HandlerStatus status);
	
	// Configuration management
	ConfigSnapshot CaptureCurrentConfig();
	void ApplyConfigSnapshot(const ConfigSnapshot& config);
	bool LoadConfigFromFile(LPCTSTR filename, ConfigSnapshot& outConfig);
	bool LoadConfigFromRegFile(LPCTSTR filename, ConfigSnapshot& outConfig);
};
#endif//_MAINDLG_A8394D0D_EE2B_4A00_9FAC_AB8D3B03F078_
