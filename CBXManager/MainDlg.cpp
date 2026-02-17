// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "about.h"
#include "MainDlg.h"
#include <vector>
#include <algorithm>
#include <map>


BOOL CMainDlg::PreTranslateMessage(MSG* pMsg) { return CWindow::IsDialogMessage(pMsg); }
BOOL CMainDlg::OnIdle() { return FALSE;}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	//add 'About' item to sys menu
	ATLASSERT(IDC_APPABOUT < 0xF000);
	CMenu pSysMenu=GetSystemMenu(FALSE);
	if (!pSysMenu.IsNull())
	{
		pSysMenu.AppendMenu(MF_SEPARATOR);
		pSysMenu.AppendMenu(MF_STRING, IDC_APPABOUT, _T("About"));
	}

	// Initialize dark mode support (Sprint D3, re-enabled Sprint 8)
	// Detects system dark mode, sets title bar and app mode.
	// Note: OnCtlColor handlers now conditionally apply dark theme.
	InitDarkMode();

	// Initialize status bar (Sprint D3)
	HWND hStatusBar = GetDlgItem(IDC_STATUSBAR);
	if (hStatusBar)
	{
		m_statusBar.Attach(hStatusBar);
		UpdateStatusBar();
	}

	// Initialize status icons for checkboxes (Sprint D3)
	InitStatusIcons();

	InitUI();

	// Initialize tooltips with handler status (Sprint D1)
	InitTooltips();

	// Force all checkboxes to be visible before layout
	int allCheckboxes[] = {
		IDC_CB_CBZ, IDC_CB_CBR, IDC_CB_CB7, IDC_CB_CBT,
		IDC_CB_EPUB, IDC_CB_MOBI, IDC_CB_AZW, IDC_CB_AZW3,
		IDC_CB_ZIP, IDC_CB_RAR, IDC_CB_7Z, IDC_CB_TAR,
		IDC_CB_PHZ, IDC_CB_FB2,
		IDC_CB_WEBP, IDC_CB_HEIF, IDC_CB_AVIF, IDC_CB_JXL,
		IDC_CB_VIDEO, IDC_CB_PDF, IDC_CB_TIFF, IDC_CB_SVG, IDC_CB_RAW
	};
	for (int id : allCheckboxes) {
		HWND hCheck = GetDlgItem(id);
		if (hCheck) {
			::ShowWindow(hCheck, SW_SHOW);
			::UpdateWindow(hCheck);
		}
	}

	// DISABLED: Dynamic layout causes control overlap at non-default DPI.
	// Tracked: Sprint 18 (WinUI Manager rewrite) replaces manual layout.
	// RECT rc;
	// GetClientRect(&rc);
	// SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));
	
	// Set initial theme button text
	UpdateThemeButton();

	//set focus to Cancel btn
	GotoDlgCtrl(GetDlgItem(IDCANCEL));
return FALSE;
}

LRESULT CMainDlg::OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	// DISABLED: OnSize handler causes control overlap; using static RC layout.
	// Tracked: Sprint 18 (WinUI Manager rewrite) replaces WTL layout engine.
	return 0;
	
	if (wParam == SIZE_MINIMIZED) return 0;
	
	int clientWidth = LOWORD(lParam);
	int clientHeight = HIWORD(lParam);
	
	// Minimum layout checks
	if (clientWidth < 360 || clientHeight < 325) return 0;
	
	const int MARGIN = 10;
	const int GROUP_SPACING = 8;        // Space between group boxes (increased)
	const int GROUPBOX_HEIGHT = 62;
	const int CHECKBOX_HEIGHT = 12;     // Increased for better readability
	const int CHECKBOX_SPACING = 16;    // Increased gap between checkboxes
	const int BUTTON_HEIGHT = 14;
	const int BUTTON_WIDTH = 60;
	const int THEME_BUTTON_WIDTH = 50;
	
	// Determine layout: portrait (vertical) vs landscape (horizontal)
	bool isPortrait = (clientHeight > clientWidth);
	
	int colWidth, col1X, col2X, numCols;
	
	if (isPortrait) {
		// Portrait: Single column layout (full width)
		numCols = 1;
		colWidth = clientWidth - (2 * MARGIN);
		col1X = MARGIN;
		col2X = MARGIN;
	} else {
		// Landscape: Two column layout
		numCols = 2;
		colWidth = (clientWidth - (3 * MARGIN)) / 2;
		col1X = MARGIN;
		col2X = col1X + colWidth + MARGIN;
	}
	
	int currentY = 5;
	
	// Helper function to move groupbox and its checkboxes
	auto MoveGroupWithChecks = [&](int groupID, int x, int y, const int* checkIDs, int numChecks) {
		HWND hGroup = GetDlgItem(groupID);
		if (hGroup) {
			::SetWindowPos(hGroup, NULL, x, y, colWidth, GROUPBOX_HEIGHT, SWP_NOZORDER | SWP_SHOWWINDOW);
			// Force group box to redraw to show text
			::InvalidateRect(hGroup, NULL, TRUE);
		}
		
		int checkY = y + 16;  // Start lower inside groupbox for better spacing
		for (int i = 0; i < numChecks; i++) {
			HWND hCheck = GetDlgItem(checkIDs[i]);
			if (hCheck) {
				::SetWindowPos(hCheck, NULL, x + 10, checkY, colWidth - 20, CHECKBOX_HEIGHT, SWP_NOZORDER | SWP_SHOWWINDOW);
				::ShowWindow(hCheck, SW_SHOW);  // Ensure visible
			}
			checkY += CHECKBOX_SPACING;
		}
	};
	
	// Layout groupboxes based on orientation
	if (numCols == 1) {
		// Portrait: Stack all groups vertically
		const int comicIDs[] = {IDC_CB_CBZ, IDC_CB_CBR, IDC_CB_CB7, IDC_CB_CBT};
		MoveGroupWithChecks(IDC_COMIC_GROUP, col1X, currentY, comicIDs, 4);
		currentY += GROUPBOX_HEIGHT + GROUP_SPACING;
		
		const int ebookIDs[] = {IDC_CB_EPUB, IDC_CB_MOBI, IDC_CB_AZW, IDC_CB_AZW3};
		MoveGroupWithChecks(IDC_EBOOK_GROUP, col1X, currentY, ebookIDs, 4);
		currentY += GROUPBOX_HEIGHT + GROUP_SPACING;
		
		const int archiveIDs[] = {IDC_CB_ZIP, IDC_CB_RAR, IDC_CB_7Z, IDC_CB_TAR};
		MoveGroupWithChecks(IDC_ARCHIVE_GROUP, col1X, currentY, archiveIDs, 4);
		currentY += GROUPBOX_HEIGHT + GROUP_SPACING;
		
		const int photoIDs[] = {IDC_CB_PHZ, IDC_CB_FB2};
		MoveGroupWithChecks(IDC_PHOTO_GROUP, col1X, currentY, photoIDs, 2);
		currentY += GROUPBOX_HEIGHT + GROUP_SPACING;
		
		const int imageIDs[] = {IDC_CB_WEBP, IDC_CB_HEIF, IDC_CB_AVIF, IDC_CB_JXL};
		MoveGroupWithChecks(IDC_IMAGE_GROUP, col1X, currentY, imageIDs, 4);
		currentY += GROUPBOX_HEIGHT + GROUP_SPACING;
		
		const int mediaIDs[] = {IDC_CB_VIDEO, IDC_CB_PDF, IDC_CB_TIFF, IDC_CB_SVG, IDC_CB_RAW};
		int mediaHeight = 16 + (5 * CHECKBOX_SPACING);  // Better spacing
		HWND hMediaGroup = GetDlgItem(IDC_MEDIA_GROUP);
		if (hMediaGroup) {
			::SetWindowPos(hMediaGroup, NULL, col1X, currentY, colWidth, mediaHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
			::InvalidateRect(hMediaGroup, NULL, TRUE);
		}
		int checkY = currentY + 16;
		for (int i = 0; i < 5; i++) {
			HWND hCheck = GetDlgItem(mediaIDs[i]);
			if (hCheck) {
				::SetWindowPos(hCheck, NULL, col1X + 10, checkY, colWidth - 20, CHECKBOX_HEIGHT, SWP_NOZORDER | SWP_SHOWWINDOW);
				::ShowWindow(hCheck, SW_SHOW);  // Ensure visible
			}
			checkY += CHECKBOX_SPACING;
		}
		currentY += mediaHeight + GROUP_SPACING;
		
		const int collageIDs[] = {IDC_RADIO_1X1, IDC_RADIO_2X2, IDC_RADIO_3X3, IDC_RADIO_4X4};
		MoveGroupWithChecks(IDC_COLLAGE_GROUP, col1X, currentY, collageIDs, 4);
		currentY += GROUPBOX_HEIGHT + GROUP_SPACING;
		
		const int advOptIDs[] = {IDC_CB_SORT, IDC_SORT_DESC, IDC_CB_SHOWICON, IDC_CB_SHOWICON_DESC};
		MoveGroupWithChecks(IDC_SORT_ADVOPTGROUP, col1X, currentY, advOptIDs, 4);
		currentY += GROUPBOX_HEIGHT + 10;
		
	} else {
		// Landscape: Two-column layout
		int leftY = 5, rightY = 5;
		
		const int comicIDs[] = {IDC_CB_CBZ, IDC_CB_CBR, IDC_CB_CB7, IDC_CB_CBT};
		MoveGroupWithChecks(IDC_COMIC_GROUP, col1X, leftY, comicIDs, 4);
		leftY += GROUPBOX_HEIGHT + GROUP_SPACING;
		
		const int ebookIDs[] = {IDC_CB_EPUB, IDC_CB_MOBI, IDC_CB_AZW, IDC_CB_AZW3};
		MoveGroupWithChecks(IDC_EBOOK_GROUP, col2X, rightY, ebookIDs, 4);
		rightY += GROUPBOX_HEIGHT + GROUP_SPACING;
		
		const int archiveIDs[] = {IDC_CB_ZIP, IDC_CB_RAR, IDC_CB_7Z, IDC_CB_TAR};
		MoveGroupWithChecks(IDC_ARCHIVE_GROUP, col1X, leftY, archiveIDs, 4);
		leftY += GROUPBOX_HEIGHT + GROUP_SPACING;
		
		const int photoIDs[] = {IDC_CB_PHZ, IDC_CB_FB2};
		MoveGroupWithChecks(IDC_PHOTO_GROUP, col2X, rightY, photoIDs, 2);
		rightY += GROUPBOX_HEIGHT + GROUP_SPACING;
		
		const int imageIDs[] = {IDC_CB_WEBP, IDC_CB_HEIF, IDC_CB_AVIF, IDC_CB_JXL};
		MoveGroupWithChecks(IDC_IMAGE_GROUP, col1X, leftY, imageIDs, 4);
		leftY += GROUPBOX_HEIGHT + GROUP_SPACING;
		
		const int mediaIDs[] = {IDC_CB_VIDEO, IDC_CB_PDF, IDC_CB_TIFF, IDC_CB_SVG, IDC_CB_RAW};
		int mediaHeight = 16 + (5 * CHECKBOX_SPACING);
		HWND hMediaGroup = GetDlgItem(IDC_MEDIA_GROUP);
		if (hMediaGroup) {
			::SetWindowPos(hMediaGroup, NULL, col2X, rightY, colWidth, mediaHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
			::InvalidateRect(hMediaGroup, NULL, TRUE);
		}
		int checkY = rightY + 16;
		for (int i = 0; i < 5; i++) {
			HWND hCheck = GetDlgItem(mediaIDs[i]);
			if (hCheck) {
				::SetWindowPos(hCheck, NULL, col2X + 10, checkY, colWidth - 20, CHECKBOX_HEIGHT, SWP_NOZORDER | SWP_SHOWWINDOW);
				::ShowWindow(hCheck, SW_SHOW);  // Ensure visible
			}
			checkY += CHECKBOX_SPACING;
		}
		rightY += mediaHeight + GROUP_SPACING;
		
		const int collageIDs[] = {IDC_RADIO_1X1, IDC_RADIO_2X2, IDC_RADIO_3X3, IDC_RADIO_4X4};
		MoveGroupWithChecks(IDC_COLLAGE_GROUP, col1X, leftY, collageIDs, 4);
		leftY += GROUPBOX_HEIGHT + GROUP_SPACING;
		
		const int advOptIDs[] = {IDC_CB_SORT, IDC_SORT_DESC, IDC_CB_SHOWICON, IDC_CB_SHOWICON_DESC};
		MoveGroupWithChecks(IDC_SORT_ADVOPTGROUP, col2X, rightY, advOptIDs, 4);
		rightY += GROUPBOX_HEIGHT + 10;
		
		currentY = max(leftY, rightY);
	}
	
	// Position buttons at bottom (leave room for status bar)
	const int STATUSBAR_HEIGHT = 20;
	int buttonY = clientHeight - STATUSBAR_HEIGHT - BUTTON_HEIGHT - 10;
	int cancelX = clientWidth - MARGIN - BUTTON_WIDTH;
	int okX = cancelX - BUTTON_WIDTH - 10;
	int applyX = okX - BUTTON_WIDTH - 10;
	int themeX = MARGIN;
	
	HWND hTheme = GetDlgItem(IDC_BTN_THEME);
	HWND hApply = GetDlgItem(IDC_APPLY);
	HWND hOK = GetDlgItem(IDOK);
	HWND hCancel = GetDlgItem(IDCANCEL);
	
	if (hTheme) ::SetWindowPos(hTheme, NULL, themeX, buttonY, THEME_BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER | SWP_SHOWWINDOW);
	if (hApply) ::SetWindowPos(hApply, NULL, applyX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER | SWP_SHOWWINDOW);
	if (hOK) ::SetWindowPos(hOK, NULL, okX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER | SWP_SHOWWINDOW);
	if (hCancel) ::SetWindowPos(hCancel, NULL, cancelX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, SWP_NOZORDER | SWP_SHOWWINDOW);
	
	// Position status bar at bottom
	if (m_statusBar.m_hWnd) {
		m_statusBar.SetWindowPos(NULL, 0, clientHeight - STATUSBAR_HEIGHT, clientWidth, STATUSBAR_HEIGHT, SWP_NOZORDER);
	}
	
	// Invalidate to force redraw
	Invalidate();
	
	return 0;
}

LRESULT CMainDlg::OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
	lpMMI->ptMinTrackSize.x = 360;  // Minimum width
	lpMMI->ptMinTrackSize.y = 325;  // Minimum height
	return 0;
}

