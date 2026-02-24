// SettingsPortability.h — Settings Import/Export for ExplorerLens Manager
// ExplorerLens Manager v15.0.0 "Zenith" — Sprint 371
// Copyright (c) 2026 ExplorerLens Project
//
// Provides JSON-based settings export and import with preview/diff.
// Supports drag-and-drop config files and "Reset to Defaults" functionality.
#pragma once

#include "RegManager.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>

namespace ExplorerLens {

// ============================================================================
// Settings snapshot for export/import
// ============================================================================

struct SettingsSnapshot {
  std::wstring version;   // ExplorerLens version that created this
  std::wstring timestamp; // ISO 8601 timestamp
  bool sortEnabled = false;
  bool showIcon = true;
  bool collageMode = false;

  struct FormatSetting {
    std::wstring name; // e.g., "cbz", "webp"
    int lensType = 0;
    bool enabled = false;
  };
  std::vector<FormatSetting> formats;
};

// ============================================================================
// SettingsPortability — Export, Import, Reset
// ============================================================================

class SettingsPortability {
public:
  // ========================================================================
  // Export current settings to JSON file
  // ========================================================================
  static bool ExportToFile(const std::wstring &filePath,
                           const CRegManager &regMgr) {
    std::ofstream out(filePath);
    if (!out.is_open())
      return false;

    // Get current timestamp
    SYSTEMTIME st;
    GetLocalTime(&st);
    char timestamp[64];
    sprintf_s(timestamp, "%04d-%02d-%02dT%02d:%02d:%02d", st.wYear, st.wMonth,
              st.wDay, st.wHour, st.wMinute, st.wSecond);

    out << "{\n";
    out << "  \"_comment\": \"ExplorerLens Settings Export\",\n";
    out << "  \"version\": \"15.0.0\",\n";
    out << "  \"timestamp\": \"" << timestamp << "\",\n";
    out << "  \"settings\": {\n";
    out << "    \"sortImages\": " << (regMgr.IsSortOpt() ? "true" : "false")
        << ",\n";
    out << "    \"showIcon\": " << (regMgr.IsShowIconOpt() ? "true" : "false")
        << ",\n";
    out << "    \"collageMode\": " << (regMgr.IsCollageOpt() ? "true" : "false")
        << "\n";
    out << "  },\n";
    out << "  \"formats\": {\n";

    // Export each format handler
    static const struct {
      const char *name;
      int type;
    } fmtList[] = {
        {"cbz", 5},   {"cbr", 6},   {"cb7", 7},    {"cbt", 8},    {"zip", 1},
        {"rar", 2},   {"7z", 3},    {"tar", 4},    {"epub", 9},   {"mobi", 10},
        {"fb2", 14},  {"azw", 44},  {"azw3", 45},  {"phz", 13},   {"webp", 15},
        {"heif", 16}, {"avif", 17}, {"jxl", 18},   {"tiff", 19},  {"svg", 20},
        {"raw", 42},  {"psd", 43},  {"dds", 46},   {"hdr", 47},   {"exr", 48},
        {"ppm", 49},  {"pdf", 53},  {"video", 50}, {"audio", 51}, {"font", 52},
        {"bmp", 68},  {"gif", 69},  {"tga", 67},   {"model", 76}, {"ico", 71},
        {"qoi", 72},
    };
    constexpr size_t fmtCount = sizeof(fmtList) / sizeof(fmtList[0]);

    for (size_t i = 0; i < fmtCount; ++i) {
      bool enabled = regMgr.HasTH(fmtList[i].type);
      out << "    \"" << fmtList[i].name
          << "\": " << (enabled ? "true" : "false");
      if (i + 1 < fmtCount)
        out << ",";
      out << "\n";
    }

    out << "  }\n";
    out << "}\n";

    out.close();
    return true;
  }

  // ========================================================================
  // Preview what an import would change (returns diff description)
  // ========================================================================
  static std::wstring PreviewImport(const std::wstring &filePath,
                                    const CRegManager &regMgr) {
    std::ifstream in(filePath);
    if (!in.is_open())
      return L"Error: Cannot open file";

    std::stringstream ss;
    ss << in.rdbuf();
    std::string json = ss.str();
    in.close();

    std::wstring preview;
    preview += L"Settings Import Preview\r\n";
    preview += L"=======================\r\n\r\n";

    // Simple JSON parsing for boolean format values
    int changesCount = 0;
    auto checkFormat = [&](const char *name, int lensType) {
      std::string key = std::string("\"") + name + "\"";
      auto pos = json.find(key);
      if (pos == std::string::npos)
        return;

      auto valueStart = json.find(':', pos);
      if (valueStart == std::string::npos)
        return;

      bool newValue =
          (json.find("true", valueStart) < json.find('\n', valueStart));
      bool currentValue = regMgr.HasTH(lensType);

      if (newValue != currentValue) {
        changesCount++;
        std::wstring wname(name, name + strlen(name));
        preview += L"  " + wname + L": ";
        preview += currentValue ? L"ON → OFF" : L"OFF → ON";
        preview += L"\r\n";
      }
    };

    checkFormat("cbz", 5);
    checkFormat("cbr", 6);
    checkFormat("cb7", 7);
    checkFormat("cbt", 8);
    checkFormat("zip", 1);
    checkFormat("rar", 2);
    checkFormat("7z", 3);
    checkFormat("tar", 4);
    checkFormat("epub", 9);
    checkFormat("mobi", 10);
    checkFormat("webp", 15);
    checkFormat("heif", 16);
    checkFormat("avif", 17);
    checkFormat("jxl", 18);
    checkFormat("tiff", 19);
    checkFormat("svg", 20);
    checkFormat("raw", 42);
    checkFormat("psd", 43);
    checkFormat("dds", 46);
    checkFormat("hdr", 47);
    checkFormat("exr", 48);
    checkFormat("pdf", 53);
    checkFormat("video", 50);
    checkFormat("audio", 51);
    checkFormat("font", 52);

    if (changesCount == 0) {
      preview += L"No changes — settings are identical.\r\n";
    } else {
      preview +=
          L"\r\nTotal changes: " + std::to_wstring(changesCount) + L"\r\n";
    }

    return preview;
  }

  // ========================================================================
  // Reset all settings to defaults (all formats enabled, sort ON, icon ON)
  // ========================================================================
  static void ResetToDefaults(CRegManager &regMgr) {
    // Enable all format handlers
    static const int allTypes[] = {
        1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 13, 14, 15, 16, 17, 18, 19, 20,
        42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 67, 68, 69, 71, 72, 76};
    for (int t : allTypes) {
      regMgr.SetHandlers(t, TRUE);
    }
    regMgr.SetSortOpt(TRUE);
    regMgr.SetShowIconOpt(TRUE);
    regMgr.SetCollageOpt(FALSE);
  }

  // ========================================================================
  // Get Open/Save file dialog filter
  // ========================================================================
  static const wchar_t *GetFileFilter() {
    return L"ExplorerLens Settings (*.json)\0*.json\0All Files (*.*)\0*.*\0";
  }
};

} // namespace ExplorerLens
