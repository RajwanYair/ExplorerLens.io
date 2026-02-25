///////////////////////////////////////////////////////////////////////////////
// RarWrapper.h — RAR Archive Reader
//
// Extracted from LENSArchive.h. Wraps the UnRAR DLL API
// to provide a simple C++ interface for reading RAR-based archives
// (RAR, CBR). Supports RAR5 and legacy RAR4 formats.
//
// Thread safety: NOT thread-safe. Create one CUnrarWrapper per thread.
//
// Dependencies: UnRAR.dll (loaded dynamically via LoadLibrary)
//
// Usage:
//   CUnrarWrapper rar;
//   if (rar.Open(L"archive.cbr")) {
//       auto data = rar.ExtractFirstImage();
//       rar.Close();
//   }
///////////////////////////////////////////////////////////////////////////////

#ifndef _RARWRAPPER_H_
#define _RARWRAPPER_H_

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <windows.h>

// UnRAR operation modes
#define RAR_OM_LIST     0
#define RAR_OM_EXTRACT  1
#define RAR_OM_LIST_INCSPLIT 2

// UnRAR callback messages
#define UCM_CHANGEVOLUME   0
#define UCM_PROCESSDATA    1
#define UCM_NEEDPASSWORD   2

// UnRAR error codes
#define ERAR_SUCCESS            0
#define ERAR_END_ARCHIVE       10
#define ERAR_NO_MEMORY         11
#define ERAR_BAD_DATA          12
#define ERAR_BAD_ARCHIVE       13
#define ERAR_UNKNOWN_FORMAT    14
#define ERAR_EOPEN             15
#define ERAR_ECREATE           16
#define ERAR_ECLOSE            17
#define ERAR_EREAD             18
#define ERAR_EWRITE            19
#define ERAR_SMALL_BUF         20
#define ERAR_UNKNOWN           21
#define ERAR_MISSING_PASSWORD  22

/// Entry metadata from a RAR archive
struct RarEntryInfo {
    std::wstring filename;
    uint64_t compressedSize = 0;
    uint64_t uncompressedSize = 0;
    uint32_t crc32 = 0;
    uint32_t hostOS = 0;
    bool isDirectory = false;
    bool isEncrypted = false;
    bool isSolid = false;
};

/// Result of a RAR extraction operation
struct RarExtractResult {
    bool success = false;
    std::vector<uint8_t> data;
    RarEntryInfo entry;
    std::string errorMessage;
    int errorCode = 0;
};

/// UnRAR DLL function pointers (dynamically loaded)
struct UnRARDLLFunctions {
    typedef HANDLE (PASCAL *RAROpenArchiveExFn)(void*);
    typedef int    (PASCAL *RARCloseArchiveFn)(HANDLE);
    typedef int    (PASCAL *RARReadHeaderExFn)(HANDLE, void*);
    typedef int    (PASCAL *RARProcessFileWFn)(HANDLE, int, const wchar_t*, const wchar_t*);
    typedef void   (PASCAL *RARSetCallbackFn)(HANDLE, void*, LPARAM);

    RAROpenArchiveExFn  OpenArchiveEx = nullptr;
    RARCloseArchiveFn   CloseArchive  = nullptr;
    RARReadHeaderExFn   ReadHeaderEx  = nullptr;
    RARProcessFileWFn   ProcessFileW  = nullptr;
    RARSetCallbackFn    SetCallback   = nullptr;
    HMODULE             hModule       = nullptr;
};

/// Lightweight RAR reader wrapping UnRAR DLL
class CUnrarWrapper {
public:
    CUnrarWrapper() {
        LoadUnRARDLL();
    }

    ~CUnrarWrapper() {
        Close();
        FreeUnRARDLL();
    }

    // Non-copyable
    CUnrarWrapper(const CUnrarWrapper&) = delete;
    CUnrarWrapper& operator=(const CUnrarWrapper&) = delete;

    /// Check if UnRAR DLL is loaded and functional
    bool IsDLLLoaded() const { return m_dll.hModule != nullptr; }

    /// Open a RAR archive
    bool Open(const wchar_t* filePath) {
        Close();
        if (!filePath || !IsDLLLoaded()) return false;
        m_filePath = filePath;
        m_isOpen = OpenArchive(filePath);
        return m_isOpen;
    }