void CMainDlg::InitUI()
{
	// Convert format checkboxes to owner-draw for status icons (Sprint D3)
	// DISABLED: BS_OWNERDRAW hides checkbox text on themed controls.
	// Tracked: Sprint 18 (WinUI Manager) will use native toggle switches.
	/*
	int formatCheckboxes[] = {
		IDC_CB_CBZ, IDC_CB_CBR, IDC_CB_CB7, IDC_CB_CBT,
		IDC_CB_EPUB, IDC_CB_MOBI, IDC_CB_AZW, IDC_CB_AZW3,
		IDC_CB_ZIP, IDC_CB_RAR, IDC_CB_7Z, IDC_CB_TAR,
		IDC_CB_PHZ, IDC_CB_FB2,
		IDC_CB_WEBP, IDC_CB_HEIF, IDC_CB_AVIF, IDC_CB_JXL,
		IDC_CB_VIDEO, IDC_CB_PDF, IDC_CB_TIFF, IDC_CB_SVG, IDC_CB_RAW
	};
	
	for (int id : formatCheckboxes)
	{
		HWND hCheckbox = GetDlgItem(id);
		if (hCheckbox)
		{
			// Add BS_OWNERDRAW style
			LONG style = ::GetWindowLong(hCheckbox, GWL_STYLE);
			::SetWindowLong(hCheckbox, GWL_STYLE, style | BS_OWNERDRAW);
			
			// Ensure checkbox is visible after style change
			::ShowWindow(hCheckbox, SW_SHOW);
			::InvalidateRect(hCheckbox, NULL, TRUE);
		}
	}
	*/

	//Button_GetCheck   BST_CHECKED : BST_UNCHECKED equals TRUE: FALSE
	// Comic Book Formats
	Button_SetCheck(GetDlgItem(IDC_CB_CBZ),  m_reg.HasTH(CBX_CBZ));
	Button_SetCheck(GetDlgItem(IDC_CB_CBR),  m_reg.HasTH(CBX_CBR));
	Button_SetCheck(GetDlgItem(IDC_CB_CB7),  m_reg.HasTH(CBX_CB7));
	Button_SetCheck(GetDlgItem(IDC_CB_CBT),  m_reg.HasTH(CBX_CBT));
	
	// E-Book Formats
	Button_SetCheck(GetDlgItem(IDC_CB_EPUB), m_reg.HasTH(CBX_EPUB));
	Button_SetCheck(GetDlgItem(IDC_CB_MOBI), m_reg.HasTH(CBX_MOBI));
	Button_SetCheck(GetDlgItem(IDC_CB_AZW),  m_reg.HasTH(CBX_AZW));
	Button_SetCheck(GetDlgItem(IDC_CB_AZW3), m_reg.HasTH(CBX_AZW3));
	
	// Archive Formats
	Button_SetCheck(GetDlgItem(IDC_CB_ZIP),  m_reg.HasTH(CBX_ZIP));
	Button_SetCheck(GetDlgItem(IDC_CB_RAR),  m_reg.HasTH(CBX_RAR));
	Button_SetCheck(GetDlgItem(IDC_CB_7Z),   m_reg.HasTH(CBX_7Z));
	Button_SetCheck(GetDlgItem(IDC_CB_TAR),  m_reg.HasTH(CBX_TAR));
	
	// Photo & Other Formats
	Button_SetCheck(GetDlgItem(IDC_CB_PHZ),  m_reg.HasTH(CBX_PHZ));
	Button_SetCheck(GetDlgItem(IDC_CB_FB2),  m_reg.HasTH(CBX_FB2));
	
	// Modern Image Formats
	Button_SetCheck(GetDlgItem(IDC_CB_WEBP), m_reg.HasTH(CBX_WEBP));
	Button_SetCheck(GetDlgItem(IDC_CB_HEIF), m_reg.HasTH(CBX_HEIF));
	Button_SetCheck(GetDlgItem(IDC_CB_AVIF), m_reg.HasTH(CBX_AVIF));
	Button_SetCheck(GetDlgItem(IDC_CB_JXL),  m_reg.HasTH(CBX_JXL));
	
	// Media & Documents
	Button_SetCheck(GetDlgItem(IDC_CB_VIDEO), m_reg.HasTH(CBX_VIDEO));
	Button_SetCheck(GetDlgItem(IDC_CB_PDF),   m_reg.HasTH(CBX_PDF));
	Button_SetCheck(GetDlgItem(IDC_CB_TIFF),  m_reg.HasTH(CBX_TIFF));
	Button_SetCheck(GetDlgItem(IDC_CB_SVG),   m_reg.HasTH(CBX_SVG));
	Button_SetCheck(GetDlgItem(IDC_CB_RAW),   m_reg.HasTH(CBX_RAW));
	
	// Sprint 8: Professional & Specialized Formats
	Button_SetCheck(GetDlgItem(IDC_CB_PSD),   m_reg.HasTH(CBX_PSD));
	Button_SetCheck(GetDlgItem(IDC_CB_DDS),   m_reg.HasTH(CBX_DDS));
	Button_SetCheck(GetDlgItem(IDC_CB_HDR),   m_reg.HasTH(CBX_HDR));
	Button_SetCheck(GetDlgItem(IDC_CB_EXR),   m_reg.HasTH(CBX_EXR));
	Button_SetCheck(GetDlgItem(IDC_CB_PPM),   m_reg.HasTH(CBX_PPM));
	Button_SetCheck(GetDlgItem(IDC_CB_ICO),   m_reg.HasTH(CBX_ICO));
	Button_SetCheck(GetDlgItem(IDC_CB_QOI),   m_reg.HasTH(CBX_QOI));
	Button_SetCheck(GetDlgItem(IDC_CB_TGA),   m_reg.HasTH(CBX_TGA));
	Button_SetCheck(GetDlgItem(IDC_CB_AUDIO), m_reg.HasTH(CBX_AUDIO));
	Button_SetCheck(GetDlgItem(IDC_CB_DOCUMENT), m_reg.HasTH(CBX_DOCUMENT));
	Button_SetCheck(GetDlgItem(IDC_CB_FONT),  m_reg.HasTH(CBX_FONT));
	Button_SetCheck(GetDlgItem(IDC_CB_MODEL), m_reg.HasTH(CBX_MODEL));
	
	// Collage Mode (Sprint C2)
	DWORD collageMode = m_reg.GetCollageMode();
	Button_SetCheck(GetDlgItem(IDC_RADIO_1X1), (collageMode == 1));
	Button_SetCheck(GetDlgItem(IDC_RADIO_2X2), (collageMode == 4));
	Button_SetCheck(GetDlgItem(IDC_RADIO_3X3), (collageMode == 9));
	Button_SetCheck(GetDlgItem(IDC_RADIO_4X4), (collageMode == 16));
	
	// Options
	Button_SetCheck(GetDlgItem(IDC_CB_SHOWICON), m_reg.IsShowIconOpt());
	Button_SetCheck(GetDlgItem(IDC_CB_SORT), m_reg.IsSortOpt());
}

