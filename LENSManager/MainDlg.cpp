// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "MainDlg.h"
#include "about.h"
#include <algorithm>
#include <map>
#include <vector>

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg) {
  // Forward mouse messages to tooltip control
  if (m_tooltip.m_hWnd) {
    m_tooltip.RelayEvent(pMsg);
  }
  return CWindow::IsDialogMessage(pMsg);
}
BOOL CMainDlg::OnIdle() { return FALSE; }

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/,
  LPARAM /*lParam*/, BOOL& /*bHandled*/) {
  // center the dialog on the screen
  CenterWindow();

  // set icons
  HICON hIcon = (HICON)::LoadImage(
    _Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON,
    ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON),
    LR_DEFAULTCOLOR);
  SetIcon(hIcon, TRUE);
  HICON hIconSmall = (HICON)::LoadImage(
    _Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON,
    ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON),
    LR_DEFAULTCOLOR);
  SetIcon(hIconSmall, FALSE);

  // register object for message filtering and idle updates
  CMessageLoop* pLoop = _Module.GetMessageLoop();
  ATLASSERT(pLoop != NULL);
  pLoop->AddMessageFilter(this);
  pLoop->AddIdleHandler(this);

  UIAddChildWindowContainer(m_hWnd);

  // add 'About' item to sys menu
  ATLASSERT(IDC_APPABOUT < 0xF000);
  CMenu pSysMenu = GetSystemMenu(FALSE);
  if (!pSysMenu.IsNull()) {
    pSysMenu.AppendMenu(MF_SEPARATOR);
    pSysMenu.AppendMenu(MF_STRING, IDC_APPABOUT, _T("About"));
  }

  // Initialize status bar
  HWND hStatusBar = GetDlgItem(IDC_STATUSBAR);
  if (hStatusBar) {
    m_statusBar.Attach(hStatusBar);
    UpdateStatusBar();
  }

  // Initialize status icons for checkboxes (handler status tracking)
  InitStatusIcons();

  InitUI();

  // Initialize tooltips with handler status
  InitTooltips();

  // Capture initial dialog client size for proportional resize
  RECT rc;
  GetClientRect(&rc);
  m_initialSize.cx = rc.right;
  m_initialSize.cy = rc.bottom;

  // Capture initial positions of all child controls
  m_anchors.clear();
  HWND hChild = ::GetWindow(m_hWnd, GW_CHILD);
  while (hChild) {
    int id = ::GetDlgCtrlID(hChild);
    RECT childRect;
    ::GetWindowRect(hChild, &childRect);
    ::MapWindowPoints(HWND_DESKTOP, m_hWnd, (LPPOINT)&childRect, 2);
    m_anchors.push_back({ id, childRect });
    hChild = ::GetWindow(hChild, GW_HWNDNEXT);
  }

  // Create initial font
  RecreateFont(m_fontSize);

  // Force all 35 format checkboxes to be visible
  int allCheckboxes[] = {
      IDC_CB_CBZ,  IDC_CB_CBR,   IDC_CB_CB7,      IDC_CB_CBT,   IDC_CB_EPUB,
      IDC_CB_MOBI, IDC_CB_AZW,   IDC_CB_AZW3,     IDC_CB_ZIP,   IDC_CB_RAR,
      IDC_CB_7Z,   IDC_CB_TAR,   IDC_CB_PHZ,      IDC_CB_FB2,   IDC_CB_WEBP,
      IDC_CB_HEIF, IDC_CB_AVIF,  IDC_CB_JXL,      IDC_CB_VIDEO, IDC_CB_PDF,
      IDC_CB_TIFF, IDC_CB_SVG,   IDC_CB_RAW,      IDC_CB_PSD,   IDC_CB_DDS,
      IDC_CB_HDR,  IDC_CB_EXR,   IDC_CB_PPM,      IDC_CB_ICO,   IDC_CB_QOI,
      IDC_CB_TGA,  IDC_CB_AUDIO, IDC_CB_DOCUMENT, IDC_CB_FONT,  IDC_CB_MODEL };
  for (int id : allCheckboxes) {
    HWND hCheck = GetDlgItem(id);
    if (hCheck) {
      ::ShowWindow(hCheck, SW_SHOW);
      ::UpdateWindow(hCheck);
    }
  }

  // set focus to Cancel btn
  GotoDlgCtrl(GetDlgItem(IDCANCEL));

  // Initialize dark mode and Windows 11 visual enhancements
  auto& darkCtrl = ExplorerLens::DarkModeController::Instance();
  darkCtrl.Initialize();
  darkCtrl.ApplyToWindow(m_hWnd);

  return FALSE;
}

LRESULT CMainDlg::OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam,
  BOOL& /*bHandled*/) {
  if (wParam == SIZE_MINIMIZED)
    return 0;
  if (m_initialSize.cx == 0 || m_anchors.empty())
    return 0;

  int clientWidth = LOWORD(lParam);
  int clientHeight = HIWORD(lParam);

  RelayoutControls(clientWidth, clientHeight);

  return 0;
}

void CMainDlg::RelayoutControls(int clientWidth, int clientHeight) {
  if (m_initialSize.cx == 0 || m_anchors.empty())
    return;

  double scaleX = (double)clientWidth / m_initialSize.cx;
  double scaleY = (double)clientHeight / m_initialSize.cy;

  HDWP hDwp = ::BeginDeferWindowPos((int)m_anchors.size());
  if (!hDwp)
    return;

  for (const auto& anchor : m_anchors) {
    HWND hCtrl = GetDlgItem(anchor.id);
    if (!hCtrl)
      continue;

    // Status bar always goes to the bottom full width
    if (anchor.id == IDC_STATUSBAR) {
      int sbHeight = anchor.initialRect.bottom - anchor.initialRect.top;
      hDwp = ::DeferWindowPos(hDwp, hCtrl, NULL, 0, clientHeight - sbHeight,
        clientWidth, sbHeight,
        SWP_NOZORDER | SWP_SHOWWINDOW);
      continue;
    }

    // Scale positions and sizes proportionally
    int newX = (int)(anchor.initialRect.left * scaleX);
    int newY = (int)(anchor.initialRect.top * scaleY);
    int newW =
      (int)((anchor.initialRect.right - anchor.initialRect.left) * scaleX);
    int newH =
      (int)((anchor.initialRect.bottom - anchor.initialRect.top) * scaleY);

    // Clamp minimum sizes
    if (newW < 20)
      newW = 20;
    if (newH < 8)
      newH = 8;

    hDwp = ::DeferWindowPos(hDwp, hCtrl, NULL, newX, newY, newW, newH,
      SWP_NOZORDER | SWP_SHOWWINDOW);
  }

  ::EndDeferWindowPos(hDwp);
  Invalidate();
}

LRESULT CMainDlg::OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/,
  LPARAM lParam, BOOL& /*bHandled*/) {
  LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
  lpMMI->ptMinTrackSize.x = 360; // Minimum width
  lpMMI->ptMinTrackSize.y = 460; // Minimum height
  return 0;
}

