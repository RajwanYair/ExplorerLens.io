#ifndef _REGMANAGER_79AE66E4_84E2_45A1_BF4F_43AA714BE55F_
#define _REGMANAGER_79AE66E4_84E2_45A1_BF4F_43AA714BE55F_
#pragma once

#include <windows.h>
#pragma comment(lib, "advapi32.lib")
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <atlstr.h> // For CString

#define LENS_MUTEX_GLOBAL L"Global\\{64DEE47D-9669-4430-9D5C-304867F87B51}"
#define LENS_MGRMUTEX L"Local\\{50D9CBE6-C168-4901-8CC9-2A7C97E558F7}"

#define LENS_GUID_KEY _T("{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}") // 38+1 TCHAR
#define LENS_GUID_KEY_SLEN 39
#define LENS_APP_KEY                                                           \
  _T("Software\\T800 Productions\\{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}")

// per-user settings (HKCU)
// thumbnail handler keys
#define LENS_ZIPTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.ZIP\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_CBZTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.CBZ\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_RARTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.RAR\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_CBRTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.CBR\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_EPUBTH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.EPUB\\shellex\\{BB2E617C-0920-11d1-9A0B-")           \
  _T("00C04FC2D6C1}")
#define LENS_7ZTH_KEY                                                          \
  _T("SOFTWARE\\Classes\\.7Z\\shellex\\{BB2E617C-0920-11d1-9A0B-")             \
  _T("00C04FC2D6C1}")
#define LENS_CB7TH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.CB7\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_TARTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.TAR\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_CBTTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.CBT\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_MOBITH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.MOBI\\shellex\\{BB2E617C-0920-11d1-9A0B-")           \
  _T("00C04FC2D6C1}")
#define LENS_FB2TH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.FB2\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_AZWTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.AZW\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_AZW3TH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.AZW3\\shellex\\{BB2E617C-0920-11d1-9A0B-")           \
  _T("00C04FC2D6C1}")
#define LENS_PHZTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.PHZ\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_WEBPTH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.WEBP\\shellex\\{BB2E617C-0920-11d1-9A0B-")           \
  _T("00C04FC2D6C1}")
#define LENS_HEIFTH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.HEIF\\shellex\\{BB2E617C-0920-11d1-9A0B-")           \
  _T("00C04FC2D6C1}")
#define LENS_HEICTH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.HEIC\\shellex\\{BB2E617C-0920-11d1-9A0B-")           \
  _T("00C04FC2D6C1}")
#define LENS_AVIFTH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.AVIF\\shellex\\{BB2E617C-0920-11d1-9A0B-")           \
  _T("00C04FC2D6C1}")
#define LENS_JXLTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.JXL\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_MP4TH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.MP4\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_AVITH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.AVI\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_MKVTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.MKV\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_WMVTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.WMV\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_PDFTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.PDF\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_TIFFTH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.TIF\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_SVGTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.SVG\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_DNGTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.DNG\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_PSDTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.PSD\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_DDSTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.DDS\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_HDRTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.HDR\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_EXRTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.EXR\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_PPMTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.PPM\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_ICOTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.ICO\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_QOITH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.QOI\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_TGATH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.TGA\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_MP3TH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.MP3\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_TTFTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.TTF\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
#define LENS_DOCXTH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.DOCX\\shellex\\{BB2E617C-0920-11d1-9A0B-")           \
  _T("00C04FC2D6C1}")
#define LENS_STLTH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.STL\\shellex\\{BB2E617C-0920-11d1-9A0B-")            \
  _T("00C04FC2D6C1}")
// infotip handler keys
#define LENS_ZIPIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.ZIP\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_CBZIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.CBZ\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_RARIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.RAR\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_CBRIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.CBR\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_EPUBIH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.EPUB\\shellex\\{00021500-0000-0000-C000-")           \
  _T("000000000046}")
#define LENS_7ZIH_KEY                                                          \
  _T("SOFTWARE\\Classes\\.7Z\\shellex\\{00021500-0000-0000-C000-")             \
  _T("000000000046}")
#define LENS_CB7IH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.CB7\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_TARIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.TAR\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_CBTIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.CBT\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_MOBIIH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.MOBI\\shellex\\{00021500-0000-0000-C000-")           \
  _T("000000000046}")
#define LENS_FB2IH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.FB2\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_AZWIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.AZW\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_AZW3IH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.AZW3\\shellex\\{00021500-0000-0000-C000-")           \
  _T("000000000046}")
#define LENS_PHZIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.PHZ\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_WEBPIH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.WEBP\\shellex\\{00021500-0000-0000-C000-")           \
  _T("000000000046}")
#define LENS_HEIFIH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.HEIF\\shellex\\{00021500-0000-0000-C000-")           \
  _T("000000000046}")
#define LENS_HEICIH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.HEIC\\shellex\\{00021500-0000-0000-C000-")           \
  _T("000000000046}")
#define LENS_AVIFIH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.AVIF\\shellex\\{00021500-0000-0000-C000-")           \
  _T("000000000046}")
#define LENS_JXLIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.JXL\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_MP4IH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.MP4\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_AVIIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.AVI\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_MKVIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.MKV\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_WMVIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.WMV\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_PDFIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.PDF\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_TIFFIH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.TIF\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_SVGIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.SVG\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_DNGIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.DNG\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_PSDIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.PSD\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_DDSIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.DDS\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_HDRIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.HDR\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_EXRIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.EXR\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_PPMIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.PPM\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_ICOIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.ICO\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_QOIIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.QOI\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_TGAIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.TGA\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_MP3IH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.MP3\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_TTFIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.TTF\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")
#define LENS_DOCXIH_KEY                                                        \
  _T("SOFTWARE\\Classes\\.DOCX\\shellex\\{00021500-0000-0000-C000-")           \
  _T("000000000046}")