//check ui state,compare to registry state->if!= refresh
void CMainDlg::OnApplyImpl()
{
	BOOL bRet, bRefresh=FALSE;

	//sort option
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_SORT)));
	if (bRet!=m_reg.IsSortOpt())
	{
		bRefresh=TRUE;
		m_reg.SetSortOpt(bRet);
	}
	//show archive icon
	bRet = (BST_CHECKED == Button_GetCheck(GetDlgItem(IDC_CB_SHOWICON)));
	if (bRet != m_reg.IsShowIconOpt())
	{
		bRefresh = TRUE;
		m_reg.SetShowIconOpt(bRet);
	}
	
	// Comic Book Formats
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_CBZ)));
	if (bRet!=m_reg.HasTH(CBX_CBZ))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_CBZ, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_CBR)));
	if (bRet!=m_reg.HasTH(CBX_CBR))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_CBR, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_CB7)));
	if (bRet!=m_reg.HasTH(CBX_CB7))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_CB7, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_CBT)));
	if (bRet!=m_reg.HasTH(CBX_CBT))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_CBT, bRet);
	}
	
	// E-Book Formats
	bRet = (BST_CHECKED == Button_GetCheck(GetDlgItem(IDC_CB_EPUB)));
	if (bRet != m_reg.HasTH(CBX_EPUB))
	{
		bRefresh = TRUE;
		m_reg.SetHandlers(CBX_EPUB, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_MOBI)));
	if (bRet!=m_reg.HasTH(CBX_MOBI))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_MOBI, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_AZW)));
	if (bRet!=m_reg.HasTH(CBX_AZW))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_AZW, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_AZW3)));
	if (bRet!=m_reg.HasTH(CBX_AZW3))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_AZW3, bRet);
	}
	
	// Archive Formats
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_ZIP)));
	if (bRet!=m_reg.HasTH(CBX_ZIP))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_ZIP, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_RAR)));
	if (bRet!=m_reg.HasTH(CBX_RAR))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_RAR, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_7Z)));
	if (bRet!=m_reg.HasTH(CBX_7Z))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_7Z, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_TAR)));
	if (bRet!=m_reg.HasTH(CBX_TAR))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_TAR, bRet);
	}
	
	// Photo & Other Formats
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_PHZ)));
	if (bRet!=m_reg.HasTH(CBX_PHZ))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_PHZ, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_FB2)));
	if (bRet!=m_reg.HasTH(CBX_FB2))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_FB2, bRet);
	}
	
	// Modern Image Formats
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_WEBP)));
	if (bRet!=m_reg.HasTH(CBX_WEBP))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_WEBP, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_HEIF)));
	if (bRet!=m_reg.HasTH(CBX_HEIF))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_HEIF, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_AVIF)));
	if (bRet!=m_reg.HasTH(CBX_AVIF))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_AVIF, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_JXL)));
	if (bRet!=m_reg.HasTH(CBX_JXL))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_JXL, bRet);
	}
	
	// Media & Documents
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_VIDEO)));
	if (bRet!=m_reg.HasTH(CBX_VIDEO))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_VIDEO, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_PDF)));
	if (bRet!=m_reg.HasTH(CBX_PDF))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_PDF, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_TIFF)));
	if (bRet!=m_reg.HasTH(CBX_TIFF))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_TIFF, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_SVG)));
	if (bRet!=m_reg.HasTH(CBX_SVG))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_SVG, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_RAW)));
	if (bRet!=m_reg.HasTH(CBX_RAW))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_RAW, bRet);
	}
	
	// Sprint 8: Professional & Specialized Formats
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_PSD)));
	if (bRet!=m_reg.HasTH(CBX_PSD))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_PSD, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_DDS)));
	if (bRet!=m_reg.HasTH(CBX_DDS))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_DDS, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_HDR)));
	if (bRet!=m_reg.HasTH(CBX_HDR))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_HDR, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_EXR)));
	if (bRet!=m_reg.HasTH(CBX_EXR))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_EXR, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_PPM)));
	if (bRet!=m_reg.HasTH(CBX_PPM))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_PPM, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_ICO)));
	if (bRet!=m_reg.HasTH(CBX_ICO))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_ICO, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_QOI)));
	if (bRet!=m_reg.HasTH(CBX_QOI))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_QOI, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_TGA)));
	if (bRet!=m_reg.HasTH(CBX_TGA))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_TGA, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_AUDIO)));
	if (bRet!=m_reg.HasTH(CBX_AUDIO))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_AUDIO, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_DOCUMENT)));
	if (bRet!=m_reg.HasTH(CBX_DOCUMENT))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_DOCUMENT, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_FONT)));
	if (bRet!=m_reg.HasTH(CBX_FONT))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_FONT, bRet);
	}
	bRet=(BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CB_MODEL)));
	if (bRet!=m_reg.HasTH(CBX_MODEL))
	{
		bRefresh=TRUE;
		m_reg.SetHandlers(CBX_MODEL, bRet);
	}
	
	// Collage Mode (Sprint C2)
	DWORD newCollageMode = 1; // Default: single page
	if (BST_CHECKED == Button_GetCheck(GetDlgItem(IDC_RADIO_2X2))) newCollageMode = 4;
	else if (BST_CHECKED == Button_GetCheck(GetDlgItem(IDC_RADIO_3X3))) newCollageMode = 9;
	else if (BST_CHECKED == Button_GetCheck(GetDlgItem(IDC_RADIO_4X4))) newCollageMode = 16;
	
	if (newCollageMode != m_reg.GetCollageMode())
	{
		bRefresh = TRUE;
		m_reg.SetCollageMode(newCollageMode);
	}

	if (bRefresh)
	{
		ATLTRACE("refreshing FS\n");
		m_statusBar.SetText(0, _T("Refreshing Explorer..."));
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT | SHCNF_NOTIFYRECURSIVE, NULL, NULL);
	}
	
	InitUI();//reload
	UpdateStatusBar(); // Update status bar after changes (Sprint D3)
}

LRESULT CMainDlg::OnApply(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Capture state before changes
	ConfigSnapshot oldConfig = CaptureCurrentConfig();
	
	OnApplyImpl();
	
	// Capture state after changes
	ConfigSnapshot newConfig = CaptureCurrentConfig();
	
	// Show change summary dialog
	CChangeSummaryDlg dlg(oldConfig, newConfig);
	INT_PTR result = dlg.DoModal(m_hWnd);
	
	// Check if user wants to restore previous settings
	if (result == IDC_BTN_RESTORE && dlg.ShouldRestore())
	{
		ApplyConfigSnapshot(oldConfig);
		MessageBox(_T("Previous configuration has been restored."), 
			_T("Configuration Restored"), MB_OK | MB_ICONINFORMATION);
	}
	
	return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Capture state before changes
	ConfigSnapshot oldConfig = CaptureCurrentConfig();
	
	OnApplyImpl();
	
	// Capture state after changes
	ConfigSnapshot newConfig = CaptureCurrentConfig();
	
	// Show change summary dialog
	CChangeSummaryDlg dlg(oldConfig, newConfig);
	INT_PTR result = dlg.DoModal(m_hWnd);
	
	// Check if user wants to restore previous settings
	if (result == IDC_BTN_RESTORE && dlg.ShouldRestore())
	{
		ApplyConfigSnapshot(oldConfig);
		MessageBox(_T("Previous configuration has been restored."), 
			_T("Configuration Restored"), MB_OK | MB_ICONINFORMATION);
		return 0; // Don't close dialog
	}
	
	CloseDialog(wID);
	return 0;
}


LRESULT CMainDlg::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam!=IDC_APPABOUT) return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	CAboutDlg _a;
	_a.DoModal(m_hWnd);
return 0;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// Clean up dark mode brushes and fonts (Sprint D3, E1.2)
	if (m_hBrushBackground) DeleteObject(m_hBrushBackground);
	if (m_hBrushGroupBox) DeleteObject(m_hBrushGroupBox);
	if (m_hGroupBoxFont) DeleteObject(m_hGroupBoxFont);

	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);
return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(wID);
return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}

LRESULT CMainDlg::OnAppHelp(LPHELPINFO lphi)
{
	switch (lphi->iCtrlId)
	{
		case IDC_TH_GROUP:
		case IDC_CB_CBZ:
		case IDC_CB_EPUB:
		case IDC_CB_CBR:
		case IDC_CB_ZIP:
		case IDC_CB_RAR:
			// '#' anchors must use id attribute
			HtmlHelp(m_hWnd, _T("CBXShellHelp.chm::manager.html#optth"), HH_DISPLAY_TOPIC, NULL);
		break;

		case IDC_SORT_ADVOPTGROUP:
			HtmlHelp(m_hWnd, _T("CBXShellHelp.chm::manager.html#advopt"), HH_DISPLAY_TOPIC, NULL);
		break;

		case IDC_CB_SORT:
		case IDC_SORT_DESC:
			ATLTRACE("HH sort opt\n");
			HtmlHelp(m_hWnd, _T("CBXShellHelp.chm::FAQ.html#custth"), HH_DISPLAY_TOPIC, NULL);
		break;

	default:
			ATLTRACE("HH default\n");
			HtmlHelp(m_hWnd, _T("CBXShellHelp.chm::manager.html"), HH_DISPLAY_TOPIC, NULL);//about?
		break;
	}
return 0;
}

//////////////////////////////////////////////////////////////////////////
// Sprint D1: Tooltip support with handler status indicators
//////////////////////////////////////////////////////////////////////////

