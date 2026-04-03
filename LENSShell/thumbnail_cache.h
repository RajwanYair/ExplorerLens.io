#pragma once

#include <shlobj.h>

#include <windows.h>

#include <string>

namespace ExplorerLens {

class ThumbnailCache
{
  public:
    // Initialize cache directory
    static bool Initialize();

    // Generate cache key from archive path and metadata
    static std::wstring GenerateCacheKey(const std::wstring& archivePath, const std::wstring& imageName,
                                         ULONGLONG fileSize, FILETIME lastModified);

    // Check if cached thumbnail exists and is valid
    static bool IsCached(const std::wstring& archivePath, const std::wstring& imageName, ULONGLONG fileSize,
                         FILETIME lastModified);

    // Load cached thumbnail
    static HBITMAP LoadFromCache(const std::wstring& archivePath, const std::wstring& imageName, ULONGLONG fileSize,
                                 FILETIME lastModified);

    // Save thumbnail to cache
    static bool SaveToCache(const std::wstring& archivePath, const std::wstring& imageName, ULONGLONG fileSize,
                            FILETIME lastModified, HBITMAP hBitmap);

    // Get cache directory path
    static std::wstring GetCacheDirectory();

    // Clear old cache entries (keep cache under 500 MB)
    static void CleanupCache();

  private:
    static std::wstring MD5Hash(const std::wstring& input);
    static std::wstring GetCachePath(const std::wstring& cacheKey);
    static bool SaveBitmapToFile(HBITMAP hBitmap, const std::wstring& filePath);
    static HBITMAP LoadBitmapFromFile(const std::wstring& filePath);
};

}  // namespace ExplorerLens