#define LENS_STLIH_KEY                                                         \
  _T("SOFTWARE\\Classes\\.STL\\shellex\\{00021500-0000-0000-C000-")            \
  _T("000000000046}")

// LENS types (must match LENSTYPE in LENSArchive.h)
#define LENS_NONE 0
#define LENS_ZIP 1
#define LENS_CBZ 2
#define LENS_RAR 3
#define LENS_CBR 4
#define LENS_EPUB 5
#define LENS_7Z 6
#define LENS_CB7 7
#define LENS_TAR 8
#define LENS_CBT 9
#define LENS_MOBI 10
#define LENS_FB2 11
#define LENS_AZW 12
#define LENS_AZW3 13
#define LENS_PHZ 14
#define LENS_WEBP 15
#define LENS_HEIF 16
#define LENS_AVIF 17
#define LENS_JXL 18
#define LENS_VIDEO 19
#define LENS_PDF 20
#define LENS_TIFF 45 // Must match LENSTYPE_TIFF in LENSArchive.h
#define LENS_SVG 46  // Must match LENSTYPE_SVG in LENSArchive.h
#define LENS_RAW                                                               \
  47 // Must match LENSTYPE_RAW in LENSArchive.h (DNG, CR2, NEF, ARW, etc.)
#define LENS_PSD 48  // Matches LENSTYPE_PSD — Photoshop
#define LENS_DDS 49  // Matches LENSTYPE_DDS — DirectDraw Surface
#define LENS_HDR 55  // Matches LENSTYPE_HDR — Radiance HDR
#define LENS_EXR 56  // Matches LENSTYPE_EXR — OpenEXR
#define LENS_PPM 57  // Matches LENSTYPE_PPM — Portable Pixmap
#define LENS_FONT 70 // Matches LENSTYPE_FONT — Font thumbnails (TTF, OTF)
#define LENS_ICO 71  // Windows Icon
#define LENS_QOI 72  // Quite OK Image Format
#define LENS_TGA 73  // Targa Image
#define LENS_AUDIO                                                             \
  74 // Audio cover art (MP3, FLAC, etc.) — Note: differs from LENSTYPE_AUDIO=20
	 // (LENS_PDF=20 conflict)
#define LENS_DOCUMENT 75 // Document thumbnails (DOCX, XLSX, PPTX)
#define LENS_MODEL 76    // 3D Model thumbnails (STL, OBJ, PLY)

// Handler status enumeration
enum HandlerStatus {
	HANDLER_NONE = 0,         // No handler registered
	HANDLER_ExplorerLens = 1, // ExplorerLens handler active
	HANDLER_NATIVE = 2,       // Windows native handler detected
	HANDLER_THIRD_PARTY = 3   // Other third-party handler
};