void CMainDlg::InitTooltips()
{
	// Create tooltip control
	m_tooltip.Create(m_hWnd);
	m_tooltip.SetMaxTipWidth(450);  // Wider for detailed info
	m_tooltip.SetDelayTime(TTDT_AUTOPOP, 15000);  // Show for 15 seconds
	m_tooltip.Activate(TRUE);

	// Comic Book Formats - Add tooltips with file extensions and details
	AddTooltipWithStatus(IDC_CB_CBZ, CBX_CBZ, _T("CBZ - Comic Book ZIP Archive\nExtensions: .cbz"));
	AddTooltipWithStatus(IDC_CB_CBR, CBX_CBR, _T("CBR - Comic Book RAR Archive\nExtensions: .cbr\nRequires: UnRAR library"));
	AddTooltipWithStatus(IDC_CB_CB7, CBX_CB7, _T("CB7 - Comic Book 7-Zip Archive\nExtensions: .cb7"));
	AddTooltipWithStatus(IDC_CB_CBT, CBX_CBT, _T("CBT - Comic Book TAR Archive\nExtensions: .cbt"));
	
	// E-Book Formats
	AddTooltipWithStatus(IDC_CB_EPUB, CBX_EPUB, _T("EPUB - Electronic Publication\nExtensions: .epub\nSupports: Cover extraction from metadata"));
	AddTooltipWithStatus(IDC_CB_MOBI, CBX_MOBI, _T("MOBI - Mobipocket E-Book\nExtensions: .mobi"));
	AddTooltipWithStatus(IDC_CB_AZW, CBX_AZW, _T("AZW - Amazon Kindle Format\nExtensions: .azw"));
	AddTooltipWithStatus(IDC_CB_AZW3, CBX_AZW3, _T("AZW3 - Kindle Format 8 (KF8)\nExtensions: .azw3"));
	
	// Archive Formats
	AddTooltipWithStatus(IDC_CB_ZIP, CBX_ZIP, _T("ZIP - Standard ZIP Archive\nExtensions: .zip\nThumbnails from first image inside"));
	AddTooltipWithStatus(IDC_CB_RAR, CBX_RAR, _T("RAR - WinRAR Archive\nExtensions: .rar\nRequires: UnRAR library"));
	AddTooltipWithStatus(IDC_CB_7Z, CBX_7Z, _T("7Z - 7-Zip Archive\nExtensions: .7z\nHigh compression ratio"));
	AddTooltipWithStatus(IDC_CB_TAR, CBX_TAR, _T("TAR - Tape Archive\nExtensions: .tar, .tar.gz, .tar.bz2"));
	
	// Photo Albums & Other
	AddTooltipWithStatus(IDC_CB_PHZ, CBX_PHZ, _T("PHZ - Photo ZIP Album\nExtensions: .phz\nOptimized for photo collections"));
	AddTooltipWithStatus(IDC_CB_FB2, CBX_FB2, _T("FB2 - FictionBook E-Book\nExtensions: .fb2\nXML-based format"));
	
	// Modern Image Formats
	AddTooltipWithStatus(IDC_CB_WEBP, CBX_WEBP, _T("WebP - Modern Image Format\nExtensions: .webp\nPerformance: Fast, built-in decoder"));
	AddTooltipWithStatus(IDC_CB_HEIF, CBX_HEIF, _T("HEIF/HEIC - High Efficiency Image\nExtensions: .heif, .heic\nUsed by: iPhone photos\nRequires: HEIF Image Extensions (Microsoft Store)"));
	AddTooltipWithStatus(IDC_CB_AVIF, CBX_AVIF, _T("AVIF - AV1 Image Format\nExtensions: .avif\nRequires: AV1 Video Extension (Microsoft Store)\nPerformance: Fast via WIC"));
	AddTooltipWithStatus(IDC_CB_JXL, CBX_JXL, _T("JPEG XL - Next-Gen Image Format\nExtensions: .jxl\nRequires: Windows Imaging Component support"));
	
	// Media & Documents
	AddTooltipWithStatus(IDC_CB_VIDEO, CBX_VIDEO, _T("Video Files - Extract Frame Thumbnails\nExtensions: .mp4, .avi, .mkv, .mov, .wmv\nRequires: DirectShow codecs\nPerformance: Cached for speed"));
	AddTooltipWithStatus(IDC_CB_PDF, CBX_PDF, _T("PDF Documents\nExtensions: .pdf\nPerformance: Fast, uses embedded thumbnails when available"));
	AddTooltipWithStatus(IDC_CB_TIFF, CBX_TIFF, _T("TIFF Images\nExtensions: .tif, .tiff\nSupports: Multi-page TIFF files"));
	AddTooltipWithStatus(IDC_CB_SVG, CBX_SVG, _T("SVG Vector Graphics\nExtensions: .svg, .svgz\nScalable vector format"));
	AddTooltipWithStatus(IDC_CB_RAW, CBX_RAW, _T("RAW Camera Photos\nExtensions: .dng, .cr2, .cr3, .nef, .arw, .orf\nRequires: Microsoft Camera Codec Pack (free)\nPerformance: Fast (uses embedded preview)"));
	
	// Sprint 8: Professional & Specialized Formats
	AddTooltipWithStatus(IDC_CB_PSD, CBX_PSD, _T("PSD - Adobe Photoshop Document\nExtensions: .psd, .psb\nShows: Composite preview layer"));
	AddTooltipWithStatus(IDC_CB_DDS, CBX_DDS, _T("DDS - DirectDraw Surface\nExtensions: .dds\nUsed by: Games, 3D applications"));
	AddTooltipWithStatus(IDC_CB_HDR, CBX_HDR, _T("HDR - Radiance High Dynamic Range\nExtensions: .hdr\nUsed by: Professional photography, 3D rendering"));
	AddTooltipWithStatus(IDC_CB_EXR, CBX_EXR, _T("EXR - OpenEXR High Dynamic Range\nExtensions: .exr\nUsed by: VFX, film production"));
	AddTooltipWithStatus(IDC_CB_PPM, CBX_PPM, _T("PPM - Portable Pixmap\nExtensions: .ppm, .pgm, .pbm\nUsed by: Scientific imaging, CLI tools"));
	AddTooltipWithStatus(IDC_CB_ICO, CBX_ICO, _T("ICO - Windows Icon\nExtensions: .ico, .cur\nShows: Largest available icon size"));
	AddTooltipWithStatus(IDC_CB_QOI, CBX_QOI, _T("QOI - Quite OK Image Format\nExtensions: .qoi\nFast lossless compression"));
	AddTooltipWithStatus(IDC_CB_TGA, CBX_TGA, _T("TGA - Targa Image\nExtensions: .tga\nUsed by: Games, 3D applications"));
	AddTooltipWithStatus(IDC_CB_AUDIO, CBX_AUDIO, _T("Audio Cover Art\nExtensions: .mp3, .flac, .ogg, .wma, .aac\nShows: Embedded album art thumbnail"));
	AddTooltipWithStatus(IDC_CB_DOCUMENT, CBX_DOCUMENT, _T("Document Thumbnails\nExtensions: .docx, .xlsx, .pptx\nShows: First page preview via OLE/COM"));
	AddTooltipWithStatus(IDC_CB_FONT, CBX_FONT, _T("Font Preview Thumbnails\nExtensions: .ttf, .otf, .woff\nShows: Sample text rendering"));
	AddTooltipWithStatus(IDC_CB_MODEL, CBX_MODEL, _T("3D Model Thumbnails\nExtensions: .stl, .obj, .ply\nShows: Wireframe/solid preview"));
	
	// Options tooltips (no status needed)
	m_tooltip.AddTool(GetDlgItem(IDC_CB_SORT), _T("Sort images alphabetically by filename\nUncheck to use archive order (creation/modification time)"));
	m_tooltip.AddTool(GetDlgItem(IDC_CB_SHOWICON), _T("Show archive type icon overlay on thumbnails\nHelps identify CBZ vs CBR vs ZIP, etc."));
	
	// Button tooltips
	m_tooltip.AddTool(GetDlgItem(IDC_BTN_LOAD_CONFIG), _T("Load configuration from REG or JSON file\nDouble-click .reg files to import directly via Registry Editor"));
	m_tooltip.AddTool(GetDlgItem(IDOK), _T("Apply changes and close (Enter)"));
	m_tooltip.AddTool(GetDlgItem(IDC_APPLY), _T("Apply changes without closing (Ctrl+S)"));
	m_tooltip.AddTool(GetDlgItem(IDCANCEL), _T("Close without saving changes (Esc)"));
	
	// Collage mode tooltips  
	m_tooltip.AddTool(GetDlgItem(IDC_RADIO_1X1), _T("Single Page Mode - Show only the first page\nDefault for most formats"));
	m_tooltip.AddTool(GetDlgItem(IDC_RADIO_2X2), _T("2x2 Grid Mode - Show first 4 pages in grid\nGood for quick preview"));
	m_tooltip.AddTool(GetDlgItem(IDC_RADIO_3X3), _T("3x3 Grid Mode - Show first 9 pages in grid\nBalanced view"));
	m_tooltip.AddTool(GetDlgItem(IDC_RADIO_4X4), _T("4x4 Grid Mode - Show first 16 pages in grid\nBest for comic books and quick content overview"));
}

void CMainDlg::AddTooltipWithStatus(int ctrlID, int cbxType, LPCTSTR formatName)
{
	// Sprint 18A: Enhanced with program name detection
	CString programName;
	HandlerStatus status = m_reg.GetHandlerStatusEx(cbxType, m_reg.GetExtension(cbxType), programName);
	CString tooltip;
	
	switch (status)
	{
	case HANDLER_DARKTHUMBS:
		tooltip.Format(_T("%s\n\n\xE2\x9C\x85 Active Handler: DarkThumbs\nStatus: Enabled and working"), formatName);
		break;
		
	case HANDLER_NATIVE:
		tooltip.Format(_T("%s\n\n\xF0\x9F\x94\xB7 Current Handler: %s (Windows built-in)\nNote: Enable DarkThumbs to use enhanced features"), formatName, (LPCTSTR)programName);
		break;
		
	case HANDLER_NONE:
		tooltip.Format(_T("%s\n\n\xE2\xAD\x95 No Handler: No thumbnail provider installed\nAction: Enable DarkThumbs to generate thumbnails"), formatName);
		break;
		
	case HANDLER_THIRD_PARTY:
		tooltip.Format(_T("%s\n\n\xE2\x9A\xA0 Current Handler: %s (Third-party)\nAction: Click Apply to use DarkThumbs instead"), formatName, (LPCTSTR)programName);
		break;
		
	default:
		tooltip = formatName;
		break;
	}
	
	// Store status for visual indicators
	m_checkboxStatus[ctrlID] = status;
	
	HWND hCtrl = GetDlgItem(ctrlID);
	if (hCtrl)
	{
		m_tooltip.AddTool(hCtrl, (LPCTSTR)tooltip);
	}
}

//////////////////////////////////////////////////////////////////////////
// Sprint D3: Dark mode and modern UI support
//////////////////////////////////////////////////////////////////////////

void CMainDlg::InitDarkMode()
{
	// Detect system dark mode preference
	m_isDarkMode = DarkMode::IsSystemDarkMode();
	m_theme = m_isDarkMode ? DarkMode::GetDarkTheme() : DarkMode::GetLightTheme();
	
	// Initialize brushes
	m_hBrushBackground = CreateSolidBrush(m_theme.background);
	m_hBrushGroupBox = CreateSolidBrush(m_theme.groupBox);
	
	// Create bold font for group box headers (Sprint E)
	m_hGroupBoxFont = CreateFont(
		0, 0, 0, 0, FW_SEMIBOLD,  // Semi-bold weight for better readability
		FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		_T("Segoe UI")
	);
	
	// Enable dark mode for the window and title bar (Windows 10 1809+)
	if (m_isDarkMode)
	{
		DarkMode::SetAppDarkMode(true);
		DarkMode::EnableDarkModeForWindow(m_hWnd, true);
		DarkMode::SetDarkModeForTitleBar(m_hWnd, true);
	}
	
	ATLTRACE("Dark mode: %s\n", m_isDarkMode ? "enabled" : "disabled");
}

LRESULT CMainDlg::OnCtlColorDlg(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	// Sprint 8: Re-enabled dark mode dialog background
	if (!m_isDarkMode)
		return FALSE;  // Let system handle light theme
	
	HDC hdc = (HDC)wParam;
	SetBkColor(hdc, m_theme.background);
	return (LRESULT)m_hBrushBackground;
}

