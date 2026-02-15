#ifndef _REGMANAGER_79AE66E4_84E2_45A1_BF4F_43AA714BE55F_
#define _REGMANAGER_79AE66E4_84E2_45A1_BF4F_43AA714BE55F_
#pragma once

#include <windows.h>
#pragma comment(lib,"advapi32.lib")
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#include <atlstr.h>  // For CString


#define CBX_MUTEX_GLOBAL	L"Global\\{64DEE47D-9669-4430-9D5C-304867F87B51}"
#define CBX_MGRMUTEX		L"Local\\{50D9CBE6-C168-4901-8CC9-2A7C97E558F7}"


#define CBX_GUID_KEY _T("{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}") //38+1 TCHAR
#define CBX_GUID_KEY_SLEN 39
#define CBX_APP_KEY _T("Software\\T800 Productions\\{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}")

// per-user settings (HKCU)
// thumbnail handler keys
#define CBX_ZIPTH_KEY _T("SOFTWARE\\Classes\\.ZIP\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_CBZTH_KEY _T("SOFTWARE\\Classes\\.CBZ\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_RARTH_KEY _T("SOFTWARE\\Classes\\.RAR\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_CBRTH_KEY _T("SOFTWARE\\Classes\\.CBR\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_EPUBTH_KEY _T("SOFTWARE\\Classes\\.EPUB\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_7ZTH_KEY _T("SOFTWARE\\Classes\\.7Z\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_CB7TH_KEY _T("SOFTWARE\\Classes\\.CB7\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_TARTH_KEY _T("SOFTWARE\\Classes\\.TAR\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_CBTTH_KEY _T("SOFTWARE\\Classes\\.CBT\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_MOBITH_KEY _T("SOFTWARE\\Classes\\.MOBI\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_FB2TH_KEY _T("SOFTWARE\\Classes\\.FB2\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_AZWTH_KEY _T("SOFTWARE\\Classes\\.AZW\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_AZW3TH_KEY _T("SOFTWARE\\Classes\\.AZW3\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_PHZTH_KEY _T("SOFTWARE\\Classes\\.PHZ\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_WEBPTH_KEY _T("SOFTWARE\\Classes\\.WEBP\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_HEIFTH_KEY _T("SOFTWARE\\Classes\\.HEIF\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_HEICTH_KEY _T("SOFTWARE\\Classes\\.HEIC\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_AVIFTH_KEY _T("SOFTWARE\\Classes\\.AVIF\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_JXLTH_KEY _T("SOFTWARE\\Classes\\.JXL\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_MP4TH_KEY _T("SOFTWARE\\Classes\\.MP4\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_AVITH_KEY _T("SOFTWARE\\Classes\\.AVI\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_MKVTH_KEY _T("SOFTWARE\\Classes\\.MKV\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_WMVTH_KEY _T("SOFTWARE\\Classes\\.WMV\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_PDFTH_KEY _T("SOFTWARE\\Classes\\.PDF\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_TIFFTH_KEY _T("SOFTWARE\\Classes\\.TIF\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_SVGTH_KEY _T("SOFTWARE\\Classes\\.SVG\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
#define CBX_DNGTH_KEY _T("SOFTWARE\\Classes\\.DNG\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
// infotip handler keys
#define CBX_ZIPIH_KEY _T("SOFTWARE\\Classes\\.ZIP\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_CBZIH_KEY _T("SOFTWARE\\Classes\\.CBZ\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_RARIH_KEY _T("SOFTWARE\\Classes\\.RAR\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_CBRIH_KEY _T("SOFTWARE\\Classes\\.CBR\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_EPUBIH_KEY _T("SOFTWARE\\Classes\\.EPUB\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_7ZIH_KEY _T("SOFTWARE\\Classes\\.7Z\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_CB7IH_KEY _T("SOFTWARE\\Classes\\.CB7\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_TARIH_KEY _T("SOFTWARE\\Classes\\.TAR\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_CBTIH_KEY _T("SOFTWARE\\Classes\\.CBT\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_MOBIIH_KEY _T("SOFTWARE\\Classes\\.MOBI\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_FB2IH_KEY _T("SOFTWARE\\Classes\\.FB2\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_AZWIH_KEY _T("SOFTWARE\\Classes\\.AZW\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_AZW3IH_KEY _T("SOFTWARE\\Classes\\.AZW3\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_PHZIH_KEY _T("SOFTWARE\\Classes\\.PHZ\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_WEBPIH_KEY _T("SOFTWARE\\Classes\\.WEBP\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_HEIFIH_KEY _T("SOFTWARE\\Classes\\.HEIF\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_HEICIH_KEY _T("SOFTWARE\\Classes\\.HEIC\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_AVIFIH_KEY _T("SOFTWARE\\Classes\\.AVIF\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_JXLIH_KEY _T("SOFTWARE\\Classes\\.JXL\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_MP4IH_KEY _T("SOFTWARE\\Classes\\.MP4\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_AVIIH_KEY _T("SOFTWARE\\Classes\\.AVI\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_MKVIH_KEY _T("SOFTWARE\\Classes\\.MKV\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_WMVIH_KEY _T("SOFTWARE\\Classes\\.WMV\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_PDFIH_KEY _T("SOFTWARE\\Classes\\.PDF\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_TIFFIH_KEY _T("SOFTWARE\\Classes\\.TIF\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_SVGIH_KEY _T("SOFTWARE\\Classes\\.SVG\\shellex\\{00021500-0000-0000-C000-000000000046}")
#define CBX_DNGIH_KEY _T("SOFTWARE\\Classes\\.DNG\\shellex\\{00021500-0000-0000-C000-000000000046}")