    /// Close the archive
    void Close() {
        if (m_isOpen && m_handle) {
            if (m_dll.CloseArchive)
                m_dll.CloseArchive(m_handle);
            m_handle = nullptr;
            m_isOpen = false;
        }
        m_entries.clear();
    }

    /// Check if archive is open
    bool IsOpen() const { return m_isOpen; }

    /// List all entries
    const std::vector<RarEntryInfo>& ListEntries() {
        if (m_entries.empty() && m_isOpen) {
            EnumerateEntries();
        }
        return m_entries;
    }

    /// Extract first image entry
    RarExtractResult ExtractFirstImage() {
        return ExtractFirst([](const RarEntryInfo& e) {
            return IsImageExtension(e.filename);
        });
    }

    /// Extract first entry matching a predicate
    RarExtractResult ExtractFirst(std::function<bool(const RarEntryInfo&)> pred) {
        RarExtractResult result;
        if (!m_isOpen) {
            result.errorMessage = "Archive not open";
            return result;
        }
        // Re-open for extraction (UnRAR API requires sequential processing)
        Close();
        m_isOpen = OpenArchive(m_filePath.c_str());
        if (!m_isOpen) {
            result.errorMessage = "Failed to reopen archive for extraction";
            return result;
        }
        result = ExtractFirstMatch(pred);
        return result;
    }

    /// Check if filename has an image extension
    static bool IsImageExtension(const std::wstring& name) {
        auto dot = name.rfind(L'.');
        if (dot == std::wstring::npos) return false;
        std::wstring ext = name.substr(dot);
        for (auto& c : ext) c = static_cast<wchar_t>(towlower(c));
        return ext == L".jpg" || ext == L".jpeg" || ext == L".png" ||
               ext == L".bmp" || ext == L".gif"  || ext == L".webp" ||
               ext == L".tiff" || ext == L".tif";
    }

    /// Get error description for UnRAR error code
    static const char* ErrorCodeToString(int code) {
        switch (code) {
            case ERAR_SUCCESS:          return "Success";
            case ERAR_END_ARCHIVE:      return "End of archive";
            case ERAR_NO_MEMORY:        return "Not enough memory";
            case ERAR_BAD_DATA:         return "Bad data (CRC error)";
            case ERAR_BAD_ARCHIVE:      return "Bad archive format";
            case ERAR_UNKNOWN_FORMAT:   return "Unknown archive format";
            case ERAR_EOPEN:            return "Cannot open file";
            case ERAR_ECREATE:          return "Cannot create file";
            case ERAR_ECLOSE:           return "Cannot close file";
            case ERAR_EREAD:            return "Read error";
            case ERAR_EWRITE:           return "Write error";
            case ERAR_SMALL_BUF:        return "Buffer too small";
            case ERAR_MISSING_PASSWORD: return "Password required";
            default:                    return "Unknown error";
        }
    }

private:
    void LoadUnRARDLL() {
        m_dll.hModule = LoadLibraryW(L"UnRAR.dll");
        if (!m_dll.hModule) return;
        m_dll.OpenArchiveEx = (UnRARDLLFunctions::RAROpenArchiveExFn)
            GetProcAddress(m_dll.hModule, "RAROpenArchiveEx");
        m_dll.CloseArchive = (UnRARDLLFunctions::RARCloseArchiveFn)
            GetProcAddress(m_dll.hModule, "RARCloseArchive");
        m_dll.ReadHeaderEx = (UnRARDLLFunctions::RARReadHeaderExFn)
            GetProcAddress(m_dll.hModule, "RARReadHeaderEx");
        m_dll.ProcessFileW = (UnRARDLLFunctions::RARProcessFileWFn)
            GetProcAddress(m_dll.hModule, "RARProcessFileW");
        m_dll.SetCallback = (UnRARDLLFunctions::RARSetCallbackFn)
            GetProcAddress(m_dll.hModule, "RARSetCallback");
    }

    void FreeUnRARDLL() {
        if (m_dll.hModule) {
            FreeLibrary(m_dll.hModule);
            m_dll = {};
        }
    }

    bool OpenArchive(const wchar_t* filePath);
    void EnumerateEntries();
    RarExtractResult ExtractFirstMatch(std::function<bool(const RarEntryInfo&)> pred);

    bool m_isOpen = false;
    HANDLE m_handle = nullptr;
    std::wstring m_filePath;
    std::vector<RarEntryInfo> m_entries;
    UnRARDLLFunctions m_dll = {};
};

#endif // _RARWRAPPER_H_