void CMainDlg::InitUI() {
  // Button_GetCheck   BST_CHECKED : BST_UNCHECKED equals TRUE: FALSE
  //  Comic Book Formats
  Button_SetCheck(GetDlgItem(IDC_CB_CBZ), m_reg.HasTH(LENS_CBZ));
  Button_SetCheck(GetDlgItem(IDC_CB_CBR), m_reg.HasTH(LENS_CBR));
  Button_SetCheck(GetDlgItem(IDC_CB_CB7), m_reg.HasTH(LENS_CB7));
  Button_SetCheck(GetDlgItem(IDC_CB_CBT), m_reg.HasTH(LENS_CBT));

  // E-Book Formats
  Button_SetCheck(GetDlgItem(IDC_CB_EPUB), m_reg.HasTH(LENS_EPUB));
  Button_SetCheck(GetDlgItem(IDC_CB_MOBI), m_reg.HasTH(LENS_MOBI));
  Button_SetCheck(GetDlgItem(IDC_CB_AZW), m_reg.HasTH(LENS_AZW));
  Button_SetCheck(GetDlgItem(IDC_CB_AZW3), m_reg.HasTH(LENS_AZW3));

  // Archive Formats
  Button_SetCheck(GetDlgItem(IDC_CB_ZIP), m_reg.HasTH(LENS_ZIP));
  Button_SetCheck(GetDlgItem(IDC_CB_RAR), m_reg.HasTH(LENS_RAR));
  Button_SetCheck(GetDlgItem(IDC_CB_7Z), m_reg.HasTH(LENS_7Z));
  Button_SetCheck(GetDlgItem(IDC_CB_TAR), m_reg.HasTH(LENS_TAR));

  // Photo & Other Formats
  Button_SetCheck(GetDlgItem(IDC_CB_PHZ), m_reg.HasTH(LENS_PHZ));
  Button_SetCheck(GetDlgItem(IDC_CB_FB2), m_reg.HasTH(LENS_FB2));

  // Modern Image Formats
  Button_SetCheck(GetDlgItem(IDC_CB_WEBP), m_reg.HasTH(LENS_WEBP));
  Button_SetCheck(GetDlgItem(IDC_CB_HEIF), m_reg.HasTH(LENS_HEIF));
  Button_SetCheck(GetDlgItem(IDC_CB_AVIF), m_reg.HasTH(LENS_AVIF));
  Button_SetCheck(GetDlgItem(IDC_CB_JXL), m_reg.HasTH(LENS_JXL));

  // Media & Documents
  Button_SetCheck(GetDlgItem(IDC_CB_VIDEO), m_reg.HasTH(LENS_VIDEO));
  Button_SetCheck(GetDlgItem(IDC_CB_PDF), m_reg.HasTH(LENS_PDF));
  Button_SetCheck(GetDlgItem(IDC_CB_TIFF), m_reg.HasTH(LENS_TIFF));
  Button_SetCheck(GetDlgItem(IDC_CB_SVG), m_reg.HasTH(LENS_SVG));
  Button_SetCheck(GetDlgItem(IDC_CB_RAW), m_reg.HasTH(LENS_RAW));
  Button_SetCheck(GetDlgItem(IDC_CB_PSD), m_reg.HasTH(LENS_PSD));
  Button_SetCheck(GetDlgItem(IDC_CB_DDS), m_reg.HasTH(LENS_DDS));
  Button_SetCheck(GetDlgItem(IDC_CB_HDR), m_reg.HasTH(LENS_HDR));
  Button_SetCheck(GetDlgItem(IDC_CB_EXR), m_reg.HasTH(LENS_EXR));
  Button_SetCheck(GetDlgItem(IDC_CB_PPM), m_reg.HasTH(LENS_PPM));
  Button_SetCheck(GetDlgItem(IDC_CB_ICO), m_reg.HasTH(LENS_ICO));
  Button_SetCheck(GetDlgItem(IDC_CB_QOI), m_reg.HasTH(LENS_QOI));
  Button_SetCheck(GetDlgItem(IDC_CB_TGA), m_reg.HasTH(LENS_TGA));
  Button_SetCheck(GetDlgItem(IDC_CB_AUDIO), m_reg.HasTH(LENS_AUDIO));
  Button_SetCheck(GetDlgItem(IDC_CB_DOCUMENT), m_reg.HasTH(LENS_DOCUMENT));
  Button_SetCheck(GetDlgItem(IDC_CB_FONT), m_reg.HasTH(LENS_FONT));
  Button_SetCheck(GetDlgItem(IDC_CB_MODEL), m_reg.HasTH(LENS_MODEL));

  // Collage Mode
  DWORD collageMode = m_reg.GetCollageMode();
  Button_SetCheck(GetDlgItem(IDC_RADIO_1X1), (collageMode == 1));
  Button_SetCheck(GetDlgItem(IDC_RADIO_2X2), (collageMode == 4));
  Button_SetCheck(GetDlgItem(IDC_RADIO_3X3), (collageMode == 9));
  Button_SetCheck(GetDlgItem(IDC_RADIO_4X4), (collageMode == 16));

  // Options
  Button_SetCheck(GetDlgItem(IDC_CB_SHOWICON), m_reg.IsShowIconOpt());
  Button_SetCheck(GetDlgItem(IDC_CB_SORT), m_reg.IsSortOpt());
}

// check ui state,compare to registry state->if!= refresh
void CMainDlg::OnApplyImpl() {
  BOOL bRet, bRefresh = FALSE;

  // sort option
  bRet = (BST_CHECKED == Button_GetCheck(GetDlgItem(IDC_CB_SORT)));
  if (bRet != m_reg.IsSortOpt()) {
    bRefresh = TRUE;
    m_reg.SetSortOpt(bRet);
  }
  // show archive icon
  bRet = (BST_CHECKED == Button_GetCheck(GetDlgItem(IDC_CB_SHOWICON)));
  if (bRet != m_reg.IsShowIconOpt()) {
    bRefresh = TRUE;
    m_reg.SetShowIconOpt(bRet);
  }
  // Each entry maps a dialog checkbox control ID to its LENS_* registry type ID
  static const struct {
    int ctrlId;
    int lensType;
  } formatHandlers[] = {
    // Comic Book Formats
    {IDC_CB_CBZ, LENS_CBZ},
    {IDC_CB_CBR, LENS_CBR},
    {IDC_CB_CB7, LENS_CB7},
    {IDC_CB_CBT, LENS_CBT},
    // E-Book Formats
    {IDC_CB_EPUB, LENS_EPUB},
    {IDC_CB_MOBI, LENS_MOBI},
    {IDC_CB_AZW, LENS_AZW},
    {IDC_CB_AZW3, LENS_AZW3},
    // Archive Formats
    {IDC_CB_ZIP, LENS_ZIP},
    {IDC_CB_RAR, LENS_RAR},
    {IDC_CB_7Z, LENS_7Z},
    {IDC_CB_TAR, LENS_TAR},
    // Photo & Other Formats
    {IDC_CB_PHZ, LENS_PHZ},
    {IDC_CB_FB2, LENS_FB2},
    // Modern Image Formats
    {IDC_CB_WEBP, LENS_WEBP},
    {IDC_CB_HEIF, LENS_HEIF},
    {IDC_CB_AVIF, LENS_AVIF},
    {IDC_CB_JXL, LENS_JXL},
    // Media & Documents
    {IDC_CB_VIDEO, LENS_VIDEO},
    {IDC_CB_PDF, LENS_PDF},
    {IDC_CB_TIFF, LENS_TIFF},
    {IDC_CB_SVG, LENS_SVG},
    {IDC_CB_RAW, LENS_RAW},
    // Professional & Specialized Formats
    {IDC_CB_PSD, LENS_PSD},
    {IDC_CB_DDS, LENS_DDS},
    {IDC_CB_HDR, LENS_HDR},
    {IDC_CB_EXR, LENS_EXR},
    {IDC_CB_PPM, LENS_PPM},
    {IDC_CB_ICO, LENS_ICO},
    {IDC_CB_QOI, LENS_QOI},
    {IDC_CB_TGA, LENS_TGA},
    {IDC_CB_AUDIO, LENS_AUDIO},
    {IDC_CB_DOCUMENT, LENS_DOCUMENT},
    {IDC_CB_FONT, LENS_FONT},
    {IDC_CB_MODEL, LENS_MODEL},
  };

  for (const auto& fh : formatHandlers) {
    bRet = (BST_CHECKED == Button_GetCheck(GetDlgItem(fh.ctrlId)));
    if (bRet != m_reg.HasTH(fh.lensType)) {
      bRefresh = TRUE;
      m_reg.SetHandlers(fh.lensType, bRet);
    }
  }

  // Collage Mode
  DWORD newCollageMode = 1; // Default: single page
  if (BST_CHECKED == Button_GetCheck(GetDlgItem(IDC_RADIO_2X2)))
    newCollageMode = 4;
  else if (BST_CHECKED == Button_GetCheck(GetDlgItem(IDC_RADIO_3X3)))
    newCollageMode = 9;
  else if (BST_CHECKED == Button_GetCheck(GetDlgItem(IDC_RADIO_4X4)))
    newCollageMode = 16;

  if (newCollageMode != m_reg.GetCollageMode()) {
    bRefresh = TRUE;
    m_reg.SetCollageMode(newCollageMode);
  }

  if (bRefresh) {
    ATLTRACE("refreshing FS\n");
    m_statusBar.SetText(0, _T("Refreshing Explorer..."));
    SHChangeNotify(SHCNE_ASSOCCHANGED,
      SHCNF_IDLIST | SHCNF_FLUSHNOWAIT | SHCNF_NOTIFYRECURSIVE,
      NULL, NULL);
  }

  InitUI();          // reload
  UpdateStatusBar();
}

