#pragma once
// ============================================================================
// ParallelIOPipeline.h — IOCP-based parallel file reader (Sprint 547)
//
// Purpose:   I/O Completion Port (IOCP) powered parallel file reader for
//            batch thumbnail generation. Opens files with FILE_FLAG_OVERLAPPED,
//            submits reads to IOCP, and dispatches completions via a managed
//            thread pool. Tracks per-I/O latency via QueryPerformanceCounter.
//
// Classes:   ParallelIOPipeline
// Structs:   IOResult, IOContext (internal)
//
// Inputs:    File paths, offsets, and read sizes
// Outputs:   Completed read buffers with timing and error information
//
// Threading: Worker thread pool (min 2, max = hardware_concurrency)
//            using GetQueuedCompletionStatus. SRWLOCK protects shared state.
//
// Lifecycle: Initialize() creates IOCP + workers.
//            Shutdown() posts sentinel completions and joins all threads.
// ============================================================================

#include <windows.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

/// Result of a completed (or failed) I/O operation.
struct IOResult {
    std::wstring           filePath;
    std::vector<uint8_t>   data;
    bool                   success = false;
    DWORD                  errorCode = 0;
    double                 elapsedMs = 0.0;
};

/// Internal per-I/O context attached to OVERLAPPED structures.
struct IOContext {
    OVERLAPPED       overlapped = {};
    HANDLE           fileHandle = INVALID_HANDLE_VALUE;
    std::wstring     filePath;
    std::vector<uint8_t> buffer;
    uint32_t         requestedSize = 0;
    LARGE_INTEGER    submitTick = {};
    uint64_t         id = 0;
};

/// IOCP-based parallel file reader with managed thread pool.
class ParallelIOPipeline {
public:
    static constexpr ULONG_PTR SHUTDOWN_KEY = 0xDEAD;

    ParallelIOPipeline() {
        QueryPerformanceFrequency(&m_freq);
        InitializeSRWLock(&m_lock);
    }

    ~ParallelIOPipeline() {
        Shutdown();
    }

    ParallelIOPipeline(const ParallelIOPipeline&) = delete;
    ParallelIOPipeline& operator=(const ParallelIOPipeline&) = delete;

    /// Create the IOCP and spawn worker threads.
    /// @param concurrency Number of concurrent threads (0 = auto-detect)
    inline bool Initialize(uint32_t concurrency = 0) {
        if (m_running.load()) return true;

        if (concurrency == 0)
            concurrency = (std::max)(2u, std::thread::hardware_concurrency());
        else
            concurrency = (std::max)(2u, concurrency);

        m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, concurrency);
        if (!m_iocp) return false;

