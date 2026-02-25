///////////////////////////////////////////////////////////////////////////////
// ZipWrapper.h — Lightweight ZIP Archive Reader
//
// Extracted from LENSArchive.h (Sprint 355). Wraps minizip-ng (mz_zip)
// to provide a simple C++ interface for reading ZIP-based archives
// (ZIP, CBZ, EPUB, DOCX, XLSX, ODP, etc.).
//
// Thread safety: NOT thread-safe. Create one CUnzipWrapper per thread.
//
// Usage:
//   CUnzipWrapper zip;
//   if (zip.Open(L"archive.cbz")) {
//       auto entries = zip.ListEntries();
//       auto data = zip.ExtractFirst(IsImageFile);
//       zip.Close();
//   }
///////////////////////////////////////////////////////////////////////////////

#ifndef _ZIPWRAPPER_H_
#define _ZIPWRAPPER_H_

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <windows.h>

// Forward declarations for minizip-ng types
typedef void* mz_zip_file;

/// Entry metadata from a ZIP archive
struct ZipEntryInfo {
    std::wstring filename;
    uint64_t compressedSize = 0;
    uint64_t uncompressedSize = 0;
    uint32_t crc32 = 0;
    uint16_t compressionMethod = 0;   // 0=store, 8=deflate, etc.
    bool isDirectory = false;
    bool isEncrypted = false;
};

/// Result of a ZIP extraction operation
struct ZipExtractResult {
    bool success = false;
    std::vector<uint8_t> data;
    ZipEntryInfo entry;
    std::string errorMessage;
};

/// Lightweight ZIP reader wrapping minizip-ng
class CUnzipWrapper {
public:
    CUnzipWrapper() = default;
    ~CUnzipWrapper() { Close(); }

    // Non-copyable
    CUnzipWrapper(const CUnzipWrapper&) = delete;
    CUnzipWrapper& operator=(const CUnzipWrapper&) = delete;

    /// Open a ZIP archive from file path
    bool Open(const wchar_t* filePath) {
        Close();
        if (!filePath) return false;

        // Convert to UTF-8 for minizip
        int len = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, nullptr, 0, nullptr, nullptr);
        if (len <= 0) return false;
        m_utf8Path.resize(len);
        WideCharToMultiByte(CP_UTF8, 0, filePath, -1, m_utf8Path.data(), len, nullptr, nullptr);

        m_isOpen = OpenArchive(m_utf8Path.c_str());
        return m_isOpen;
    }

    /// Open a ZIP archive from IStream (COM shell extension path)
    bool OpenFromStream(IStream* pStream) {
        Close();
        if (!pStream) return false;
        m_isOpen = OpenArchiveFromStream(pStream);
        return m_isOpen;
    }

    /// Close the archive
    void Close() {
        if (m_isOpen) {
            CloseArchive();
            m_isOpen = false;
        }
        m_entries.clear();
    }

    /// Check if archive is open
    bool IsOpen() const { return m_isOpen; }

    /// List all entries in the archive
    const std::vector<ZipEntryInfo>& ListEntries() {
        if (m_entries.empty() && m_isOpen) {
            EnumerateEntries();
        }
        return m_entries;
    }

    /// Get the number of entries
    uint32_t GetEntryCount() {
        if (m_entries.empty() && m_isOpen)
            EnumerateEntries();
        return static_cast<uint32_t>(m_entries.size());
    }

    /// Extract first entry matching a predicate
    /// @param pred Function returning true for entries to extract
    ZipExtractResult ExtractFirst(std::function<bool(const ZipEntryInfo&)> pred) {
        ZipExtractResult result;
        if (!m_isOpen) {
            result.errorMessage = "Archive not open";
            return result;
        }

        auto& entries = ListEntries();
        for (const auto& entry : entries) {
            if (!entry.isDirectory && pred(entry)) {
                result = ExtractEntry(entry);
                if (result.success) return result;
            }
        }
        result.errorMessage = "No matching entry found";
        return result;
    }

    /// Extract entry by index
    ZipExtractResult ExtractByIndex(uint32_t index) {
        ZipExtractResult result;
        auto& entries = ListEntries();
        if (index >= entries.size()) {
            result.errorMessage = "Index out of range";
            return result;
        }
        return ExtractEntry(entries[index]);
    }

    /// Find first image entry (by extension)
    ZipExtractResult ExtractFirstImage() {
        return ExtractFirst([](const ZipEntryInfo& e) {
            return IsImageExtension(e.filename);
        });
    }

    /// Check if a filename has an image extension
    static bool IsImageExtension(const std::wstring& name) {
        auto dot = name.rfind(L'.');
        if (dot == std::wstring::npos) return false;
        std::wstring ext = name.substr(dot);
        // Convert to lowercase
        for (auto& c : ext) c = static_cast<wchar_t>(towlower(c));
        return ext == L".jpg" || ext == L".jpeg" || ext == L".png" ||
               ext == L".bmp" || ext == L".gif"  || ext == L".webp" ||
               ext == L".tiff" || ext == L".tif" || ext == L".avif" ||
               ext == L".jxl"  || ext == L".heif" || ext == L".heic";
    }

    /// Sort entries by name (natural sort for comic archives)
    void SortEntriesByName() {
        std::sort(m_entries.begin(), m_entries.end(),
            [](const ZipEntryInfo& a, const ZipEntryInfo& b) {
                return a.filename < b.filename;
            });
    }

private:
    bool OpenArchive(const char* utf8Path);
    bool OpenArchiveFromStream(IStream* pStream);
    void CloseArchive();
    void EnumerateEntries();
    ZipExtractResult ExtractEntry(const ZipEntryInfo& entry);

    bool m_isOpen = false;
    std::string m_utf8Path;
    std::vector<ZipEntryInfo> m_entries;
    void* m_zipHandle = nullptr;  // opaque minizip-ng handle
};

#endif // _ZIPWRAPPER_H_