LRESULT CMainDlg::OnApply(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/,
  BOOL& /*bHandled*/) {
  // Capture state before changes
  ConfigSnapshot oldConfig = CaptureCurrentConfig();

  OnApplyImpl();

  // Capture state after changes
  ConfigSnapshot newConfig = CaptureCurrentConfig();

  // Show change summary dialog
  CChangeSummaryDlg dlg(oldConfig, newConfig);
  INT_PTR result = dlg.DoModal(m_hWnd);

  // Check if user wants to restore previous settings
  if (result == IDC_BTN_RESTORE && dlg.ShouldRestore()) {
    ApplyConfigSnapshot(oldConfig);
    MessageBox(_T("Previous configuration has been restored."),
      _T("Configuration Restored"), MB_OK | MB_ICONINFORMATION);
  }

  return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/,
  BOOL& /*bHandled*/) {
  // Capture state before changes
  ConfigSnapshot oldConfig = CaptureCurrentConfig();

  OnApplyImpl();

  // Capture state after changes
  ConfigSnapshot newConfig = CaptureCurrentConfig();

  // Show change summary dialog
  CChangeSummaryDlg dlg(oldConfig, newConfig);
  INT_PTR result = dlg.DoModal(m_hWnd);

  // Check if user wants to restore previous settings
  if (result == IDC_BTN_RESTORE && dlg.ShouldRestore()) {
    ApplyConfigSnapshot(oldConfig);
    MessageBox(_T("Previous configuration has been restored."),
      _T("Configuration Restored"), MB_OK | MB_ICONINFORMATION);
    return 0; // Don't close dialog
  }

  CloseDialog(wID);
  return 0;
}

LRESULT CMainDlg::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam,
  BOOL& bHandled) {
  if (wParam != IDC_APPABOUT)
    return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
  CAboutDlg _a;
  _a.DoModal(m_hWnd);
  return 0;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/,
  BOOL& /*bHandled*/) {
  // Clean up font
  if (m_hFont) {
    ::DeleteObject(m_hFont);
    m_hFont = NULL;
  }

  // unregister message filtering and idle updates
  CMessageLoop* pLoop = _Module.GetMessageLoop();
  ATLASSERT(pLoop != NULL);
  pLoop->RemoveMessageFilter(this);
  pLoop->RemoveIdleHandler(this);
  return 0;
}

LRESULT CMainDlg::OnSettingChange(UINT /*uMsg*/, WPARAM /*wParam*/,
  LPARAM lParam, BOOL& /*bHandled*/) {
  auto& darkCtrl = ExplorerLens::DarkModeController::Instance();
  darkCtrl.OnSettingChange(m_hWnd, lParam);
  return 0;
}

LRESULT CMainDlg::OnCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam,
  BOOL& /*bHandled*/) {
  auto& darkCtrl = ExplorerLens::DarkModeController::Instance();
  return reinterpret_cast<LRESULT>(
    darkCtrl.OnCtlColor(reinterpret_cast<HDC>(wParam),
      reinterpret_cast<HWND>(lParam)));
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/,
  BOOL& /*bHandled*/) {
  CloseDialog(wID);
  return 0;
}

void CMainDlg::CloseDialog(int nVal) {
  DestroyWindow();
  ::PostQuitMessage(nVal);
}

