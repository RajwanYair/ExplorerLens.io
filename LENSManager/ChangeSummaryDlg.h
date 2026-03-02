#ifndef _CHANGESUMMARYDLG_H_
#define _CHANGESUMMARYDLG_H_
#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlctrls.h>
#include <atlcrack.h>
#include <atlfile.h>
#include <vector>
#include <string>
#include "DarkModeController.h"

// Structure to hold configuration change information
struct ConfigChange
{
	CString formatName;
	CString extension;
	bool wasEnabled;
	bool isEnabled;

	CString GetChangeDescription() const {
		if (!wasEnabled && isEnabled)
			return _T("[OK] ENABLED");
		else if (wasEnabled && !isEnabled)
			return _T("[FAIL] DISABLED");
		else
			return _T("(no change)");
	}

	bool HasChanged() const { return wasEnabled != isEnabled; }
};

// Structure to hold complete configuration snapshot
struct ConfigSnapshot
{
	// Format handlers (23 core + 12 specialized = 35 formats)
	bool cbz, cbr, cb7, cbt;
	bool epub, mobi, azw, azw3;
	bool zip, rar, z7, tar;
	bool phz, fb2;
	bool webp, heif, avif, jxl;
	bool video, pdf, tiff, svg, raw;
	bool psd, dds, hdr, exr;
	bool ppm, ico, qoi, tga;
	bool audio, document, font, model;
	bool extImage, texture, extArchive, extDocument;

	// Options
	bool sortOpt;
	bool showIconOpt;
	DWORD collageMode;  // 1, 4, 9, or 16

	CString GetCollageModeString() const {
		switch (collageMode) {
		case 1: return _T("Single Page (1x1)");
		case 4: return _T("2x2 Grid (4 pages)");
		case 9: return _T("3x3 Grid (9 pages)");
		case 16: return _T("4x4 Grid (16 pages)");
		default: return _T("Unknown");
		}
	}

	// Get timestamp
	CString GetTimestamp() const {
		SYSTEMTIME st;
		GetLocalTime(&st);
		CString timestamp;
		timestamp.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"),
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		return timestamp;
	}
};

class CChangeSummaryDlg : public CDialogImpl<CChangeSummaryDlg>
{
public:
	enum { IDD = IDD_CHANGE_SUMMARY };

	CChangeSummaryDlg(const ConfigSnapshot& oldConfig, const ConfigSnapshot& newConfig)
		: m_oldConfig(oldConfig), m_newConfig(newConfig) {
		BuildChangeList();
	}

