// WinRTThumbnailBridge.h — WinRT Thumbnail Bridge (StorageFile / ThumbnailMode)
// Copyright (c) 2026 ExplorerLens Project
//
// Bridges the ExplorerLens thumbnail pipeline to the WinRT StorageFile thumbnail
// APIs, enabling preview in WinUI 3 apps, XAML Islands, and the modern Windows Shell.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class WinRTThumbnailMode {
    SingleItem,
    ListView,
    PicturesView,
    VideosView,
    MusicView,
    DocumentsView
};

enum class WinRTThumbnailRequestedSize {
    Small = 32,
    Medium = 128,
    Large = 256,
    ExtraLarge = 1024
};

struct WinRTThumbnailRequest
{
    std::wstring filePath;
    WinRTThumbnailMode mode = WinRTThumbnailMode::SingleItem;
    WinRTThumbnailRequestedSize requestedSize = WinRTThumbnailRequestedSize::Medium;
    bool allowSlowStorageItems = false;
};

struct WinRTThumbnailResult
{
    bool success = false;
    std::vector<uint8_t> imageBytes;  // PNG bytes
    int widthPx = 0;
    int heightPx = 0;
    std::string errorMsg;
    bool Ok() const noexcept
    {
        return success;
    }
};

using WinRTThumbCallback = std::function<void(const WinRTThumbnailResult&)>;

class WinRTThumbnailBridge
{
  public:
    using SyncThumbFn = std::function<WinRTThumbnailResult(const WinRTThumbnailRequest&)>;

    explicit WinRTThumbnailBridge() = default;
    void SetSyncHandler(SyncThumbFn fn)
    {
        m_syncFn = std::move(fn);
    }

    WinRTThumbnailResult GetThumbnail(const WinRTThumbnailRequest& req) const
    {
        if (req.filePath.empty())
            return {false, {}, 0, 0, "Empty file path"};
        if (!m_syncFn)
            return {false, {}, 0, 0, "No sync handler"};
        return m_syncFn(req);
    }

    void GetThumbnailAsync(const WinRTThumbnailRequest& req, WinRTThumbCallback cb) const
    {
        auto result = GetThumbnail(req);
        if (cb)
            cb(result);
    }

    static std::string ModeName(WinRTThumbnailMode mode) noexcept
    {
        switch (mode) {
            case WinRTThumbnailMode::SingleItem:
                return "SingleItem";
            case WinRTThumbnailMode::ListView:
                return "ListView";
            case WinRTThumbnailMode::PicturesView:
                return "PicturesView";
            case WinRTThumbnailMode::VideosView:
                return "VideosView";
            case WinRTThumbnailMode::MusicView:
                return "MusicView";
            case WinRTThumbnailMode::DocumentsView:
                return "DocumentsView";
        }
        return "Unknown";
    }

  private:
    SyncThumbFn m_syncFn;
};

}  // namespace Engine
}  // namespace ExplorerLens
