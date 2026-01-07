#include "StdAfx.h"
#include "thumbnail_cache.h"
#include <wincrypt.h>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <gdiplus.h>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "gdiplus.lib")

namespace fs = std::filesystem;

namespace DarkThumbs {

std::wstring ThumbnailCache::GetCacheDirectory() {
    wchar_t localAppData[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, localAppData))) {
        std::wstring cachePath = std::wstring(localAppData) + L"\\DarkThumbs\\cache";
        return cachePath;
    }
    return L"";
}

bool ThumbnailCache::Initialize() {
    std::wstring cacheDir = GetCacheDirectory();
    if (cacheDir.empty()) return false;
    
    try {
        fs::create_directories(cacheDir);
        if (!fs::exists(cacheDir)) return false;
        
        // Perform cleanup on initialization to keep cache size under control
        CleanupCache();
        
        return true;
    }
    catch (...) {
        return false;
    }
}

std::wstring ThumbnailCache::MD5Hash(const std::wstring& input) {
    // Convert wstring to UTF-8 bytes
    int utf8Length = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (utf8Length <= 0) return L"";
    
    std::vector<char> utf8(utf8Length);
    WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, utf8.data(), utf8Length, nullptr, nullptr);
    
    // Calculate MD5
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    
    if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        return L"";
    
    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        return L"";
    }
    
    if (!CryptHashData(hHash, (BYTE*)utf8.data(), utf8Length - 1, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return L"";
    }
    
    BYTE hash[16];
    DWORD hashLen = 16;
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return L"";
    }
    
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    
    // Convert to hex string
    std::wstringstream ss;
    for (int i = 0; i < 16; i++) {
        ss << std::hex << std::setw(2) << std::setfill(L'0') << (int)hash[i];
    }
    
    return ss.str();
}

std::wstring ThumbnailCache::GenerateCacheKey(
    const std::wstring& archivePath,
    const std::wstring& imageName,
    ULONGLONG fileSize,
    FILETIME lastModified
) {
    // Create unique key: path + image + size + mtime
    std::wstringstream ss;
    ss << archivePath << L"|" << imageName << L"|" 
       << fileSize << L"|"
       << ((ULONGLONG)lastModified.dwHighDateTime << 32 | lastModified.dwLowDateTime);
    
    return MD5Hash(ss.str());
}

std::wstring ThumbnailCache::GetCachePath(const std::wstring& cacheKey) {
    return GetCacheDirectory() + L"\\" + cacheKey + L".png";  // PNG format for better compression
}

bool ThumbnailCache::SaveBitmapToFile(HBITMAP hBitmap, const std::wstring& filePath) {
    if (!hBitmap) return false;
    
    // Initialize GDI+
    static bool gdiplusInitialized = false;
    static ULONG_PTR gdiplusToken = 0;
    if (!gdiplusInitialized) {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
        gdiplusInitialized = true;
    }
    
    // Create GDI+ bitmap from HBITMAP
    Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromHBITMAP(hBitmap, nullptr);
    if (!bitmap) return false;
    
    // Get PNG encoder CLSID
    CLSID pngClsid;
    UINT numCodecs = 0, sizeCodecs = 0;
    Gdiplus::GetImageEncodersSize(&numCodecs, &sizeCodecs);
    if (sizeCodecs == 0) {
        delete bitmap;
        return false;
    }
    
    std::vector<BYTE> codecInfo(sizeCodecs);
    Gdiplus::ImageCodecInfo* pCodecInfo = (Gdiplus::ImageCodecInfo*)codecInfo.data();
    Gdiplus::GetImageEncoders(numCodecs, sizeCodecs, pCodecInfo);
    
    bool foundPNG = false;
    for (UINT i = 0; i < numCodecs; i++) {
        if (wcscmp(pCodecInfo[i].MimeType, L"image/png") == 0) {
            pngClsid = pCodecInfo[i].Clsid;
            foundPNG = true;
            break;
        }
    }
    
    if (!foundPNG) {
        delete bitmap;
        return false;
    }
    
    // Save as PNG (smaller than BMP)
    Gdiplus::Status status = bitmap->Save(filePath.c_str(), &pngClsid);
    delete bitmap;
    
    return (status == Gdiplus::Ok);
}

HBITMAP ThumbnailCache::LoadBitmapFromFile(const std::wstring& filePath) {
    // Initialize GDI+
    static bool gdiplusInitialized = false;
    static ULONG_PTR gdiplusToken = 0;
    if (!gdiplusInitialized) {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
        gdiplusInitialized = true;
    }
    
    // Load PNG from file
    Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(filePath.c_str());
    if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok) {
        if (bitmap) delete bitmap;
        return nullptr;
    }
    
    // Convert to HBITMAP
    HBITMAP hBitmap = nullptr;
    bitmap->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hBitmap);
    delete bitmap;
    
    return hBitmap;
}

bool ThumbnailCache::IsCached(
    const std::wstring& archivePath,
    const std::wstring& imageName,
    ULONGLONG fileSize,
    FILETIME lastModified
) {
    std::wstring cacheKey = GenerateCacheKey(archivePath, imageName, fileSize, lastModified);
    std::wstring cachePath = GetCachePath(cacheKey);
    
    return fs::exists(cachePath);
}

HBITMAP ThumbnailCache::LoadFromCache(
    const std::wstring& archivePath,
    const std::wstring& imageName,
    ULONGLONG fileSize,
    FILETIME lastModified
) {
    std::wstring cacheKey = GenerateCacheKey(archivePath, imageName, fileSize, lastModified);
    std::wstring cachePath = GetCachePath(cacheKey);
    
    if (!fs::exists(cachePath)) return nullptr;
    
    return LoadBitmapFromFile(cachePath);
}

bool ThumbnailCache::SaveToCache(
    const std::wstring& archivePath,
    const std::wstring& imageName,
    ULONGLONG fileSize,
    FILETIME lastModified,
    HBITMAP hBitmap
) {
    if (!hBitmap) return false;
    
    // Ensure cache directory exists
    if (!Initialize()) return false;
    
    std::wstring cacheKey = GenerateCacheKey(archivePath, imageName, fileSize, lastModified);
    std::wstring cachePath = GetCachePath(cacheKey);
    
    return SaveBitmapToFile(hBitmap, cachePath);
}

void ThumbnailCache::CleanupCache() {
    std::wstring cacheDir = GetCacheDirectory();
    if (cacheDir.empty() || !fs::exists(cacheDir)) return;
    
    try {
        // Calculate total cache size
        uintmax_t totalSize = 0;
        std::vector<std::pair<fs::file_time_type, fs::path>> files;
        
        for (const auto& entry : fs::directory_iterator(cacheDir)) {
            if (entry.is_regular_file()) {
                totalSize += entry.file_size();
                files.push_back({ entry.last_write_time(), entry.path() });
            }
        }
        
        // If cache > 500 MB, delete oldest files
        const uintmax_t MAX_CACHE_SIZE = 500 * 1024 * 1024; // 500 MB
        if (totalSize > MAX_CACHE_SIZE) {
            // Sort by last write time (oldest first)
            std::sort(files.begin(), files.end());
            
            // Delete oldest until under limit
            for (const auto& [time, path] : files) {
                if (totalSize <= MAX_CACHE_SIZE) break;
                
                uintmax_t fileSize = fs::file_size(path);
                fs::remove(path);
                totalSize -= fileSize;
            }
        }
    }
    catch (...) {
        // Ignore cleanup errors
    }
}

} // namespace DarkThumbs
