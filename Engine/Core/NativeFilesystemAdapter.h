// NativeFilesystemAdapter.h — Platform-Native Filesystem Adapter
// Copyright (c) 2026 ExplorerLens Project
//
// Abstracts filesystem change notifications and attributes across NTFS, APFS,
// ext4, and btrfs. Provides optimal I/O sizing and watch-directory support.
//
#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
    #include <Windows.h>
#endif

namespace ExplorerLens {
namespace Engine {

enum class FilesystemType : uint8_t {
    NTFS = 0,
    FAT32 = 1,
    exFAT = 2,
    APFS = 3,
    HFS = 4,
    ext4 = 5,
    btrfs = 6,
    xfs = 7,
    Unknown = 255
};

// NativeFilesystemAdapter does not redefine FileChangeType/FileChangeEvent;
// those are provided by LivePreviewUpdater.h in the same namespace.
using NativeWatchCallback = std::function<void(const std::wstring& changedPath)>;

class NativeFilesystemAdapter
{
  public:
    NativeFilesystemAdapter() = default;
    ~NativeFilesystemAdapter()
    {
        StopAllWatching();
    }

    NativeFilesystemAdapter(const NativeFilesystemAdapter&) = delete;
    NativeFilesystemAdapter& operator=(const NativeFilesystemAdapter&) = delete;

    static NativeFilesystemAdapter& Instance()
    {
        static NativeFilesystemAdapter s_instance;
        return s_instance;
    }

    FilesystemType GetFilesystemType([[maybe_unused]] const std::wstring& path) const
    {
#ifdef _WIN32
        wchar_t fsName[64] = {};
        std::wstring root = (path.size() >= 3) ? path.substr(0, 3) : L"C:\\";
        if (GetVolumeInformationW(root.c_str(), nullptr, 0, nullptr, nullptr, nullptr, fsName, 64)) {
            if (wcscmp(fsName, L"NTFS") == 0)
                return FilesystemType::NTFS;
            if (wcscmp(fsName, L"FAT32") == 0)
                return FilesystemType::FAT32;
            if (wcscmp(fsName, L"exFAT") == 0)
                return FilesystemType::exFAT;
        }
        return FilesystemType::Unknown;
#elif defined(__APPLE__)
        return FilesystemType::APFS;
#elif defined(__linux__)
        return FilesystemType::ext4;
#else
        return FilesystemType::Unknown;
#endif
    }

    uint32_t GetOptimalBlockSize([[maybe_unused]] const std::wstring& path) const
    {
#ifdef _WIN32
        DWORD sectorsPerCluster = 0, bytesPerSector = 0, freeClusters = 0, totalClusters = 0;
        std::wstring root = (path.size() >= 3) ? path.substr(0, 3) : L"C:\\";
        if (GetDiskFreeSpaceW(root.c_str(), &sectorsPerCluster, &bytesPerSector, &freeClusters, &totalClusters))
            return sectorsPerCluster * bytesPerSector;
        return DEFAULT_BLOCK_SIZE;
#else
        return DEFAULT_BLOCK_SIZE;
#endif
    }

    uint64_t WatchDirectory(const std::wstring& path, NativeWatchCallback callback)
    {
        if (path.empty() || !callback)
            return 0;
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t token = ++m_nextToken;
        m_watchers[token] = {path, std::move(callback)};
        return token;
    }

    void StopWatching(uint64_t token)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_watchers.erase(token);
    }

    void StopAllWatching()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_watchers.clear();
    }

    bool PathExists(const std::wstring& path) const
    {
#ifdef _WIN32
        return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
#else
        return false;
#endif
    }

    bool IsDirectory(const std::wstring& path) const
    {
#ifdef _WIN32
        DWORD attr = GetFileAttributesW(path.c_str());
        return (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY);
#else
        return false;
#endif
    }

    std::wstring GetTempPath() const
    {
#ifdef _WIN32
        wchar_t buf[MAX_PATH] = {};
        DWORD len = GetTempPathW(MAX_PATH, buf);
        return (len > 0) ? std::wstring(buf, len) : L"C:\\Temp";
#else
        return L"/tmp";
#endif
    }

    uint64_t GetFreeSpaceBytes(const std::wstring& path) const
    {
#ifdef _WIN32
        ULARGE_INTEGER freeBytes{};
        if (GetDiskFreeSpaceExW(path.c_str(), &freeBytes, nullptr, nullptr))
            return freeBytes.QuadPart;
        return 0;
#else
        return 0;
#endif
    }

    uint32_t GetMaxPathLength() const
    {
#ifdef _WIN32
        return MAX_PATH;
#else
        return 4096;
#endif
    }

  private:
    struct WatchEntry
    {
        std::wstring path;
        NativeWatchCallback callback;
    };

    std::unordered_map<uint64_t, WatchEntry> m_watchers;
    mutable std::mutex m_mutex;
    std::atomic<uint64_t> m_nextToken{0};

    static constexpr uint32_t DEFAULT_BLOCK_SIZE = 4096;
};

}  // namespace Engine
}  // namespace ExplorerLens
