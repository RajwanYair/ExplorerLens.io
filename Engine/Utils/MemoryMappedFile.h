//==============================================================================
// MemoryMappedFile.h - Memory-Mapped File I/O for Efficient Image Loading
// Part of ExplorerLens Engine Utils
//==============================================================================

#pragma once

#include <windows.h>
#include <memory>
#include <string>

namespace ExplorerLens {
namespace Engine {

//==============================================================================
/// RA II wrapper for memory-mapped files (Faster than fread for large images)
///
/// Usage:
/// MemoryMappedFile file(L"C:\\image.jpg");
/// if (file.IsValid()) {
/// const uint8_t* data = file.GetData();
/// size_t size = file.GetSize();
/// // Process data...
/// }
//==============================================================================
class MemoryMappedFile
{
  public:
    MemoryMappedFile() : m_fileHandle(INVALID_HANDLE_VALUE), m_mappingHandle(nullptr), m_data(nullptr), m_size(0) {}

    explicit MemoryMappedFile(const wchar_t* filePath) : MemoryMappedFile()
    {
        Open(filePath);
    }

    ~MemoryMappedFile()
    {
        Close();
    }

    // Non-copyable
    MemoryMappedFile(const MemoryMappedFile&) = delete;
    MemoryMappedFile& operator=(const MemoryMappedFile&) = delete;

    // Movable
    MemoryMappedFile(MemoryMappedFile&& other) noexcept
        : m_fileHandle(other.m_fileHandle)
        , m_mappingHandle(other.m_mappingHandle)
        , m_data(other.m_data)
        , m_size(other.m_size)
    {
        other.m_fileHandle = INVALID_HANDLE_VALUE;
        other.m_mappingHandle = nullptr;
        other.m_data = nullptr;
        other.m_size = 0;
    }

    MemoryMappedFile& operator=(MemoryMappedFile&& other) noexcept
    {
        if (this != &other) {
            Close();
            m_fileHandle = other.m_fileHandle;
            m_mappingHandle = other.m_mappingHandle;
            m_data = other.m_data;
            m_size = other.m_size;
            other.m_fileHandle = INVALID_HANDLE_VALUE;
            other.m_mappingHandle = nullptr;
            other.m_data = nullptr;
            other.m_size = 0;
        }
        return *this;
    }

    /// Open a file for memory-mapped reading
    /// @param filePath Full path to file
    /// @param maxSize Optional size limit (0 = no limit)
    /// @return true if file was mapped successfully
    bool Open(const wchar_t* filePath, size_t maxSize = 0)
    {
        Close();

        // Open file
        m_fileHandle = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL, nullptr);

        if (m_fileHandle == INVALID_HANDLE_VALUE) {
            return false;
        }

        // Get file size
        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(m_fileHandle, &fileSize)) {
            Close();
            return false;
        }

        m_size = static_cast<size_t>(fileSize.QuadPart);

        // Apply size limit if specified
        if (maxSize > 0 && m_size > maxSize) {
            m_size = maxSize;
        }

        // Don't map empty files
        if (m_size == 0) {
            Close();
            return false;
        }

        // Create file mapping
        m_mappingHandle = CreateFileMappingW(m_fileHandle, nullptr, PAGE_READONLY, 0,
                                             0,  // Map entire file
                                             nullptr);

        if (!m_mappingHandle) {
            Close();
            return false;
        }

        // Map view
        m_data = static_cast<const uint8_t*>(MapViewOfFile(m_mappingHandle, FILE_MAP_READ, 0, 0, m_size));

        if (!m_data) {
            Close();
            return false;
        }

        return true;
    }

    /// Close the memory-mapped file
    void Close()
    {
        if (m_data) {
            UnmapViewOfFile(m_data);
            m_data = nullptr;
        }
        if (m_mappingHandle) {
            CloseHandle(m_mappingHandle);
            m_mappingHandle = nullptr;
        }
        if (m_fileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_fileHandle);
            m_fileHandle = INVALID_HANDLE_VALUE;
        }
        m_size = 0;
    }

    /// Check if file is currently mapped
    /// @return true if valid mapping exists
    bool IsValid() const
    {
        return m_data != nullptr;
    }

    /// Get pointer to mapped file data
    /// @return Pointer to read-only file contents
    const uint8_t* GetData() const
    {
        return m_data;
    }

    /// Get size of mapped region
    /// @return Size in bytes
    size_t GetSize() const
    {
        return m_size;
    }

    /// Get a specific byte (with bounds checking)
    /// @param offset Byte offset
    /// @param outValue Output value
    /// @return true if offset is valid
    bool ReadByte(size_t offset, uint8_t& outValue) const
    {
        if (offset >= m_size) {
            return false;
        }
        outValue = m_data[offset];
        return true;
    }

    /// Read a range of bytes
    /// @param offset Starting offset
    /// @param buffer Destination buffer
    /// @param size Number of bytes to read
    /// @return Number of bytes successfully read
    size_t Read(size_t offset, void* buffer, size_t size) const
    {
        if (!buffer || offset >= m_size) {
            return 0;
        }
        size_t bytesAvailable = m_size - offset;
        size_t bytesToRead = (size < bytesAvailable) ? size : bytesAvailable;
        memcpy(buffer, m_data + offset, bytesToRead);
        return bytesToRead;
    }

  private:
    HANDLE m_fileHandle;
    HANDLE m_mappingHandle;
    const uint8_t* m_data;
    size_t m_size;
};

}  // namespace Engine
}  // namespace ExplorerLens
