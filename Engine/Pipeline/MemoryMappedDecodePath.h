// MemoryMappedDecodePath.h — Memory-Mapped File Decode Path
// Copyright (c) 2026 ExplorerLens Project
//
// Uses Windows memory-mapped file I/O for zero-copy decode of large
// archives and images. Falls back to buffered I/O for small files or
// when mmap fails. Integrates with buffer pool for output staging.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct MappedFileView
{
    const uint8_t* data = nullptr;
    uint64_t size = 0;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMapping = nullptr;
    bool isValid = false;
};

struct MMapDecodeStats
{
    uint32_t filesProcessed = 0;
    uint32_t mmapSuccesses = 0;
    uint32_t mmapFallbacks = 0;
    uint64_t totalBytesMapped = 0;
    double avgMapTimeUs = 0.0;
};

class MemoryMappedDecodePath
{
  public:
    static constexpr uint64_t MIN_MMAP_SIZE = 64 * 1024;             // 64KB
    static constexpr uint64_t MAX_MMAP_SIZE = 512ULL * 1024 * 1024;  // 512MB

    MemoryMappedDecodePath() = default;
    ~MemoryMappedDecodePath() = default;

    static const wchar_t* GetName()
    {
        return L"MemoryMappedDecodePath";
    }

    /// Should this file use mmap? (size heuristic)
    bool ShouldUseMmap(uint64_t fileSize) const
    {
        return fileSize >= MIN_MMAP_SIZE && fileSize <= MAX_MMAP_SIZE;
    }

    /// Create a memory-mapped view of a file.
    MappedFileView MapFile(const wchar_t* filePath) const
    {
        MappedFileView view;
        if (!filePath)
            return view;

        view.hFile = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                                 nullptr);
        if (view.hFile == INVALID_HANDLE_VALUE) {
            m_stats.mmapFallbacks++;
            return view;
        }

        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(view.hFile, &fileSize)) {
            CloseHandle(view.hFile);
            view.hFile = INVALID_HANDLE_VALUE;
            m_stats.mmapFallbacks++;
            return view;
        }
        view.size = fileSize.QuadPart;

        if (!ShouldUseMmap(view.size)) {
            CloseHandle(view.hFile);
            view.hFile = INVALID_HANDLE_VALUE;
            m_stats.mmapFallbacks++;
            return view;
        }

        view.hMapping = CreateFileMappingW(view.hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if (!view.hMapping) {
            CloseHandle(view.hFile);
            view.hFile = INVALID_HANDLE_VALUE;
            m_stats.mmapFallbacks++;
            return view;
        }

        view.data = static_cast<const uint8_t*>(MapViewOfFile(view.hMapping, FILE_MAP_READ, 0, 0, 0));
        if (!view.data) {
            CloseHandle(view.hMapping);
            CloseHandle(view.hFile);
            view.hMapping = nullptr;
            view.hFile = INVALID_HANDLE_VALUE;
            m_stats.mmapFallbacks++;
            return view;
        }

        view.isValid = true;
        m_stats.mmapSuccesses++;
        m_stats.totalBytesMapped += view.size;
        m_stats.filesProcessed++;
        return view;
    }

    /// Unmap and close a previously mapped file.
    void UnmapFile(MappedFileView& view) const
    {
        if (view.data) {
            UnmapViewOfFile(view.data);
            view.data = nullptr;
        }
        if (view.hMapping) {
            CloseHandle(view.hMapping);
            view.hMapping = nullptr;
        }
        if (view.hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(view.hFile);
            view.hFile = INVALID_HANDLE_VALUE;
        }
        view.isValid = false;
    }

    MMapDecodeStats GetStats() const
    {
        return m_stats;
    }

  private:
    mutable MMapDecodeStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
