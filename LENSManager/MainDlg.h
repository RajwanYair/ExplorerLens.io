#ifndef _MAINDLG_A8394D0D_EE2B_4A00_9FAC_AB8D3B03F078_
#define _MAINDLG_A8394D0D_EE2B_4A00_9FAC_AB8D3B03F078_

#include "tools.h"
#include "regmanager.h"
#include "ChangeSummaryDlg.h"
#include "DarkModeController.h"
#include "FormatStatusProvider.h"
#include "PerformanceDashboard.h"
#include "SystemTrayIcon.h"

#include <Htmlhelp.h>
#include <map>
#include <vector>
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
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnCtlColor)
		MESSAGE_HANDLER(ExplorerLens::WM_TRAYICON, OnTrayMessage)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDC_APPLY, OnApply)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_BTN_LOAD_CONFIG, OnLoadConfig)
		COMMAND_ID_HANDLER(IDC_BTN_THEME, OnToggleTheme)
		COMMAND_ID_HANDLER(IDC_BTN_EXPORT_DIAG, OnExportDiagnostics)
		COMMAND_ID_HANDLER(IDC_BTN_BENCHMARK, OnBenchmark)
		COMMAND_ID_HANDLER(IDC_BTN_ABOUT, OnAboutBtn)
		COMMAND_ID_HANDLER(ExplorerLens::ID_TRAY_OPEN, OnTrayOpen)
		COMMAND_ID_HANDLER(ExplorerLens::ID_TRAY_ABOUT, OnTrayAbout)
		COMMAND_ID_HANDLER(ExplorerLens::ID_TRAY_EXIT, OnTrayExit)
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
		COMMAND_HANDLER(IDC_CB_PSD, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_DDS, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_HDR, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_AUDIO, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_DOCUMENT, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_FONT, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_EXR, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_ICO, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_QOI, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_PPM, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_TGA, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_MODEL, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_EXT_IMAGE, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_TEXTURE, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_EXT_ARCHIVE, BN_CLICKED, OnCheckboxClicked)
		COMMAND_HANDLER(IDC_CB_EXT_DOCUMENT, BN_CLICKED, OnCheckboxClicked)
		COMMAND_ID_HANDLER(IDC_BTN_RESET_DEFAULTS, OnResetDefaults)
		COMMAND_ID_HANDLER(IDC_BTN_EXPORT_CONFIG, OnExportConfig)
		COMMAND_ID_HANDLER(IDC_BTN_SELECT_ALL, OnSelectAllBtn)
		COMMAND_ID_HANDLER(IDC_BTN_DESELECT_ALL, OnDeselectAllBtn)
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
	LRESULT OnMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnApply(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCheckboxClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLoadConfig(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnResetDefaults(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnExportConfig(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnToggleTheme(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnExportDiagnostics(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBenchmark(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAboutBtn(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTrayMessage(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnTrayOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTrayAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTrayExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	void OnSelectAll();
	void OnDeselectAll();
	LRESULT OnSelectAllBtn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { OnSelectAll(); return 0; }
	LRESULT OnDeselectAllBtn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) { OnDeselectAll(); return 0; }
	void InitUI();
	void OnApplyImpl();
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void CloseDialog(int nVal);

	LRESULT OnAppHelp(LPHELPINFO lphi);

private:
	CRegManager m_reg;
	CToolTipCtrl m_tooltip;
	CStatusBarCtrl m_statusBar;
	ExplorerLens::SystemTrayIcon m_trayIcon;

	// Handler status mapping for tooltips
	std::map<int, HandlerStatus> m_checkboxStatus;

	// Font resize support
	HFONT m_hFont = NULL;
	int m_fontSize = 8;       // Current font size in points (default: 8)
	static const int FONT_SIZE_MIN = 7;
	static const int FONT_SIZE_MAX = 16;

	// Layout anchoring for resize
	struct ControlAnchor {
		int id;
		RECT initialRect;   // Position in initial dialog DU (stored as pixels at init)
	};
	std::vector<ControlAnchor> m_anchors;
	SIZE m_initialSize = { 0, 0 };  // Initial dialog client size

	// Helper methods
	void InitTooltips();
	void AddTooltipWithStatus(int ctrlID, int LENSTYPE, LPCTSTR formatName);
	void InitStatusIcons();
	void UpdateStatusBar();
	int GetEnabledFormatCount();
	void RecreateFont(int pointSize);
	void RelayoutControls(int clientWidth, int clientHeight);

	// Configuration management
	ConfigSnapshot CaptureCurrentConfig();
	void ApplyConfigSnapshot(const ConfigSnapshot& config);
	bool LoadConfigFromFile(LPCTSTR filename, ConfigSnapshot& outConfig);
	bool LoadConfigFromRegFile(LPCTSTR filename, ConfigSnapshot& outConfig);
};
#endif//_MAINDLG_A8394D0D_EE2B_4A00_9FAC_AB8D3B03F078_