LRESULT CMainDlg::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	// Sprint 8: Re-enabled dark mode static/group box colors
	if (!m_isDarkMode)
		return FALSE;  // Let system handle light theme
	
	HDC hdc = (HDC)wParam;
	HWND hCtrl = (HWND)lParam;
	
	// Get control class name to differentiate group boxes from static text
	TCHAR className[256];
	GetClassName(hCtrl, className, 256);
	
	if (_tcscmp(className, _T("Button")) == 0)
	{
		// This is a group box (group boxes are Button class with BS_GROUPBOX style)
		LONG style = ::GetWindowLong(hCtrl, GWL_STYLE);
		if (style & BS_GROUPBOX)
		{
			SetTextColor(hdc, m_theme.text);
			SetBkMode(hdc, TRANSPARENT);
			
			// Apply bold font to group box header (Sprint E)
			if (m_hGroupBoxFont) {
				::SendMessage(hCtrl, WM_SETFONT, (WPARAM)m_hGroupBoxFont, TRUE);
			}
			
			return (LRESULT)m_hBrushBackground; // Use dialog background for group boxes
		}
	}
	
	// Regular static text
	SetTextColor(hdc, m_theme.text);
	SetBkMode(hdc, TRANSPARENT);
	return (LRESULT)m_hBrushBackground;
}

LRESULT CMainDlg::OnCtlColorBtn(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	// Sprint 8: Re-enabled dark mode button colors
	if (!m_isDarkMode)
		return FALSE;  // Let system handle light theme
	
	HDC hdc = (HDC)wParam;
	SetTextColor(hdc, m_theme.text);
	SetBkMode(hdc, TRANSPARENT);
	return (LRESULT)m_hBrushBackground;
}

void CMainDlg::UpdateStatusBar()
{
	if (!m_statusBar.m_hWnd)
		return;
	
	int enabledCount = GetEnabledFormatCount();
	
	// Sprint 18A: Count conflicts (third-party handlers)
	int conflictCount = 0;
	const int allFormats[] = {
		CBX_CBZ, CBX_CBR, CBX_CB7, CBX_CBT,
		CBX_EPUB, CBX_MOBI, CBX_AZW, CBX_AZW3,
		CBX_ZIP, CBX_RAR, CBX_7Z, CBX_TAR,
		CBX_PHZ, CBX_FB2,
		CBX_WEBP, CBX_HEIF, CBX_AVIF, CBX_JXL,
		CBX_VIDEO, CBX_PDF, CBX_TIFF, CBX_SVG, CBX_RAW
	};
	
	for (int format : allFormats)
	{
		HandlerStatus status = m_reg.GetHandlerStatus(format, m_reg.GetExtension(format));
		if (status == HANDLER_THIRD_PARTY)
			conflictCount++;
	}
	
	CString statusText;
	if (conflictCount > 0)
	{
		statusText.Format(_T("Ready - %d of 31+ formats enabled | \xE2\x9A\xA0 %d conflict(s) detected (hover for details)"), 
		                  enabledCount, conflictCount);
	}
	else
	{
		statusText.Format(_T("Ready - %d of 31+ formats enabled | Windows 11 25H2 Compatible"), enabledCount);
	}
	
	m_statusBar.SetText(0, statusText);
}

int CMainDlg::GetEnabledFormatCount()
{
	int count = 0;
	
	// Comic Book Formats (4)
	if (m_reg.HasTH(CBX_CBZ)) count++;
	if (m_reg.HasTH(CBX_CBR)) count++;
	if (m_reg.HasTH(CBX_CB7)) count++;
	if (m_reg.HasTH(CBX_CBT)) count++;
	
	// E-Book Formats (4)
	if (m_reg.HasTH(CBX_EPUB)) count++;
	if (m_reg.HasTH(CBX_MOBI)) count++;
	if (m_reg.HasTH(CBX_AZW)) count++;
	if (m_reg.HasTH(CBX_AZW3)) count++;
	
	// Archive Formats (4)
	if (m_reg.HasTH(CBX_ZIP)) count++;
	if (m_reg.HasTH(CBX_RAR)) count++;
	if (m_reg.HasTH(CBX_7Z)) count++;
	if (m_reg.HasTH(CBX_TAR)) count++;
	
	// Photo & Other Formats (2)
	if (m_reg.HasTH(CBX_PHZ)) count++;
	if (m_reg.HasTH(CBX_FB2)) count++;
	
	// Modern Image Formats (4)
	if (m_reg.HasTH(CBX_WEBP)) count++;
	if (m_reg.HasTH(CBX_HEIF)) count++;
	if (m_reg.HasTH(CBX_AVIF)) count++;
	if (m_reg.HasTH(CBX_JXL)) count++;
	
	// Media & Documents (5)
	if (m_reg.HasTH(CBX_VIDEO)) count++;
	if (m_reg.HasTH(CBX_PDF)) count++;
	if (m_reg.HasTH(CBX_TIFF)) count++;
	if (m_reg.HasTH(CBX_SVG)) count++;  
	if (m_reg.HasTH(CBX_RAW)) count++;
	
	// Sprint 8: Professional & Specialized Formats (12)
	if (m_reg.HasTH(CBX_PSD)) count++;
	if (m_reg.HasTH(CBX_DDS)) count++;
	if (m_reg.HasTH(CBX_HDR)) count++;
	if (m_reg.HasTH(CBX_EXR)) count++;
	if (m_reg.HasTH(CBX_PPM)) count++;
	if (m_reg.HasTH(CBX_ICO)) count++;
	if (m_reg.HasTH(CBX_QOI)) count++;
	if (m_reg.HasTH(CBX_TGA)) count++;
	if (m_reg.HasTH(CBX_AUDIO)) count++;
	if (m_reg.HasTH(CBX_DOCUMENT)) count++;
	if (m_reg.HasTH(CBX_FONT)) count++;
	if (m_reg.HasTH(CBX_MODEL)) count++;
	
	// Note: We have 35 format handler checkboxes (50+ file extensions supported)
	// Format handlers: 4 comic + 4 ebook + 4 archive + 2 photo + 4 image + 5 media/doc + 12 specialized = 35
	// Actual extensions: VIDEO covers 4+ formats, RAW covers DNG/CR2/NEF/etc,
	// HEIF covers .heif/.heic, AUDIO covers MP3/FLAC/etc, DOCUMENT covers DOCX/XLSX/PPTX/etc,
	// FONT covers TTF/OTF, MODEL covers STL/OBJ/PLY, bringing total to 50+
	
	return count;
}

void CMainDlg::InitStatusIcons()
{
	// Map each format checkbox to its handler status
	// This is used for owner-draw rendering with status icons
	
	// Comic Book Formats
	m_checkboxStatus[IDC_CB_CBZ] = m_reg.GetHandlerStatus(CBX_CBZ, m_reg.GetExtension(CBX_CBZ));
	m_checkboxStatus[IDC_CB_CBR] = m_reg.GetHandlerStatus(CBX_CBR, m_reg.GetExtension(CBX_CBR));
	m_checkboxStatus[IDC_CB_CB7] = m_reg.GetHandlerStatus(CBX_CB7, m_reg.GetExtension(CBX_CB7));
	m_checkboxStatus[IDC_CB_CBT] = m_reg.GetHandlerStatus(CBX_CBT, m_reg.GetExtension(CBX_CBT));
	
	// E-Book Formats
	m_checkboxStatus[IDC_CB_EPUB] = m_reg.GetHandlerStatus(CBX_EPUB, m_reg.GetExtension(CBX_EPUB));
	m_checkboxStatus[IDC_CB_MOBI] = m_reg.GetHandlerStatus(CBX_MOBI, m_reg.GetExtension(CBX_MOBI));
	m_checkboxStatus[IDC_CB_AZW] = m_reg.GetHandlerStatus(CBX_AZW, m_reg.GetExtension(CBX_AZW));
	m_checkboxStatus[IDC_CB_AZW3] = m_reg.GetHandlerStatus(CBX_AZW3, m_reg.GetExtension(CBX_AZW3));
	
	// Archive Formats
	m_checkboxStatus[IDC_CB_ZIP] = m_reg.GetHandlerStatus(CBX_ZIP, m_reg.GetExtension(CBX_ZIP));
	m_checkboxStatus[IDC_CB_RAR] = m_reg.GetHandlerStatus(CBX_RAR, m_reg.GetExtension(CBX_RAR));
	m_checkboxStatus[IDC_CB_7Z] = m_reg.GetHandlerStatus(CBX_7Z, m_reg.GetExtension(CBX_7Z));
	m_checkboxStatus[IDC_CB_TAR] = m_reg.GetHandlerStatus(CBX_TAR, m_reg.GetExtension(CBX_TAR));
	
	// Photo & Other Formats
	m_checkboxStatus[IDC_CB_PHZ] = m_reg.GetHandlerStatus(CBX_PHZ, m_reg.GetExtension(CBX_PHZ));
	m_checkboxStatus[IDC_CB_FB2] = m_reg.GetHandlerStatus(CBX_FB2, m_reg.GetExtension(CBX_FB2));
	
	// Modern Image Formats
	m_checkboxStatus[IDC_CB_WEBP] = m_reg.GetHandlerStatus(CBX_WEBP, m_reg.GetExtension(CBX_WEBP));
	m_checkboxStatus[IDC_CB_HEIF] = m_reg.GetHandlerStatus(CBX_HEIF, m_reg.GetExtension(CBX_HEIF));
	m_checkboxStatus[IDC_CB_AVIF] = m_reg.GetHandlerStatus(CBX_AVIF, m_reg.GetExtension(CBX_AVIF));
	m_checkboxStatus[IDC_CB_JXL] = m_reg.GetHandlerStatus(CBX_JXL, m_reg.GetExtension(CBX_JXL));
	
	// Media & Documents
	m_checkboxStatus[IDC_CB_VIDEO] = m_reg.GetHandlerStatus(CBX_VIDEO, m_reg.GetExtension(CBX_VIDEO));
	m_checkboxStatus[IDC_CB_PDF] = m_reg.GetHandlerStatus(CBX_PDF, m_reg.GetExtension(CBX_PDF));
	m_checkboxStatus[IDC_CB_TIFF] = m_reg.GetHandlerStatus(CBX_TIFF, m_reg.GetExtension(CBX_TIFF));
	m_checkboxStatus[IDC_CB_SVG] = m_reg.GetHandlerStatus(CBX_SVG, m_reg.GetExtension(CBX_SVG));
	m_checkboxStatus[IDC_CB_RAW] = m_reg.GetHandlerStatus(CBX_RAW, m_reg.GetExtension(CBX_RAW));
	
	// Sprint 8: Professional & Specialized Formats
	m_checkboxStatus[IDC_CB_PSD] = m_reg.GetHandlerStatus(CBX_PSD, m_reg.GetExtension(CBX_PSD));
	m_checkboxStatus[IDC_CB_DDS] = m_reg.GetHandlerStatus(CBX_DDS, m_reg.GetExtension(CBX_DDS));
	m_checkboxStatus[IDC_CB_HDR] = m_reg.GetHandlerStatus(CBX_HDR, m_reg.GetExtension(CBX_HDR));
	m_checkboxStatus[IDC_CB_EXR] = m_reg.GetHandlerStatus(CBX_EXR, m_reg.GetExtension(CBX_EXR));
	m_checkboxStatus[IDC_CB_PPM] = m_reg.GetHandlerStatus(CBX_PPM, m_reg.GetExtension(CBX_PPM));
	m_checkboxStatus[IDC_CB_ICO] = m_reg.GetHandlerStatus(CBX_ICO, m_reg.GetExtension(CBX_ICO));
	m_checkboxStatus[IDC_CB_QOI] = m_reg.GetHandlerStatus(CBX_QOI, m_reg.GetExtension(CBX_QOI));
	m_checkboxStatus[IDC_CB_TGA] = m_reg.GetHandlerStatus(CBX_TGA, m_reg.GetExtension(CBX_TGA));
	m_checkboxStatus[IDC_CB_AUDIO] = m_reg.GetHandlerStatus(CBX_AUDIO, m_reg.GetExtension(CBX_AUDIO));
	m_checkboxStatus[IDC_CB_DOCUMENT] = m_reg.GetHandlerStatus(CBX_DOCUMENT, m_reg.GetExtension(CBX_DOCUMENT));
	m_checkboxStatus[IDC_CB_FONT] = m_reg.GetHandlerStatus(CBX_FONT, m_reg.GetExtension(CBX_FONT));
	m_checkboxStatus[IDC_CB_MODEL] = m_reg.GetHandlerStatus(CBX_MODEL, m_reg.GetExtension(CBX_MODEL));
}