class CRegManager {
public:
	// CRegManager(void){}
	// virtual ~CRegManager(void){}
public:
	///////////////
	// sort option
	BOOL IsSortOpt() {
		DWORD d;
		CRegKey rk;
		if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, LENS_APP_KEY, KEY_READ)) {
			if (ERROR_SUCCESS == rk.QueryDWORDValue(_T("NoSort"), d))
				return (d == FALSE);
		}
		return TRUE;
	}

	void SetSortOpt(BOOL bSort) {
		CRegKey rk;
		if (ERROR_SUCCESS == rk.Create(HKEY_CURRENT_USER, LENS_APP_KEY, NULL,
			REG_OPTION_NON_VOLATILE, KEY_WRITE))
			rk.SetDWORDValue(_T("NoSort"), (DWORD)(bSort ? FALSE : TRUE));
	}

	// sort option
	BOOL IsShowIconOpt() {
		DWORD d;
		CRegKey rk;
		if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, LENS_APP_KEY, KEY_READ)) {
			if (ERROR_SUCCESS == rk.QueryDWORDValue(_T("ShowIcon"), d))
				return (d == TRUE);
		}
		return TRUE;
	}

	//////////////
	// set show archive Icon
	void SetShowIconOpt(BOOL bShowIcon) {
		CRegKey rk;
		if (ERROR_SUCCESS == rk.Create(HKEY_CURRENT_USER, LENS_APP_KEY, NULL,
			REG_OPTION_NON_VOLATILE, KEY_WRITE))
			rk.SetDWORDValue(_T("ShowIcon"), (DWORD)(bShowIcon ? TRUE : FALSE));
	}

	///////////////
	// Collage mode (1=single, 4=2x2, 9=3x3, 16=4x4)
	DWORD GetCollageMode() {
		DWORD mode = 1; // Default: single page
		CRegKey rk;
		if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, LENS_APP_KEY, KEY_READ)) {
			rk.QueryDWORDValue(_T("CollagePages"), mode);
		}
		// Validate mode
		if (mode != 1 && mode != 4 && mode != 9 && mode != 16)
			mode = 1;
		return mode;
	}

	void SetCollageMode(DWORD mode) {
		CRegKey rk;
		if (ERROR_SUCCESS == rk.Create(HKEY_CURRENT_USER, LENS_APP_KEY, NULL,
			REG_OPTION_NON_VOLATILE, KEY_WRITE))
			rk.SetDWORDValue(_T("CollagePages"), mode);
	}

	///////////////////////////////
	// check for thumbnail handlers
	BOOL HasTH(int LENSTYPE) {
		ATLASSERT(LENSTYPE > LENS_NONE);
		ULONG n = LENS_GUID_KEY_SLEN;
		TCHAR s[LENS_GUID_KEY_SLEN];

		CRegKey rk;
		if (ERROR_SUCCESS ==
			rk.Open(HKEY_CURRENT_USER, GetTHKeyName(LENSTYPE), KEY_READ)) {
			if (ERROR_SUCCESS == rk.QueryStringValue(NULL, s, &n)) {
				if (0 == StrCmpI(s, LENS_GUID_KEY))
					return TRUE;
			}
		}
		return FALSE;
	}

	/////////////////////////////
	// check for infotip handlers
	BOOL HasIH(int LENSTYPE) {
		ATLASSERT(LENSTYPE > LENS_NONE);
		ULONG n = LENS_GUID_KEY_SLEN;
		TCHAR s[LENS_GUID_KEY_SLEN];

		CRegKey rk;
		if (ERROR_SUCCESS ==
			rk.Open(HKEY_CURRENT_USER, GetIHKeyName(LENSTYPE), KEY_READ)) {
			if (ERROR_SUCCESS == rk.QueryStringValue(NULL, s, &n)) {
				if (StrCmpI(s, LENS_GUID_KEY) == 0)
					return TRUE;
			}
		}
		return FALSE;
	}

	/////////////////////////
	// Get handler status for a specific format
	// Returns: HANDLER_NONE, HANDLER_ExplorerLens, HANDLER_NATIVE, or
	// HANDLER_THIRD_PARTY
	HandlerStatus GetHandlerStatus(int LENSTYPE, LPCTSTR extension) {
		ATLASSERT(LENSTYPE > LENS_NONE);

		TCHAR guid[256] = { 0 };
		ULONG len = 256;
		CRegKey rk;

		// First check HKCU (user-specific - where ExplorerLens registers)
		if (ERROR_SUCCESS ==
			rk.Open(HKEY_CURRENT_USER, GetTHKeyName(LENSTYPE), KEY_READ)) {
			if (ERROR_SUCCESS == rk.QueryStringValue(NULL, guid, &len)) {
				if (StrCmpI(guid, LENS_GUID_KEY) == 0) {
					rk.Close();
					return HANDLER_ExplorerLens;
				}
			}
			rk.Close();
		}

		// Check HKCR (system-wide handlers)
		CString regPath;
		regPath.Format(_T("%s\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
			extension);

		len = 256;
		ZeroMemory(guid, sizeof(guid));

		if (ERROR_SUCCESS == rk.Open(HKEY_CLASSES_ROOT, regPath, KEY_READ)) {
			if (ERROR_SUCCESS == rk.QueryStringValue(NULL, guid, &len)) {
				// Check for known Windows native handler GUIDs

				// Windows Media Foundation (video files)
				if (StrStrI(guid, _T("c5aec3ec-e812-4677-a9a7-4fee1f9aa000")) != NULL) {
					rk.Close();
					return HANDLER_NATIVE;
				}

				// Windows PDF handler
				if (StrStrI(guid, _T("DC6EFB56-9CFA-464D-8880-44885D7DC193")) != NULL) {
					rk.Close();
					return HANDLER_NATIVE;
				}

				// Windows Photo Viewer (various image formats)
				if (StrStrI(guid, _T("C7657C4A-9F70-11D0-A999-00C04FD655E1")) != NULL) {
					rk.Close();
					return HANDLER_NATIVE;
				}

				// Windows Imaging Component
				if (StrStrI(guid, _T("7D2B9654-0AE1-4BBD-BD42-7A0C3A23E787")) != NULL) {
					rk.Close();
					return HANDLER_NATIVE;
				}

				// Not our GUID and not a known Windows handler
				if (StrCmpI(guid, LENS_GUID_KEY) != 0) {
					rk.Close();
					return HANDLER_THIRD_PARTY;
				}
			}
			rk.Close();
		}

		return HANDLER_NONE;
	}

	/////////////////////////
	// Registry Backup System
	// Backup existing handler GUID before installation
	BOOL BackupHandler(int LENSTYPE, LPCTSTR keyPath, LPCTSTR backupName) {
		TCHAR existingGuid[256] = { 0 };
		ULONG len = 256;
		CRegKey rk;

		// Read existing handler GUID
		if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, keyPath, KEY_READ)) {
			if (ERROR_SUCCESS == rk.QueryStringValue(NULL, existingGuid, &len)) {
				// Don't backup if it's already our GUID
				if (StrCmpI(existingGuid, LENS_GUID_KEY) == 0) {
					rk.Close();
					return FALSE; // No backup needed
				}

				// Store backup
				CString backupKey;
				backupKey.Format(_T("%s\\Backups\\%s"), LENS_APP_KEY, backupName);

				CRegKey rkBackup;
				if (ERROR_SUCCESS == rkBackup.Create(HKEY_CURRENT_USER, backupKey, NULL,
					REG_OPTION_NON_VOLATILE,
					KEY_WRITE)) {
					rkBackup.SetStringValue(NULL, existingGuid);
					rkBackup.Close();
					rk.Close();
					return TRUE; // Backup successful
				}
			}
			rk.Close();
		}
		return FALSE; // Nothing to backup
	}

	/////////////////////////
	// Restore backed-up handler
	BOOL RestoreHandler(LPCTSTR keyPath, LPCTSTR backupName) {
		CString backupKey;
		backupKey.Format(_T("%s\\Backups\\%s"), LENS_APP_KEY, backupName);

		TCHAR backedUpGuid[256] = { 0 };
		ULONG len = 256;
		CRegKey rkBackup;

		// Read backup
		if (ERROR_SUCCESS ==
			rkBackup.Open(HKEY_CURRENT_USER, backupKey, KEY_READ)) {
			if (ERROR_SUCCESS ==
				rkBackup.QueryStringValue(NULL, backedUpGuid, &len)) {
				// Restore the backed-up handler
				CRegKey rk;
				if (ERROR_SUCCESS == rk.Create(HKEY_CURRENT_USER, keyPath, NULL,
					REG_OPTION_NON_VOLATILE, KEY_WRITE)) {
					rk.SetStringValue(NULL, backedUpGuid);
					rk.Close();
				}

				// Delete backup entry
				rkBackup.Close();
				RegDeleteKey(HKEY_CURRENT_USER, backupKey);
				return TRUE;
			}
			rkBackup.Close();
		}
		return FALSE; // No backup to restore
	}

	/////////////////////////
	// Get program name from handler GUID
	CString GetHandlerProgramName(LPCTSTR guid) {
		if (!guid || guid[0] == 0)
			return _T("None");

		// Check for known handler GUIDs first
		if (StrCmpI(guid, LENS_GUID_KEY) == 0)
			return _T("ExplorerLens");
		if (StrStrI(guid, _T("c5aec3ec-e812-4677-a9a7-4fee1f9aa000")) != NULL)
			return _T("Windows Media Foundation");
		if (StrStrI(guid, _T("DC6EFB56-9CFA-464D-8880-44885D7DC193")) != NULL)
			return _T("Windows PDF Handler");
		if (StrStrI(guid, _T("C7657C4A-9F70-11D0-A999-00C04FD655E1")) != NULL)
			return _T("Windows Photo Viewer");
		if (StrStrI(guid, _T("7D2B9654-0AE1-4BBD-BD42-7A0C3A23E787")) != NULL)
			return _T("Windows Imaging Component");

		// Try to resolve CLSID to program name
		CString clsidKey;
		clsidKey.Format(_T("CLSID\\%s\\InprocServer32"), guid);

		TCHAR dllPath[MAX_PATH] = { 0 };
		ULONG len = MAX_PATH;
		CRegKey rk;

		if (ERROR_SUCCESS == rk.Open(HKEY_CLASSES_ROOT, clsidKey, KEY_READ)) {
			if (ERROR_SUCCESS == rk.QueryStringValue(NULL, dllPath, &len)) {
				// Extract filename from path
				LPCTSTR filename = PathFindFileName(dllPath);
				if (filename && filename[0]) {
					// Remove .dll extension
					CString name(filename);
					int dotPos = name.ReverseFind('.');
					if (dotPos > 0)
						name = name.Left(dotPos);

					rk.Close();
					return name;
				}
			}
			rk.Close();
		}

		// Return short GUID if we can't resolve
		CString shortGuid(guid);
		if (shortGuid.GetLength() > 12)
			shortGuid = shortGuid.Left(12) + _T("...");
		return shortGuid;
	}

	/////////////////////////
	// Enhanced handler status with program info
	// Returns status and fills programName with handler application name
	HandlerStatus GetHandlerStatusEx(int LENSTYPE, LPCTSTR extension,
		CString& programName) {
		ATLASSERT(LENSTYPE > LENS_NONE);

		TCHAR guid[256] = { 0 };
		ULONG len = 256;
		CRegKey rk;

		programName = _T("None");

		// First check HKCU (user-specific - where ExplorerLens registers)
		if (ERROR_SUCCESS ==
			rk.Open(HKEY_CURRENT_USER, GetTHKeyName(LENSTYPE), KEY_READ)) {
			if (ERROR_SUCCESS == rk.QueryStringValue(NULL, guid, &len)) {
				programName = GetHandlerProgramName(guid);
				rk.Close();

				if (StrCmpI(guid, LENS_GUID_KEY) == 0)
					return HANDLER_ExplorerLens;
				else
					return HANDLER_THIRD_PARTY;
			}
			rk.Close();
		}

		// Check HKCR (system-wide handlers)
		CString regPath;
		regPath.Format(_T("%s\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
			extension);

		len = 256;
		ZeroMemory(guid, sizeof(guid));

		if (ERROR_SUCCESS == rk.Open(HKEY_CLASSES_ROOT, regPath, KEY_READ)) {
			if (ERROR_SUCCESS == rk.QueryStringValue(NULL, guid, &len)) {
				programName = GetHandlerProgramName(guid);
				rk.Close();

				// Check for known Windows native handler GUIDs
				if (StrStrI(guid, _T("c5aec3ec-e812-4677-a9a7-4fee1f9aa000")) != NULL ||
					StrStrI(guid, _T("DC6EFB56-9CFA-464D-8880-44885D7DC193")) != NULL ||
					StrStrI(guid, _T("C7657C4A-9F70-11D0-A999-00C04FD655E1")) != NULL ||
					StrStrI(guid, _T("7D2B9654-0AE1-4BBD-BD42-7A0C3A23E787")) != NULL) {
					return HANDLER_NATIVE;
				}

				// Not our GUID and not a known Windows handler
				if (StrCmpI(guid, LENS_GUID_KEY) != 0)
					return HANDLER_THIRD_PARTY;
			}
			rk.Close();
		}

		programName = _T("None");
		return HANDLER_NONE;
	}

	/////////////////////////
	// Get extension for a LENS type (for status detection)
	LPCTSTR GetExtension(int LENSTYPE) {
		switch (LENSTYPE) {
		case LENS_ZIP:
			return _T(".zip");
		case LENS_CBZ:
			return _T(".cbz");
		case LENS_RAR:
			return _T(".rar");
		case LENS_CBR:
			return _T(".cbr");
		case LENS_EPUB:
			return _T(".epub");
		case LENS_7Z:
			return _T(".7z");
		case LENS_CB7:
			return _T(".cb7");
		case LENS_TAR:
			return _T(".tar");
		case LENS_CBT:
			return _T(".cbt");
		case LENS_MOBI:
			return _T(".mobi");
		case LENS_FB2:
			return _T(".fb2");
		case LENS_AZW:
			return _T(".azw");
		case LENS_AZW3:
			return _T(".azw3");
		case LENS_PHZ:
			return _T(".phz");
		case LENS_WEBP:
			return _T(".webp");
		case LENS_HEIF:
			return _T(".heif");
		case LENS_AVIF:
			return _T(".avif");
		case LENS_JXL:
			return _T(".jxl");
		case LENS_VIDEO:
			return _T(".mp4"); // Representative video format
		case LENS_PDF:
			return _T(".pdf");
		case LENS_TIFF:
			return _T(".tif");
		case LENS_SVG:
			return _T(".svg");
		case LENS_RAW:
			return _T(".dng"); // Representative RAW format (DNG, CR2, NEF, ARW)
		default:
			break;
		}
		return NULL;
	}

	/////////////////////////
	// Set thumbnail / infotip handlers with backup/restore support
	void SetHandlers(int LENSTYPE, BOOL bSet) {
		ATLASSERT(LENSTYPE > LENS_NONE);

		// Special handling for multi-extension formats (VIDEO, HEIF, TIFF, RAW)
		if (LENSTYPE == LENS_VIDEO) {
			// Video formats: .mp4, .avi, .mkv, .mov, .wmv, .flv, .webm, .m4v, .mpg,
			// .mpeg
			const LPCTSTR videoExts[] = { LENS_MP4TH_KEY,
										 LENS_AVITH_KEY,
										 LENS_MKVTH_KEY,
										 _T("SOFTWARE\\Classes\\.MOV\\shellex\\{")
										 _T("BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
										 LENS_WMVTH_KEY,
										 _T("SOFTWARE\\Classes\\.FLV\\shellex\\{")
										 _T("BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
										 _T("SOFTWARE\\Classes\\.WEBM\\shellex\\{")
										 _T("BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
										 _T("SOFTWARE\\Classes\\.M4V\\shellex\\{")
										 _T("BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
										 _T("SOFTWARE\\Classes\\.MPG\\shellex\\{")
										 _T("BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
										 _T("SOFTWARE\\Classes\\.MPEG\\shellex\\{")
										 _T("BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}") };
			const LPCTSTR videoInfoTips[] = {
				LENS_MP4IH_KEY,
				LENS_AVIIH_KEY,
				LENS_MKVIH_KEY,
				_T("SOFTWARE\\Classes\\.MOV\\shellex\\{00021500-0000-0000-C000-")
				_T("000000000046}"),
				LENS_WMVIH_KEY,
				_T("SOFTWARE\\Classes\\.FLV\\shellex\\{00021500-0000-0000-C000-")
				_T("000000000046}"),
				_T("SOFTWARE\\Classes\\.WEBM\\shellex\\{00021500-0000-0000-C000-")
				_T("000000000046}"),
				_T("SOFTWARE\\Classes\\.M4V\\shellex\\{00021500-0000-0000-C000-")
				_T("000000000046}"),
				_T("SOFTWARE\\Classes\\.MPG\\shellex\\{00021500-0000-0000-C000-")
				_T("000000000046}"),
				_T("SOFTWARE\\Classes\\.MPEG\\shellex\\{00021500-0000-0000-C000-")
				_T("000000000046}") };
			const LPCTSTR backupNames[] = { _T("mp4_th"),  _T("avi_th"), _T("mkv_th"),
										   _T("mov_th"),  _T("wmv_th"), _T("flv_th"),
										   _T("webm_th"), _T("m4v_th"), _T("mpg_th"),
										   _T("mpeg_th") };
			const LPCTSTR backupNamesInfo[] = {
				_T("mp4_ih"), _T("avi_ih"), _T("mkv_ih"),  _T("mov_ih"),
				_T("wmv_ih"), _T("flv_ih"), _T("webm_ih"), _T("m4v_ih"),
				_T("mpg_ih"), _T("mpeg_ih") };

			for (int i = 0; i < _countof(videoExts); i++) {
				if (bSet) {
					// Backup existing handlers before overwriting
					BackupHandler(LENSTYPE, videoExts[i], backupNames[i]);
					BackupHandler(LENSTYPE, videoInfoTips[i], backupNamesInfo[i]);

					CRegKey rkt, rki;
					if (ERROR_SUCCESS == rkt.Create(HKEY_CURRENT_USER, videoExts[i], NULL,
						REG_OPTION_NON_VOLATILE, KEY_WRITE))
						rkt.SetStringValue(NULL, LENS_GUID_KEY);
					if (ERROR_SUCCESS == rki.Create(HKEY_CURRENT_USER, videoInfoTips[i],
						NULL, REG_OPTION_NON_VOLATILE,
						KEY_WRITE))
						rki.SetStringValue(NULL, LENS_GUID_KEY);
				}
				else {
					// Smart uninstall - only remove if it's our handler
					TCHAR currentGuid[256] = { 0 };
					ULONG len = 256;
					CRegKey rk;

					// Check thumbnail handler
					if (ERROR_SUCCESS ==
						rk.Open(HKEY_CURRENT_USER, videoExts[i], KEY_READ)) {
						if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len)) {
							if (StrCmpI(currentGuid, LENS_GUID_KEY) == 0) {
								rk.Close();
								// It's our handler - remove or restore backup
								if (!RestoreHandler(videoExts[i], backupNames[i]))
									RegDeleteKey(HKEY_CURRENT_USER, videoExts[i]);
							}
							else {
								rk.Close(); // Not our handler, don't touch it
							}
						}
						else
							rk.Close();
					}

					// Check infotip handler
					len = 256;
					ZeroMemory(currentGuid, sizeof(currentGuid));
					if (ERROR_SUCCESS ==
						rk.Open(HKEY_CURRENT_USER, videoInfoTips[i], KEY_READ)) {
						if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len)) {
							if (StrCmpI(currentGuid, LENS_GUID_KEY) == 0) {
								rk.Close();
								if (!RestoreHandler(videoInfoTips[i], backupNamesInfo[i]))
									RegDeleteKey(HKEY_CURRENT_USER, videoInfoTips[i]);
							}
							else {
								rk.Close();
							}
						}
						else
							rk.Close();
					}
				}
			}
			return;
		}
		else if (LENSTYPE == LENS_HEIF) {
			// HEIF formats: .heif, .heic
			const LPCTSTR heifExts[] = { LENS_HEIFTH_KEY, LENS_HEICTH_KEY };
			const LPCTSTR heifInfoTips[] = { LENS_HEIFIH_KEY, LENS_HEICIH_KEY };
			const LPCTSTR backupNames[] = { _T("heif_th"), _T("heic_th") };
			const LPCTSTR backupNamesInfo[] = { _T("heif_ih"), _T("heic_ih") };

			for (int i = 0; i < _countof(heifExts); i++) {
				if (bSet) {
					BackupHandler(LENSTYPE, heifExts[i], backupNames[i]);
					BackupHandler(LENSTYPE, heifInfoTips[i], backupNamesInfo[i]);

					CRegKey rkt, rki;
					if (ERROR_SUCCESS == rkt.Create(HKEY_CURRENT_USER, heifExts[i], NULL,
						REG_OPTION_NON_VOLATILE, KEY_WRITE))
						rkt.SetStringValue(NULL, LENS_GUID_KEY);
					if (ERROR_SUCCESS == rki.Create(HKEY_CURRENT_USER, heifInfoTips[i],
						NULL, REG_OPTION_NON_VOLATILE,
						KEY_WRITE))
						rki.SetStringValue(NULL, LENS_GUID_KEY);
				}
				else {
					// Smart uninstall
					TCHAR currentGuid[256] = { 0 };
					ULONG len = 256;
					CRegKey rk;

					if (ERROR_SUCCESS ==
						rk.Open(HKEY_CURRENT_USER, heifExts[i], KEY_READ)) {
						if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len)) {
							if (StrCmpI(currentGuid, LENS_GUID_KEY) == 0) {
								rk.Close();
								if (!RestoreHandler(heifExts[i], backupNames[i]))
									RegDeleteKey(HKEY_CURRENT_USER, heifExts[i]);
							}
							else
								rk.Close();
						}
						else
							rk.Close();
					}

					len = 256;
					ZeroMemory(currentGuid, sizeof(currentGuid));
					if (ERROR_SUCCESS ==
						rk.Open(HKEY_CURRENT_USER, heifInfoTips[i], KEY_READ)) {
						if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len)) {
							if (StrCmpI(currentGuid, LENS_GUID_KEY) == 0) {
								rk.Close();
								if (!RestoreHandler(heifInfoTips[i], backupNamesInfo[i]))
									RegDeleteKey(HKEY_CURRENT_USER, heifInfoTips[i]);
							}
							else
								rk.Close();
						}
						else
							rk.Close();
					}
				}
			}
			return;
		}
		else if (LENSTYPE == LENS_TIFF) {
			// TIFF formats: .tif, .tiff
			const LPCTSTR tiffExts[] = { LENS_TIFFTH_KEY,
										_T("SOFTWARE\\Classes\\.TIFF\\shellex\\{")
										_T("BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}") };
			const LPCTSTR tiffInfoTips[] = {
				LENS_TIFFIH_KEY, _T("SOFTWARE\\Classes\\.TIFF\\shellex\\{00021500-")
								 _T("0000-0000-C000-000000000046}") };
			const LPCTSTR backupNames[] = { _T("tif_th"), _T("tiff_th") };
			const LPCTSTR backupNamesInfo[] = { _T("tif_ih"), _T("tiff_ih") };

			for (int i = 0; i < _countof(tiffExts); i++) {
				if (bSet) {
					BackupHandler(LENSTYPE, tiffExts[i], backupNames[i]);
					BackupHandler(LENSTYPE, tiffInfoTips[i], backupNamesInfo[i]);

					CRegKey rkt, rki;
					if (ERROR_SUCCESS == rkt.Create(HKEY_CURRENT_USER, tiffExts[i], NULL,
						REG_OPTION_NON_VOLATILE, KEY_WRITE))
						rkt.SetStringValue(NULL, LENS_GUID_KEY);
					if (ERROR_SUCCESS == rki.Create(HKEY_CURRENT_USER, tiffInfoTips[i],
						NULL, REG_OPTION_NON_VOLATILE,
						KEY_WRITE))
						rki.SetStringValue(NULL, LENS_GUID_KEY);
				}
				else {
					// Smart uninstall
					TCHAR currentGuid[256] = { 0 };
					ULONG len = 256;
					CRegKey rk;

					if (ERROR_SUCCESS ==
						rk.Open(HKEY_CURRENT_USER, tiffExts[i], KEY_READ)) {
						if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len)) {
							if (StrCmpI(currentGuid, LENS_GUID_KEY) == 0) {
								rk.Close();
								if (!RestoreHandler(tiffExts[i], backupNames[i]))
									RegDeleteKey(HKEY_CURRENT_USER, tiffExts[i]);
							}
							else
								rk.Close();
						}
						else
							rk.Close();
					}

					len = 256;
					ZeroMemory(currentGuid, sizeof(currentGuid));
					if (ERROR_SUCCESS ==
						rk.Open(HKEY_CURRENT_USER, tiffInfoTips[i], KEY_READ)) {
						if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len)) {
							if (StrCmpI(currentGuid, LENS_GUID_KEY) == 0) {
								rk.Close();
								if (!RestoreHandler(tiffInfoTips[i], backupNamesInfo[i]))
									RegDeleteKey(HKEY_CURRENT_USER, tiffInfoTips[i]);
							}
							else
								rk.Close();
						}
						else
							rk.Close();
					}
				}
			}
			return;
		}
		else if (LENSTYPE == LENS_RAW) {
			// RAW formats: .dng, .cr2, .cr3, .nef, .arw, .orf
			const LPCTSTR rawExts[] = { LENS_DNGTH_KEY,
									   _T("SOFTWARE\\Classes\\.CR2\\shellex\\{")
									   _T("BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
									   _T("SOFTWARE\\Classes\\.CR3\\shellex\\{")
									   _T("BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
									   _T("SOFTWARE\\Classes\\.NEF\\shellex\\{")
									   _T("BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
									   _T("SOFTWARE\\Classes\\.ARW\\shellex\\{")
									   _T("BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
									   _T("SOFTWARE\\Classes\\.ORF\\shellex\\{")
									   _T("BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}") };
			const LPCTSTR rawInfoTips[] = {
				LENS_DNGIH_KEY,
				_T("SOFTWARE\\Classes\\.CR2\\shellex\\{00021500-0000-0000-C000-")
				_T("000000000046}"),
				_T("SOFTWARE\\Classes\\.CR3\\shellex\\{00021500-0000-0000-C000-")
				_T("000000000046}"),
				_T("SOFTWARE\\Classes\\.NEF\\shellex\\{00021500-0000-0000-C000-")
				_T("000000000046}"),
				_T("SOFTWARE\\Classes\\.ARW\\shellex\\{00021500-0000-0000-C000-")
				_T("000000000046}"),
				_T("SOFTWARE\\Classes\\.ORF\\shellex\\{00021500-0000-0000-C000-")
				_T("000000000046}") };
			const LPCTSTR backupNames[] = { _T("dng_th"), _T("cr2_th"), _T("cr3_th"),
										   _T("nef_th"), _T("arw_th"), _T("orf_th") };
			const LPCTSTR backupNamesInfo[] = { _T("dng_ih"), _T("cr2_ih"),
											   _T("cr3_ih"), _T("nef_ih"),
											   _T("arw_ih"), _T("orf_ih") };

			for (int i = 0; i < _countof(rawExts); i++) {
				if (bSet) {
					BackupHandler(LENSTYPE, rawExts[i], backupNames[i]);
					BackupHandler(LENSTYPE, rawInfoTips[i], backupNamesInfo[i]);

					CRegKey rkt, rki;
					if (ERROR_SUCCESS == rkt.Create(HKEY_CURRENT_USER, rawExts[i], NULL,
						REG_OPTION_NON_VOLATILE, KEY_WRITE))
						rkt.SetStringValue(NULL, LENS_GUID_KEY);
					if (ERROR_SUCCESS == rki.Create(HKEY_CURRENT_USER, rawInfoTips[i],
						NULL, REG_OPTION_NON_VOLATILE,
						KEY_WRITE))
						rki.SetStringValue(NULL, LENS_GUID_KEY);
				}
				else {
					// Smart uninstall
					TCHAR currentGuid[256] = { 0 };
					ULONG len = 256;
					CRegKey rk;

					if (ERROR_SUCCESS ==
						rk.Open(HKEY_CURRENT_USER, rawExts[i], KEY_READ)) {
						if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len)) {
							if (StrCmpI(currentGuid, LENS_GUID_KEY) == 0) {
								rk.Close();
								if (!RestoreHandler(rawExts[i], backupNames[i]))
									RegDeleteKey(HKEY_CURRENT_USER, rawExts[i]);
							}
							else
								rk.Close();
						}
						else
							rk.Close();
					}

					len = 256;
					ZeroMemory(currentGuid, sizeof(currentGuid));
					if (ERROR_SUCCESS ==
						rk.Open(HKEY_CURRENT_USER, rawInfoTips[i], KEY_READ)) {
						if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len)) {
							if (StrCmpI(currentGuid, LENS_GUID_KEY) == 0) {
								rk.Close();
								if (!RestoreHandler(rawInfoTips[i], backupNamesInfo[i]))
									RegDeleteKey(HKEY_CURRENT_USER, rawInfoTips[i]);
							}
							else
								rk.Close();
						}
						else
							rk.Close();
					}
				}
			}
			return;
		}

		// Standard single-extension formats
		CString backupTH, backupIH;
		backupTH.Format(_T("%s_th"), GetExtension(LENSTYPE));
		backupIH.Format(_T("%s_ih"), GetExtension(LENSTYPE));

		if (bSet) {
			// Backup before overwriting
			BackupHandler(LENSTYPE, GetTHKeyName(LENSTYPE), backupTH);
			BackupHandler(LENSTYPE, GetIHKeyName(LENSTYPE), backupIH);

			// thumbnail
			CRegKey rkt, rki;
			if (ERROR_SUCCESS == rkt.Create(HKEY_CURRENT_USER, GetTHKeyName(LENSTYPE),
				NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
				rkt.SetStringValue(NULL, LENS_GUID_KEY);
			// infotip
			if (ERROR_SUCCESS == rki.Create(HKEY_CURRENT_USER, GetIHKeyName(LENSTYPE),
				NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
				rki.SetStringValue(NULL, LENS_GUID_KEY);
		}
		else {
			// Smart uninstall - only remove if it's our handler
			TCHAR currentGuid[256] = { 0 };
			ULONG len = 256;
			CRegKey rk;

			// thumbnail
			if (HasTH(LENSTYPE)) {
				if (ERROR_SUCCESS ==
					rk.Open(HKEY_CURRENT_USER, GetTHKeyName(LENSTYPE), KEY_READ)) {
					if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len)) {
						if (StrCmpI(currentGuid, LENS_GUID_KEY) == 0) {
							rk.Close();
							if (!RestoreHandler(GetTHKeyName(LENSTYPE), backupTH))
								RegDeleteKey(HKEY_CURRENT_USER, GetTHKeyName(LENSTYPE));
						}
						else
							rk.Close();
					}
					else
						rk.Close();
				}
			}

			// infotip
			if (HasIH(LENSTYPE)) {
				len = 256;
				ZeroMemory(currentGuid, sizeof(currentGuid));
				if (ERROR_SUCCESS ==
					rk.Open(HKEY_CURRENT_USER, GetIHKeyName(LENSTYPE), KEY_READ)) {
					if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len)) {
						if (StrCmpI(currentGuid, LENS_GUID_KEY) == 0) {
							rk.Close();
							if (!RestoreHandler(GetIHKeyName(LENSTYPE), backupIH))
								RegDeleteKey(HKEY_CURRENT_USER, GetIHKeyName(LENSTYPE));
						}
						else
							rk.Close();
					}
					else
						rk.Close();
				}
			}
		}
	}

	// get handler reg key names
	LPCTSTR GetTHKeyName(int LENSTYPE) {
		ATLASSERT(LENSTYPE > LENS_NONE);
		switch (LENSTYPE) {
		case LENS_ZIP:
			return LENS_ZIPTH_KEY;
		case LENS_CBZ:
			return LENS_CBZTH_KEY;
		case LENS_EPUB:
			return LENS_EPUBTH_KEY;
		case LENS_RAR:
			return LENS_RARTH_KEY;
		case LENS_CBR:
			return LENS_CBRTH_KEY;
		case LENS_7Z:
			return LENS_7ZTH_KEY;
		case LENS_CB7:
			return LENS_CB7TH_KEY;
		case LENS_TAR:
			return LENS_TARTH_KEY;
		case LENS_CBT:
			return LENS_CBTTH_KEY;
		case LENS_MOBI:
			return LENS_MOBITH_KEY;
		case LENS_FB2:
			return LENS_FB2TH_KEY;
		case LENS_AZW:
			return LENS_AZWTH_KEY;
		case LENS_AZW3:
			return LENS_AZW3TH_KEY;
		case LENS_PHZ:
			return LENS_PHZTH_KEY;
		case LENS_WEBP:
			return LENS_WEBPTH_KEY;
		case LENS_HEIF:
			return LENS_HEIFTH_KEY;
		case LENS_AVIF:
			return LENS_AVIFTH_KEY;
		case LENS_JXL:
			return LENS_JXLTH_KEY;
		case LENS_VIDEO:
			return LENS_MP4TH_KEY; // Primary video format
		case LENS_PDF:
			return LENS_PDFTH_KEY;
		case LENS_TIFF:
			return LENS_TIFFTH_KEY;
		case LENS_SVG:
			return LENS_SVGTH_KEY;
		case LENS_RAW:
			return LENS_DNGTH_KEY;
		case LENS_PSD:
			return LENS_PSDTH_KEY;
		case LENS_DDS:
			return LENS_DDSTH_KEY;
		case LENS_HDR:
			return LENS_HDRTH_KEY;
		case LENS_EXR:
			return LENS_EXRTH_KEY;
		case LENS_PPM:
			return LENS_PPMTH_KEY;
		case LENS_ICO:
			return LENS_ICOTH_KEY;
		case LENS_QOI:
			return LENS_QOITH_KEY;
		case LENS_TGA:
			return LENS_TGATH_KEY;
		case LENS_AUDIO:
			return LENS_MP3TH_KEY; // Primary audio format
		case LENS_FONT:
			return LENS_TTFTH_KEY; // Primary font format
		case LENS_DOCUMENT:
			return LENS_DOCXTH_KEY; // Primary document format
		case LENS_MODEL:
			return LENS_STLTH_KEY; // Primary 3D model format
		default:
			break;
		}
		return NULL;
	}
	LPCTSTR GetIHKeyName(int LENSTYPE) {
		ATLASSERT(LENSTYPE > LENS_NONE);
		switch (LENSTYPE) {
		case LENS_ZIP:
			return LENS_ZIPIH_KEY;
		case LENS_CBZ:
			return LENS_CBZIH_KEY;
		case LENS_EPUB:
			return LENS_EPUBIH_KEY;
		case LENS_RAR:
			return LENS_RARIH_KEY;
		case LENS_CBR:
			return LENS_CBRIH_KEY;
		case LENS_7Z:
			return LENS_7ZIH_KEY;
		case LENS_CB7:
			return LENS_CB7IH_KEY;
		case LENS_TAR:
			return LENS_TARIH_KEY;
		case LENS_CBT:
			return LENS_CBTIH_KEY;
		case LENS_MOBI:
			return LENS_MOBIIH_KEY;
		case LENS_FB2:
			return LENS_FB2IH_KEY;
		case LENS_AZW:
			return LENS_AZWIH_KEY;
		case LENS_AZW3:
			return LENS_AZW3IH_KEY;
		case LENS_PHZ:
			return LENS_PHZIH_KEY;
		case LENS_WEBP:
			return LENS_WEBPIH_KEY;
		case LENS_HEIF:
			return LENS_HEIFIH_KEY;
		case LENS_AVIF:
			return LENS_AVIFIH_KEY;
		case LENS_JXL:
			return LENS_JXLIH_KEY;
		case LENS_VIDEO:
			return LENS_MP4IH_KEY; // Primary video format
		case LENS_PDF:
			return LENS_PDFIH_KEY;
		case LENS_TIFF:
			return LENS_TIFFIH_KEY;
		case LENS_SVG:
			return LENS_SVGIH_KEY;
		case LENS_RAW:
			return LENS_DNGIH_KEY;
		case LENS_PSD:
			return LENS_PSDIH_KEY;
		case LENS_DDS:
			return LENS_DDSIH_KEY;
		case LENS_HDR:
			return LENS_HDRIH_KEY;
		case LENS_EXR:
			return LENS_EXRIH_KEY;
		case LENS_PPM:
			return LENS_PPMIH_KEY;
		case LENS_ICO:
			return LENS_ICOIH_KEY;
		case LENS_QOI:
			return LENS_QOIIH_KEY;
		case LENS_TGA:
			return LENS_TGAIH_KEY;
		case LENS_AUDIO:
			return LENS_MP3IH_KEY;
		case LENS_FONT:
			return LENS_TTFIH_KEY;
		case LENS_DOCUMENT:
			return LENS_DOCXIH_KEY;
		case LENS_MODEL:
			return LENS_STLIH_KEY;
		default:
			break;
		}
		return NULL;
	}
};

#endif //_REGMANAGER_79AE66E4_84E2_45A1_BF4F_43AA714BE55F_
