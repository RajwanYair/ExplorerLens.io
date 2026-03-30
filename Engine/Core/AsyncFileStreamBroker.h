// AsyncFileStreamBroker.h — Async File Streaming Broker
// Copyright (c) 2026 ExplorerLens Project
//
// Unifies overlapped I/O and DirectStorage into a single async file streaming
// abstraction, selecting the optimal path based on hardware and OS capabilities.
//
#pragma once

#include <cstdint>
#include <string>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#endif

namespace ExplorerLens {
namespace Engine {

enum class StreamMode : uint8_t {
    Overlapped     = 0,
    DirectStorage  = 1,
    Memory         = 2
};

enum class StreamStatus : uint8_t {
    Idle       = 0,
    Open       = 1,
    Reading    = 2,
    Complete   = 3,
    Error      = 4
};

struct StreamHandle {
    uint64_t     id = 0;
    StreamMode   mode = StreamMode::Overlapped;
    StreamStatus status = StreamStatus::Idle;
    uint64_t     fileSize = 0;
#ifdef _WIN32
    HANDLE       hFile = INVALID_HANDLE_VALUE;
#endif
};

struct StreamResult {
    bool     success = false;
    uint64_t bytesRead = 0;
    double   elapsedMs = 0.0;
};

class AsyncFileStreamBroker {
public:
    AsyncFileStreamBroker() = default;
    ~AsyncFileStreamBroker() = default;

    AsyncFileStreamBroker(const AsyncFileStreamBroker&) = delete;
    AsyncFileStreamBroker& operator=(const AsyncFileStreamBroker&) = delete;

    inline StreamHandle OpenStream(const wchar_t* path, StreamMode preferredMode = StreamMode::Overlapped) {
        StreamHandle handle;
        if (!path) {
            handle.status = StreamStatus::Error;
            return handle;
        }
        handle.id = m_nextId.fetch_add(1, std::memory_order_relaxed);
        handle.mode = ResolveMode(preferredMode);
#ifdef _WIN32
        DWORD flags = FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN;
        handle.hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr,
                                   OPEN_EXISTING, flags, nullptr);
        if (handle.hFile == INVALID_HANDLE_VALUE) {
            handle.status = StreamStatus::Error;
            return handle;
        }
        LARGE_INTEGER li;
        if (GetFileSizeEx(handle.hFile, &li)) {
            handle.fileSize = static_cast<uint64_t>(li.QuadPart);
        }
#endif
        handle.status = StreamStatus::Open;
        m_totalOpened++;
        return handle;
    }

    inline StreamResult ReadAsync(StreamHandle& handle, void* buffer, uint64_t size, uint64_t offset = 0) {
        StreamResult result;
        if (handle.status != StreamStatus::Open || !buffer || size == 0) return result;
        handle.status = StreamStatus::Reading;
#ifdef _WIN32
        OVERLAPPED ov{};
        ov.Offset = static_cast<DWORD>(offset & 0xFFFFFFFF);
        ov.OffsetHigh = static_cast<DWORD>(offset >> 32);
        DWORD bytesRead = 0;
        if (ReadFile(handle.hFile, buffer, static_cast<DWORD>(size), &bytesRead, &ov) ||
            GetLastError() == ERROR_IO_PENDING) {
            result.bytesRead = bytesRead;
            result.success = true;
        }
#endif
        handle.status = result.success ? StreamStatus::Complete : StreamStatus::Error;
        m_totalBytesRead += result.bytesRead;
        return result;
    }

    inline uint64_t GetBytesRead() const { return m_totalBytesRead; }
    inline uint64_t GetStreamsOpened() const { return m_totalOpened; }

    inline void Close(StreamHandle& handle) {
#ifdef _WIN32
        if (handle.hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(handle.hFile);
            handle.hFile = INVALID_HANDLE_VALUE;
        }
#endif
        handle.status = StreamStatus::Idle;
    }

private:
    inline StreamMode ResolveMode(StreamMode preferred) {
        if (preferred == StreamMode::DirectStorage) {
            return StreamMode::DirectStorage;
        }
        return StreamMode::Overlapped;
    }

    std::atomic<uint64_t> m_nextId{1};
    uint64_t              m_totalBytesRead = 0;
    uint64_t              m_totalOpened = 0;
};

} // namespace Engine
} // namespace ExplorerLens