LRESULT CMainDlg::OnAppHelp(LPHELPINFO lphi) {
  switch (lphi->iCtrlId) {
  case IDC_TH_GROUP:
  case IDC_CB_CBZ:
  case IDC_CB_EPUB:
  case IDC_CB_CBR:
  case IDC_CB_ZIP:
  case IDC_CB_RAR:
    // '#' anchors must use id attribute
    HtmlHelp(m_hWnd, _T("LENSShellHelp.chm::manager.html#optth"),
      HH_DISPLAY_TOPIC, NULL);
    break;

  case IDC_SORT_ADVOPTGROUP:
    HtmlHelp(m_hWnd, _T("LENSShellHelp.chm::manager.html#advopt"),
      HH_DISPLAY_TOPIC, NULL);
    break;

  case IDC_CB_SORT:
  case IDC_SORT_DESC:
    ATLTRACE("HH sort opt\n");
    HtmlHelp(m_hWnd, _T("LENSShellHelp.chm::FAQ.html#custth"), HH_DISPLAY_TOPIC,
      NULL);
    break;

  default:
    ATLTRACE("HH default\n");
    HtmlHelp(m_hWnd, _T("LENSShellHelp.chm::manager.html"), HH_DISPLAY_TOPIC,
      NULL); // about?
    break;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////
// Tooltip support with handler status indicators
//////////////////////////////////////////////////////////////////////////

void CMainDlg::InitTooltips() {
  // Create tooltip control
  m_tooltip.Create(m_hWnd);
  m_tooltip.SetMaxTipWidth(450);               // Wider for detailed info
  m_tooltip.SetDelayTime(TTDT_AUTOPOP, 15000); // Show for 15 seconds
  m_tooltip.Activate(TRUE);

  // Comic Book Formats - Add tooltips with file extensions and details
  AddTooltipWithStatus(IDC_CB_CBZ, LENS_CBZ,
    _T("CBZ - Comic Book ZIP Archive\nExtensions: .cbz"));
  AddTooltipWithStatus(IDC_CB_CBR, LENS_CBR,
    _T("CBR - Comic Book RAR Archive\nExtensions: ")
    _T(".cbr\nRequires: UnRAR library"));
  AddTooltipWithStatus(IDC_CB_CB7, LENS_CB7,
    _T("CB7 - Comic Book 7-Zip Archive\nExtensions: .cb7"));
  AddTooltipWithStatus(IDC_CB_CBT, LENS_CBT,
    _T("CBT - Comic Book TAR Archive\nExtensions: .cbt"));

  // E-Book Formats
  AddTooltipWithStatus(IDC_CB_EPUB, LENS_EPUB,
    _T("EPUB - Electronic Publication\nExtensions: ")
    _T(".epub\nSupports: Cover extraction from metadata"));
  AddTooltipWithStatus(IDC_CB_MOBI, LENS_MOBI,
    _T("MOBI - Mobipocket E-Book\nExtensions: .mobi"));
  AddTooltipWithStatus(IDC_CB_AZW, LENS_AZW,
    _T("AZW - Amazon Kindle Format\nExtensions: .azw"));
  AddTooltipWithStatus(IDC_CB_AZW3, LENS_AZW3,
    _T("AZW3 - Kindle Format 8 (KF8)\nExtensions: .azw3"));

  // Archive Formats
  AddTooltipWithStatus(IDC_CB_ZIP, LENS_ZIP,
    _T("ZIP - Standard ZIP Archive\nExtensions: ")
    _T(".zip\nThumbnails from first image inside"));
  AddTooltipWithStatus(
    IDC_CB_RAR, LENS_RAR,
    _T("RAR - WinRAR Archive\nExtensions: .rar\nRequires: UnRAR library"));
  AddTooltipWithStatus(
    IDC_CB_7Z, LENS_7Z,
    _T("7Z - 7-Zip Archive\nExtensions: .7z\nHigh compression ratio"));
  AddTooltipWithStatus(
    IDC_CB_TAR, LENS_TAR,
    _T("TAR - Tape Archive\nExtensions: .tar, .tar.gz, .tar.bz2"));

  // Photo Albums & Other
  AddTooltipWithStatus(IDC_CB_PHZ, LENS_PHZ,
    _T("PHZ - Photo ZIP Album\nExtensions: .phz\nOptimized ")
    _T("for photo collections"));
  AddTooltipWithStatus(
    IDC_CB_FB2, LENS_FB2,
    _T("FB2 - FictionBook E-Book\nExtensions: .fb2\nXML-based format"));

  // Modern Image Formats
  AddTooltipWithStatus(IDC_CB_WEBP, LENS_WEBP,
    _T("WebP - Modern Image Format\nExtensions: ")
    _T(".webp\nPerformance: Fast, built-in decoder"));
  AddTooltipWithStatus(IDC_CB_HEIF, LENS_HEIF,
    _T("HEIF/HEIC - High Efficiency Image\nExtensions: ")
    _T(".heif, .heic\nUsed by: iPhone photos\nRequires: ")
    _T("HEIF Image Extensions (Microsoft Store)"));
  AddTooltipWithStatus(
    IDC_CB_AVIF, LENS_AVIF,
    _T("AVIF - AV1 Image Format\nExtensions: .avif\nRequires: AV1 Video ")
    _T("Extension (Microsoft Store)\nPerformance: Fast via WIC"));
  AddTooltipWithStatus(IDC_CB_JXL, LENS_JXL,
    _T("JPEG XL - Next-Gen Image Format\nExtensions: ")
    _T(".jxl\nRequires: Windows Imaging Component support"));

  // Media & Documents
  AddTooltipWithStatus(
    IDC_CB_VIDEO, LENS_VIDEO,
    _T("Video Files - Extract Frame Thumbnails\nExtensions: .mp4, .avi, ")
    _T(".mkv, .mov, .wmv\nRequires: DirectShow codecs\nPerformance: Cached ")
    _T("for speed"));
  AddTooltipWithStatus(IDC_CB_PDF, LENS_PDF,
    _T("PDF Documents\nExtensions: .pdf\nPerformance: ")
    _T("Fast, uses embedded thumbnails when available"));
  AddTooltipWithStatus(IDC_CB_TIFF, LENS_TIFF,
    _T("TIFF Images\nExtensions: .tif, .tiff\nSupports: ")
    _T("Multi-page TIFF files"));
  AddTooltipWithStatus(IDC_CB_SVG, LENS_SVG,
    _T("SVG Vector Graphics\nExtensions: .svg, ")
    _T(".svgz\nScalable vector format"));
  AddTooltipWithStatus(
    IDC_CB_RAW, LENS_RAW,
    _T("RAW Camera Photos\nExtensions: .dng, .cr2, .cr3, .nef, .arw, ")
    _T(".orf, .rw2, .pef, .raf, .srw, .nrw\nRequires: LibRaw\n")
    _T("Performance: Fast (uses embedded preview)"));
  AddTooltipWithStatus(IDC_CB_PSD, LENS_PSD,
    _T("PSD - Adobe Photoshop Document\nExtensions: .psd, ")
    _T(".psb\nShows: Composite preview layer"));
  AddTooltipWithStatus(IDC_CB_DDS, LENS_DDS,
    _T("DDS - DirectDraw Surface\nExtensions: .dds\nUsed ")
    _T("by: Games, 3D applications"));
  AddTooltipWithStatus(
    IDC_CB_HDR, LENS_HDR,
    _T("HDR - Radiance High Dynamic Range\nExtensions: .hdr\nUsed by: ")
    _T("Professional photography, 3D rendering"));
  AddTooltipWithStatus(IDC_CB_EXR, LENS_EXR,
    _T("EXR - OpenEXR High Dynamic Range\nExtensions: ")
    _T(".exr\nUsed by: VFX, film production"));
  AddTooltipWithStatus(IDC_CB_PPM, LENS_PPM,
    _T("PPM - Portable Pixmap\nExtensions: .ppm, .pgm, ")
    _T(".pbm\nUsed by: Scientific imaging, CLI tools"));
  AddTooltipWithStatus(IDC_CB_ICO, LENS_ICO,
    _T("ICO - Windows Icon\nExtensions: .ico, .cur\nShows: ")
    _T("Largest available icon size"));
  AddTooltipWithStatus(IDC_CB_QOI, LENS_QOI,
    _T("QOI - Quite OK Image Format\nExtensions: ")
    _T(".qoi\nFast lossless compression"));
  AddTooltipWithStatus(IDC_CB_TGA, LENS_TGA,
    _T("TGA - Targa Image\nExtensions: .tga\nUsed by: ")
    _T("Games, 3D applications"));
  AddTooltipWithStatus(IDC_CB_AUDIO, LENS_AUDIO,
    _T("Audio Cover Art\nExtensions: .mp3, .flac, .wav, .ogg, ")
    _T(".m4a, .wma, .aac, .opus\nShows: Embedded album art thumbnail"));
  AddTooltipWithStatus(IDC_CB_DOCUMENT, LENS_DOCUMENT,
    _T("Document Thumbnails\nExtensions: .docx, .pptx, .xlsx, ")
    _T(".doc, .ppt, .xls\nShows: First page preview via OLE/COM"));
  AddTooltipWithStatus(IDC_CB_FONT, LENS_FONT,
    _T("Font Preview Thumbnails\nExtensions: .ttf, .otf, ")
    _T(".woff, .woff2\nShows: Sample text rendering"));
  AddTooltipWithStatus(IDC_CB_MODEL, LENS_MODEL,
    _T("3D Model Thumbnails\nExtensions: .stl, .obj, .gltf, ")
    _T(".glb, .fbx, .3ds, .ply\nShows: Wireframe/solid preview"));

  // Options tooltips (no status needed)
  m_tooltip.AddTool(GetDlgItem(IDC_CB_SORT),
    _T("Sort images alphabetically by filename\nUncheck to ")
    _T("use archive order (creation/modification time)"));
  m_tooltip.AddTool(GetDlgItem(IDC_CB_SHOWICON),
    _T("Show archive type icon overlay on thumbnails\nHelps ")
    _T("identify CBZ vs CBR vs ZIP, etc."));

  // Button tooltips
  m_tooltip.AddTool(
    GetDlgItem(IDC_BTN_LOAD_CONFIG),
    _T("Load configuration from REG or JSON file\nDouble-click .reg files ")
    _T("to import directly via Registry Editor"));
  m_tooltip.AddTool(GetDlgItem(IDOK), _T("Apply changes and close (Enter)"));
  m_tooltip.AddTool(GetDlgItem(IDC_APPLY),
    _T("Apply changes without closing (Ctrl+S)"));
  m_tooltip.AddTool(GetDlgItem(IDCANCEL),
    _T("Close without saving changes (Esc)"));

  // Collage mode tooltips
  m_tooltip.AddTool(GetDlgItem(IDC_RADIO_1X1),
    _T("Single Page Mode - Show only the first page\nDefault ")
    _T("for most formats"));
  m_tooltip.AddTool(
    GetDlgItem(IDC_RADIO_2X2),
    _T("2x2 Grid Mode - Show first 4 pages in grid\nGood for quick preview"));
  m_tooltip.AddTool(
    GetDlgItem(IDC_RADIO_3X3),
    _T("3x3 Grid Mode - Show first 9 pages in grid\nBalanced view"));
  m_tooltip.AddTool(GetDlgItem(IDC_RADIO_4X4),
    _T("4x4 Grid Mode - Show first 16 pages in grid\nBest for ")
    _T("comic books and quick content overview"));
}

void CMainDlg::AddTooltipWithStatus(int ctrlID, int LENSTYPE,
  LPCTSTR formatName) {
  // Enhanced with program name detection
  CString programName;
  HandlerStatus status = m_reg.GetHandlerStatusEx(
    LENSTYPE, m_reg.GetExtension(LENSTYPE), programName);
  CString tooltip;

  switch (status) {
  case HANDLER_ExplorerLens:
    tooltip.Format(_T("%s\n\n\xE2\x9C\x85 Active Handler: ")
      _T("ExplorerLens\nStatus: Enabled and working"),
      formatName);
    break;

  case HANDLER_NATIVE:
    tooltip.Format(
      _T("%s\n\n\xF0\x9F\x94\xB7 Current Handler: %s (Windows ")
      _T("built-in)\nNote: Enable ExplorerLens to use enhanced features"),
      formatName, (LPCTSTR)programName);
    break;

  case HANDLER_NONE:
    tooltip.Format(
      _T("%s\n\n\xE2\xAD\x95 No Handler: No thumbnail provider ")
      _T("installed\nAction: Enable ExplorerLens to generate thumbnails"),
      formatName);
    break;

  case HANDLER_THIRD_PARTY:
    tooltip.Format(
      _T("%s\n\n\xE2\x9A\xA0 Current Handler: %s (Third-party)\nAction: ")
      _T("Click Apply to use ExplorerLens instead"),
      formatName, (LPCTSTR)programName);
    break;

  default:
    tooltip = formatName;
    break;
  }

  // Store status for visual indicators
  m_checkboxStatus[ctrlID] = status;

  HWND hCtrl = GetDlgItem(ctrlID);
  if (hCtrl) {
    m_tooltip.AddTool(hCtrl, (LPCTSTR)tooltip);
  }
}

//////////////////////////////////////////////////////////////////////////
// Status bar and UI updates
//////////////////////////////////////////////////////////////////////////

void CMainDlg::UpdateStatusBar() {
  if (!m_statusBar.m_hWnd)
    return;

  int enabledCount = GetEnabledFormatCount();

  // Count conflicts (third-party handlers)
  int conflictCount = 0;
  const int allFormats[] = {
      LENS_CBZ,   LENS_CBR,  LENS_CB7,  LENS_CBT,  LENS_EPUB, LENS_MOBI,
      LENS_AZW,   LENS_AZW3, LENS_ZIP,  LENS_RAR,  LENS_7Z,   LENS_TAR,
      LENS_PHZ,   LENS_FB2,  LENS_WEBP, LENS_HEIF, LENS_AVIF, LENS_JXL,
      LENS_VIDEO, LENS_PDF,  LENS_TIFF, LENS_SVG,  LENS_RAW };

  for (int format : allFormats) {
    HandlerStatus status =
      m_reg.GetHandlerStatus(format, m_reg.GetExtension(format));
    if (status == HANDLER_THIRD_PARTY)
      conflictCount++;
  }

  CString statusText;
  if (conflictCount > 0) {
    statusText.Format(_T("Ready - %d of 31+ formats enabled | \xE2\x9A\xA0 %d ")
      _T("conflict(s) detected (hover for details)"),
      enabledCount, conflictCount);
  }
  else {
    statusText.Format(
      _T("Ready - %d of 31+ formats enabled | Windows 11 25H2 Compatible"),
      enabledCount);
  }

  m_statusBar.SetText(0, statusText);
}

int CMainDlg::GetEnabledFormatCount() {
  int count = 0;

  // Comic Book Formats (4)
  if (m_reg.HasTH(LENS_CBZ))
    count++;
  if (m_reg.HasTH(LENS_CBR))
    count++;
  if (m_reg.HasTH(LENS_CB7))
    count++;
  if (m_reg.HasTH(LENS_CBT))
    count++;

  // E-Book Formats (4)
  if (m_reg.HasTH(LENS_EPUB))
    count++;
  if (m_reg.HasTH(LENS_MOBI))
    count++;
  if (m_reg.HasTH(LENS_AZW))
    count++;
  if (m_reg.HasTH(LENS_AZW3))
    count++;

  // Archive Formats (4)
  if (m_reg.HasTH(LENS_ZIP))
    count++;
  if (m_reg.HasTH(LENS_RAR))
    count++;
  if (m_reg.HasTH(LENS_7Z))
    count++;
  if (m_reg.HasTH(LENS_TAR))
    count++;

  // Photo & Other Formats (2)
  if (m_reg.HasTH(LENS_PHZ))
    count++;
  if (m_reg.HasTH(LENS_FB2))
    count++;

  // Modern Image Formats (4)
  if (m_reg.HasTH(LENS_WEBP))
    count++;
  if (m_reg.HasTH(LENS_HEIF))
    count++;
  if (m_reg.HasTH(LENS_AVIF))
    count++;
  if (m_reg.HasTH(LENS_JXL))
    count++;

  // Media & Documents (5)
  if (m_reg.HasTH(LENS_VIDEO))
    count++;
  if (m_reg.HasTH(LENS_PDF))
    count++;
  if (m_reg.HasTH(LENS_TIFF))
    count++;
  if (m_reg.HasTH(LENS_SVG))
    count++;
  if (m_reg.HasTH(LENS_RAW))
    count++;
  if (m_reg.HasTH(LENS_PSD))
    count++;
  if (m_reg.HasTH(LENS_DDS))
    count++;
  if (m_reg.HasTH(LENS_HDR))
    count++;
  if (m_reg.HasTH(LENS_EXR))
    count++;
  if (m_reg.HasTH(LENS_PPM))
    count++;
  if (m_reg.HasTH(LENS_ICO))
    count++;
  if (m_reg.HasTH(LENS_QOI))
    count++;
  if (m_reg.HasTH(LENS_TGA))
    count++;
  if (m_reg.HasTH(LENS_AUDIO))
    count++;
  if (m_reg.HasTH(LENS_DOCUMENT))
    count++;
  if (m_reg.HasTH(LENS_FONT))
    count++;
  if (m_reg.HasTH(LENS_MODEL))
    count++;

  // Note: We have 35 format handler checkboxes (50+ file extensions supported)
  // Format handlers: 4 comic + 4 ebook + 4 archive + 2 photo + 4 image + 5
  // media/doc + 12 specialized = 35 Actual extensions: VIDEO covers 4+ formats,
  // RAW covers DNG/CR2/NEF/etc, HEIF covers .heif/.heic, AUDIO covers
  // MP3/FLAC/etc, DOCUMENT covers DOCX/XLSX/PPTX/etc, FONT covers TTF/OTF,
  // MODEL covers STL/OBJ/PLY, bringing total to 50+

  return count;
}

void CMainDlg::InitStatusIcons() {
  // Map each format checkbox to its handler status
  // This is used for owner-draw rendering with status icons

  // Comic Book Formats
  m_checkboxStatus[IDC_CB_CBZ] =
    m_reg.GetHandlerStatus(LENS_CBZ, m_reg.GetExtension(LENS_CBZ));
  m_checkboxStatus[IDC_CB_CBR] =
    m_reg.GetHandlerStatus(LENS_CBR, m_reg.GetExtension(LENS_CBR));
  m_checkboxStatus[IDC_CB_CB7] =
    m_reg.GetHandlerStatus(LENS_CB7, m_reg.GetExtension(LENS_CB7));
  m_checkboxStatus[IDC_CB_CBT] =
    m_reg.GetHandlerStatus(LENS_CBT, m_reg.GetExtension(LENS_CBT));

  // E-Book Formats
  m_checkboxStatus[IDC_CB_EPUB] =
    m_reg.GetHandlerStatus(LENS_EPUB, m_reg.GetExtension(LENS_EPUB));
  m_checkboxStatus[IDC_CB_MOBI] =
    m_reg.GetHandlerStatus(LENS_MOBI, m_reg.GetExtension(LENS_MOBI));
  m_checkboxStatus[IDC_CB_AZW] =
    m_reg.GetHandlerStatus(LENS_AZW, m_reg.GetExtension(LENS_AZW));
  m_checkboxStatus[IDC_CB_AZW3] =
    m_reg.GetHandlerStatus(LENS_AZW3, m_reg.GetExtension(LENS_AZW3));

  // Archive Formats
  m_checkboxStatus[IDC_CB_ZIP] =
    m_reg.GetHandlerStatus(LENS_ZIP, m_reg.GetExtension(LENS_ZIP));
  m_checkboxStatus[IDC_CB_RAR] =
    m_reg.GetHandlerStatus(LENS_RAR, m_reg.GetExtension(LENS_RAR));
  m_checkboxStatus[IDC_CB_7Z] =
    m_reg.GetHandlerStatus(LENS_7Z, m_reg.GetExtension(LENS_7Z));
  m_checkboxStatus[IDC_CB_TAR] =
    m_reg.GetHandlerStatus(LENS_TAR, m_reg.GetExtension(LENS_TAR));

  // Photo & Other Formats
  m_checkboxStatus[IDC_CB_PHZ] =
    m_reg.GetHandlerStatus(LENS_PHZ, m_reg.GetExtension(LENS_PHZ));
  m_checkboxStatus[IDC_CB_FB2] =
    m_reg.GetHandlerStatus(LENS_FB2, m_reg.GetExtension(LENS_FB2));

  // Modern Image Formats
  m_checkboxStatus[IDC_CB_WEBP] =
    m_reg.GetHandlerStatus(LENS_WEBP, m_reg.GetExtension(LENS_WEBP));
  m_checkboxStatus[IDC_CB_HEIF] =
    m_reg.GetHandlerStatus(LENS_HEIF, m_reg.GetExtension(LENS_HEIF));
  m_checkboxStatus[IDC_CB_AVIF] =
    m_reg.GetHandlerStatus(LENS_AVIF, m_reg.GetExtension(LENS_AVIF));
  m_checkboxStatus[IDC_CB_JXL] =
    m_reg.GetHandlerStatus(LENS_JXL, m_reg.GetExtension(LENS_JXL));

  // Media & Documents
  m_checkboxStatus[IDC_CB_VIDEO] =
    m_reg.GetHandlerStatus(LENS_VIDEO, m_reg.GetExtension(LENS_VIDEO));
  m_checkboxStatus[IDC_CB_PDF] =
    m_reg.GetHandlerStatus(LENS_PDF, m_reg.GetExtension(LENS_PDF));
  m_checkboxStatus[IDC_CB_TIFF] =
    m_reg.GetHandlerStatus(LENS_TIFF, m_reg.GetExtension(LENS_TIFF));
  m_checkboxStatus[IDC_CB_SVG] =
    m_reg.GetHandlerStatus(LENS_SVG, m_reg.GetExtension(LENS_SVG));
  m_checkboxStatus[IDC_CB_RAW] =
    m_reg.GetHandlerStatus(LENS_RAW, m_reg.GetExtension(LENS_RAW));
  m_checkboxStatus[IDC_CB_PSD] =
    m_reg.GetHandlerStatus(LENS_PSD, m_reg.GetExtension(LENS_PSD));
  m_checkboxStatus[IDC_CB_DDS] =
    m_reg.GetHandlerStatus(LENS_DDS, m_reg.GetExtension(LENS_DDS));
  m_checkboxStatus[IDC_CB_HDR] =
    m_reg.GetHandlerStatus(LENS_HDR, m_reg.GetExtension(LENS_HDR));
  m_checkboxStatus[IDC_CB_EXR] =
    m_reg.GetHandlerStatus(LENS_EXR, m_reg.GetExtension(LENS_EXR));
  m_checkboxStatus[IDC_CB_PPM] =
    m_reg.GetHandlerStatus(LENS_PPM, m_reg.GetExtension(LENS_PPM));
  m_checkboxStatus[IDC_CB_ICO] =
    m_reg.GetHandlerStatus(LENS_ICO, m_reg.GetExtension(LENS_ICO));
  m_checkboxStatus[IDC_CB_QOI] =
    m_reg.GetHandlerStatus(LENS_QOI, m_reg.GetExtension(LENS_QOI));
  m_checkboxStatus[IDC_CB_TGA] =
    m_reg.GetHandlerStatus(LENS_TGA, m_reg.GetExtension(LENS_TGA));
  m_checkboxStatus[IDC_CB_AUDIO] =
    m_reg.GetHandlerStatus(LENS_AUDIO, m_reg.GetExtension(LENS_AUDIO));
  m_checkboxStatus[IDC_CB_DOCUMENT] =
    m_reg.GetHandlerStatus(LENS_DOCUMENT, m_reg.GetExtension(LENS_DOCUMENT));
  m_checkboxStatus[IDC_CB_FONT] =
    m_reg.GetHandlerStatus(LENS_FONT, m_reg.GetExtension(LENS_FONT));
  m_checkboxStatus[IDC_CB_MODEL] =
    m_reg.GetHandlerStatus(LENS_MODEL, m_reg.GetExtension(LENS_MODEL));
}

// Handle checkbox clicks - BS_AUTO3STATE handles toggling automatically
LRESULT CMainDlg::OnCheckboxClicked(WORD /*wNotifyCode*/, WORD wID,
  HWND hWndCtl, BOOL& /*bHandled*/) {
  return 0;
}

//////////////////////////////////////////////////////////////////////////
// Keyboard Shortcuts
//////////////////////////////////////////////////////////////////////////

LRESULT CMainDlg::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/,
  BOOL& /*bHandled*/) {
  bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

  if (ctrlPressed) {
    switch (wParam) {
    case 'A': // Ctrl+A: Select all format checkboxes
      OnSelectAll();
      return 0;
    case 'D': // Ctrl+D: Deselect all format checkboxes
      OnDeselectAll();
      return 0;
    case 'S': // Ctrl+S: Apply changes
    {
      BOOL bHandled = FALSE;
      OnApply(0, IDC_APPLY, NULL, bHandled);
      return 0;
    }
    case VK_OEM_PLUS: // Ctrl+= (Ctrl+=): Increase font size
    case VK_ADD:      // Ctrl+NumPad+
      if (m_fontSize < FONT_SIZE_MAX) {
        RecreateFont(m_fontSize + 1);
      }
      return 0;
    case VK_OEM_MINUS: // Ctrl+-: Decrease font size
    case VK_SUBTRACT:  // Ctrl+NumPad-
      if (m_fontSize > FONT_SIZE_MIN) {
        RecreateFont(m_fontSize - 1);
      }
      return 0;
    case '0': // Ctrl+0: Reset font to default
      RecreateFont(8);
      return 0;
    }
  }
  else {
    switch (wParam) {
    case VK_F1: // F1: Show help
      // Display comprehensive help dialog
      MessageBox(_T("ExplorerLens Shell Manager - Quick Help\n\n")
        _T("KEYBOARD SHORTCUTS:\n")
        _T("  Ctrl+A     - Select all formats\n")
        _T("  Ctrl+D     - Deselect all formats\n")
        _T("  Ctrl+S     - Apply changes\n")
        _T("  Ctrl+=     - Increase font size\n")
        _T("  Ctrl+-     - Decrease font size\n")
        _T("  Ctrl+0     - Reset font size\n")
        _T("  Ctrl+Wheel - Zoom font in/out\n")
        _T("  F1         - Show this help\n")
        _T("  F5         - Refresh status\n\n")
        _T("FEATURES:\n")
        _T("  \x2022 50+ supported formats\n")
        _T("  \x2022 Modern images: WebP, AVIF, JXL, HEIF\n")
        _T("  \x2022 Conflict detection\n")
        _T("  \x2022 Resizable window with font scaling\n")
        _T("  \x2022 Windows 11 25H2 compatible\n\n")
        _T("TIPS:\n")
        _T("  \x2022 Hover over icons for details\n")
        _T("  \x2022 Check status bar for count\n")
        _T("\nFor full docs, press F1 on any control."),
        _T("ExplorerLens Help"), MB_OK | MB_ICONINFORMATION);
      return 0;
    }
  }

  return 0;
}

LRESULT CMainDlg::OnMouseWheel(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/,
  BOOL& bHandled) {
  bool ctrlPressed = (LOWORD(wParam) & MK_CONTROL) != 0;
  if (!ctrlPressed) {
    bHandled = FALSE;
    return 0;
  }

  short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
  if (zDelta > 0 && m_fontSize < FONT_SIZE_MAX) {
    RecreateFont(m_fontSize + 1);
  }
  else if (zDelta < 0 && m_fontSize > FONT_SIZE_MIN) {
    RecreateFont(m_fontSize - 1);
  }

  return 0;
}

void CMainDlg::RecreateFont(int pointSize) {
  m_fontSize = pointSize;

  // Delete old font if it exists
  if (m_hFont) {
    ::DeleteObject(m_hFont);
    m_hFont = NULL;
  }

  // Create new font at the requested point size
  HDC hDC = ::GetDC(m_hWnd);
  int logPixelsY = ::GetDeviceCaps(hDC, LOGPIXELSY);
  ::ReleaseDC(m_hWnd, hDC);

  int lfHeight = -MulDiv(pointSize, logPixelsY, 72);

  m_hFont = ::CreateFont(lfHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
    DEFAULT_PITCH | FF_DONTCARE, _T("Segoe UI"));

  if (!m_hFont) {
    // Fallback to MS Shell Dlg
    m_hFont = ::CreateFont(lfHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
      CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
      DEFAULT_PITCH | FF_DONTCARE, _T("MS Shell Dlg"));
  }

  if (m_hFont) {
    // Apply font to all child controls
    HWND hChild = ::GetWindow(m_hWnd, GW_CHILD);
    while (hChild) {
      ::SendMessage(hChild, WM_SETFONT, (WPARAM)m_hFont, TRUE);
      hChild = ::GetWindow(hChild, GW_HWNDNEXT);
    }

    // Update status bar with font info
    CString statusText;
    statusText.Format(_T("Font: %dpt | Ctrl+=/- to resize"), m_fontSize);
    if (m_statusBar.m_hWnd)
      m_statusBar.SetText(0, statusText);

    Invalidate();
  }
}

void CMainDlg::OnSelectAll() {
  // Select all format checkboxes
  int formatCheckboxes[] = {
      IDC_CB_CBZ,  IDC_CB_CBR,   IDC_CB_CB7,      IDC_CB_CBT,   IDC_CB_EPUB,
      IDC_CB_MOBI, IDC_CB_AZW,   IDC_CB_AZW3,     IDC_CB_ZIP,   IDC_CB_RAR,
      IDC_CB_7Z,   IDC_CB_TAR,   IDC_CB_PHZ,      IDC_CB_FB2,   IDC_CB_WEBP,
      IDC_CB_HEIF, IDC_CB_AVIF,  IDC_CB_JXL,      IDC_CB_VIDEO, IDC_CB_PDF,
      IDC_CB_TIFF, IDC_CB_SVG,   IDC_CB_RAW,      IDC_CB_PSD,   IDC_CB_DDS,
      IDC_CB_HDR,  IDC_CB_EXR,   IDC_CB_PPM,      IDC_CB_ICO,   IDC_CB_QOI,
      IDC_CB_TGA,  IDC_CB_AUDIO, IDC_CB_DOCUMENT, IDC_CB_FONT,  IDC_CB_MODEL };

  for (int id : formatCheckboxes) {
    Button_SetCheck(GetDlgItem(id), BST_CHECKED);
  }
}

void CMainDlg::OnDeselectAll() {
  // Deselect all format checkboxes
  int formatCheckboxes[] = {
      IDC_CB_CBZ,  IDC_CB_CBR,   IDC_CB_CB7,      IDC_CB_CBT,   IDC_CB_EPUB,
      IDC_CB_MOBI, IDC_CB_AZW,   IDC_CB_AZW3,     IDC_CB_ZIP,   IDC_CB_RAR,
      IDC_CB_7Z,   IDC_CB_TAR,   IDC_CB_PHZ,      IDC_CB_FB2,   IDC_CB_WEBP,
      IDC_CB_HEIF, IDC_CB_AVIF,  IDC_CB_JXL,      IDC_CB_VIDEO, IDC_CB_PDF,
      IDC_CB_TIFF, IDC_CB_SVG,   IDC_CB_RAW,      IDC_CB_PSD,   IDC_CB_DDS,
      IDC_CB_HDR,  IDC_CB_EXR,   IDC_CB_PPM,      IDC_CB_ICO,   IDC_CB_QOI,
      IDC_CB_TGA,  IDC_CB_AUDIO, IDC_CB_DOCUMENT, IDC_CB_FONT,  IDC_CB_MODEL };

  for (int id : formatCheckboxes) {
    Button_SetCheck(GetDlgItem(id), BST_UNCHECKED);
  }
}

// Configuration Management Functions

ConfigSnapshot CMainDlg::CaptureCurrentConfig() {
  ConfigSnapshot config = { 0 };

  // Capture format states
  config.cbz = (m_reg.HasTH(LENS_CBZ) != FALSE);
  config.cbr = (m_reg.HasTH(LENS_CBR) != FALSE);
  config.cb7 = (m_reg.HasTH(LENS_CB7) != FALSE);
  config.cbt = (m_reg.HasTH(LENS_CBT) != FALSE);
  config.epub = (m_reg.HasTH(LENS_EPUB) != FALSE);
  config.mobi = (m_reg.HasTH(LENS_MOBI) != FALSE);
  config.azw = (m_reg.HasTH(LENS_AZW) != FALSE);
  config.azw3 = (m_reg.HasTH(LENS_AZW3) != FALSE);
  config.zip = (m_reg.HasTH(LENS_ZIP) != FALSE);
  config.rar = (m_reg.HasTH(LENS_RAR) != FALSE);
  config.z7 = (m_reg.HasTH(LENS_7Z) != FALSE);
  config.tar = (m_reg.HasTH(LENS_TAR) != FALSE);
  config.phz = (m_reg.HasTH(LENS_PHZ) != FALSE);
  config.fb2 = (m_reg.HasTH(LENS_FB2) != FALSE);
  config.webp = (m_reg.HasTH(LENS_WEBP) != FALSE);
  config.heif = (m_reg.HasTH(LENS_HEIF) != FALSE);
  config.avif = (m_reg.HasTH(LENS_AVIF) != FALSE);
  config.jxl = (m_reg.HasTH(LENS_JXL) != FALSE);
  config.video = (m_reg.HasTH(LENS_VIDEO) != FALSE);
  config.pdf = (m_reg.HasTH(LENS_PDF) != FALSE);
  config.tiff = (m_reg.HasTH(LENS_TIFF) != FALSE);
  config.svg = (m_reg.HasTH(LENS_SVG) != FALSE);
  config.raw = (m_reg.HasTH(LENS_RAW) != FALSE);
  config.psd = (m_reg.HasTH(LENS_PSD) != FALSE);
  config.dds = (m_reg.HasTH(LENS_DDS) != FALSE);
  config.hdr = (m_reg.HasTH(LENS_HDR) != FALSE);
  config.exr = (m_reg.HasTH(LENS_EXR) != FALSE);
  config.ppm = (m_reg.HasTH(LENS_PPM) != FALSE);
  config.ico = (m_reg.HasTH(LENS_ICO) != FALSE);
  config.qoi = (m_reg.HasTH(LENS_QOI) != FALSE);
  config.tga = (m_reg.HasTH(LENS_TGA) != FALSE);
  config.audio = (m_reg.HasTH(LENS_AUDIO) != FALSE);
  config.document = (m_reg.HasTH(LENS_DOCUMENT) != FALSE);
  config.font = (m_reg.HasTH(LENS_FONT) != FALSE);
  config.model = (m_reg.HasTH(LENS_MODEL) != FALSE);

  // Capture options
  config.sortOpt = (m_reg.IsSortOpt() != FALSE);
  config.showIconOpt = (m_reg.IsShowIconOpt() != FALSE);
  config.collageMode = m_reg.GetCollageMode();

  return config;
}

void CMainDlg::ApplyConfigSnapshot(const ConfigSnapshot& config) {
  // Apply format handlers
  m_reg.SetHandlers(LENS_CBZ, config.cbz);
  m_reg.SetHandlers(LENS_CBR, config.cbr);
  m_reg.SetHandlers(LENS_CB7, config.cb7);
  m_reg.SetHandlers(LENS_CBT, config.cbt);
  m_reg.SetHandlers(LENS_EPUB, config.epub);
  m_reg.SetHandlers(LENS_MOBI, config.mobi);
  m_reg.SetHandlers(LENS_AZW, config.azw);
  m_reg.SetHandlers(LENS_AZW3, config.azw3);
  m_reg.SetHandlers(LENS_ZIP, config.zip);
  m_reg.SetHandlers(LENS_RAR, config.rar);
  m_reg.SetHandlers(LENS_7Z, config.z7);
  m_reg.SetHandlers(LENS_TAR, config.tar);
  m_reg.SetHandlers(LENS_PHZ, config.phz);
  m_reg.SetHandlers(LENS_FB2, config.fb2);
  m_reg.SetHandlers(LENS_WEBP, config.webp);
  m_reg.SetHandlers(LENS_HEIF, config.heif);
  m_reg.SetHandlers(LENS_AVIF, config.avif);
  m_reg.SetHandlers(LENS_JXL, config.jxl);
  m_reg.SetHandlers(LENS_VIDEO, config.video);
  m_reg.SetHandlers(LENS_PDF, config.pdf);
  m_reg.SetHandlers(LENS_TIFF, config.tiff);
  m_reg.SetHandlers(LENS_SVG, config.svg);
  m_reg.SetHandlers(LENS_RAW, config.raw);
  m_reg.SetHandlers(LENS_PSD, config.psd);
  m_reg.SetHandlers(LENS_DDS, config.dds);
  m_reg.SetHandlers(LENS_HDR, config.hdr);
  m_reg.SetHandlers(LENS_EXR, config.exr);
  m_reg.SetHandlers(LENS_PPM, config.ppm);
  m_reg.SetHandlers(LENS_ICO, config.ico);
  m_reg.SetHandlers(LENS_QOI, config.qoi);
  m_reg.SetHandlers(LENS_TGA, config.tga);
  m_reg.SetHandlers(LENS_AUDIO, config.audio);
  m_reg.SetHandlers(LENS_DOCUMENT, config.document);
  m_reg.SetHandlers(LENS_FONT, config.font);
  m_reg.SetHandlers(LENS_MODEL, config.model);

  // Apply options
  m_reg.SetSortOpt(config.sortOpt);
  m_reg.SetShowIconOpt(config.showIconOpt);
  m_reg.SetCollageMode(config.collageMode);

  // Refresh Explorer
  SHChangeNotify(SHCNE_ASSOCCHANGED,
    SHCNF_IDLIST | SHCNF_FLUSHNOWAIT | SHCNF_NOTIFYRECURSIVE, NULL,
    NULL);

  // Reload UI
  InitUI();
  UpdateStatusBar();
}

bool CMainDlg::LoadConfigFromFile(LPCTSTR filename, ConfigSnapshot& outConfig) {
  CAtlFile file;
  HRESULT hr =
    file.Create(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
  if (FAILED(hr))
    return false;

  ULONGLONG fileSize = 0;
  hr = file.GetSize(fileSize);
  if (FAILED(hr) || fileSize > 1024 * 1024) { // Max 1MB
    file.Close();
    return false;
  }

  CStringA jsonData;
  char* buffer = jsonData.GetBuffer((int)fileSize + 1);
  DWORD bytesRead = 0;
  hr = file.Read(buffer, (DWORD)fileSize, bytesRead);
  jsonData.ReleaseBuffer(bytesRead);
  file.Close();

  if (FAILED(hr))
    return false;

  // Simple JSON parsing (basic approach - looks for true/false after field
  // names)
  auto getBool = [&jsonData](const char* key) -> bool {
    CStringA searchKey;
    searchKey.Format("\"%s\"", key);
    int pos = jsonData.Find(searchKey);
    if (pos == -1)
      return false;
    int valuePos = jsonData.Find(":", pos);
    if (valuePos == -1)
      return false;
    int truePos = jsonData.Find("true", valuePos);
    int falsePos = jsonData.Find("false", valuePos);
    int commaPos = jsonData.Find(",", valuePos);
    int bracePos = jsonData.Find("}", valuePos);
    int endPos = (commaPos != -1 && commaPos < bracePos) ? commaPos : bracePos;

    if (truePos != -1 && truePos < endPos)
      return true;
    if (falsePos != -1 && falsePos < endPos)
      return false;
    return false;
    };

  auto getInt = [&jsonData](const char* key) -> int {
    CStringA searchKey;
    searchKey.Format("\"%s\"", key);
    int pos = jsonData.Find(searchKey);
    if (pos == -1)
      return 0;
    int valuePos = jsonData.Find(":", pos);
    if (valuePos == -1)
      return 0;

    // Skip whitespace
    while (valuePos < jsonData.GetLength() &&
      (jsonData[valuePos] == ':' || jsonData[valuePos] == ' ' ||
        jsonData[valuePos] == '\t'))
      valuePos++;

    // Parse integer
    int value = 0;
    while (valuePos < jsonData.GetLength() && jsonData[valuePos] >= '0' &&
      jsonData[valuePos] <= '9') {
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

bool CMainDlg::LoadConfigFromRegFile(LPCTSTR filename,
  ConfigSnapshot& outConfig) {
  CAtlFile file;
  HRESULT hr =
    file.Create(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
  if (FAILED(hr))
    return false;

  ULONGLONG fileSize = 0;
  hr = file.GetSize(fileSize);
  if (FAILED(hr) || fileSize > 5 * 1024 * 1024) { // Max 5MB for .reg files
    file.Close();
    return false;
  }

  CStringA regData;
  char* buffer = regData.GetBuffer((int)fileSize + 1);
  DWORD bytesRead = 0;
  hr = file.Read(buffer, (DWORD)fileSize, bytesRead);
  regData.ReleaseBuffer(bytesRead);
  file.Close();

  if (FAILED(hr))
    return false;

  // Simple .reg parser - looks for handler GUID presence/absence
  auto isFormatEnabled = [&regData](const char* ext) -> bool {
    CStringA searchKey;
    searchKey.Format("SOFTWARE\\Classes\\.%s\\shellex\\{BB2E617C-0920-11d1-"
      "9A0B-00C04FC2D6C1}",
      ext);
    int pos = regData.Find(searchKey);
    if (pos == -1)
      return false;

    // Check if the next line has @="{GUID}" (enabled) or @=- (disabled)
    int nextLine = regData.Find("\r\n", pos);
    if (nextLine == -1)
      return false;

    int guidPos =
      regData.Find("{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}", nextLine);
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
    if (pos == -1)
      return 0;

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
  outConfig.psd = isFormatEnabled("psd");
  outConfig.dds = isFormatEnabled("dds");
  outConfig.hdr = isFormatEnabled("hdr");
  outConfig.exr = isFormatEnabled("exr");
  outConfig.ppm = isFormatEnabled("ppm");
  outConfig.ico = isFormatEnabled("ico");
  outConfig.qoi = isFormatEnabled("qoi");
  outConfig.tga = isFormatEnabled("tga");
  outConfig.audio = isFormatEnabled("mp3"); // Use mp3 as proxy for all audio
  outConfig.document =
    isFormatEnabled("docx"); // Use docx as proxy for all documents
  outConfig.font = isFormatEnabled("ttf"); // Use ttf as proxy for all fonts
  outConfig.model =
    isFormatEnabled("stl"); // Use stl as proxy for all 3D models

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

LRESULT CMainDlg::OnLoadConfig(WORD /*wNotifyCode*/, WORD /*wID*/,
  HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
  CFileDialog dlg(TRUE, _T("reg"), NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
    _T("Windows Registry File (*.reg)\0*.reg\0JSON ")
    _T("Configuration (*.json)\0*.json\0All Files (*.*)\0*.*\0"));

  if (dlg.DoModal() == IDOK) {
    CString filename(dlg.m_szFileName);
    bool isRegFile = (filename.Right(4).CompareNoCase(_T(".reg")) == 0);

    if (isRegFile) {
      // For .reg files, offer to import directly or use shell execute
      CString msg;
      msg.Format(_T("Registry file selected:\r\n%s\r\n\r\n")
        _T("How do you want to import this configuration?\r\n\r\n")
        _T("YES = Import via Registry Editor (recommended)\r\n")
        _T("NO = Load and apply through ExplorerLens\r\n")
        _T("CANCEL = Cancel operation"),
        dlg.m_szFileName);

      int result = MessageBox(msg, _T("Import Registry File"),
        MB_YESNOCANCEL | MB_ICONQUESTION);

      if (result == IDYES) {
        // Use regedit to import the file
        CString cmdLine;
        cmdLine.Format(_T("regedit.exe /s \"%s\""), dlg.m_szFileName);

        STARTUPINFO si = { sizeof(STARTUPINFO) };
        PROCESS_INFORMATION pi = { 0 };

        if (CreateProcess(NULL, cmdLine.GetBuffer(), NULL, NULL, FALSE,
          CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
          // Wait for regedit to complete
          WaitForSingleObject(pi.hProcess, 10000); // 10 second timeout
          CloseHandle(pi.hProcess);
          CloseHandle(pi.hThread);

          // Refresh Explorer
          SHChangeNotify(SHCNE_ASSOCCHANGED,
            SHCNF_IDLIST | SHCNF_FLUSHNOWAIT |
            SHCNF_NOTIFYRECURSIVE,
            NULL, NULL);

          // Reload UI to reflect registry changes
          InitUI();
          UpdateStatusBar();

          MessageBox(_T("Registry configuration imported successfully!\r\n\r\n")
            _T("Windows Explorer has been refreshed.\r\n")
            _T("The UI has been updated to reflect the new settings."),
            _T("Import Complete"), MB_OK | MB_ICONINFORMATION);
        }
        else {
          MessageBox(
            _T("Failed to launch Registry Editor.\r\n\r\n")
            _T("You can manually double-click the .reg file to import it."),
            _T("Import Error"), MB_OK | MB_ICONERROR);
        }
      }
      else if (result == IDNO) {
        // Parse .reg file and apply through our code
        ConfigSnapshot loadedConfig;
        if (LoadConfigFromRegFile(dlg.m_szFileName, loadedConfig)) {
          ApplyConfigSnapshot(loadedConfig);
          MessageBox(_T("Configuration applied successfully!\r\n\r\n")
            _T("Windows Explorer has been refreshed."),
            _T("Configuration Applied"), MB_OK | MB_ICONINFORMATION);
        }
        else {
          MessageBox(_T("Failed to parse registry file."), _T("Load Error"),
            MB_OK | MB_ICONERROR);
        }
      }
    }
    else {
      // JSON file handling (existing code)
      ConfigSnapshot loadedConfig;
      if (LoadConfigFromFile(dlg.m_szFileName, loadedConfig)) {
        CString msg;
        msg.Format(
          _T("Configuration loaded from:\r\n%s\r\n\r\n")
          _T("Do you want to apply this configuration?\r\n\r\n")
          _T("This will change your current thumbnail handler settings."),
          dlg.m_szFileName);

        if (MessageBox(msg, _T("Apply Configuration?"),
          MB_YESNO | MB_ICONQUESTION) == IDYES) {
          ApplyConfigSnapshot(loadedConfig);
          MessageBox(_T("Configuration applied successfully!\r\n\r\n")
            _T("Windows Explorer has been refreshed."),
            _T("Configuration Applied"), MB_OK | MB_ICONINFORMATION);
        }
      }
      else {
        MessageBox(_T("Failed to load configuration file.\r\n\r\n")
          _T("The file may be corrupted or in an unsupported format."),
          _T("Load Error"), MB_OK | MB_ICONERROR);
      }
    }
  }

  return 0;
}
