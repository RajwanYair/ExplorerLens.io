#pragma once
/**
 * @file InstallerLifecycleManager.h
 * @brief Manages ExplorerLens install / upgrade / uninstall lifecycle and COM registration.
 * @version 15.0.0
 * @date 2026-03-02
 *
 * Detects installation state from the Windows registry, performs COM and shell-extension
 * registration/unregistration, handles backup/restore of registry state, and orchestrates
 * full install actions with structured logging.
 *
 * @note Header-only. Uses Windows API + C++20 standard library only.
 * @note Does NOT include <psapi.h> or <versionhelpers.h>.
 *
 * @copyright (c) 2026 ExplorerLens Contributors. All rights reserved.
 */

#include <shlobj.h>
#include <windows.h>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// @brief Manages ExplorerLens installer lifecycle including COM registration.
class InstallerLifecycleManager
{
  public:
    /// @brief Type of installer action to perform.
    enum class InstallAction : uint32_t {
        Install = 0,
        Upgrade = 1,
        Repair = 2,
        Uninstall = 3
    };

    /// @brief Snapshot of the current installation state.
    struct InstallState
    {
        bool isInstalled = false;
        std::wstring version;
        std::wstring installPath;
        bool comRegistered = false;
        bool shellExtRegistered = false;
        uint32_t registeredExtensionCount = 0;
    };

    /// @brief A single entry in an install log.
    struct LogEntry
    {
        std::wstring timestamp;
        std::wstring message;
        bool success = true;
    };

    /// @brief Full log for an install action.
    struct InstallLog
    {
        InstallAction action{};
        bool overallSuccess = true;
        std::vector<LogEntry> entries;
    };

    InstallerLifecycleManager() noexcept
    {
        InitializeSRWLock(&m_lock);
    }

    ~InstallerLifecycleManager() = default;

    InstallerLifecycleManager(const InstallerLifecycleManager&) = delete;
    InstallerLifecycleManager& operator=(const InstallerLifecycleManager&) = delete;

    // COM CLSID for ExplorerLens thumbnail handler
    static constexpr const wchar_t* kCLSID = L"{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}";
    static constexpr const wchar_t* kAppKey = L"SOFTWARE\\ExplorerLens";
    // IThumbnailProvider handler GUID
    static constexpr const wchar_t* kShellExGUID = L"{E357FCCD-A995-4576-B01F-234630154E96}";

    // -----------------------------------------------------------------
    //  State detection
    // -----------------------------------------------------------------

    /// @brief Detect the current installation state by reading the registry.
    inline InstallState DetectCurrentState() const
    {
        InstallState st{};

        // Check HKLM\SOFTWARE\ExplorerLens
        HKEY hKey = nullptr;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, kAppKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            st.isInstalled = true;
            st.version = ReadRegString(hKey, L"Version");
            st.installPath = ReadRegString(hKey, L"InstallPath");
            RegCloseKey(hKey);
        }

        // Check COM registration
        std::wstring clsidKey = std::wstring(L"CLSID\\") + kCLSID + L"\\InprocServer32";
        HKEY hCom = nullptr;
        if (RegOpenKeyExW(HKEY_CLASSES_ROOT, clsidKey.c_str(), 0, KEY_READ, &hCom) == ERROR_SUCCESS) {
            st.comRegistered = true;
            RegCloseKey(hCom);
        }

        // Count registered extensions by scanning well-known ones
        uint32_t extCount = 0;
        for (auto& ext : GetSupportedExtensions()) {
            std::wstring extKey = ext + L"\\ShellEx\\" + kShellExGUID;
            HKEY hExt = nullptr;
            if (RegOpenKeyExW(HKEY_CLASSES_ROOT, extKey.c_str(), 0, KEY_READ, &hExt) == ERROR_SUCCESS) {
                ++extCount;
                RegCloseKey(hExt);
            }
        }
        st.registeredExtensionCount = extCount;
        st.shellExtRegistered = (extCount > 0);

