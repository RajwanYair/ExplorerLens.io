// ClipboardMonitorIntegration.h — Clipboard Content Thumbnail Preview
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors clipboard for image/file content changes and generates
// live thumbnails. Supports CF_BITMAP, CF_DIB, CF_HDROP (file list),
// CF_UNICODETEXT (file paths), and PNG clipboard format.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ClipboardContentType : uint8_t {
    None,
    Bitmap,
    DIB,
    FileList,
    FilePaths,
    PNG,
    Text,
    Unknown
};

struct ClipboardSnapshot
{
    ClipboardContentType type = ClipboardContentType::None;
    uint32_t dataSize = 0;
    uint32_t fileCount = 0;
    std::vector<std::wstring> filePaths;
    uint32_t bitmapWidth = 0;
    uint32_t bitmapHeight = 0;
    uint64_t sequenceNumber = 0;
};

struct ClipboardStats
{
    uint32_t snapshotsTaken = 0;
    uint32_t bitmapClips = 0;
    uint32_t fileClips = 0;
    uint64_t totalDataBytes = 0;
};

class ClipboardMonitorIntegration
{
  public:
    ClipboardMonitorIntegration() = default;
    ~ClipboardMonitorIntegration() = default;

    static const wchar_t* GetName()
    {
        return L"ClipboardMonitorIntegration";
    }

    /// Get current clipboard sequence number (changes on each clipboard update).
    uint64_t GetSequenceNumber() const
    {
        return GetClipboardSequenceNumber();
    }

    /// Detect what type of content is on the clipboard.
    ClipboardContentType DetectContent() const
    {
        if (IsClipboardFormatAvailable(CF_HDROP))
            return ClipboardContentType::FileList;
        if (IsClipboardFormatAvailable(CF_BITMAP))
            return ClipboardContentType::Bitmap;
        if (IsClipboardFormatAvailable(CF_DIB))
            return ClipboardContentType::DIB;
        // Check for PNG format
        UINT pngFormat = RegisterClipboardFormatW(L"PNG");
        if (pngFormat && IsClipboardFormatAvailable(pngFormat))
            return ClipboardContentType::PNG;
        if (IsClipboardFormatAvailable(CF_UNICODETEXT))
            return ClipboardContentType::Text;
        return ClipboardContentType::None;
    }

    /// Take a snapshot of clipboard state (without reading data).
    ClipboardSnapshot TakeSnapshot() const
    {
        ClipboardSnapshot snap;
        snap.type = DetectContent();
        snap.sequenceNumber = GetSequenceNumber();

        if (snap.type == ClipboardContentType::FileList && OpenClipboard(nullptr)) {
            HGLOBAL hDrop = GetClipboardData(CF_HDROP);
            if (hDrop) {
                HDROP hDropData = static_cast<HDROP>(GlobalLock(hDrop));
                if (hDropData) {
                    snap.fileCount = DragQueryFileW(hDropData, 0xFFFFFFFF, nullptr, 0);
                    for (uint32_t i = 0; i < std::min(snap.fileCount, 10u); ++i) {
                        wchar_t path[MAX_PATH] = {};
                        DragQueryFileW(hDropData, i, path, MAX_PATH);
                        snap.filePaths.push_back(path);
                    }
                    GlobalUnlock(hDrop);
                }
            }
            CloseClipboard();
        }

        m_stats.snapshotsTaken++;
        if (snap.type == ClipboardContentType::Bitmap || snap.type == ClipboardContentType::DIB
            || snap.type == ClipboardContentType::PNG)
            m_stats.bitmapClips++;
        if (snap.type == ClipboardContentType::FileList)
            m_stats.fileClips++;

        return snap;
    }

    ClipboardStats GetStats() const
    {
        return m_stats;
    }

  private:
    mutable ClipboardStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
