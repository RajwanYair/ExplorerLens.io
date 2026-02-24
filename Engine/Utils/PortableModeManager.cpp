//==============================================================================
// PortableModeManager
//==============================================================================

#include "PortableModeManager.h"
#include <fstream>
#include <sstream>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace ExplorerLens { namespace Engine {

PortableModeManager::PortableModeManager() {
    m_config.status = PortableStatus::Unknown;
}

PortableDetectionResult PortableModeManager::Detect() const {
    PortableDetectionResult result;

    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    result.exePath = exePath;

    if (wcslen(exePath) >= 3) {
        result.driveLetter = std::wstring(exePath, 2);
    }

    // Check for portable.ini next to exe
    std::wstring iniPath = result.exePath;
    auto lastSlash = iniPath.find_last_of(L'\\');
    if (lastSlash != std::wstring::npos) {
        iniPath = iniPath.substr(0, lastSlash + 1) + L"portable.ini";
    }

    DWORD attrs = GetFileAttributesW(iniPath.c_str());
    result.hasIniFile = (attrs != INVALID_FILE_ATTRIBUTES);

    // Check drive type
    if (result.driveLetter.size() >= 2) {
        std::wstring root = result.driveLetter + L"\\";
        UINT driveType = GetDriveTypeW(root.c_str());
        result.isPortable = (driveType == DRIVE_REMOVABLE);
    }

    result.status = result.hasIniFile ? PortableStatus::Portable :
                    result.isPortable ? PortableStatus::Portable :
                    PortableStatus::Installed;

    // Check write access
    std::wstring testFile = result.exePath;
    lastSlash = testFile.find_last_of(L'\\');
    if (lastSlash != std::wstring::npos) {
        testFile = testFile.substr(0, lastSlash + 1) + L"_write_test.tmp";
    }
    HANDLE hTest = CreateFileW(testFile.c_str(), GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);
    if (hTest != INVALID_HANDLE_VALUE) {
        result.hasWriteAccess = true;
        CloseHandle(hTest);
        DeleteFileW(testFile.c_str());
    }

    return result;
}

bool PortableModeManager::InitializePortableMode(const std::wstring& basePath) {
    m_config.status = PortableStatus::Portable;
    m_config.basePath = basePath;
    m_config.configLocation = StorageLocation::IniFile;
    m_config.configPath = basePath + L"\\config";
    m_config.cachePath = basePath + L"\\cache";
    m_config.pluginPath = basePath + L"\\plugins";

    // Create directories
    CreateDirectoryW(m_config.configPath.c_str(), nullptr);
    CreateDirectoryW(m_config.cachePath.c_str(), nullptr);
    CreateDirectoryW(m_config.pluginPath.c_str(), nullptr);

    return true;
}

bool PortableModeManager::SaveConfig(
    const std::map<std::wstring, std::wstring>& settings)
{
    std::wstring path = m_config.configPath + L"\\settings.ini";
    FILE* f = nullptr;
    _wfopen_s(&f, path.c_str(), L"w");
    if (!f) return false;

    fwprintf(f, L"[ExplorerLens]\n");
    for (const auto& [key, value] : settings) {
        fwprintf(f, L"%s=%s\n", key.c_str(), value.c_str());
    }

    fclose(f);
    return true;
}

std::map<std::wstring, std::wstring> PortableModeManager::LoadConfig() const {
    std::map<std::wstring, std::wstring> settings;
    // In production: parse INI file from m_config.configPath
    return settings;
}

const wchar_t* PortableModeManager::GetStatusName(PortableStatus status) {
    switch (status) {
        case PortableStatus::Installed: return L"Installed";
        case PortableStatus::Portable:  return L"Portable";
        case PortableStatus::Hybrid:    return L"Hybrid";
        case PortableStatus::Unknown:   return L"Unknown";
        default: return L"Unknown";
    }
}

const wchar_t* PortableModeManager::GetLocationName(StorageLocation location) {
    switch (location) {
        case StorageLocation::Registry:     return L"Registry";
        case StorageLocation::IniFile:      return L"INI File";
        case StorageLocation::JsonFile:     return L"JSON File";
        case StorageLocation::AppData:      return L"AppData";
        case StorageLocation::ExeDirectory: return L"Exe Directory";
        default: return L"Unknown";
    }
}

}} // namespace ExplorerLens::Engine

