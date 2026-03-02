#pragma once
// ============================================================================
// CacheCoherencyManager.h — Multi-process cache consistency via named mutexes
// ExplorerLens Engine v15.0.0 "Zenith"
// ============================================================================

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <atomic>
#include <vector>
#include <functional>

namespace ExplorerLens {
namespace Engine {

// Coherency event types
enum class CoherencyEvent : uint32_t {
    FileModified = 0,
    FileDeleted = 1,
    FileRenamed = 2,
    DirectoryChanged = 3,
    ExternalInvalidation = 4,
    ProcessAttach = 5,
    ProcessDetach = 6
};

// Invalidation request
struct CacheInvalidationRequest {
    std::wstring filePath;
    CoherencyEvent event = CoherencyEvent::FileModified;
    uint64_t      timestamp = 0;
    uint32_t      sourceProcessId = 0;
};

// Coherency stats
struct CacheCoherencyStats {
    uint64_t invalidationsSent = 0;
    uint64_t invalidationsReceived = 0;
    uint64_t lockAcquisitions = 0;
    uint64_t lockTimeouts = 0;
    uint64_t changeNotifications = 0;
    uint32_t activeProcesses = 0;
};

// Configuration
struct CacheCoherencyConfig {
    std::wstring mutexName = L"Global\\ExplorerLensCacheCoherency";
    std::wstring sharedMemName = L"Local\\ExplorerLensCacheInvalidation";
    uint32_t     lockTimeoutMs = 1000;
    uint32_t     maxPendingInvalidations = 256;
    bool         enableFileWatching = true;
};

// ========================================================================
// CacheCoherencyManager — Cross-process cache invalidation
// ========================================================================
class CacheCoherencyManager {
public:
    static CacheCoherencyManager& Instance() {
        static CacheCoherencyManager instance;
        return instance;
    }

    bool Initialize(const CacheCoherencyConfig& config = {}) {
        m_config = config;

        // Create or open named mutex for cross-process synchronization
        m_hMutex = CreateMutexW(nullptr, FALSE, m_config.mutexName.c_str());
        if (!m_hMutex) return false;

        // Create or open shared memory for invalidation signaling
        m_hSharedMem = CreateFileMappingW(
            INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
            0, sizeof(SharedInvalidationBuffer), m_config.sharedMemName.c_str());

        if (m_hSharedMem) {
            m_pSharedBuffer = static_cast<SharedInvalidationBuffer*>(
                MapViewOfFile(m_hSharedMem, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedInvalidationBuffer)));
        }

        m_processId = GetCurrentProcessId();
        m_initialized = true;
        return true;
    }

    bool IsInitialized() const { return m_initialized; }

    // Acquire cross-process lock for cache operations
    bool AcquireLock() {
        if (!m_hMutex) return false;
        DWORD result = WaitForSingleObject(m_hMutex, m_config.lockTimeoutMs);
        if (result == WAIT_OBJECT_0 || result == WAIT_ABANDONED) {
            m_stats.lockAcquisitions++;
            return true;
        }
        m_stats.lockTimeouts++;
        return false;
    }

    // Release cross-process lock
    void ReleaseLock() {
        if (m_hMutex) ReleaseMutex(m_hMutex);
    }

    // Post an invalidation event (visible to other processes)
    bool PostInvalidation(const std::wstring& filePath, CoherencyEvent event) {
        if (!m_pSharedBuffer) return false;

        if (AcquireLock()) {
            uint32_t idx = m_pSharedBuffer->writeIndex % m_config.maxPendingInvalidations;
            auto& entry = m_pSharedBuffer->entries[idx % 256];
            wcsncpy_s(entry.filePath, filePath.c_str(), 259);
            entry.event = static_cast<uint32_t>(event);
            entry.processId = m_processId;
            entry.timestamp = GetTickCount64();
            m_pSharedBuffer->writeIndex++;
            m_stats.invalidationsSent++;
            ReleaseLock();
            return true;
        }
        return false;
    }

    // Check for pending invalidations from other processes
    uint32_t ProcessPendingInvalidations(std::function<void(const CacheInvalidationRequest&)> callback) {
        if (!m_pSharedBuffer || !callback) return 0;

        uint32_t processed = 0;
        if (AcquireLock()) {
            while (m_localReadIndex < m_pSharedBuffer->writeIndex) {
                uint32_t idx = m_localReadIndex % 256;
                auto& entry = m_pSharedBuffer->entries[idx];

                // Only process entries from other processes
                if (entry.processId != m_processId) {
                    CacheInvalidationRequest req;
                    req.filePath = entry.filePath;
                    req.event = static_cast<CoherencyEvent>(entry.event);
                    req.timestamp = entry.timestamp;
                    req.sourceProcessId = entry.processId;
                    callback(req);
                    processed++;
                    m_stats.invalidationsReceived++;
                }
                m_localReadIndex++;
            }
            ReleaseLock();
        }
        return processed;
    }

    // Get stats
    CacheCoherencyStats GetStats() const { return m_stats; }

    // Shutdown
    void Shutdown() {
        if (m_pSharedBuffer) {
            UnmapViewOfFile(m_pSharedBuffer);
            m_pSharedBuffer = nullptr;
        }
        if (m_hSharedMem) {
            CloseHandle(m_hSharedMem);
            m_hSharedMem = nullptr;
        }
        if (m_hMutex) {
            CloseHandle(m_hMutex);
            m_hMutex = nullptr;
        }
        m_initialized = false;
    }

    ~CacheCoherencyManager() {
        Shutdown();
    }

private:
    CacheCoherencyManager() = default;
    CacheCoherencyManager(const CacheCoherencyManager&) = delete;
    CacheCoherencyManager& operator=(const CacheCoherencyManager&) = delete;

    // Shared memory layout for cross-process invalidation ring buffer
    struct SharedInvalidationEntry {
        wchar_t  filePath[260] = {};
        uint32_t event = 0;
        uint32_t processId = 0;
        uint64_t timestamp = 0;
    };

    struct SharedInvalidationBuffer {
        volatile uint32_t writeIndex = 0;
        SharedInvalidationEntry entries[256];
    };

    CacheCoherencyConfig     m_config;
    CacheCoherencyStats      m_stats;
    HANDLE                   m_hMutex = nullptr;
    HANDLE                   m_hSharedMem = nullptr;
    SharedInvalidationBuffer* m_pSharedBuffer = nullptr;
    uint32_t                 m_processId = 0;
    uint32_t                 m_localReadIndex = 0;
    bool                     m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
