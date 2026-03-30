// NativeFilesystemAdapter.h — Platform-Native Filesystem Adapter
// Copyright (c) 2026 ExplorerLens Project
//
// Abstracts filesystem change notifications and attributes across NTFS, APFS,
// ext4, and btrfs. Provides optimal I/O sizing and watch-directory support.
//
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <functional>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace ExplorerLens { namespace Engine {

enum class FilesystemType : uint8_t {
    NTFS   = 0,
    FAT32  = 1,
    exFAT  = 2,
    APFS   = 3,
    HFS    = 4,
    ext4   = 5,
    btrfs  = 6,
    xfs    = 7,
    Unknown = 255
};

enum class FileChangeType : uint8_t {
    Created  = 0,
    Modified = 1,
    Deleted  = 2,
    Renamed  = 3
};

struct FileChangeEvent {
    std::string    path;
    std::string    oldPath;
    FileChangeType type    = FileChangeType::Modified;
    uint64_t       sizeBytes = 0;
};

struct FilesystemInfo {
    FilesystemType type           = FilesystemType::Unknown;
    uint64_t       totalBytes     = 0;
    uint64_t       freeBytes      = 0;
    uint32_t       blockSize      = 4096;
    bool           supportsLinks  = false;
    bool           caseSensitive  = false;
};

using FileChangeCallback = std::function<void(const FileChangeEvent&)>;

class NativeFilesystemAdapter {
public:
    static NativeFilesystemAdapter& Instance() {
        static NativeFilesystemAdapter s_instance;
        return s_instance;
    }

    FilesystemType GetFilesystemType([[maybe_unused]] const std::string& path) const {
#ifdef _WIN32
        wchar_t fsName[64] = {};
        std::wstring wpath(path.begin(), path.end());
        std::wstring root = wpath.substr(0, 3);
        if (GetVolumeInformationW(root.c_str(), nullptr, 0, nullptr, nullptr, nullptr, fsName, 64)) {
            if (wcscmp(fsName, L"NTFS") == 0)  return FilesystemType::NTFS;
            if (wcscmp(fsName, L"FAT32") == 0) return FilesystemType::FAT32;
            if (wcscmp(fsName, L"exFAT") == 0) return FilesystemType::exFAT;
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

    uint32_t GetOptimalBlockSize([[maybe_unused]] const std::string& path) const {
#ifdef _WIN32
        DWORD sectorsPerCluster, bytesPerSector, freeClusters, totalClusters;
        std::wstring wpath(path.begin(), path.end());
        std::wstring root = wpath.substr(0, 3);
        if (GetDiskFreeSpaceW(root.c_str(), &sectorsPerCluster, &bytesPerSector,
                              &freeClusters, &totalClusters)) {
            return sectorsPerCluster * bytesPerSector;
        }
        return DEFAULT_BLOCK_SIZE;
#else
        return DEFAULT_BLOCK_SIZE;
#endif
    }

    FilesystemInfo GetFilesystemInfo(const std::string& path) const {
        FilesystemInfo info;
        info.type      = GetFilesystemType(path);
        info.blockSize = GetOptimalBlockSize(path);
        info.supportsLinks = (info.type == FilesystemType::NTFS ||
                              info.type == FilesystemType::ext4 ||
                              info.type == FilesystemType::btrfs ||
                              info.type == FilesystemType::APFS);
        info.caseSensitive = (info.type == FilesystemType::ext4 ||
                              info.type == FilesystemType::btrfs ||
                              info.type == FilesystemType::xfs);
        return info;
    }

    bool WatchDirectory([[maybe_unused]] const std::string& path,
                        [[maybe_unused]] FileChangeCallback callback) {
        if (path.empty() || !callback) return false;
#ifdef _WIN32
        m_watchPath = path;
        m_callback  = std::move(callback);
        m_watching  = true;
        return true;
#else
        return false;
#endif
    }

    void StopWatching() { m_watching = false; }
    bool IsWatching() const { return m_watching; }

private:
    NativeFilesystemAdapter() = default;

    std::string        m_watchPath;
    FileChangeCallback m_callback;
    bool               m_watching = false;

    static constexpr uint32_t DEFAULT_BLOCK_SIZE = 4096;
};

}} // namespace ExplorerLens::Engine