LRESULT CMainDlg::OnDrawItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;
	
	// Only handle checkbox owner-draw
	if (lpDIS->CtlType != ODT_BUTTON)
		return FALSE;
	
	// Check if this is a format checkbox with status
	auto it = m_checkboxStatus.find(lpDIS->CtlID);
	if (it == m_checkboxStatus.end())
		return FALSE; // Not a format checkbox, use default drawing
	
	HandlerStatus status = it->second;
	HDC hdc = lpDIS->hDC;
	RECT rcItem = lpDIS->rcItem;
	
	// Get checkbox state
	BOOL isChecked = (Button_GetCheck(lpDIS->hwndItem) == BST_CHECKED);
	BOOL isDisabled = ((lpDIS->itemState & ODS_DISABLED) != 0);
	BOOL isFocused = ((lpDIS->itemState & ODS_FOCUS) != 0);
	
	// Fill background
	HBRUSH hBrush = CreateSolidBrush(m_theme.background);
	FillRect(hdc, &rcItem, hBrush);
	DeleteObject(hBrush);
	
	// Draw checkbox box
	int checkBoxSize = 13;
	int checkBoxY = rcItem.top + (rcItem.bottom - rcItem.top - checkBoxSize) / 2;
	RECT rcCheckBox = { rcItem.left + 2, checkBoxY, rcItem.left + 2 + checkBoxSize, checkBoxY + checkBoxSize };
	
	// Draw checkbox border and background
	HBRUSH hCheckBrush = CreateSolidBrush(m_isDarkMode ? RGB(55, 55, 55) : RGB(255, 255, 255));
	HPEN hCheckPen = CreatePen(PS_SOLID, 1, m_theme.border);
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hCheckBrush);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hCheckPen);
	
	Rectangle(hdc, rcCheckBox.left, rcCheckBox.top, rcCheckBox.right, rcCheckBox.bottom);
	
	SelectObject(hdc, hOldPen);
	SelectObject(hdc, hOldBrush);
	DeleteObject(hCheckPen);
	DeleteObject(hCheckBrush);
	
	// Draw checkmark if checked
	if (isChecked)
	{
		HPEN hCheckmarkPen = CreatePen(PS_SOLID, 2, m_isDarkMode ? RGB(100, 180, 255) : RGB(0, 120, 215));
		SelectObject(hdc, hCheckmarkPen);
		
		// Draw checkmark
		int cx = rcCheckBox.left + 3;
		int cy = rcCheckBox.top + 6;
		MoveToEx(hdc, cx, cy, NULL);
		LineTo(hdc, cx + 3, cy + 3);
		LineTo(hdc, cx + 8, cy - 2);
		
		DeleteObject(hCheckmarkPen);
	}
	
	// Draw status icon (small circle with color)
	int iconX = rcCheckBox.right + 6;
	int iconY = checkBoxY + checkBoxSize / 2;
	DrawStatusIcon(hdc, iconX, iconY, status);
	
	// Draw text
	TCHAR text[256];
	::GetWindowText(lpDIS->hwndItem, text, 256);
	
	RECT rcText = rcItem;
	rcText.left = iconX + 12; // After status icon
	
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, isDisabled ? m_theme.disabledText : m_theme.text);
	
	HFONT hFont = (HFONT)SendMessage(lpDIS->hwndItem, WM_GETFONT, 0, 0);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
	
	DrawText(hdc, text, -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	
	SelectObject(hdc, hOldFont);
	
	// Draw focus rectangle
	if (isFocused)
	{
		RECT rcFocus = rcItem;
		rcFocus.left += 1;
		rcFocus.right -= 1;
		rcFocus.top += 1;
		rcFocus.bottom -= 1;
		DrawFocusRect(hdc, &rcFocus);
	}
	
	return TRUE;
}

void CMainDlg::DrawStatusIcon(HDC hdc, int x, int y, HandlerStatus status)
{
	COLORREF color = GetStatusColor(status);
	int radius = 3;
	
	// Draw filled circle
	HBRUSH hBrush = CreateSolidBrush(color);
	HPEN hPen = CreatePen(PS_SOLID, 1, color);
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	
	Ellipse(hdc, x - radius, y - radius, x + radius, y + radius);
	
	SelectObject(hdc, hOldPen);
	SelectObject(hdc, hOldBrush);
	DeleteObject(hPen);
	DeleteObject(hBrush);
	
	// Note: Status icon symbols not rendered to avoid font compatibility issues
	// The colored dot itself is sufficient for visual indication
	// Full status details are available in tooltips
}

COLORREF CMainDlg::GetStatusColor(HandlerStatus status)
{
	switch (status)
	{
	case HANDLER_DARKTHUMBS:
		return RGB(76, 175, 80);  // Material Green (DarkThumbs active)
		
	case HANDLER_NATIVE:
		return RGB(33, 150, 243);  // Material Blue (Windows native handler)
		
	case HANDLER_NONE:
		return RGB(158, 158, 158); // Gray (No handler)
		
	case HANDLER_THIRD_PARTY:
		return RGB(255, 152, 0);   // Material Orange (Third-party handler)
		
	default:
		return RGB(128, 128, 128); // Default gray
	}
}

// Theme toggle handler
LRESULT CMainDlg::OnThemeToggle(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Toggle dark mode state
	m_isDarkMode = !m_isDarkMode;
	
	// Update theme colors
	m_theme = m_isDarkMode ? DarkMode::GetDarkTheme() : DarkMode::GetLightTheme();
	
	// Recreate brushes and fonts
	if (m_hBrushBackground) DeleteObject(m_hBrushBackground);
	if (m_hBrushGroupBox) DeleteObject(m_hBrushGroupBox);
	if (m_hGroupBoxFont) DeleteObject(m_hGroupBoxFont);
	
	m_hBrushBackground = CreateSolidBrush(m_theme.background);
	m_hBrushGroupBox = CreateSolidBrush(m_theme.groupBox);
	m_hGroupBoxFont = CreateFont(
		0, 0, 0, 0, FW_SEMIBOLD,
		FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		_T("Segoe UI")
	);
	
	// Apply theme to window
	ApplyTheme();
	
	// Update button text
	UpdateThemeButton();
	
	// Force full repaint
	Invalidate(TRUE);
	UpdateWindow();
	
	// Repaint all child windows
	EnumChildWindows(m_hWnd, [](HWND hwnd, LPARAM) -> BOOL {
		::InvalidateRect(hwnd, NULL, TRUE);
		return TRUE;
	}, 0);
	
	return 0;
}

// Apply theme to dialog and controls
void CMainDlg::ApplyTheme()
{
	// Set title bar dark mode
	DarkMode::SetDarkModeForTitleBar(m_hWnd, m_isDarkMode);
	DarkMode::EnableDarkModeForWindow(m_hWnd, m_isDarkMode);
	
	// Update status bar
	if (m_statusBar.m_hWnd)
	{
		m_statusBar.SetBkColor(m_theme.background);
		m_statusBar.Invalidate();
	}
}

// Update theme button text
void CMainDlg::UpdateThemeButton()
{
	HWND hThemeBtn = GetDlgItem(IDC_BTN_THEME);
	if (hThemeBtn)
	{
		if (m_isDarkMode)
			::SetWindowText(hThemeBtn, _T("Light Light"));
		else
			::SetWindowText(hThemeBtn, _T("🌙 Dark"));
	}
}

// Handle checkbox clicks for owner-draw checkboxes
LRESULT CMainDlg::OnCheckboxClicked(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
	// BS_AUTO3STATE handles toggling automatically, no manual intervention needed
	// Just let Windows handle the state changes
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Keyboard Shortcuts - Sprint E Integration
//////////////////////////////////////////////////////////////////////////

LRESULT CMainDlg::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	
	if (ctrlPressed) {
		switch (wParam) {
			case 'A':  // Ctrl+A: Select all format checkboxes
				OnSelectAll();
				return 0;
			case 'D':  // Ctrl+D: Deselect all format checkboxes
				OnDeselectAll();
				return 0;
			case 'S':  // Ctrl+S: Apply changes
			{
				BOOL bHandled = FALSE;
				OnApply(0, IDC_APPLY, NULL, bHandled);
				return 0;
			}
		}
	}
	else {
		switch (wParam) {
			case VK_F1:  // F1: Show help
				// Display comprehensive help dialog
				MessageBox(
					_T("DarkThumbs Shell Manager - Quick Help\n\n")
					_T("KEYBOARD SHORTCUTS:\n")
					_T("  Ctrl+A     - Select all formats\n")
					_T("  Ctrl+D     - Deselect all formats\n")
					_T("  Ctrl+S     - Apply changes\n")
					_T("  F1         - Show this help\n")
					_T("  F5         - Refresh status\n\n")
					_T("FEATURES:\n")
					_T("  \x2022 50+ supported formats\n")
					_T("  \x2022 Modern images: WebP, AVIF, JXL, HEIF\n")
					_T("  \x2022 Conflict detection\n")
					_T("  \x2022 Windows 11 25H2 compatible\n\n")
					_T("TIPS:\n")
					_T("  \x2022 Hover over icons for details\n")
					_T("  \x2022 Check status bar for count\n")
					_T("\nFor full docs, press F1 on any control."),
					_T("DarkThumbs Help"), 
					MB_OK | MB_ICONINFORMATION);
				 return 0;
		}
	}
	
	return 0;
}