// cbx types (must match CBXTYPE in cbxArchive.h)
#define CBX_NONE 0
#define CBX_ZIP 1
#define CBX_CBZ 2
#define CBX_RAR 3
#define CBX_CBR 4
#define CBX_EPUB 5
#define CBX_7Z 6
#define CBX_CB7 7
#define CBX_TAR 8
#define CBX_CBT 9
#define CBX_MOBI 10
#define CBX_FB2 11
#define CBX_AZW 12
#define CBX_AZW3 13
#define CBX_PHZ 14
#define CBX_WEBP 15
#define CBX_HEIF 16
#define CBX_AVIF 17
#define CBX_JXL 18
#define CBX_VIDEO 19
#define CBX_PDF 20
#define CBX_TIFF 45  // Must match CBXTYPE_TIFF in cbxArchive.h
#define CBX_SVG 46   // Must match CBXTYPE_SVG in cbxArchive.h
#define CBX_RAW 47   // Must match CBXTYPE_RAW in cbxArchive.h (DNG, CR2, NEF, ARW, etc.)

// Handler status enumeration
enum HandlerStatus
{
	HANDLER_NONE = 0,         // No handler registered
	HANDLER_DARKTHUMBS = 1,   // DarkThumbs handler active
	HANDLER_NATIVE = 2,       // Windows native handler detected
	HANDLER_THIRD_PARTY = 3   // Other third-party handler
};


class CRegManager
{
public:
	//CRegManager(void){}
	//virtual ~CRegManager(void){}
public:

	///////////////
	// sort option
	BOOL IsSortOpt()
	{
		DWORD d;
		CRegKey rk;
		if (ERROR_SUCCESS==rk.Open(HKEY_CURRENT_USER, CBX_APP_KEY, KEY_READ))
		{
			if (ERROR_SUCCESS==rk.QueryDWORDValue(_T("NoSort"), d))
				return (d==FALSE);
		}
	return TRUE;
	}

	void SetSortOpt(BOOL bSort)
	{
		CRegKey rk;
		if (ERROR_SUCCESS==rk.Create(HKEY_CURRENT_USER, CBX_APP_KEY, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
			rk.SetDWORDValue(_T("NoSort"), (DWORD)(bSort ? FALSE : TRUE));
	}

	// sort option
	BOOL IsShowIconOpt()
	{
		DWORD d;
		CRegKey rk;
		if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, CBX_APP_KEY, KEY_READ))
		{
			if (ERROR_SUCCESS == rk.QueryDWORDValue(_T("ShowIcon"), d))
				return (d == TRUE);
		}
		return TRUE;
	}

	//////////////
	// set show archive Icon
	void SetShowIconOpt(BOOL bShowIcon)
	{
		CRegKey rk;
		if (ERROR_SUCCESS == rk.Create(HKEY_CURRENT_USER, CBX_APP_KEY, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
			rk.SetDWORDValue(_T("ShowIcon"), (DWORD)(bShowIcon ? TRUE : FALSE));
	}

	///////////////
	// Sprint C2: Collage mode (1=single, 4=2x2, 9=3x3, 16=4x4)
	DWORD GetCollageMode()
	{
		DWORD mode = 1; // Default: single page
		CRegKey rk;
		if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, CBX_APP_KEY, KEY_READ))
		{
			rk.QueryDWORDValue(_T("CollagePages"), mode);
		}
		// Validate mode
		if (mode != 1 && mode != 4 && mode != 9 && mode != 16)
			mode = 1;
		return mode;
	}

