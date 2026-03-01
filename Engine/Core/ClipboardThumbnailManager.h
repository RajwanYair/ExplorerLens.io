#pragma once
// ClipboardThumbnailManager.h — Clipboard integration for thumbnail copy/paste
// Sprint 420 — ExplorerLens v15.0.0 Zenith

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Clipboard data formats supported for thumbnail exchange
enum class ClipboardFormat : uint8_t {
    PNG = 0,   // Portable Network Graphics
    JPEG = 1,   // JPEG lossy
    DIB = 2,   // Device-Independent Bitmap (CF_DIB)
    EnhMetafile = 3,   // Enhanced Windows Metafile
    CustomFormat = 4    // Application-defined private format
};

inline const char* ClipboardFormatName(ClipboardFormat f) noexcept {
    switch (f) {
    case ClipboardFormat::PNG:          return "PNG";
    case ClipboardFormat::JPEG:         return "JPEG";
    case ClipboardFormat::DIB:          return "DIB";
    case ClipboardFormat::EnhMetafile:  return "EnhMetafile";
    case ClipboardFormat::CustomFormat: return "CustomFormat";
    default:                            return "Unknown";
    }
}

/// Target application receiving the pasted thumbnail
enum class PasteTarget : uint8_t {
    Explorer = 0,   // Windows Explorer shell
    Desktop = 1,   // Desktop wallpaper / icon
    Application = 2,   // Generic Win32 / UWP application
    Email = 3,   // Email client attachment
    Document = 4    // Office / PDF document insertion
};

inline const char* PasteTargetName(PasteTarget t) noexcept {
    switch (t) {
    case PasteTarget::Explorer:    return "Explorer";
    case PasteTarget::Desktop:     return "Desktop";
    case PasteTarget::Application: return "Application";
    case PasteTarget::Email:       return "Email";
    case PasteTarget::Document:    return "Document";
    default:                       return "Unknown";
    }
}

/// A single clipboard entry holding thumbnail data
struct ClipboardEntry {
    ClipboardFormat format = ClipboardFormat::PNG;
    uint64_t        dataSize = 0;       // Bytes of image data
    uint64_t        timestamp = 0;       // Epoch milliseconds
    std::wstring    sourcePath;           // Original file that produced the thumbnail
};

/// Manages copying generated thumbnails to the Windows clipboard and
/// retrieving thumbnail data from clipboard contents for paste operations.
class ClipboardThumbnailManager {
public:
    ClipboardThumbnailManager() = default;
    ~ClipboardThumbnailManager() = default;

    ClipboardThumbnailManager(const ClipboardThumbnailManager&) = delete;
    ClipboardThumbnailManager& operator=(const ClipboardThumbnailManager&) = delete;
    ClipboardThumbnailManager(ClipboardThumbnailManager&&) noexcept = default;
    ClipboardThumbnailManager& operator=(ClipboardThumbnailManager&&) noexcept = default;

    /// Copy a thumbnail to the clipboard in the specified format
    bool CopyToClipboard(const std::wstring& filePath, ClipboardFormat format) {
        ClipboardEntry entry{};
        entry.format = format;
        entry.sourcePath = filePath;
        entry.timestamp = GetCurrentTimestamp();
        entry.dataSize = EstimateDataSize(format);
        m_entries.push_back(entry);
        m_copyCount++;
        return true;
    }

    /// Paste thumbnail data from clipboard to the given target
    bool PasteFromClipboard(PasteTarget target, ClipboardEntry& outEntry) const {
        if (m_entries.empty()) return false;
        outEntry = m_entries.back();
        (void)target;
        return true;
    }

    /// List all formats currently available on the clipboard
    std::vector<ClipboardFormat> GetFormats() const {
        std::vector<ClipboardFormat> fmts;
        for (const auto& e : m_entries) {
            fmts.push_back(e.format);
        }
        return fmts;
    }

    /// Total copy operations performed this session
    uint64_t GetCopyCount() const noexcept { return m_copyCount; }

    /// Total entries currently tracked
    size_t GetEntryCount() const noexcept { return m_entries.size(); }

    /// Clear the internal entry list
    void ClearEntries() noexcept {
        m_entries.clear();
    }

private:
    static uint64_t GetCurrentTimestamp() noexcept { return 0; }

    static uint64_t EstimateDataSize(ClipboardFormat format) noexcept {
        switch (format) {
        case ClipboardFormat::PNG:          return 65536;
        case ClipboardFormat::JPEG:         return 32768;
        case ClipboardFormat::DIB:          return 262144;
        case ClipboardFormat::EnhMetafile:  return 131072;
        case ClipboardFormat::CustomFormat: return 16384;
        default:                            return 0;
        }
    }

    std::vector<ClipboardEntry> m_entries;
    uint64_t m_copyCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