void CMainDlg::OnSelectAll()
{
	// Select all 35 format checkboxes (Sprint 8: added 12 specialized formats)
	int formatCheckboxes[] = {
		IDC_CB_CBZ, IDC_CB_CBR, IDC_CB_CB7, IDC_CB_CBT,
		IDC_CB_EPUB, IDC_CB_MOBI, IDC_CB_AZW, IDC_CB_AZW3,
		IDC_CB_ZIP, IDC_CB_RAR, IDC_CB_7Z, IDC_CB_TAR,
		IDC_CB_PHZ, IDC_CB_FB2,
		IDC_CB_WEBP, IDC_CB_HEIF, IDC_CB_AVIF, IDC_CB_JXL,
		IDC_CB_VIDEO, IDC_CB_PDF, IDC_CB_TIFF, IDC_CB_SVG, IDC_CB_RAW,
		IDC_CB_PSD, IDC_CB_DDS, IDC_CB_HDR, IDC_CB_EXR,
		IDC_CB_PPM, IDC_CB_ICO, IDC_CB_QOI, IDC_CB_TGA,
		IDC_CB_AUDIO, IDC_CB_DOCUMENT, IDC_CB_FONT, IDC_CB_MODEL
	};
	
	for (int id : formatCheckboxes) {
		Button_SetCheck(GetDlgItem(id), BST_CHECKED);
	}
}

void CMainDlg::OnDeselectAll()
{
	// Deselect all 35 format checkboxes (Sprint 8: added 12 specialized formats)
	int formatCheckboxes[] = {
		IDC_CB_CBZ, IDC_CB_CBR, IDC_CB_CB7, IDC_CB_CBT,
		IDC_CB_EPUB, IDC_CB_MOBI, IDC_CB_AZW, IDC_CB_AZW3,
		IDC_CB_ZIP, IDC_CB_RAR, IDC_CB_7Z, IDC_CB_TAR,
		IDC_CB_PHZ, IDC_CB_FB2,
		IDC_CB_WEBP, IDC_CB_HEIF, IDC_CB_AVIF, IDC_CB_JXL,
		IDC_CB_VIDEO, IDC_CB_PDF, IDC_CB_TIFF, IDC_CB_SVG, IDC_CB_RAW,
		IDC_CB_PSD, IDC_CB_DDS, IDC_CB_HDR, IDC_CB_EXR,
		IDC_CB_PPM, IDC_CB_ICO, IDC_CB_QOI, IDC_CB_TGA,
		IDC_CB_AUDIO, IDC_CB_DOCUMENT, IDC_CB_FONT, IDC_CB_MODEL
	};
	
	for (int id : formatCheckboxes) {
		Button_SetCheck(GetDlgItem(id), BST_UNCHECKED);
	}
}

// Configuration Management Functions

ConfigSnapshot CMainDlg::CaptureCurrentConfig()
{
	ConfigSnapshot config = {0};
	
	// Capture format states
	config.cbz = (m_reg.HasTH(CBX_CBZ) != FALSE);
	config.cbr = (m_reg.HasTH(CBX_CBR) != FALSE);
	config.cb7 = (m_reg.HasTH(CBX_CB7) != FALSE);
	config.cbt = (m_reg.HasTH(CBX_CBT) != FALSE);
	config.epub = (m_reg.HasTH(CBX_EPUB) != FALSE);
	config.mobi = (m_reg.HasTH(CBX_MOBI) != FALSE);
	config.azw = (m_reg.HasTH(CBX_AZW) != FALSE);
	config.azw3 = (m_reg.HasTH(CBX_AZW3) != FALSE);
	config.zip = (m_reg.HasTH(CBX_ZIP) != FALSE);
	config.rar = (m_reg.HasTH(CBX_RAR) != FALSE);
	config.z7 = (m_reg.HasTH(CBX_7Z) != FALSE);
	config.tar = (m_reg.HasTH(CBX_TAR) != FALSE);
	config.phz = (m_reg.HasTH(CBX_PHZ) != FALSE);
	config.fb2 = (m_reg.HasTH(CBX_FB2) != FALSE);
	config.webp = (m_reg.HasTH(CBX_WEBP) != FALSE);
	config.heif = (m_reg.HasTH(CBX_HEIF) != FALSE);
	config.avif = (m_reg.HasTH(CBX_AVIF) != FALSE);
	config.jxl = (m_reg.HasTH(CBX_JXL) != FALSE);
	config.video = (m_reg.HasTH(CBX_VIDEO) != FALSE);
	config.pdf = (m_reg.HasTH(CBX_PDF) != FALSE);
	config.tiff = (m_reg.HasTH(CBX_TIFF) != FALSE);
	config.svg = (m_reg.HasTH(CBX_SVG) != FALSE);
	config.raw = (m_reg.HasTH(CBX_RAW) != FALSE);
	// Sprint 8: Professional & Specialized Formats
	config.psd = (m_reg.HasTH(CBX_PSD) != FALSE);
	config.dds = (m_reg.HasTH(CBX_DDS) != FALSE);
	config.hdr = (m_reg.HasTH(CBX_HDR) != FALSE);
	config.exr = (m_reg.HasTH(CBX_EXR) != FALSE);
	config.ppm = (m_reg.HasTH(CBX_PPM) != FALSE);
	config.ico = (m_reg.HasTH(CBX_ICO) != FALSE);
	config.qoi = (m_reg.HasTH(CBX_QOI) != FALSE);
	config.tga = (m_reg.HasTH(CBX_TGA) != FALSE);
	config.audio = (m_reg.HasTH(CBX_AUDIO) != FALSE);
	config.document = (m_reg.HasTH(CBX_DOCUMENT) != FALSE);
	config.font = (m_reg.HasTH(CBX_FONT) != FALSE);
	config.model = (m_reg.HasTH(CBX_MODEL) != FALSE);
	
	// Capture options
	config.sortOpt = (m_reg.IsSortOpt() != FALSE);
	config.showIconOpt = (m_reg.IsShowIconOpt() != FALSE);
	config.collageMode = m_reg.GetCollageMode();
	
	return config;
}

void CMainDlg::ApplyConfigSnapshot(const ConfigSnapshot& config)
{
	// Apply format handlers
	m_reg.SetHandlers(CBX_CBZ, config.cbz);
	m_reg.SetHandlers(CBX_CBR, config.cbr);
	m_reg.SetHandlers(CBX_CB7, config.cb7);
	m_reg.SetHandlers(CBX_CBT, config.cbt);
	m_reg.SetHandlers(CBX_EPUB, config.epub);
	m_reg.SetHandlers(CBX_MOBI, config.mobi);
	m_reg.SetHandlers(CBX_AZW, config.azw);
	m_reg.SetHandlers(CBX_AZW3, config.azw3);
	m_reg.SetHandlers(CBX_ZIP, config.zip);
	m_reg.SetHandlers(CBX_RAR, config.rar);
	m_reg.SetHandlers(CBX_7Z, config.z7);
	m_reg.SetHandlers(CBX_TAR, config.tar);
	m_reg.SetHandlers(CBX_PHZ, config.phz);
	m_reg.SetHandlers(CBX_FB2, config.fb2);
	m_reg.SetHandlers(CBX_WEBP, config.webp);
	m_reg.SetHandlers(CBX_HEIF, config.heif);
	m_reg.SetHandlers(CBX_AVIF, config.avif);
	m_reg.SetHandlers(CBX_JXL, config.jxl);
	m_reg.SetHandlers(CBX_VIDEO, config.video);
	m_reg.SetHandlers(CBX_PDF, config.pdf);
	m_reg.SetHandlers(CBX_TIFF, config.tiff);
	m_reg.SetHandlers(CBX_SVG, config.svg);
	m_reg.SetHandlers(CBX_RAW, config.raw);
	// Sprint 8: Professional & Specialized Formats
	m_reg.SetHandlers(CBX_PSD, config.psd);
	m_reg.SetHandlers(CBX_DDS, config.dds);
	m_reg.SetHandlers(CBX_HDR, config.hdr);
	m_reg.SetHandlers(CBX_EXR, config.exr);
	m_reg.SetHandlers(CBX_PPM, config.ppm);
	m_reg.SetHandlers(CBX_ICO, config.ico);
	m_reg.SetHandlers(CBX_QOI, config.qoi);
	m_reg.SetHandlers(CBX_TGA, config.tga);
	m_reg.SetHandlers(CBX_AUDIO, config.audio);
	m_reg.SetHandlers(CBX_DOCUMENT, config.document);
	m_reg.SetHandlers(CBX_FONT, config.font);
	m_reg.SetHandlers(CBX_MODEL, config.model);
	
	// Apply options
	m_reg.SetSortOpt(config.sortOpt);
	m_reg.SetShowIconOpt(config.showIconOpt);
	m_reg.SetCollageMode(config.collageMode);
	
	// Refresh Explorer
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT | SHCNF_NOTIFYRECURSIVE, NULL, NULL);
	
	// Reload UI
	InitUI();
	UpdateStatusBar();
}

