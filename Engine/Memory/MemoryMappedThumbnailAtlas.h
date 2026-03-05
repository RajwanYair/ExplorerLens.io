// MemoryMappedThumbnailAtlas.h — Zero-Copy Thumbnail Atlas Storage
// Copyright (c) 2026 ExplorerLens Project
//
// Memory-mapped atlas file for batch thumbnail storage, enabling
// zero-copy read access to frequently requested thumbnails without
// per-file disk seeks.  Uses a directory header + packed pixel blocks.
//
#pragma once

#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// Atlas entry metadata stored in the directory
struct AtlasEntry {
    uint64_t keyHash = 0;
    uint32_t offsetBytes = 0;    // Offset into atlas data region
    uint32_t sizeBytes = 0;    // Compressed size
    uint16_t width = 0;
    uint16_t height = 0;
    uint32_t originalSize = 0;    // Uncompressed size
    bool     compressed = false;
};

/// Atlas file header (serialized at byte 0)
struct AtlasHeader {
    uint32_t magic = 0x4C454E53; // "LENS"
    uint16_t version = 1;
    uint16_t reserved = 0;
    uint32_t entryCount = 0;
    uint32_t directoryOffset = 0;   // Byte offset to directory
    uint64_t dataSizeBytes = 0;    // Total data region size
    uint64_t totalFileSize = 0;
};

/// Atlas statistics for memory-mapped thumbnail atlas
struct MmapAtlasStats {
    uint64_t reads = 0;
    uint64_t writes = 0;
    uint64_t cacheHits = 0;    // Reads where data was already mapped
    uint32_t entryCount = 0;
    uint64_t totalDataBytes = 0;
    uint64_t mappedBytes = 0;
    bool     isMapped = false;
};

/// Memory-mapped thumbnail atlas for zero-copy reads
class MemoryMappedThumbnailAtlas {
public:
    MemoryMappedThumbnailAtlas() = default;

    ~MemoryMappedThumbnailAtlas() { Close(); }

    /// Create a new atlas file at the given path
    bool Create(const std::wstring& path, uint64_t reserveSizeBytes = 64 * 1024 * 1024) {
        std::lock_guard<std::mutex> lock(m_mutex);
        Close();

        m_filePath = path;
        m_fileHandle = CreateFileW(path.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ, nullptr, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, nullptr);
        if (m_fileHandle == INVALID_HANDLE_VALUE) return false;

        // Reserve file size
        LARGE_INTEGER li;
        li.QuadPart = static_cast<LONGLONG>(reserveSizeBytes);
        SetFilePointerEx(m_fileHandle, li, nullptr, FILE_BEGIN);
        SetEndOfFile(m_fileHandle);

        // Create file mapping
        m_mappingHandle = CreateFileMappingW(m_fileHandle, nullptr,
            PAGE_READWRITE, static_cast<DWORD>(reserveSizeBytes >> 32),
            static_cast<DWORD>(reserveSizeBytes & 0xFFFFFFFF), nullptr);
        if (!m_mappingHandle) { Close(); return false; }

        m_basePtr = static_cast<uint8_t*>(MapViewOfFile(m_mappingHandle,
            FILE_MAP_ALL_ACCESS, 0, 0, 0));
        if (!m_basePtr) { Close(); return false; }

        m_mappedSize = reserveSizeBytes;
        m_stats.isMapped = true;
        m_stats.mappedBytes = reserveSizeBytes;

        // Write initial header
        m_header = {};
        m_header.directoryOffset = sizeof(AtlasHeader);
        m_dataOffset = sizeof(AtlasHeader) + 1024 * 1024; // 1 MB for directory
        WriteHeader();

        return true;
    }