	void SetCollageMode(DWORD mode)
	{
		CRegKey rk;
		if (ERROR_SUCCESS == rk.Create(HKEY_CURRENT_USER, CBX_APP_KEY, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
			rk.SetDWORDValue(_T("CollagePages"), mode);
	}

	///////////////////////////////
	// check for thumbnail handlers
	BOOL HasTH(int cbxType)
	{
		ATLASSERT(cbxType>CBX_NONE);
		ULONG n=CBX_GUID_KEY_SLEN;
		TCHAR s[CBX_GUID_KEY_SLEN];

		CRegKey rk;
		if (ERROR_SUCCESS==rk.Open(HKEY_CURRENT_USER, GetTHKeyName(cbxType), KEY_READ))
		{
			if (ERROR_SUCCESS==rk.QueryStringValue(NULL, s, &n))
			{
				if (0==StrCmpI(s, CBX_GUID_KEY))
					return TRUE;
			}
		}
	return FALSE;
	}

	/////////////////////////////
	// check for infotip handlers
	BOOL HasIH(int cbxType)
	{
		ATLASSERT(cbxType>CBX_NONE);
		ULONG n=CBX_GUID_KEY_SLEN;
		TCHAR s[CBX_GUID_KEY_SLEN];

		CRegKey rk;
		if (ERROR_SUCCESS==rk.Open(HKEY_CURRENT_USER, GetIHKeyName(cbxType), KEY_READ))
		{
			if (ERROR_SUCCESS==rk.QueryStringValue(NULL, s, &n))
			{
				if (StrCmpI(s, CBX_GUID_KEY)==0)
					return TRUE;
			}
		}
	return FALSE;
	}

	/////////////////////////
	// Get handler status for a specific format
	// Returns: HANDLER_NONE, HANDLER_DARKTHUMBS, HANDLER_NATIVE, or HANDLER_THIRD_PARTY
	HandlerStatus GetHandlerStatus(int cbxType, LPCTSTR extension)
	{
		ATLASSERT(cbxType > CBX_NONE);
		
		TCHAR guid[256] = {0};
		ULONG len = 256;
		CRegKey rk;

		// First check HKCU (user-specific - where DarkThumbs registers)
		if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, GetTHKeyName(cbxType), KEY_READ))
		{
			if (ERROR_SUCCESS == rk.QueryStringValue(NULL, guid, &len))
			{
				if (StrCmpI(guid, CBX_GUID_KEY) == 0)
				{
					rk.Close();
					return HANDLER_DARKTHUMBS;
				}
			}
			rk.Close();
		}

		// Check HKCR (system-wide handlers)
		CString regPath;
		regPath.Format(_T("%s\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"), extension);
		
		len = 256;
		ZeroMemory(guid, sizeof(guid));
		
		if (ERROR_SUCCESS == rk.Open(HKEY_CLASSES_ROOT, regPath, KEY_READ))
		{
			if (ERROR_SUCCESS == rk.QueryStringValue(NULL, guid, &len))
			{
				// Check for known Windows native handler GUIDs
				
				// Windows Media Foundation (video files)
				if (StrStrI(guid, _T("c5aec3ec-e812-4677-a9a7-4fee1f9aa000")) != NULL)
				{
					rk.Close();
					return HANDLER_NATIVE;
				}
				
				// Windows PDF handler
				if (StrStrI(guid, _T("DC6EFB56-9CFA-464D-8880-44885D7DC193")) != NULL)
				{
					rk.Close();
					return HANDLER_NATIVE;
				}
				
				// Windows Photo Viewer (various image formats)
				if (StrStrI(guid, _T("C7657C4A-9F70-11D0-A999-00C04FD655E1")) != NULL)
				{
					rk.Close();
					return HANDLER_NATIVE;
				}
				
				// Windows Imaging Component
				if (StrStrI(guid, _T("7D2B9654-0AE1-4BBD-BD42-7A0C3A23E787")) != NULL)
				{
					rk.Close();
					return HANDLER_NATIVE;
				}
				
				// Not our GUID and not a known Windows handler
				if (StrCmpI(guid, CBX_GUID_KEY) != 0)
				{
					rk.Close();
					return HANDLER_THIRD_PARTY;
				}
			}
			rk.Close();
		}

		return HANDLER_NONE;
	}