	BEGIN_MSG_MAP(CChangeSummaryDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnCtlColor)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColor)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDC_BTN_SAVE_CONFIG, OnSaveConfig)
		COMMAND_ID_HANDLER(IDC_BTN_RESTORE, OnRestore)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	LRESULT OnCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
		auto& darkCtrl = ExplorerLens::DarkModeController::Instance();
		return reinterpret_cast<LRESULT>(
			darkCtrl.OnCtlColor(reinterpret_cast<HDC>(wParam),
				reinterpret_cast<HWND>(lParam)));
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CenterWindow(GetParent());

		// Apply dark mode theming
		DarkModeController& darkCtrl = DarkModeController::Instance();
		darkCtrl.ApplyToWindow(m_hWnd);

		// Set dialog title
		SetWindowText(_T("Configuration Changes Applied - ExplorerLens"));

		// Populate the change list
		CListViewCtrl listChanges = GetDlgItem(IDC_LIST_CHANGES);
		listChanges.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

		// Add columns
		listChanges.InsertColumn(0, _T("Format"), LVCFMT_LEFT, 120);
		listChanges.InsertColumn(1, _T("Extension"), LVCFMT_LEFT, 100);
		listChanges.InsertColumn(2, _T("Previous"), LVCFMT_CENTER, 80);
		listChanges.InsertColumn(3, _T("Current"), LVCFMT_CENTER, 80);
		listChanges.InsertColumn(4, _T("Change"), LVCFMT_LEFT, 120);

		// Populate with changes (show only changed items)
		int index = 0;
		for (size_t i = 0; i < m_changes.size(); i++) {
			if (m_changes[i].HasChanged()) {
				listChanges.InsertItem(index, m_changes[i].formatName);
				listChanges.SetItemText(index, 1, m_changes[i].extension);
				listChanges.SetItemText(index, 2, m_changes[i].wasEnabled ? _T("Enabled") : _T("Disabled"));
				listChanges.SetItemText(index, 3, m_changes[i].isEnabled ? _T("Enabled") : _T("Disabled"));
				listChanges.SetItemText(index, 4, m_changes[i].GetChangeDescription());
				index++;
			}
		}

		// Check for options changes
		CString optionsInfo;
		bool hasOptionsChanges = false;

		if (m_oldConfig.sortOpt != m_newConfig.sortOpt) {
			hasOptionsChanges = true;
			optionsInfo += _T("• Sort Pages: ");
			optionsInfo += m_oldConfig.sortOpt ? _T("ON -> OFF\r\n") : _T("OFF -> ON\r\n");
		}

		if (m_oldConfig.showIconOpt != m_newConfig.showIconOpt) {
			hasOptionsChanges = true;
			optionsInfo += _T("• Show Icon Overlay: ");
			optionsInfo += m_oldConfig.showIconOpt ? _T("ON -> OFF\r\n") : _T("OFF -> ON\r\n");
		}

		if (m_oldConfig.collageMode != m_newConfig.collageMode) {
			hasOptionsChanges = true;
			optionsInfo.AppendFormat(_T("• Collage Mode: %s -> %s\r\n"),
				m_oldConfig.GetCollageModeString(), m_newConfig.GetCollageModeString());
		}

		if (hasOptionsChanges) {
			SetDlgItemText(IDC_EDIT_OPTIONS_CHANGES, optionsInfo);
		}
		else {
			SetDlgItemText(IDC_EDIT_OPTIONS_CHANGES, _T("No option changes."));
		}

		// Summary text
		int enabledCount = 0, disabledCount = 0;
		for (const auto& change : m_changes) {
			if (change.HasChanged()) {
				if (change.isEnabled) enabledCount++;
				else disabledCount++;
			}
		}

		CString summary;
		if (enabledCount > 0 || disabledCount > 0 || hasOptionsChanges) {
			summary.Format(_T("Changes applied successfully!\r\n\r\n")
				_T("Formats enabled: %d\r\n")
				_T("Formats disabled: %d\r\n")
				_T("Options changed: %s\r\n\r\n")
				_T("Windows Explorer has been refreshed to apply thumbnail changes.\r\n\r\n")
				_T("To restore previous settings:\r\n")
				_T("1. Click 'Save Configuration' to backup current state\r\n")
				_T("2. Click 'Restore Previous' to undo changes\r\n")
				_T("3. Or manually load a saved configuration file"),
				enabledCount, disabledCount, hasOptionsChanges ? _T("Yes") : _T("No"));
		}
		else {
			summary = _T("No configuration changes were made.\r\n\r\n")
				_T("The current settings match the previous state.");

			// Disable restore button if nothing changed
			::EnableWindow(GetDlgItem(IDC_BTN_RESTORE), FALSE);
		}

		SetDlgItemText(IDC_EDIT_SUMMARY, summary);

		return TRUE;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(IDOK);
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(IDCANCEL);
		return 0;
	}

	LRESULT OnSaveConfig(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		// File save dialog with REG and JSON format support
		CFileDialog dlg(FALSE, _T("reg"), _T("ExplorerLens_Config"),
			OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			_T("Windows Registry File (*.reg)\0*.reg\0JSON Configuration (*.json)\0*.json\0All Files (*.*)\0*.*\0"));

		if (dlg.DoModal() == IDOK) {
			CString filename(dlg.m_szFileName);
			bool isRegFile = (dlg.m_ofn.nFilterIndex == 1 || filename.Right(4).CompareNoCase(_T(".reg")) == 0);

			bool success = false;
			if (isRegFile) {
				success = SaveConfigToRegFile(dlg.m_szFileName, m_newConfig);
			}
			else {
				success = SaveConfigToFile(dlg.m_szFileName, m_newConfig);
			}

			if (success) {
				CString msg;
				if (isRegFile) {
					msg.Format(_T("Configuration saved successfully to:\r\n%s\r\n\r\n")
						_T("To restore this configuration:\r\n")
						_T("• Double-click the .reg file (recommended)\r\n")
						_T("• Or use 'Load Config...' button in ExplorerLens\r\n\r\n")
						_T("The .reg file can be imported directly into Windows Registry."),
						dlg.m_szFileName);
				}
				else {
					msg.Format(_T("Configuration saved successfully to:\r\n%s\r\n\r\n")
						_T("You can restore this configuration later using 'Load Config...' in the main window."),
						dlg.m_szFileName);
				}
				MessageBox(msg, _T("Configuration Saved"), MB_OK | MB_ICONINFORMATION);
			}
			else {
				MessageBox(_T("Failed to save configuration file."), _T("Error"), MB_OK | MB_ICONERROR);
			}
		}

		return 0;
	}

	LRESULT OnRestore(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		if (MessageBox(_T("This will restore the previous configuration and undo all changes made in this session.\r\n\r\n")
			_T("Do you want to continue?"), _T("Confirm Restore"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
			m_shouldRestore = true;
			EndDialog(IDC_BTN_RESTORE);
		}
		return 0;
	}

	bool ShouldRestore() const { return m_shouldRestore; }
	const ConfigSnapshot& GetOldConfig() const { return m_oldConfig; }

private:
	ConfigSnapshot m_oldConfig;
	ConfigSnapshot m_newConfig;
	std::vector<ConfigChange> m_changes;
	bool m_shouldRestore = false;

	void BuildChangeList() {
		m_changes.clear();

		// Comic Book Formats
		m_changes.push_back({ _T("CBZ"), _T(".cbz"), m_oldConfig.cbz, m_newConfig.cbz });
		m_changes.push_back({ _T("CBR"), _T(".cbr"), m_oldConfig.cbr, m_newConfig.cbr });
		m_changes.push_back({ _T("CB7"), _T(".cb7"), m_oldConfig.cb7, m_newConfig.cb7 });
		m_changes.push_back({ _T("CBT"), _T(".cbt"), m_oldConfig.cbt, m_newConfig.cbt });

		// E-Books
		m_changes.push_back({ _T("EPUB"), _T(".epub"), m_oldConfig.epub, m_newConfig.epub });
		m_changes.push_back({ _T("MOBI"), _T(".mobi"), m_oldConfig.mobi, m_newConfig.mobi });
		m_changes.push_back({ _T("AZW"), _T(".azw"), m_oldConfig.azw, m_newConfig.azw });
		m_changes.push_back({ _T("AZW3"), _T(".azw3"), m_oldConfig.azw3, m_newConfig.azw3 });

		// Archives
		m_changes.push_back({ _T("ZIP"), _T(".zip"), m_oldConfig.zip, m_newConfig.zip });
		m_changes.push_back({ _T("RAR"), _T(".rar"), m_oldConfig.rar, m_newConfig.rar });
		m_changes.push_back({ _T("7Z"), _T(".7z"), m_oldConfig.z7, m_newConfig.z7 });
		m_changes.push_back({ _T("TAR"), _T(".tar"), m_oldConfig.tar, m_newConfig.tar });

		// Photo & Other
		m_changes.push_back({ _T("PHZ"), _T(".phz"), m_oldConfig.phz, m_newConfig.phz });
		m_changes.push_back({ _T("FB2"), _T(".fb2"), m_oldConfig.fb2, m_newConfig.fb2 });

		// Modern Images
		m_changes.push_back({ _T("WebP"), _T(".webp"), m_oldConfig.webp, m_newConfig.webp });
		m_changes.push_back({ _T("HEIF"), _T(".heif/.heic"), m_oldConfig.heif, m_newConfig.heif });
		m_changes.push_back({ _T("AVIF"), _T(".avif"), m_oldConfig.avif, m_newConfig.avif });
		m_changes.push_back({ _T("JPEG XL"), _T(".jxl"), m_oldConfig.jxl, m_newConfig.jxl });

		// Media & Documents
		m_changes.push_back({ _T("Video"), _T(".mp4/.avi/.mkv"), m_oldConfig.video, m_newConfig.video });
		m_changes.push_back({ _T("PDF"), _T(".pdf"), m_oldConfig.pdf, m_newConfig.pdf });
		m_changes.push_back({ _T("TIFF"), _T(".tif/.tiff"), m_oldConfig.tiff, m_newConfig.tiff });
		m_changes.push_back({ _T("SVG"), _T(".svg"), m_oldConfig.svg, m_newConfig.svg });
		m_changes.push_back({ _T("RAW"), _T(".dng/.cr2/.nef"), m_oldConfig.raw, m_newConfig.raw });
	}

	static bool SaveConfigToRegFile(LPCTSTR filename, const ConfigSnapshot& config) {
		CAtlFile file;
		HRESULT hr = file.Create(filename, GENERIC_WRITE, 0, CREATE_ALWAYS);
		if (FAILED(hr)) return false;

		CStringA regData;

		// REG file header (Windows Registry Editor Version 5.00)
		regData = "Windows Registry Editor Version 5.00\r\n\r\n";
		regData += "; ExplorerLens Shell Manager Configuration\r\n";
		regData += "; Generated: ";
		regData += CT2A(config.GetTimestamp());
		regData += "\r\n";
		regData += "; Import this file by double-clicking it or running: regedit /s filename.reg\r\n\r\n";

		// Helper lambda to add registry key with GUID
		auto addHandler = [&regData](const char* ext, bool enabled, const char* guid = "{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}") {
			// Thumbnail handler
			regData.AppendFormat("[HKEY_CURRENT_USER\\SOFTWARE\\Classes\\.%s\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}]\r\n", ext);
			if (enabled) {
				regData.AppendFormat("@=\"%s\"\r\n\r\n", guid);
			}
			else {
				regData += "@=-\r\n\r\n"; // Delete value
			}

			// InfoTip handler
			regData.AppendFormat("[HKEY_CURRENT_USER\\SOFTWARE\\Classes\\.%s\\shellex\\{00021500-0000-0000-C000-000000000046}]\r\n", ext);
			if (enabled) {
				regData.AppendFormat("@=\"%s\"\r\n\r\n", guid);
			}
			else {
				regData += "@=-\r\n\r\n";
			}
			};

		// Comic Book Formats
		addHandler("cbz", config.cbz);
		addHandler("cbr", config.cbr);
		addHandler("cb7", config.cb7);
		addHandler("cbt", config.cbt);

		// E-Book Formats
		addHandler("epub", config.epub);
		addHandler("mobi", config.mobi);
		addHandler("azw", config.azw);
		addHandler("azw3", config.azw3);

		// Archive Formats
		addHandler("zip", config.zip);
		addHandler("rar", config.rar);
		addHandler("7z", config.z7);
		addHandler("tar", config.tar);

		// Photo & Other
		addHandler("phz", config.phz);
		addHandler("fb2", config.fb2);

		// Modern Image Formats
		addHandler("webp", config.webp);
		addHandler("heif", config.heif);
		addHandler("heic", config.heif); // HEIF uses both extensions
		addHandler("avif", config.avif);
		addHandler("jxl", config.jxl);

		// Media & Documents (multi-extension support)
		if (config.video) {
			addHandler("mp4", true);
			addHandler("avi", true);
			addHandler("mkv", true);
			addHandler("wmv", true);
		}
		else {
			addHandler("mp4", false);
			addHandler("avi", false);
			addHandler("mkv", false);
			addHandler("wmv", false);
		}

		addHandler("pdf", config.pdf);
		addHandler("tif", config.tiff);
		addHandler("tiff", config.tiff);
		addHandler("svg", config.svg);

		// RAW formats (multiple extensions)
		if (config.raw) {
			addHandler("dng", true);
			addHandler("cr2", true);
			addHandler("cr3", true);
			addHandler("nef", true);
			addHandler("arw", true);
			addHandler("orf", true);
		}
		else {
			addHandler("dng", false);
			addHandler("cr2", false);
			addHandler("cr3", false);
			addHandler("nef", false);
			addHandler("arw", false);
			addHandler("orf", false);
		}

		// Options (stored in app-specific registry key)
		regData += "; Application Settings\r\n";
		regData += "[HKEY_CURRENT_USER\\Software\\T800 Productions\\{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}]\r\n";
		regData.AppendFormat("\"SortOpt\"=dword:%08x\r\n", config.sortOpt ? 1 : 0);
		regData.AppendFormat("\"ShowIconOpt\"=dword:%08x\r\n", config.showIconOpt ? 1 : 0);
		regData.AppendFormat("\"CollageMode\"=dword:%08x\r\n", config.collageMode);
		regData += "\r\n";

		DWORD written;
		hr = file.Write(regData.GetString(), regData.GetLength(), &written);
		file.Close();

		return SUCCEEDED(hr);
	}

	static bool SaveConfigToFile(LPCTSTR filename, const ConfigSnapshot& config) {
		CAtlFile file;
		HRESULT hr = file.Create(filename, GENERIC_WRITE, 0, CREATE_ALWAYS);
		if (FAILED(hr)) return false;

		CStringA json;
		json = "{\r\n";
		json += "  \"version\": \"1.0\",\r\n";
		json += "  \"application\": \"ExplorerLens Shell Manager\",\r\n";
		json.AppendFormat("  \"timestamp\": \"%s\",\r\n", CT2A(config.GetTimestamp()));
		json += "  \"formats\": {\r\n";

		// Format handlers
		json.AppendFormat("    \"cbz\": %s,\r\n", config.cbz ? "true" : "false");
		json.AppendFormat("    \"cbr\": %s,\r\n", config.cbr ? "true" : "false");
		json.AppendFormat("    \"cb7\": %s,\r\n", config.cb7 ? "true" : "false");
		json.AppendFormat("    \"cbt\": %s,\r\n", config.cbt ? "true" : "false");
		json.AppendFormat("    \"epub\": %s,\r\n", config.epub ? "true" : "false");
		json.AppendFormat("    \"mobi\": %s,\r\n", config.mobi ? "true" : "false");
		json.AppendFormat("    \"azw\": %s,\r\n", config.azw ? "true" : "false");
		json.AppendFormat("    \"azw3\": %s,\r\n", config.azw3 ? "true" : "false");
		json.AppendFormat("    \"zip\": %s,\r\n", config.zip ? "true" : "false");
		json.AppendFormat("    \"rar\": %s,\r\n", config.rar ? "true" : "false");
		json.AppendFormat("    \"7z\": %s,\r\n", config.z7 ? "true" : "false");
		json.AppendFormat("    \"tar\": %s,\r\n", config.tar ? "true" : "false");
		json.AppendFormat("    \"phz\": %s,\r\n", config.phz ? "true" : "false");
		json.AppendFormat("    \"fb2\": %s,\r\n", config.fb2 ? "true" : "false");
		json.AppendFormat("    \"webp\": %s,\r\n", config.webp ? "true" : "false");
		json.AppendFormat("    \"heif\": %s,\r\n", config.heif ? "true" : "false");
		json.AppendFormat("    \"avif\": %s,\r\n", config.avif ? "true" : "false");
		json.AppendFormat("    \"jxl\": %s,\r\n", config.jxl ? "true" : "false");
		json.AppendFormat("    \"video\": %s,\r\n", config.video ? "true" : "false");
		json.AppendFormat("    \"pdf\": %s,\r\n", config.pdf ? "true" : "false");
		json.AppendFormat("    \"tiff\": %s,\r\n", config.tiff ? "true" : "false");
		json.AppendFormat("    \"svg\": %s,\r\n", config.svg ? "true" : "false");
		json.AppendFormat("    \"raw\": %s\r\n", config.raw ? "true" : "false");

		json += "  },\r\n";
		json += "  \"options\": {\r\n";
		json.AppendFormat("    \"sortPages\": %s,\r\n", config.sortOpt ? "true" : "false");
		json.AppendFormat("    \"showIconOverlay\": %s,\r\n", config.showIconOpt ? "true" : "false");
		json.AppendFormat("    \"collageMode\": %d\r\n", config.collageMode);
		json += "  }\r\n";
		json += "}\r\n";

		DWORD written;
		hr = file.Write(json.GetString(), json.GetLength(), &written);
		file.Close();

		return SUCCEEDED(hr);
	}
};

#endif // _CHANGESUMMARYDLG_H_