    /// Open an existing atlas file for reading
    bool Open(const std::wstring& path) {
        std::lock_guard<std::mutex> lock(m_mutex);
        Close();

        m_filePath = path;
        m_fileHandle = CreateFileW(path.c_str(),
            GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, nullptr);
        if (m_fileHandle == INVALID_HANDLE_VALUE) return false;

        LARGE_INTEGER fileSize;
        GetFileSizeEx(m_fileHandle, &fileSize);
        m_mappedSize = static_cast<uint64_t>(fileSize.QuadPart);

        m_mappingHandle = CreateFileMappingW(m_fileHandle, nullptr,
            PAGE_READONLY, 0, 0, nullptr);
        if (!m_mappingHandle) { Close(); return false; }

        m_basePtr = static_cast<uint8_t*>(MapViewOfFile(m_mappingHandle,
            FILE_MAP_READ, 0, 0, 0));
        if (!m_basePtr) { Close(); return false; }

        m_stats.isMapped = true;
        m_stats.mappedBytes = m_mappedSize;

        // Read header
        if (m_mappedSize >= sizeof(AtlasHeader)) {
            std::memcpy(&m_header, m_basePtr, sizeof(AtlasHeader));
            if (m_header.magic != 0x4C454E53) { Close(); return false; }
        }

        return true;
    }

    /// Store a thumbnail in the atlas
    bool Store(uint64_t keyHash, const uint8_t* data, uint32_t size,
        uint16_t width, uint16_t height) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_basePtr || m_dataOffset + size > m_mappedSize) return false;

        // Write pixel data at current offset
        std::memcpy(m_basePtr + m_dataOffset, data, size);

        AtlasEntry entry;
        entry.keyHash = keyHash;
        entry.offsetBytes = static_cast<uint32_t>(m_dataOffset);
        entry.sizeBytes = size;
        entry.width = width;
        entry.height = height;
        entry.originalSize = size;

        m_directory[keyHash] = entry;
        m_dataOffset += size;

        m_header.entryCount++;
        m_header.dataSizeBytes = m_dataOffset;
        WriteHeader();

        m_stats.writes++;
        m_stats.entryCount = m_header.entryCount;
        m_stats.totalDataBytes = m_dataOffset;
        return true;
    }

    /// Read a thumbnail from the atlas (zero-copy — returns pointer into mapping)
    const uint8_t* Read(uint64_t keyHash, AtlasEntry& entryOut) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_basePtr) return nullptr;

        auto it = m_directory.find(keyHash);
        if (it == m_directory.end()) return nullptr;

        entryOut = it->second;
        m_stats.reads++;
        m_stats.cacheHits++;
        return m_basePtr + entryOut.offsetBytes;
    }

    /// Check if a key exists
    bool Contains(uint64_t keyHash) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_directory.count(keyHash) > 0;
    }

    /// Close and unmap the atlas
    void Close() {
        if (m_basePtr) { UnmapViewOfFile(m_basePtr); m_basePtr = nullptr; }
        if (m_mappingHandle) { CloseHandle(m_mappingHandle); m_mappingHandle = nullptr; }
        if (m_fileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_fileHandle); m_fileHandle = INVALID_HANDLE_VALUE;
        }
        m_stats.isMapped = false;
        m_stats.mappedBytes = 0;
    }

    bool IsMapped() const { return m_basePtr != nullptr; }
    uint32_t EntryCount() const { return m_header.entryCount; }

    MmapAtlasStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

private:
    void WriteHeader() {
        if (m_basePtr && m_mappedSize >= sizeof(AtlasHeader)) {
            std::memcpy(m_basePtr, &m_header, sizeof(AtlasHeader));
        }
    }

    std::wstring     m_filePath;
    HANDLE           m_fileHandle = INVALID_HANDLE_VALUE;
    HANDLE           m_mappingHandle = nullptr;
    uint8_t* m_basePtr = nullptr;
    uint64_t         m_mappedSize = 0;
    uint64_t         m_dataOffset = 0;
    AtlasHeader      m_header;
    mutable std::mutex m_mutex;
    mutable MmapAtlasStats m_stats;
    std::unordered_map<uint64_t, AtlasEntry> m_directory;
};

} // namespace Engine
} // namespace ExplorerLens