        m_running.store(true);
        m_workers.reserve(concurrency);
        for (uint32_t i = 0; i < concurrency; ++i) {
            m_workers.emplace_back(&ParallelIOPipeline::WorkerProc, this);
        }
        return true;
    }

    /// Submit an asynchronous overlapped read to the IOCP.
    /// @param filePath  Full path to the file to read.
    /// @param offset    Byte offset within the file.
    /// @param size      Number of bytes to read.
    /// @return true if the read was successfully submitted.
    inline bool SubmitRead(const std::wstring& filePath, uint64_t offset, uint32_t size) {
        if (!m_running.load() || !m_iocp || filePath.empty() || size == 0) return false;

        // Open file with overlapped I/O
        HANDLE hFile = CreateFileW(
            filePath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN,
            nullptr
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            // Record failure result immediately
            IOResult fail;
            fail.filePath = filePath;
            fail.success = false;
            fail.errorCode = GetLastError();
            fail.elapsedMs = 0.0;

            AcquireSRWLockExclusive(&m_lock);
            m_results.push_back(std::move(fail));
            ReleaseSRWLockExclusive(&m_lock);
            m_completedEvent.notify_all();
            return false;
        }

        // Associate file handle with the IOCP
        HANDLE assoc = CreateIoCompletionPort(hFile, m_iocp, 0, 0);
        if (!assoc) {
            DWORD err = GetLastError();
            CloseHandle(hFile);

            IOResult fail;
            fail.filePath = filePath;
            fail.success = false;
            fail.errorCode = err;

            AcquireSRWLockExclusive(&m_lock);
            m_results.push_back(std::move(fail));
            ReleaseSRWLockExclusive(&m_lock);
            m_completedEvent.notify_all();
            return false;
        }

        // Allocate context (heap — outlives this scope; freed by worker)
        auto* ctx = new IOContext();
        ctx->fileHandle = hFile;
        ctx->filePath = filePath;
        ctx->requestedSize = size;
        ctx->buffer.resize(size, 0);
        ctx->id = ++m_nextId;

        // Set OVERLAPPED offset
        ctx->overlapped.Offset = static_cast<DWORD>(offset & 0xFFFFFFFF);
        ctx->overlapped.OffsetHigh = static_cast<DWORD>((offset >> 32) & 0xFFFFFFFF);

        QueryPerformanceCounter(&ctx->submitTick);
        m_pendingCount.fetch_add(1);

        // Track handle for CancelAll
        AcquireSRWLockExclusive(&m_lock);
        m_activeContexts.push_back(ctx);
        ReleaseSRWLockExclusive(&m_lock);

        // Issue the overlapped read
        BOOL ok = ReadFile(hFile, ctx->buffer.data(), size, nullptr, &ctx->overlapped);
        if (!ok) {
            DWORD err = GetLastError();
            if (err != ERROR_IO_PENDING) {
                // Immediate failure — clean up
                m_pendingCount.fetch_sub(1);
                CloseHandle(hFile);

                IOResult fail;
                fail.filePath = filePath;
                fail.success = false;
                fail.errorCode = err;

                AcquireSRWLockExclusive(&m_lock);
                RemoveContext(ctx);
                m_results.push_back(std::move(fail));
                ReleaseSRWLockExclusive(&m_lock);
                m_completedEvent.notify_all();

                delete ctx;
                return false;
            }
            // ERROR_IO_PENDING is expected for async — completion via IOCP
        }

        return true;
    }

    /// Wait for all submitted I/Os to complete and return their results.
    /// @param timeoutMs Maximum wait time in milliseconds. 0 = infinite.
    inline std::vector<IOResult> WaitForAll(uint32_t timeoutMs = 0) {
        std::unique_lock<std::mutex> ul(m_cvMutex);
        auto pred = [this]() { return m_pendingCount.load() == 0; };

        if (timeoutMs == 0) {
            m_completedEvent.wait(ul, pred);
        }
        else {
            m_completedEvent.wait_for(ul, std::chrono::milliseconds(timeoutMs), pred);
        }

        AcquireSRWLockExclusive(&m_lock);
        std::vector<IOResult> results = std::move(m_results);
        m_results.clear();
        ReleaseSRWLockExclusive(&m_lock);
        return results;
    }

    /// Cancel all pending I/O operations.
    inline void CancelAll() {
        AcquireSRWLockShared(&m_lock);
        for (auto* ctx : m_activeContexts) {
            if (ctx && ctx->fileHandle != INVALID_HANDLE_VALUE) {
                CancelIoEx(ctx->fileHandle, &ctx->overlapped);
            }
        }
        ReleaseSRWLockShared(&m_lock);
    }

    /// Clean shutdown: post sentinel packets to all workers and join threads.
    inline void Shutdown() {
        if (!m_running.exchange(false)) return;

        // Cancel remaining I/Os
        CancelAll();

        // Post shutdown sentinels (one per worker thread)
        if (m_iocp) {
            for (size_t i = 0; i < m_workers.size(); ++i) {
                PostQueuedCompletionStatus(m_iocp, 0, SHUTDOWN_KEY, nullptr);
            }
        }

        // Join all workers
        for (auto& t : m_workers) {
            if (t.joinable()) t.join();
        }
        m_workers.clear();

        // Close the IOCP handle
        if (m_iocp) {
            CloseHandle(m_iocp);
            m_iocp = nullptr;
        }

        // Clean up any remaining contexts
        AcquireSRWLockExclusive(&m_lock);
        for (auto* ctx : m_activeContexts) {
            if (ctx) {
                if (ctx->fileHandle != INVALID_HANDLE_VALUE)
                    CloseHandle(ctx->fileHandle);
                delete ctx;
            }
        }
        m_activeContexts.clear();
        ReleaseSRWLockExclusive(&m_lock);
    }

    /// Number of currently pending I/O operations.
    inline uint32_t PendingCount() const noexcept {
        return m_pendingCount.load();
    }

    /// Number of completed I/O results waiting to be consumed.
    inline uint32_t ResultCount() {
        AcquireSRWLockShared(&m_lock);
        uint32_t count = static_cast<uint32_t>(m_results.size());
        ReleaseSRWLockShared(&m_lock);
        return count;
    }

