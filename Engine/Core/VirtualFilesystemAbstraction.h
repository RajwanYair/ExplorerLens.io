#pragma once
// VirtualFilesystemAbstraction.h — Virtual Filesystem Abstraction
// Unified file access layer for local, cloud, network, and virtual filesystems
// (OneDrive placeholders, FUSE mounts, WSL, etc.) with transparent caching.
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

/// Virtual filesystem backend
enum class VFSBackend : uint8_t {
    LocalNTFS = 0,
    LocalReFS,
    NetworkSMB,
    NetworkNFS,
    OneDrivePlaceholder,
    GoogleDriveFUSE,
    WSL2,
    MemoryVFS,  // In-memory virtual FS for testing
    COUNT
};

/// File availability status (for cloud placeholders)
enum class FileAvailability : uint8_t {
    FullyLocal = 0,
    PartiallyLocal,  // Some data cached locally
    CloudOnly,       // Needs download
    Downloading,
    Offline,
    COUNT
};

struct VFSFileInfo
{
    VFSBackend backend = VFSBackend::LocalNTFS;
    FileAvailability availability = FileAvailability::FullyLocal;
    uint64_t fileSize = 0;
    uint64_t localCacheSize = 0;
    bool isReadOnly = false;
    bool isSymlink = false;
    bool isPlaceholder = false;
};

struct VFSStats
{
    uint64_t totalAccesses = 0;
    uint64_t cacheHits = 0;
    uint64_t cloudDownloads = 0;
    double avgAccessTimeMs = 0.0;
    uint64_t bytesDownloaded = 0;
};

class VirtualFilesystemAbstraction
{
  public:
    static constexpr size_t BackendCount()
    {
        return static_cast<size_t>(VFSBackend::COUNT);
    }
    static constexpr size_t AvailabilityCount()
    {
        return static_cast<size_t>(FileAvailability::COUNT);
    }

    static const wchar_t* BackendName(VFSBackend b)
    {
        switch (b) {
            case VFSBackend::LocalNTFS:
                return L"Local NTFS";
            case VFSBackend::LocalReFS:
                return L"Local ReFS";
            case VFSBackend::NetworkSMB:
                return L"Network SMB";
            case VFSBackend::NetworkNFS:
                return L"Network NFS";
            case VFSBackend::OneDrivePlaceholder:
                return L"OneDrive Placeholder";
            case VFSBackend::GoogleDriveFUSE:
                return L"Google Drive FUSE";
            case VFSBackend::WSL2:
                return L"WSL2";
            case VFSBackend::MemoryVFS:
                return L"Memory VFS";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* AvailabilityName(FileAvailability a)
    {
        switch (a) {
            case FileAvailability::FullyLocal:
                return L"Fully Local";
            case FileAvailability::PartiallyLocal:
                return L"Partially Local";
            case FileAvailability::CloudOnly:
                return L"Cloud Only";
            case FileAvailability::Downloading:
                return L"Downloading";
            case FileAvailability::Offline:
                return L"Offline";
            default:
                return L"Unknown";
        }
    }

    /// Determine if file needs download before thumbnail generation
    static bool NeedsDownload(FileAvailability avail)
    {
        return avail == FileAvailability::CloudOnly;
    }

    /// Check if backend supports zero-copy memory mapping
    static bool SupportsMemoryMapping(VFSBackend b)
    {
        return b == VFSBackend::LocalNTFS || b == VFSBackend::LocalReFS;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