        return st;
    }

    // -----------------------------------------------------------------
    //  COM registration
    // -----------------------------------------------------------------

    /// @brief Register the COM InprocServer32 for ExplorerLens.
    inline bool RegisterCOM(const std::wstring& dllPath)
    {
        AcquireSRWLockExclusive(&m_lock);
        bool result = RegisterCOMInternal(dllPath);
        ReleaseSRWLockExclusive(&m_lock);
        return result;
    }

    /// @brief Remove COM registration keys.
    inline bool UnregisterCOM()
    {
        AcquireSRWLockExclusive(&m_lock);
        bool result = UnregisterCOMInternal();
        ReleaseSRWLockExclusive(&m_lock);
        return result;
    }

    // -----------------------------------------------------------------
    //  Shell extension registration
    // -----------------------------------------------------------------

    /// @brief Register thumbnail handler for a single file extension (e.g. ".png").
    inline bool RegisterShellExtension(const std::wstring& extension)
    {
        AcquireSRWLockExclusive(&m_lock);
        bool result = RegisterExtInternal(extension);
        ReleaseSRWLockExclusive(&m_lock);
        return result;
    }

    /// @brief Unregister thumbnail handler for a single file extension.
    inline bool UnregisterShellExtension(const std::wstring& extension)
    {
        AcquireSRWLockExclusive(&m_lock);
        bool result = UnregisterExtInternal(extension);
        ReleaseSRWLockExclusive(&m_lock);
        return result;
    }

    /// @brief Register for all supported extensions.
    inline bool RegisterAllExtensions()
    {
        AcquireSRWLockExclusive(&m_lock);
        bool allOk = true;
        for (auto& ext : GetSupportedExtensions()) {
            if (!RegisterExtInternal(ext))
                allOk = false;
        }
        ReleaseSRWLockExclusive(&m_lock);
        return allOk;
    }

    /// @brief Notify the shell that file associations have changed.
    inline bool NotifyShell() const
    {
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
        return true;
    }

    // -----------------------------------------------------------------
    //  Backup / Restore
    // -----------------------------------------------------------------

    /// @brief Export current registration state to a .reg-like text file.
    inline bool BackupRegistration(const std::wstring& backupPath) const
    {
        std::wofstream ofs(backupPath, std::ios::out | std::ios::trunc);
        if (!ofs.is_open())
            return false;

        ofs << L"; ExplorerLens Registration Backup\n";
        ofs << L"; Generated: " << GetTimestamp() << L"\n\n";

        auto state = DetectCurrentState();
        ofs << L"[State]\n";
        ofs << L"IsInstalled=" << (state.isInstalled ? L"1" : L"0") << L"\n";
        ofs << L"Version=" << state.version << L"\n";
        ofs << L"InstallPath=" << state.installPath << L"\n";
        ofs << L"COMRegistered=" << (state.comRegistered ? L"1" : L"0") << L"\n";
        ofs << L"RegisteredExtensionCount=" << state.registeredExtensionCount << L"\n\n";

        // Dump registered extensions
        ofs << L"[Extensions]\n";
        for (auto& ext : GetSupportedExtensions()) {
            std::wstring extKey = ext + L"\\ShellEx\\" + kShellExGUID;
            HKEY hExt = nullptr;
            if (RegOpenKeyExW(HKEY_CLASSES_ROOT, extKey.c_str(), 0, KEY_READ, &hExt) == ERROR_SUCCESS) {
                ofs << ext << L"=registered\n";
                RegCloseKey(hExt);
            }
        }

        ofs << L"\n; End of backup\n";
        ofs.close();
        return true;
    }

    /// @brief Restore registration from a backup file.
    inline bool RestoreRegistration(const std::wstring& backupPath)
    {
        std::wifstream ifs(backupPath);
        if (!ifs.is_open())
            return false;

        AcquireSRWLockExclusive(&m_lock);
        bool inExtensions = false;
        std::wstring line;
        while (std::getline(ifs, line)) {
            if (line.empty() || line[0] == L';')
                continue;
            if (line == L"[Extensions]") {
                inExtensions = true;
                continue;
            }
            if (line[0] == L'[') {
                inExtensions = false;
                continue;
            }

            if (inExtensions) {
                auto eq = line.find(L'=');
                if (eq != std::wstring::npos) {
                    std::wstring ext = line.substr(0, eq);
                    RegisterExtInternal(ext);
                }
            }
        }
        ReleaseSRWLockExclusive(&m_lock);
        return true;
    }

    // -----------------------------------------------------------------
    //  Full action orchestration
    // -----------------------------------------------------------------

    /// @brief Execute a full install/upgrade/repair/uninstall action.
    inline InstallLog PerformAction(InstallAction action, const std::wstring& dllPath)
    {
        InstallLog log;
        log.action = action;

        switch (action) {
            case InstallAction::Install:
                AppendLog(log, L"Starting Install...", true);
                if (!WriteAppRegistration(dllPath)) {
                    AppendLog(log, L"Failed to write app registration", false);
                    log.overallSuccess = false;
                    break;
                }
                AppendLog(log, L"App key written", true);
                if (!RegisterCOM(dllPath)) {
                    AppendLog(log, L"COM registration failed", false);
                    log.overallSuccess = false;
                    break;
                }
                AppendLog(log, L"COM registered", true);
                if (!RegisterAllExtensions()) {
                    AppendLog(log, L"Some extensions failed to register", false);
                }
                AppendLog(log, L"Shell extensions registered", true);
                NotifyShell();
                AppendLog(log, L"Shell notified", true);
                AppendLog(log, L"Install complete", true);
                break;

            case InstallAction::Upgrade:
                AppendLog(log, L"Starting Upgrade...", true);
                UnregisterCOM();
                AppendLog(log, L"Old COM unregistered", true);
                if (!RegisterCOM(dllPath)) {
                    AppendLog(log, L"COM re-registration failed", false);
                    log.overallSuccess = false;
                }
                AppendLog(log, L"COM re-registered", true);
                WriteAppRegistration(dllPath);
                NotifyShell();
                AppendLog(log, L"Upgrade complete", true);
                break;

            case InstallAction::Repair:
                AppendLog(log, L"Starting Repair...", true);
                RegisterCOM(dllPath);
                RegisterAllExtensions();
                WriteAppRegistration(dllPath);
                NotifyShell();
                AppendLog(log, L"Repair complete", true);
                break;

            case InstallAction::Uninstall:
                AppendLog(log, L"Starting Uninstall...", true);
                UnregisterAllExtensions();
                AppendLog(log, L"Extensions unregistered", true);
                UnregisterCOM();
                AppendLog(log, L"COM unregistered", true);
                RemoveAppRegistration();
                AppendLog(log, L"App key removed", true);
                NotifyShell();
                AppendLog(log, L"Uninstall complete", true);
                break;
        }
        return log;
    }

  private:
    SRWLOCK m_lock{};

    // -- Internal COM helpers (caller must hold write lock) --

    inline bool RegisterCOMInternal(const std::wstring& dllPath)
    {
        std::wstring clsidPath = std::wstring(L"CLSID\\") + kCLSID;
        HKEY hKey = nullptr;
        DWORD disp = 0;
        if (RegCreateKeyExW(HKEY_CLASSES_ROOT, clsidPath.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE,
                            nullptr, &hKey, &disp)
            != ERROR_SUCCESS)
            return false;

        const wchar_t* desc = L"ExplorerLens Thumbnail Handler";
        RegSetValueExW(hKey, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(desc),
                       static_cast<DWORD>((wcslen(desc) + 1) * sizeof(wchar_t)));
        RegCloseKey(hKey);

        // InprocServer32
        std::wstring ips32Path = clsidPath + L"\\InprocServer32";
        if (RegCreateKeyExW(HKEY_CLASSES_ROOT, ips32Path.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE,
                            nullptr, &hKey, &disp)
            != ERROR_SUCCESS)
            return false;

        RegSetValueExW(hKey, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(dllPath.c_str()),
                       static_cast<DWORD>((dllPath.size() + 1) * sizeof(wchar_t)));

        const wchar_t* model = L"Apartment";
        RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ, reinterpret_cast<const BYTE*>(model),
                       static_cast<DWORD>((wcslen(model) + 1) * sizeof(wchar_t)));
        RegCloseKey(hKey);
        return true;
    }

    inline bool UnregisterCOMInternal()
    {
        std::wstring ips32 = std::wstring(L"CLSID\\") + kCLSID + L"\\InprocServer32";
        RegDeleteKeyW(HKEY_CLASSES_ROOT, ips32.c_str());

        std::wstring clsid = std::wstring(L"CLSID\\") + kCLSID;
        return RegDeleteKeyW(HKEY_CLASSES_ROOT, clsid.c_str()) == ERROR_SUCCESS;
    }

    inline bool RegisterExtInternal(const std::wstring& extension)
    {
        std::wstring keyPath = extension + L"\\ShellEx\\" + kShellExGUID;
        HKEY hKey = nullptr;
        DWORD disp = 0;
        if (RegCreateKeyExW(HKEY_CLASSES_ROOT, keyPath.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr,
                            &hKey, &disp)
            != ERROR_SUCCESS)
            return false;

        RegSetValueExW(hKey, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(kCLSID),
                       static_cast<DWORD>((wcslen(kCLSID) + 1) * sizeof(wchar_t)));
        RegCloseKey(hKey);
        return true;
    }

    inline bool UnregisterExtInternal(const std::wstring& extension)
    {
        std::wstring keyPath = extension + L"\\ShellEx\\" + kShellExGUID;
        return RegDeleteKeyW(HKEY_CLASSES_ROOT, keyPath.c_str()) == ERROR_SUCCESS;
    }

    inline void UnregisterAllExtensions()
    {
        for (auto& ext : GetSupportedExtensions()) {
            UnregisterExtInternal(ext);
        }
    }

    inline bool WriteAppRegistration(const std::wstring& dllPath)
    {
        HKEY hKey = nullptr;
        DWORD disp = 0;
        if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, kAppKey, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey,
                            &disp)
            != ERROR_SUCCESS)
            return false;

        const wchar_t* ver = L"15.0.0";
        RegSetValueExW(hKey, L"Version", 0, REG_SZ, reinterpret_cast<const BYTE*>(ver),
                       static_cast<DWORD>((wcslen(ver) + 1) * sizeof(wchar_t)));

        RegSetValueExW(hKey, L"InstallPath", 0, REG_SZ, reinterpret_cast<const BYTE*>(dllPath.c_str()),
                       static_cast<DWORD>((dllPath.size() + 1) * sizeof(wchar_t)));
        RegCloseKey(hKey);
        return true;
    }

    inline void RemoveAppRegistration()
    {
        RegDeleteKeyW(HKEY_LOCAL_MACHINE, kAppKey);
    }

    // -- Utility --

    static inline std::wstring ReadRegString(HKEY hKey, const wchar_t* valueName)
    {
        wchar_t buf[512]{};
        DWORD size = sizeof(buf);
        DWORD type = 0;
        if (RegQueryValueExW(hKey, valueName, nullptr, &type, reinterpret_cast<BYTE*>(buf), &size) == ERROR_SUCCESS
            && type == REG_SZ) {
            return std::wstring(buf);
        }
        return {};
    }

    static inline std::wstring GetTimestamp()
    {
        SYSTEMTIME st{};
        GetLocalTime(&st);
        std::wostringstream ws;
        ws << std::setfill(L'0') << st.wYear << L'-' << std::setw(2) << st.wMonth << L'-' << std::setw(2) << st.wDay
           << L' ' << std::setw(2) << st.wHour << L':' << std::setw(2) << st.wMinute << L':' << std::setw(2)
           << st.wSecond;
        return ws.str();
    }

    static inline void AppendLog(InstallLog& log, const std::wstring& msg, bool ok)
    {
        LogEntry e;
        e.timestamp = GetTimestamp();
        e.message = msg;
        e.success = ok;
        log.entries.emplace_back(std::move(e));
        if (!ok)
            log.overallSuccess = false;
    }

    /// @brief Return a representative set of supported file extensions.
    static inline std::vector<std::wstring> GetSupportedExtensions()
    {
        return {
            // Images
            L".png",
            L".jpg",
            L".jpeg",
            L".bmp",
            L".gif",
            L".tiff",
            L".tif",
            L".webp",
            L".avif",
            L".jxl",
            L".heif",
            L".heic",
            L".svg",
            L".ico",
            L".tga",
            L".psd",
            L".dds",
            L".hdr",
            L".exr",
            L".pcx",
            L".ppm",
            L".pgm",
            L".pbm",
            L".pfm",
            L".qoi",
            L".xcf",
            L".sgi",
            L".rgb",
            L".farbfeld",
            L".ff",
            L".xpm",
            L".jxr",
            L".wdp",
            L".hdp",
            L".dpx",
            L".ora",
            L".vtf",
            L".ktx",
            L".ktx2",
            // RAW camera
            L".cr2",
            L".cr3",
            L".nef",
            L".arw",
            L".dng",
            L".orf",
            L".rw2",
            L".pef",
            L".srw",
            L".raf",
            L".raw",
            L".3fr",
            L".dcr",
            L".kdc",
            L".mrw",
            L".nrw",
            L".srf",
            L".x3f",
            // Archives
            L".zip",
            L".7z",
            L".rar",
            L".tar",
            L".gz",
            L".bz2",
            L".xz",
            L".zst",
            L".lz4",
            L".cab",
            // Documents
            L".pdf",
            L".epub",
            L".mobi",
            L".djvu",
            // 3D / CAD
            L".stl",
            L".obj",
            L".gltf",
            L".glb",
            L".fbx",
            L".3ds",
            L".ply",
            L".usd",
            L".usda",
            L".usdc",
            L".usdz",
            L".step",
            L".iges",
            // Video
            L".mp4",
            L".mkv",
            L".avi",
            L".mov",
            L".wmv",
            L".webm",
            L".flv",
            // Audio
            L".mp3",
            L".flac",
            L".ogg",
            L".wav",
            L".aac",
            L".wma",
            // Fonts
            L".ttf",
            L".otf",
            L".woff",
            L".woff2",
            // Scientific
            L".fits",
            L".nii",
            L".dicom",
            L".dcm",
        };
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