bool CMainDlg::LoadConfigFromFile(LPCTSTR filename, ConfigSnapshot& outConfig)
{
	CAtlFile file;
	HRESULT hr = file.Create(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
	if (FAILED(hr)) return false;
	
	ULONGLONG fileSize = 0;
	hr = file.GetSize(fileSize);
	if (FAILED(hr) || fileSize > 1024*1024) { // Max 1MB
		file.Close();
		return false;
	}
	
	CStringA jsonData;
	char* buffer = jsonData.GetBuffer((int)fileSize + 1);
	DWORD bytesRead = 0;
	hr = file.Read(buffer, (DWORD)fileSize, bytesRead);
	jsonData.ReleaseBuffer(bytesRead);
	file.Close();
	
	if (FAILED(hr)) return false;
	
	// Simple JSON parsing (basic approach - looks for true/false after field names)
	auto getBool = [&jsonData](const char* key) -> bool {
		CStringA searchKey;
		searchKey.Format("\"%s\"", key);
		int pos = jsonData.Find(searchKey);
		if (pos == -1) return false;
		int valuePos = jsonData.Find(":", pos);
		if (valuePos == -1) return false;
		int truePos = jsonData.Find("true", valuePos);
		int falsePos = jsonData.Find("false", valuePos);
		int commaPos = jsonData.Find(",", valuePos);
		int bracePos = jsonData.Find("}", valuePos);
		int endPos = (commaPos != -1 && commaPos < bracePos) ? commaPos : bracePos;
		
		if (truePos != -1 && truePos < endPos) return true;
		if (falsePos != -1 && falsePos < endPos) return false;
		return false;
	};
	
	auto getInt = [&jsonData](const char* key) -> int {
		CStringA searchKey;
		searchKey.Format("\"%s\"", key);
		int pos = jsonData.Find(searchKey);
		if (pos == -1) return 0;
		int valuePos = jsonData.Find(":", pos);
		if (valuePos == -1) return 0;
		
		// Skip whitespace
		while (valuePos < jsonData.GetLength() && (jsonData[valuePos] == ':' || jsonData[valuePos] == ' ' || jsonData[valuePos] == '\t'))
			valuePos++;
		
		// Parse integer
		int value = 0;
		while (valuePos < jsonData.GetLength() && jsonData[valuePos] >= '0' && jsonData[valuePos] <= '9') {
			value = value * 10 + (jsonData[valuePos] - '0');
			valuePos++;
		}
		return value;
	};
	
	// Parse formats
	outConfig.cbz = getBool("cbz");
	outConfig.cbr = getBool("cbr");
	outConfig.cb7 = getBool("cb7");
	outConfig.cbt = getBool("cbt");
	outConfig.epub = getBool("epub");
	outConfig.mobi = getBool("mobi");
	outConfig.azw = getBool("azw");
	outConfig.azw3 = getBool("azw3");
	outConfig.zip = getBool("zip");
	outConfig.rar = getBool("rar");
	outConfig.z7 = getBool("7z");
	outConfig.tar = getBool("tar");
	outConfig.phz = getBool("phz");
	outConfig.fb2 = getBool("fb2");
	outConfig.webp = getBool("webp");
	outConfig.heif = getBool("heif");
	outConfig.avif = getBool("avif");
	outConfig.jxl = getBool("jxl");
	outConfig.video = getBool("video");
	outConfig.pdf = getBool("pdf");
	outConfig.tiff = getBool("tiff");
	outConfig.svg = getBool("svg");
	outConfig.raw = getBool("raw");
	
	// Sprint 8: Professional & Specialized Formats
	outConfig.psd = getBool("psd");
	outConfig.dds = getBool("dds");
	outConfig.hdr = getBool("hdr");
	outConfig.exr = getBool("exr");
	outConfig.ppm = getBool("ppm");
	outConfig.ico = getBool("ico");
	outConfig.qoi = getBool("qoi");
	outConfig.tga = getBool("tga");
	outConfig.audio = getBool("audio");
	outConfig.document = getBool("document");
	outConfig.font = getBool("font");
	outConfig.model = getBool("model");
	
	// Parse options
	outConfig.sortOpt = getBool("sortPages");
	outConfig.showIconOpt = getBool("showIconOverlay");
	outConfig.collageMode = getInt("collageMode");
	
	// Validate collage mode
	if (outConfig.collageMode != 1 && outConfig.collageMode != 4 && 
		outConfig.collageMode != 9 && outConfig.collageMode != 16) {
		outConfig.collageMode = 1; // Default to single page
	}
	
	return true;
}

bool CMainDlg::LoadConfigFromRegFile(LPCTSTR filename, ConfigSnapshot& outConfig)
{
	CAtlFile file;
	HRESULT hr = file.Create(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
	if (FAILED(hr)) return false;
	
	ULONGLONG fileSize = 0;
	hr = file.GetSize(fileSize);
	if (FAILED(hr) || fileSize > 5*1024*1024) { // Max 5MB for .reg files
		file.Close();
		return false;
	}
	
	CStringA regData;
	char* buffer = regData.GetBuffer((int)fileSize + 1);
	DWORD bytesRead = 0;
	hr = file.Read(buffer, (DWORD)fileSize, bytesRead);
	regData.ReleaseBuffer(bytesRead);
	file.Close();
	
	if (FAILED(hr)) return false;
	
	// Simple .reg parser - looks for handler GUID presence/absence
	auto isFormatEnabled = [&regData](const char* ext) -> bool {
		CStringA searchKey;
		searchKey.Format("SOFTWARE\\Classes\\.%s\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", ext);
		int pos = regData.Find(searchKey);
		if (pos == -1) return false;
		
		// Check if the next line has @="{GUID}" (enabled) or @=- (disabled)
		int nextLine = regData.Find("\r\n", pos);
		if (nextLine == -1) return false;
		
		int guidPos = regData.Find("{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}", nextLine);
		int deletePos = regData.Find("@=-", nextLine);
		
		// If GUID found before next section or delete marker
		if (guidPos != -1 && (deletePos == -1 || guidPos < deletePos)) {
			return true;
		}
		return false;
	};
	
	auto getDwordValue = [&regData](const char* valueName) -> DWORD {
		CStringA searchKey;
		searchKey.Format("\"%s\"=dword:", valueName);
		int pos = regData.Find(searchKey);
		if (pos == -1) return 0;
		
		pos += searchKey.GetLength();
		DWORD value = 0;
		sscanf_s(regData.Mid(pos, 8).GetString(), "%x", &value);
		return value;
	};
	
	// Parse format handlers
	outConfig.cbz = isFormatEnabled("cbz");
	outConfig.cbr = isFormatEnabled("cbr");
	outConfig.cb7 = isFormatEnabled("cb7");
	outConfig.cbt = isFormatEnabled("cbt");
	outConfig.epub = isFormatEnabled("epub");
	outConfig.mobi = isFormatEnabled("mobi");
	outConfig.azw = isFormatEnabled("azw");
	outConfig.azw3 = isFormatEnabled("azw3");
	outConfig.zip = isFormatEnabled("zip");
	outConfig.rar = isFormatEnabled("rar");
	outConfig.z7 = isFormatEnabled("7z");
	outConfig.tar = isFormatEnabled("tar");
	outConfig.phz = isFormatEnabled("phz");
	outConfig.fb2 = isFormatEnabled("fb2");
	outConfig.webp = isFormatEnabled("webp");
	outConfig.heif = isFormatEnabled("heif") || isFormatEnabled("heic");
	outConfig.avif = isFormatEnabled("avif");
	outConfig.jxl = isFormatEnabled("jxl");
	outConfig.video = isFormatEnabled("mp4"); // Use mp4 as proxy for all video
	outConfig.pdf = isFormatEnabled("pdf");
	outConfig.tiff = isFormatEnabled("tif") || isFormatEnabled("tiff");
	outConfig.svg = isFormatEnabled("svg");
	outConfig.raw = isFormatEnabled("dng"); // Use dng as proxy for all RAW
	
	// Sprint 8: Professional & Specialized Formats
	outConfig.psd = isFormatEnabled("psd");
	outConfig.dds = isFormatEnabled("dds");
	outConfig.hdr = isFormatEnabled("hdr");
	outConfig.exr = isFormatEnabled("exr");
	outConfig.ppm = isFormatEnabled("ppm");
	outConfig.ico = isFormatEnabled("ico");
	outConfig.qoi = isFormatEnabled("qoi");
	outConfig.tga = isFormatEnabled("tga");
	outConfig.audio = isFormatEnabled("mp3"); // Use mp3 as proxy for all audio
	outConfig.document = isFormatEnabled("docx"); // Use docx as proxy for all documents
	outConfig.font = isFormatEnabled("ttf"); // Use ttf as proxy for all fonts
	outConfig.model = isFormatEnabled("stl"); // Use stl as proxy for all 3D models
	
	// Parse options
	outConfig.sortOpt = (getDwordValue("SortOpt") != 0);
	outConfig.showIconOpt = (getDwordValue("ShowIconOpt") != 0);
	outConfig.collageMode = getDwordValue("CollageMode");
	
	// Validate collage mode
	if (outConfig.collageMode != 1 && outConfig.collageMode != 4 && 
		outConfig.collageMode != 9 && outConfig.collageMode != 16) {
		outConfig.collageMode = 1;
	}
	
	return true;
}

LRESULT CMainDlg::OnLoadConfig(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFileDialog dlg(TRUE, _T("reg"), NULL,
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
		_T("Windows Registry File (*.reg)\0*.reg\0JSON Configuration (*.json)\0*.json\0All Files (*.*)\0*.*\0"));
	
	if (dlg.DoModal() == IDOK)
	{
		CString filename(dlg.m_szFileName);
		bool isRegFile = (filename.Right(4).CompareNoCase(_T(".reg")) == 0);
		
		if (isRegFile)
		{
			// For .reg files, offer to import directly or use shell execute
			CString msg;
			msg.Format(_T("Registry file selected:\r\n%s\r\n\r\n")
				_T("How do you want to import this configuration?\r\n\r\n")
				_T("YES = Import via Registry Editor (recommended)\r\n")
				_T("NO = Load and apply through DarkThumbs\r\n")
				_T("CANCEL = Cancel operation"),
				dlg.m_szFileName);
			
			int result = MessageBox(msg, _T("Import Registry File"), MB_YESNOCANCEL | MB_ICONQUESTION);
			
			if (result == IDYES)
			{
				// Use regedit to import the file
				CString cmdLine;
				cmdLine.Format(_T("regedit.exe /s \"%s\""), dlg.m_szFileName);
				
				STARTUPINFO si = { sizeof(STARTUPINFO) };
				PROCESS_INFORMATION pi = { 0 };
				
				if (CreateProcess(NULL, cmdLine.GetBuffer(), NULL, NULL, FALSE, 
					CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
				{
					// Wait for regedit to complete
					WaitForSingleObject(pi.hProcess, 10000); // 10 second timeout
					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);
					
					// Refresh Explorer
					SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT | SHCNF_NOTIFYRECURSIVE, NULL, NULL);
					
					// Reload UI to reflect registry changes
					InitUI();
					UpdateStatusBar();
					
					MessageBox(_T("Registry configuration imported successfully!\r\n\r\n")
						_T("Windows Explorer has been refreshed.\r\n")
						_T("The UI has been updated to reflect the new settings."),
						_T("Import Complete"), MB_OK | MB_ICONINFORMATION);
				}
				else
				{
					MessageBox(_T("Failed to launch Registry Editor.\r\n\r\n")
						_T("You can manually double-click the .reg file to import it."),
						_T("Import Error"), MB_OK | MB_ICONERROR);
				}
			}
			else if (result == IDNO)
			{
				// Parse .reg file and apply through our code
				ConfigSnapshot loadedConfig;
				if (LoadConfigFromRegFile(dlg.m_szFileName, loadedConfig))
				{
					ApplyConfigSnapshot(loadedConfig);
					MessageBox(_T("Configuration applied successfully!\r\n\r\n")
						_T("Windows Explorer has been refreshed."),
						_T("Configuration Applied"), MB_OK | MB_ICONINFORMATION);
				}
				else
				{
					MessageBox(_T("Failed to parse registry file."),
						_T("Load Error"), MB_OK | MB_ICONERROR);
				}
			}
		}
		else
		{
			// JSON file handling (existing code)
			ConfigSnapshot loadedConfig;
			if (LoadConfigFromFile(dlg.m_szFileName, loadedConfig))
			{
				CString msg;
				msg.Format(_T("Configuration loaded from:\r\n%s\r\n\r\n")
					_T("Do you want to apply this configuration?\r\n\r\n")
					_T("This will change your current thumbnail handler settings."),
					dlg.m_szFileName);
				
				if (MessageBox(msg, _T("Apply Configuration?"), MB_YESNO | MB_ICONQUESTION) == IDYES)
				{
					ApplyConfigSnapshot(loadedConfig);
					MessageBox(_T("Configuration applied successfully!\r\n\r\n")
						_T("Windows Explorer has been refreshed."),
						_T("Configuration Applied"), MB_OK | MB_ICONINFORMATION);
				}
			}
			else
			{
				MessageBox(_T("Failed to load configuration file.\r\n\r\n")
					_T("The file may be corrupted or in an unsupported format."),
					_T("Load Error"), MB_OK | MB_ICONERROR);
			}
		}
	}
	
	return 0;
}

