// MemoryMappedLoader.h — File-Backed Memory-Mapped I/O for Large Archives
// Copyright (c) 2026 ExplorerLens Project
//
// Provides zero-copy file access via Win32 CreateFileMapping / MapViewOfFile.
// Decoders consume raw byte spans without buffering the entire file in RAM.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstdint>
#include <span>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class MapAccess : uint8_t {
    ReadOnly,
    ReadWrite,
};

struct MappedRegion
{
    const uint8_t* data{nullptr};
    size_t size{0};

    [[nodiscard]] bool IsValid() const noexcept
    {
        return data != nullptr && size > 0;
    }

    [[nodiscard]] std::span<const uint8_t> Span() const noexcept
    {
        return {data, size};
    }
};

class MemoryMappedLoader
{
  public:
    MemoryMappedLoader() = default;
    ~MemoryMappedLoader()
    {
        Close();
    }

    MemoryMappedLoader(const MemoryMappedLoader&) = delete;
    MemoryMappedLoader& operator=(const MemoryMappedLoader&) = delete;

    MemoryMappedLoader(MemoryMappedLoader&& o) noexcept
        : m_file(o.m_file)
        , m_mapping(o.m_mapping)
        , m_view(o.m_view)
        , m_size(o.m_size)
    {
        o.m_file = INVALID_HANDLE_VALUE;
        o.m_mapping = nullptr;
        o.m_view = nullptr;
        o.m_size = 0;
    }

    // Open and map an entire file into the process address space.
    [[nodiscard]] bool Open(const std::wstring& path, MapAccess access = MapAccess::ReadOnly);

    // Map a specific byte range (useful for large archives — map only the needed entry).
    [[nodiscard]] bool OpenRange(const std::wstring& path, uint64_t offset, size_t length,
                                 MapAccess access = MapAccess::ReadOnly);

    void Close() noexcept;

    [[nodiscard]] bool IsOpen() const noexcept
    {
        return m_view != nullptr;
    }
    [[nodiscard]] MappedRegion Region() const noexcept
    {
        return {static_cast<const uint8_t*>(m_view), m_size};
    }
    [[nodiscard]] size_t Size() const noexcept
    {
        return m_size;
    }
    [[nodiscard]] const void* Data() const noexcept
    {
        return m_view;
    }

    // Advise the kernel about upcoming sequential / random access patterns.
    void PrefetchSequential() const noexcept;
    void PrefetchRandom() const noexcept;

    // Flush dirty pages (ReadWrite mode only).
    bool Flush() noexcept;

  private:
    HANDLE m_file = INVALID_HANDLE_VALUE;
    HANDLE m_mapping = nullptr;
    void* m_view = nullptr;
    size_t m_size = 0;

    // Offset adjustment for aligned mapping (system page-size alignment).
    uint64_t m_alignmentOffset = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