private:
    /// Worker thread procedure — blocks on GetQueuedCompletionStatus.
    inline void WorkerProc() {
        while (m_running.load()) {
            DWORD bytesTransferred = 0;
            ULONG_PTR completionKey = 0;
            LPOVERLAPPED pOverlapped = nullptr;

            BOOL ok = GetQueuedCompletionStatus(
                m_iocp,
                &bytesTransferred,
                &completionKey,
                &pOverlapped,
                1000 // 1-second timeout to check m_running flag
            );

            // Check for shutdown sentinel
            if (completionKey == SHUTDOWN_KEY) break;

            if (!pOverlapped) {
                // Timeout or spurious wake — loop to check m_running
                continue;
            }

            // Recover our context from the OVERLAPPED pointer
            auto* ctx = reinterpret_cast<IOContext*>(
                reinterpret_cast<char*>(pOverlapped) - offsetof(IOContext, overlapped));

            LARGE_INTEGER endTick;
            QueryPerformanceCounter(&endTick);

            IOResult result;
            result.filePath = ctx->filePath;
            result.elapsedMs = static_cast<double>(endTick.QuadPart - ctx->submitTick.QuadPart)
                * 1000.0 / static_cast<double>(m_freq.QuadPart);

            if (ok && bytesTransferred > 0) {
                result.success = true;
                result.errorCode = 0;
                result.data.assign(ctx->buffer.begin(),
                    ctx->buffer.begin() + bytesTransferred);
            }
            else {
                result.success = false;
                result.errorCode = GetLastError();
            }

            // Close file handle
            if (ctx->fileHandle != INVALID_HANDLE_VALUE) {
                CloseHandle(ctx->fileHandle);
                ctx->fileHandle = INVALID_HANDLE_VALUE;
            }

            // Store result and clean up context
            AcquireSRWLockExclusive(&m_lock);
            m_results.push_back(std::move(result));
            RemoveContext(ctx);
            ReleaseSRWLockExclusive(&m_lock);

            delete ctx;
            m_pendingCount.fetch_sub(1);
            m_completedEvent.notify_all();
        }
    }

    /// Remove a context pointer from the active list (called under exclusive lock).
    inline void RemoveContext(IOContext* ctx) {
        auto it = std::find(m_activeContexts.begin(), m_activeContexts.end(), ctx);
        if (it != m_activeContexts.end()) {
            m_activeContexts.erase(it);
        }
    }

    HANDLE                              m_iocp = nullptr;
    SRWLOCK                             m_lock{};
    LARGE_INTEGER                       m_freq{};
    std::atomic<bool>                   m_running{ false };
    std::atomic<uint32_t>               m_pendingCount{ 0 };
    std::atomic<uint64_t>               m_nextId{ 0 };
    std::vector<std::thread>            m_workers;
    std::vector<IOResult>               m_results;
    std::vector<IOContext*>             m_activeContexts;
    std::mutex                          m_cvMutex;
    std::condition_variable             m_completedEvent;
};

} // namespace Engine
} // namespace ExplorerLens