	/////////////////////////
	// Sprint 18A: Registry Backup System
	// Backup existing handler GUID before installation
	BOOL BackupHandler(int cbxType, LPCTSTR keyPath, LPCTSTR backupName)
	{
		TCHAR existingGuid[256] = {0};
		ULONG len = 256;
		CRegKey rk;

		// Read existing handler GUID
		if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, keyPath, KEY_READ))
		{
			if (ERROR_SUCCESS == rk.QueryStringValue(NULL, existingGuid, &len))
			{
				// Don't backup if it's already our GUID
				if (StrCmpI(existingGuid, CBX_GUID_KEY) == 0)
				{
					rk.Close();
					return FALSE; // No backup needed
				}

				// Store backup
				CString backupKey;
				backupKey.Format(_T("%s\\Backups\\%s"), CBX_APP_KEY, backupName);
				
				CRegKey rkBackup;
				if (ERROR_SUCCESS == rkBackup.Create(HKEY_CURRENT_USER, backupKey, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
				{
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
	// Sprint 18A: Restore backed-up handler
	BOOL RestoreHandler(LPCTSTR keyPath, LPCTSTR backupName)
	{
		CString backupKey;
		backupKey.Format(_T("%s\\Backups\\%s"), CBX_APP_KEY, backupName);
		
		TCHAR backedUpGuid[256] = {0};
		ULONG len = 256;
		CRegKey rkBackup;

		// Read backup
		if (ERROR_SUCCESS == rkBackup.Open(HKEY_CURRENT_USER, backupKey, KEY_READ))
		{
			if (ERROR_SUCCESS == rkBackup.QueryStringValue(NULL, backedUpGuid, &len))
			{
				// Restore the backed-up handler
				CRegKey rk;
				if (ERROR_SUCCESS == rk.Create(HKEY_CURRENT_USER, keyPath, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
				{
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
	// Sprint 18A: Get program name from handler GUID
	CString GetHandlerProgramName(LPCTSTR guid)
	{
		if (!guid || guid[0] == 0)
			return _T("None");

		// Check for known handler GUIDs first
		if (StrCmpI(guid, CBX_GUID_KEY) == 0)
			return _T("DarkThumbs");
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
		
		TCHAR dllPath[MAX_PATH] = {0};
		ULONG len = MAX_PATH;
		CRegKey rk;

		if (ERROR_SUCCESS == rk.Open(HKEY_CLASSES_ROOT, clsidKey, KEY_READ))
		{
			if (ERROR_SUCCESS == rk.QueryStringValue(NULL, dllPath, &len))
			{
				// Extract filename from path
				LPCTSTR filename = PathFindFileName(dllPath);
				if (filename && filename[0])
				{
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
	// Sprint 18A: Enhanced handler status with program info
	// Returns status and fills programName with handler application name
	HandlerStatus GetHandlerStatusEx(int cbxType, LPCTSTR extension, CString& programName)
	{
		ATLASSERT(cbxType > CBX_NONE);
		
		TCHAR guid[256] = {0};
		ULONG len = 256;
		CRegKey rk;

		programName = _T("None");

		// First check HKCU (user-specific - where DarkThumbs registers)
		if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, GetTHKeyName(cbxType), KEY_READ))
		{
			if (ERROR_SUCCESS == rk.QueryStringValue(NULL, guid, &len))
			{
				programName = GetHandlerProgramName(guid);
				rk.Close();
				
				if (StrCmpI(guid, CBX_GUID_KEY) == 0)
					return HANDLER_DARKTHUMBS;
				else
					return HANDLER_THIRD_PARTY;
			}
			rk.Close();
		}

		// Check HKCR (system-wide handlers)
		CString regPath;
		regPath.Format(_T("%s\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"), extension);
		
		len = 256;
		ZeroMemory(guid, sizeof(guid));
		
		if (ERROR_SUCCESS == rk.Open(HKEY_CLASSES_ROOT, regPath, KEY_READ))
		{
			if (ERROR_SUCCESS == rk.QueryStringValue(NULL, guid, &len))
			{
				programName = GetHandlerProgramName(guid);
				rk.Close();

				// Check for known Windows native handler GUIDs
				if (StrStrI(guid, _T("c5aec3ec-e812-4677-a9a7-4fee1f9aa000")) != NULL ||
				    StrStrI(guid, _T("DC6EFB56-9CFA-464D-8880-44885D7DC193")) != NULL ||
				    StrStrI(guid, _T("C7657C4A-9F70-11D0-A999-00C04FD655E1")) != NULL ||
				    StrStrI(guid, _T("7D2B9654-0AE1-4BBD-BD42-7A0C3A23E787")) != NULL)
				{
					return HANDLER_NATIVE;
				}
				
				// Not our GUID and not a known Windows handler
				if (StrCmpI(guid, CBX_GUID_KEY) != 0)
					return HANDLER_THIRD_PARTY;
			}
			rk.Close();
		}

		programName = _T("None");
		return HANDLER_NONE;
	}

	/////////////////////////
	// Get extension for a CBX type (for status detection)
	LPCTSTR GetExtension(int cbxType)
	{
		switch (cbxType)
		{
		case CBX_ZIP: return _T(".zip");
		case CBX_CBZ: return _T(".cbz");
		case CBX_RAR: return _T(".rar");
		case CBX_CBR: return _T(".cbr");
		case CBX_EPUB: return _T(".epub");
		case CBX_7Z: return _T(".7z");
		case CBX_CB7: return _T(".cb7");
		case CBX_TAR: return _T(".tar");
		case CBX_CBT: return _T(".cbt");
		case CBX_MOBI: return _T(".mobi");
		case CBX_FB2: return _T(".fb2");
		case CBX_AZW: return _T(".azw");
		case CBX_AZW3: return _T(".azw3");
		case CBX_PHZ: return _T(".phz");
		case CBX_WEBP: return _T(".webp");
		case CBX_HEIF: return _T(".heif");
		case CBX_AVIF: return _T(".avif");
		case CBX_JXL: return _T(".jxl");
		case CBX_VIDEO: return _T(".mp4"); // Representative video format
		case CBX_PDF: return _T(".pdf");
		case CBX_TIFF: return _T(".tif");
		case CBX_SVG: return _T(".svg");
		case CBX_RAW: return _T(".dng"); // Representative RAW format (DNG, CR2, NEF, ARW)
		default: break;
		}
		return NULL;
	}

	/////////////////////////
	// set thumbnail / infotip handlers (Sprint 18A: Enhanced with backup/restore)
	void SetHandlers(int cbxType, BOOL bSet)
	{
		ATLASSERT(cbxType>CBX_NONE);

		// Special handling for multi-extension formats (VIDEO, HEIF, TIFF, RAW)
		if (cbxType == CBX_VIDEO)
		{
			// Video formats: .mp4, .avi, .mkv, .mov, .wmv, .flv, .webm, .m4v, .mpg, .mpeg
			const LPCTSTR videoExts[] = {
				CBX_MP4TH_KEY, CBX_AVITH_KEY, CBX_MKVTH_KEY, _T("SOFTWARE\\Classes\\.MOV\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
				CBX_WMVTH_KEY, _T("SOFTWARE\\Classes\\.FLV\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
				_T("SOFTWARE\\Classes\\.WEBM\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
				_T("SOFTWARE\\Classes\\.M4V\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
				_T("SOFTWARE\\Classes\\.MPG\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
				_T("SOFTWARE\\Classes\\.MPEG\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
			};
			const LPCTSTR videoInfoTips[] = {
				CBX_MP4IH_KEY, CBX_AVIIH_KEY, CBX_MKVIH_KEY, _T("SOFTWARE\\Classes\\.MOV\\shellex\\{00021500-0000-0000-C000-000000000046}"),
				CBX_WMVIH_KEY, _T("SOFTWARE\\Classes\\.FLV\\shellex\\{00021500-0000-0000-C000-000000000046}"),
				_T("SOFTWARE\\Classes\\.WEBM\\shellex\\{00021500-0000-0000-C000-000000000046}"),
				_T("SOFTWARE\\Classes\\.M4V\\shellex\\{00021500-0000-0000-C000-000000000046}"),
				_T("SOFTWARE\\Classes\\.MPG\\shellex\\{00021500-0000-0000-C000-000000000046}"),
				_T("SOFTWARE\\Classes\\.MPEG\\shellex\\{00021500-0000-0000-C000-000000000046}")
			};
			const LPCTSTR backupNames[] = {
				_T("mp4_th"), _T("avi_th"), _T("mkv_th"), _T("mov_th"), _T("wmv_th"), 
				_T("flv_th"), _T("webm_th"), _T("m4v_th"), _T("mpg_th"), _T("mpeg_th")
			};
			const LPCTSTR backupNamesInfo[] = {
				_T("mp4_ih"), _T("avi_ih"), _T("mkv_ih"), _T("mov_ih"), _T("wmv_ih"),
				_T("flv_ih"), _T("webm_ih"), _T("m4v_ih"), _T("mpg_ih"), _T("mpeg_ih")
			};
			
			for (int i = 0; i < _countof(videoExts); i++)
			{
				if (bSet)
				{
					// Sprint 18A: Backup existing handlers before overwriting
					BackupHandler(cbxType, videoExts[i], backupNames[i]);
					BackupHandler(cbxType, videoInfoTips[i], backupNamesInfo[i]);

					CRegKey rkt, rki;
					if (ERROR_SUCCESS == rkt.Create(HKEY_CURRENT_USER, videoExts[i], NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
						rkt.SetStringValue(NULL, CBX_GUID_KEY);
					if (ERROR_SUCCESS == rki.Create(HKEY_CURRENT_USER, videoInfoTips[i], NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
						rki.SetStringValue(NULL, CBX_GUID_KEY);
				}
				else
				{
					// Sprint 18A: Smart uninstall - only remove if it's our handler
					TCHAR currentGuid[256] = {0};
					ULONG len = 256;
					CRegKey rk;

					// Check thumbnail handler
					if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, videoExts[i], KEY_READ))
					{
						if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len))
						{
							if (StrCmpI(currentGuid, CBX_GUID_KEY) == 0)
							{
								rk.Close();
								// It's our handler - remove or restore backup
								if (!RestoreHandler(videoExts[i], backupNames[i]))
									RegDeleteKey(HKEY_CURRENT_USER, videoExts[i]);
							}
							else
							{
								rk.Close(); // Not our handler, don't touch it
							}
						}
						else
							rk.Close();
					}

					// Check infotip handler
					len = 256;
					ZeroMemory(currentGuid, sizeof(currentGuid));
					if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, videoInfoTips[i], KEY_READ))
					{
						if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len))
						{
							if (StrCmpI(currentGuid, CBX_GUID_KEY) == 0)
							{
								rk.Close();
								if (!RestoreHandler(videoInfoTips[i], backupNamesInfo[i]))
									RegDeleteKey(HKEY_CURRENT_USER, videoInfoTips[i]);
							}
							else
							{
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
		else if (cbxType == CBX_HEIF)
		{
			// HEIF formats: .heif, .heic
			const LPCTSTR heifExts[] = { CBX_HEIFTH_KEY, CBX_HEICTH_KEY };
			const LPCTSTR heifInfoTips[] = { CBX_HEIFIH_KEY, CBX_HEICIH_KEY };
			const LPCTSTR backupNames[] = { _T("heif_th"), _T("heic_th") };
			const LPCTSTR backupNamesInfo[] = { _T("heif_ih"), _T("heic_ih") };
			
			for (int i = 0; i < _countof(heifExts); i++)
			{
				if (bSet)
				{
					BackupHandler(cbxType, heifExts[i], backupNames[i]);
					BackupHandler(cbxType, heifInfoTips[i], backupNamesInfo[i]);

					CRegKey rkt, rki;
					if (ERROR_SUCCESS == rkt.Create(HKEY_CURRENT_USER, heifExts[i], NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
						rkt.SetStringValue(NULL, CBX_GUID_KEY);
					if (ERROR_SUCCESS == rki.Create(HKEY_CURRENT_USER, heifInfoTips[i], NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
						rki.SetStringValue(NULL, CBX_GUID_KEY);
				}
				else
				{
					// Smart uninstall
					TCHAR currentGuid[256] = {0};
					ULONG len = 256;
					CRegKey rk;

					if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, heifExts[i], KEY_READ))
					{
						if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len))
						{
							if (StrCmpI(currentGuid, CBX_GUID_KEY) == 0)
							{
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
					if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, heifInfoTips[i], KEY_READ))
					{
						if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len))
						{
							if (StrCmpI(currentGuid, CBX_GUID_KEY) == 0)
							{
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
		else if (cbxType == CBX_TIFF)
		{
			// TIFF formats: .tif, .tiff
			const LPCTSTR tiffExts[] = { CBX_TIFFTH_KEY, _T("SOFTWARE\\Classes\\.TIFF\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}") };
			const LPCTSTR tiffInfoTips[] = { CBX_TIFFIH_KEY, _T("SOFTWARE\\Classes\\.TIFF\\shellex\\{00021500-0000-0000-C000-000000000046}") };
			const LPCTSTR backupNames[] = { _T("tif_th"), _T("tiff_th") };
			const LPCTSTR backupNamesInfo[] = { _T("tif_ih"), _T("tiff_ih") };
			
			for (int i = 0; i < _countof(tiffExts); i++)
			{
				if (bSet)
				{
					BackupHandler(cbxType, tiffExts[i], backupNames[i]);
					BackupHandler(cbxType, tiffInfoTips[i], backupNamesInfo[i]);

					CRegKey rkt, rki;
					if (ERROR_SUCCESS == rkt.Create(HKEY_CURRENT_USER, tiffExts[i], NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
						rkt.SetStringValue(NULL, CBX_GUID_KEY);
					if (ERROR_SUCCESS == rki.Create(HKEY_CURRENT_USER, tiffInfoTips[i], NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
						rki.SetStringValue(NULL, CBX_GUID_KEY);
				}
				else
				{
					// Smart uninstall
					TCHAR currentGuid[256] = {0};
					ULONG len = 256;
					CRegKey rk;

					if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, tiffExts[i], KEY_READ))
					{
						if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len))
						{
							if (StrCmpI(currentGuid, CBX_GUID_KEY) == 0)
							{
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
					if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, tiffInfoTips[i], KEY_READ))
					{
						if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len))
						{
							if (StrCmpI(currentGuid, CBX_GUID_KEY) == 0)
							{
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
		else if (cbxType == CBX_RAW)
		{
			// RAW formats: .dng, .cr2, .cr3, .nef, .arw, .orf
			const LPCTSTR rawExts[] = {
				CBX_DNGTH_KEY,
				_T("SOFTWARE\\Classes\\.CR2\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
				_T("SOFTWARE\\Classes\\.CR3\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
				_T("SOFTWARE\\Classes\\.NEF\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
				_T("SOFTWARE\\Classes\\.ARW\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"),
				_T("SOFTWARE\\Classes\\.ORF\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}")
			};
			const LPCTSTR rawInfoTips[] = {
				CBX_DNGIH_KEY,
				_T("SOFTWARE\\Classes\\.CR2\\shellex\\{00021500-0000-0000-C000-000000000046}"),
				_T("SOFTWARE\\Classes\\.CR3\\shellex\\{00021500-0000-0000-C000-000000000046}"),
				_T("SOFTWARE\\Classes\\.NEF\\shellex\\{00021500-0000-0000-C000-000000000046}"),
				_T("SOFTWARE\\Classes\\.ARW\\shellex\\{00021500-0000-0000-C000-000000000046}"),
				_T("SOFTWARE\\Classes\\.ORF\\shellex\\{00021500-0000-0000-C000-000000000046}")
			};
			const LPCTSTR backupNames[] = { _T("dng_th"), _T("cr2_th"), _T("cr3_th"), _T("nef_th"), _T("arw_th"), _T("orf_th") };
			const LPCTSTR backupNamesInfo[] = { _T("dng_ih"), _T("cr2_ih"), _T("cr3_ih"), _T("nef_ih"), _T("arw_ih"), _T("orf_ih") };
			
			for (int i = 0; i < _countof(rawExts); i++)
			{
				if (bSet)
				{
					BackupHandler(cbxType, rawExts[i], backupNames[i]);
					BackupHandler(cbxType, rawInfoTips[i], backupNamesInfo[i]);

					CRegKey rkt, rki;
					if (ERROR_SUCCESS == rkt.Create(HKEY_CURRENT_USER, rawExts[i], NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
						rkt.SetStringValue(NULL, CBX_GUID_KEY);
					if (ERROR_SUCCESS == rki.Create(HKEY_CURRENT_USER, rawInfoTips[i], NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
						rki.SetStringValue(NULL, CBX_GUID_KEY);
				}
				else
				{
					// Smart uninstall
					TCHAR currentGuid[256] = {0};
					ULONG len = 256;
					CRegKey rk;

					if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, rawExts[i], KEY_READ))
					{
						if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len))
						{
							if (StrCmpI(currentGuid, CBX_GUID_KEY) == 0)
							{
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
					if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, rawInfoTips[i], KEY_READ))
					{
						if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len))
						{
							if (StrCmpI(currentGuid, CBX_GUID_KEY) == 0)
							{
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
		backupTH.Format(_T("%s_th"), GetExtension(cbxType));
		backupIH.Format(_T("%s_ih"), GetExtension(cbxType));

		if (bSet)
		{
			// Sprint 18A: Backup before overwriting
			BackupHandler(cbxType, GetTHKeyName(cbxType), backupTH);
			BackupHandler(cbxType, GetIHKeyName(cbxType), backupIH);

			//thumbnail
			CRegKey rkt, rki;
			if (ERROR_SUCCESS==rkt.Create(HKEY_CURRENT_USER, GetTHKeyName(cbxType), NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
				rkt.SetStringValue(NULL, CBX_GUID_KEY);
			//infotip
			if (ERROR_SUCCESS==rki.Create(HKEY_CURRENT_USER, GetIHKeyName(cbxType), NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE))
				rki.SetStringValue(NULL, CBX_GUID_KEY);
		}
		else
		{
			// Sprint 18A: Smart uninstall - only remove if it's our handler
			TCHAR currentGuid[256] = {0};
			ULONG len = 256;
			CRegKey rk;

			//thumbnail
			if (HasTH(cbxType))
			{
				if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, GetTHKeyName(cbxType), KEY_READ))
				{
					if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len))
					{
						if (StrCmpI(currentGuid, CBX_GUID_KEY) == 0)
						{
							rk.Close();
							if (!RestoreHandler(GetTHKeyName(cbxType), backupTH))
								RegDeleteKey(HKEY_CURRENT_USER, GetTHKeyName(cbxType));
						}
						else
							rk.Close();
					}
					else
						rk.Close();
				}
			}

			//infotip
			if (HasIH(cbxType))
			{
				len = 256;
				ZeroMemory(currentGuid, sizeof(currentGuid));
				if (ERROR_SUCCESS == rk.Open(HKEY_CURRENT_USER, GetIHKeyName(cbxType), KEY_READ))
				{
					if (ERROR_SUCCESS == rk.QueryStringValue(NULL, currentGuid, &len))
					{
						if (StrCmpI(currentGuid, CBX_GUID_KEY) == 0)
						{
							rk.Close();
							if (!RestoreHandler(GetIHKeyName(cbxType), backupIH))
								RegDeleteKey(HKEY_CURRENT_USER, GetIHKeyName(cbxType));
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

	//get handler reg key names
	LPCTSTR GetTHKeyName(int cbxType)
	{
		ATLASSERT(cbxType>CBX_NONE);
		switch (cbxType)
		{
		case CBX_ZIP: return CBX_ZIPTH_KEY;
		case CBX_CBZ: return CBX_CBZTH_KEY;
		case CBX_EPUB: return CBX_EPUBTH_KEY;
		case CBX_RAR: return CBX_RARTH_KEY;
		case CBX_CBR: return CBX_CBRTH_KEY;
		case CBX_7Z: return CBX_7ZTH_KEY;
		case CBX_CB7: return CBX_CB7TH_KEY;
		case CBX_TAR: return CBX_TARTH_KEY;
		case CBX_CBT: return CBX_CBTTH_KEY;
		case CBX_MOBI: return CBX_MOBITH_KEY;
		case CBX_FB2: return CBX_FB2TH_KEY;
		case CBX_AZW: return CBX_AZWTH_KEY;
		case CBX_AZW3: return CBX_AZW3TH_KEY;
		case CBX_PHZ: return CBX_PHZTH_KEY;
		case CBX_WEBP: return CBX_WEBPTH_KEY;
		case CBX_HEIF: return CBX_HEIFTH_KEY;
		case CBX_AVIF: return CBX_AVIFTH_KEY;
		case CBX_JXL: return CBX_JXLTH_KEY;
		case CBX_VIDEO: return CBX_MP4TH_KEY; // Primary video format
		case CBX_PDF: return CBX_PDFTH_KEY;
		case CBX_TIFF: return CBX_TIFFTH_KEY;
		case CBX_SVG: return CBX_SVGTH_KEY;
		case CBX_RAW: return CBX_DNGTH_KEY;
		default:break;
		}
	return NULL;
	}
	LPCTSTR GetIHKeyName(int cbxType)
	{
		ATLASSERT(cbxType>CBX_NONE);
		switch (cbxType)
		{
		case CBX_ZIP: return CBX_ZIPIH_KEY;
		case CBX_CBZ: return CBX_CBZIH_KEY;
		case CBX_EPUB: return CBX_EPUBIH_KEY;
		case CBX_RAR: return CBX_RARIH_KEY;
		case CBX_CBR: return CBX_CBRIH_KEY;
		case CBX_7Z: return CBX_7ZIH_KEY;
		case CBX_CB7: return CBX_CB7IH_KEY;
		case CBX_TAR: return CBX_TARIH_KEY;
		case CBX_CBT: return CBX_CBTIH_KEY;
		case CBX_MOBI: return CBX_MOBIIH_KEY;
		case CBX_FB2: return CBX_FB2IH_KEY;
		case CBX_AZW: return CBX_AZWIH_KEY;
		case CBX_AZW3: return CBX_AZW3IH_KEY;
		case CBX_PHZ: return CBX_PHZIH_KEY;
		case CBX_WEBP: return CBX_WEBPIH_KEY;
		case CBX_HEIF: return CBX_HEIFIH_KEY;
		case CBX_AVIF: return CBX_AVIFIH_KEY;
		case CBX_JXL: return CBX_JXLIH_KEY;
		case CBX_VIDEO: return CBX_MP4IH_KEY; // Primary video format
		case CBX_PDF: return CBX_PDFIH_KEY;
		case CBX_TIFF: return CBX_TIFFIH_KEY;
		case CBX_SVG: return CBX_SVGIH_KEY;
		case CBX_RAW: return CBX_DNGIH_KEY;
		default:break;
		}
	return NULL;
	}
};

#endif//_REGMANAGER_79AE66E4_84E2_45A1_BF4F_43AA714BE55F_
