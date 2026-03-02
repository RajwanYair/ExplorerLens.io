#pragma once
/******************************************************************************
 * USNCacheInvalidation.h
 * Copyright (c) 2026 ExplorerLens Project
 *
 * PURPOSE:
 *   Uses NTFS USN (Update Sequence Number) Journal to detect file changes
 *   for cache invalidation. Opens volume handle, queries USN journal via
 *   FSCTL_QUERY_USN_JOURNAL, and reads entries via FSCTL_READ_USN_JOURNAL
 *   in a background thread. Falls back to GetFileAttributesExW polling
 *   on non-NTFS volumes.
 *
 * CLASSES:
 *   FileIdentity    — Robust cache key based on volume/file/size/write-time.
 *   USNCacheInvalidation — Main USN journal invalidation engine with file
 *     tracking (FRN resolution), background monitoring, filtered change
 *     callbacks, and fallback polling.
 *
 * INPUTS:
 *   Initialize(volumeLetter) — open volume for USN journal access.
 *   TrackFile(path) — start tracking file changes.
 *   SetInvalidationCallback(fn) — receive (path, reason) notifications.
 *
 * OUTPUTS:
 *   USNStats — entries processed, invalidations, reads/sec, fallback state.
 *
 * THREAD SAFETY:
 *   All public methods are thread-safe via SRWLOCK.
 *****************************************************************************/

#include <windows.h>
#include <winioctl.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <chrono>
#include <cstdint>

namespace ExplorerLens {
namespace USNCache {

// ============================================================================
// File Identity — robust cache key tuple
// ============================================================================

struct FileIdentity {
    uint64_t volume_id = 0;
    uint64_t file_id = 0;
    uint64_t file_size = 0;
    uint64_t last_write_time = 0;

    uint64_t ToCacheKey() const {
        uint64_t hash = 14695981039346656037ULL;
        auto mix = [&hash](uint64_t val) {
            for (int i = 0; i < 8; ++i) {
                hash ^= (val >> (i * 8)) & 0xFF;
                hash *= 1099511628211ULL;
            }
            };
        mix(volume_id);
        mix(file_id);
        mix(file_size);
        mix(last_write_time);
        return hash;
    }

    bool operator==(const FileIdentity& other) const {
        return volume_id == other.volume_id &&
            file_id == other.file_id &&
            file_size == other.file_size &&
            last_write_time == other.last_write_time;
    }

    bool operator!=(const FileIdentity& other) const { return !(*this == other); }

    bool IsStale(const FileIdentity& current) const {
        return file_size != current.file_size ||
            last_write_time != current.last_write_time;
    }
};

inline FileIdentity GetFileIdentity(HANDLE hFile) {
    FileIdentity id;
    BY_HANDLE_FILE_INFORMATION info = {};
    if (::GetFileInformationByHandle(hFile, &info)) {
        id.volume_id = info.dwVolumeSerialNumber;
        id.file_id = (static_cast<uint64_t>(info.nFileIndexHigh) << 32) | info.nFileIndexLow;
        id.file_size = (static_cast<uint64_t>(info.nFileSizeHigh) << 32) | info.nFileSizeLow;
        ULARGE_INTEGER wt;
        wt.LowPart = info.ftLastWriteTime.dwLowDateTime;
        wt.HighPart = info.ftLastWriteTime.dwHighDateTime;
        id.last_write_time = wt.QuadPart;
    }
    return id;
}

inline FileIdentity GetFileIdentityFromPath(const std::wstring& path) {
    FileIdentity id;
    HANDLE hFile = ::CreateFileW(path.c_str(), FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        id = GetFileIdentity(hFile);
        ::CloseHandle(hFile);
    }
    return id;
}

// ============================================================================
// USN change reasons (bitmask constants from winioctl.h)
// ============================================================================

namespace USNReasons {
inline constexpr DWORD DataOverwrite = 0x00000001;  // USN_REASON_DATA_OVERWRITE
inline constexpr DWORD DataExtend = 0x00000002;  // USN_REASON_DATA_EXTEND
inline constexpr DWORD DataTruncation = 0x00000004;  // USN_REASON_DATA_TRUNCATION
inline constexpr DWORD RenameNewName = 0x00002000;  // USN_REASON_RENAME_NEW_NAME
inline constexpr DWORD FileDelete = 0x00000200;  // USN_REASON_FILE_DELETE
inline constexpr DWORD RelevantMask = DataOverwrite | DataExtend | DataTruncation
| RenameNewName | FileDelete;
}

// ============================================================================
// USN Statistics
// ============================================================================

struct USNStats {
    uint64_t entriesProcessed = 0;
    uint64_t invalidationsTriggered = 0;
    double   journalReadsPerSecond = 0.0;
    bool     fallbackActive = false;
    uint64_t fallbackPolls = 0;
    uint32_t trackedFiles = 0;
};

// ============================================================================
// Tracked file entry
// ============================================================================

struct TrackedFile {
    std::wstring path;
    uint64_t     fileReferenceNumber = 0;
    FileIdentity identity;
};

// ============================================================================
// USN Cache Invalidation
// ============================================================================

class USNCacheInvalidation {
public:
    USNCacheInvalidation() {
        ::InitializeSRWLock(&m_srwLock);
    }

    ~USNCacheInvalidation() {
        StopMonitoring();
        if (m_volumeHandle != INVALID_HANDLE_VALUE) {
            ::CloseHandle(m_volumeHandle);
        }
    }

    USNCacheInvalidation(const USNCacheInvalidation&) = delete;
    USNCacheInvalidation& operator=(const USNCacheInvalidation&) = delete;

    // ── Initialize volume access ────────────────────────────────────────────

    bool Initialize(wchar_t volumeLetter = L'C') {
        ::AcquireSRWLockExclusive(&m_srwLock);
        m_volumeLetter = volumeLetter;

        // Open volume handle for USN journal access
        wchar_t volumePath[16] = {};
        _snwprintf_s(volumePath, _countof(volumePath), _TRUNCATE, L"\\\\.\\%c:", volumeLetter);

        m_volumeHandle = ::CreateFileW(
            volumePath,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr
        );

        if (m_volumeHandle == INVALID_HANDLE_VALUE) {
            // Fallback: no USN journal available (non-NTFS or insufficient privilege)
            m_fallbackMode = true;
            ::ReleaseSRWLockExclusive(&m_srwLock);
            return true; // Succeed with fallback
        }

        // Query USN journal
        USN_JOURNAL_DATA_V0 journalData{};
        DWORD bytesReturned = 0;
        BOOL ok = ::DeviceIoControl(
            m_volumeHandle,
            FSCTL_QUERY_USN_JOURNAL,
            nullptr, 0,
            &journalData, sizeof(journalData),
            &bytesReturned, nullptr
        );

        if (!ok) {
            // Journal not available — use fallback
            ::CloseHandle(m_volumeHandle);
            m_volumeHandle = INVALID_HANDLE_VALUE;
            m_fallbackMode = true;
            ::ReleaseSRWLockExclusive(&m_srwLock);
            return true;
        }

        m_journalId = journalData.UsnJournalID;
        m_nextUsn = journalData.NextUsn;
        m_fallbackMode = false;
        m_initialized = true;

        ::ReleaseSRWLockExclusive(&m_srwLock);
        return true;
    }

    // ── Monitoring ──────────────────────────────────────────────────────────

    void StartMonitoring() {
        if (m_monitorRunning.exchange(true)) return;

        m_monitorThread = std::thread([this]() {
            if (m_fallbackMode) {
                FallbackPollingLoop();
            }
            else {
                USNJournalLoop();
            }
            });
    }

    void StopMonitoring() {
        if (!m_monitorRunning.exchange(false)) return;
        if (m_monitorThread.joinable()) {
            m_monitorThread.join();
        }
    }

    // ── Invalidation callback ───────────────────────────────────────────────

    void SetInvalidationCallback(std::function<void(const std::wstring& path, uint32_t reason)> cb) {
        ::AcquireSRWLockExclusive(&m_srwLock);
        m_invalidationCallback = std::move(cb);
        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

    // ── File tracking ───────────────────────────────────────────────────────

    void TrackFile(const std::wstring& path) {
        ::AcquireSRWLockExclusive(&m_srwLock);

        TrackedFile tf;
        tf.path = path;
        tf.identity = GetFileIdentityFromPath(path);
        tf.fileReferenceNumber = tf.identity.file_id;

        if (tf.fileReferenceNumber != 0) {
            m_trackedByFRN[tf.fileReferenceNumber] = tf;
        }
        m_trackedByPath[path] = tf;

        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

    void UntrackFile(const std::wstring& path) {
        ::AcquireSRWLockExclusive(&m_srwLock);

        auto it = m_trackedByPath.find(path);
        if (it != m_trackedByPath.end()) {
            m_trackedByFRN.erase(it->second.fileReferenceNumber);
            m_trackedByPath.erase(it);
        }

        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

    // ── Statistics ──────────────────────────────────────────────────────────

    USNStats GetStats() const {
        ::AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_srwLock));
        USNStats stats;
        stats.entriesProcessed = m_entriesProcessed.load();
        stats.invalidationsTriggered = m_invalidationsTriggered.load();
        stats.fallbackActive = m_fallbackMode;
        stats.fallbackPolls = m_fallbackPolls.load();
        stats.trackedFiles = static_cast<uint32_t>(m_trackedByPath.size());

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double>(now - m_startTime).count();
        stats.journalReadsPerSecond = (elapsed > 0.0)
            ? static_cast<double>(m_journalReads.load()) / elapsed : 0.0;

        ::ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_srwLock));
        return stats;
    }

    bool IsInitialized() const { return m_initialized; }
    bool IsFallbackMode() const { return m_fallbackMode; }

private:
    // ── USN Journal reading loop ────────────────────────────────────────────

    void USNJournalLoop() {
        m_startTime = std::chrono::steady_clock::now();
        alignas(8) BYTE buffer[65536];

        while (m_monitorRunning.load()) {
            READ_USN_JOURNAL_DATA_V0 readData{};
            readData.StartUsn = m_nextUsn;
            readData.ReasonMask = USNReasons::RelevantMask;
            readData.ReturnOnlyOnClose = 0;
            readData.Timeout = 0;
            readData.BytesToWaitFor = 0;
            readData.UsnJournalID = m_journalId;

            DWORD bytesReturned = 0;
            BOOL ok = ::DeviceIoControl(
                m_volumeHandle,
                FSCTL_READ_USN_JOURNAL,
                &readData, sizeof(readData),
                buffer, sizeof(buffer),
                &bytesReturned, nullptr
            );

            m_journalReads++;

            if (!ok || bytesReturned <= sizeof(USN)) {
                // No new entries or error — sleep and retry
                ::Sleep(250);
                continue;
            }

            // First 8 bytes is the next USN
            USN nextUsn = *reinterpret_cast<USN*>(buffer);
            m_nextUsn = nextUsn;

            DWORD offset = sizeof(USN);
            while (offset < bytesReturned) {
                auto* record = reinterpret_cast<USN_RECORD*>(buffer + offset);
                if (record->RecordLength == 0) break;

                ProcessUSNRecord(record);
                offset += record->RecordLength;
                m_entriesProcessed++;
            }

            ::Sleep(100);
        }
    }

    void ProcessUSNRecord(USN_RECORD* record) {
        uint64_t frn = static_cast<uint64_t>(record->FileReferenceNumber);
        DWORD reason = record->Reason;

        if ((reason & USNReasons::RelevantMask) == 0) return;

        ::AcquireSRWLockShared(&m_srwLock);
        auto it = m_trackedByFRN.find(frn);
        if (it != m_trackedByFRN.end()) {
            std::wstring path = it->second.path;
            auto cb = m_invalidationCallback;
            ::ReleaseSRWLockShared(&m_srwLock);

            if (cb) {
                cb(path, reason);
            }
            m_invalidationsTriggered++;
        }
        else {
            ::ReleaseSRWLockShared(&m_srwLock);
        }
    }

    // ── Fallback polling loop (non-NTFS) ────────────────────────────────────

    void FallbackPollingLoop() {
        m_startTime = std::chrono::steady_clock::now();

        while (m_monitorRunning.load()) {
            ::AcquireSRWLockShared(&m_srwLock);
            auto snapshot = m_trackedByPath;
            auto cb = m_invalidationCallback;
            ::ReleaseSRWLockShared(&m_srwLock);

            for (const auto& [path, tracked] : snapshot) {
                WIN32_FILE_ATTRIBUTE_DATA attrs{};
                if (::GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &attrs)) {
                    ULARGE_INTEGER wt;
                    wt.LowPart = attrs.ftLastWriteTime.dwLowDateTime;
                    wt.HighPart = attrs.ftLastWriteTime.dwHighDateTime;
                    uint64_t currentWriteTime = wt.QuadPart;
                    uint64_t currentSize = (static_cast<uint64_t>(attrs.nFileSizeHigh) << 32) | attrs.nFileSizeLow;

                    if (currentWriteTime != tracked.identity.last_write_time ||
                        currentSize != tracked.identity.file_size) {
                        if (cb) {
                            cb(path, USNReasons::DataOverwrite);
                        }
                        m_invalidationsTriggered++;

                        // Update tracked identity
                        ::AcquireSRWLockExclusive(&m_srwLock);
                        auto it2 = m_trackedByPath.find(path);
                        if (it2 != m_trackedByPath.end()) {
                            it2->second.identity.last_write_time = currentWriteTime;
                            it2->second.identity.file_size = currentSize;
                        }
                        ::ReleaseSRWLockExclusive(&m_srwLock);
                    }
                }
                else {
                    // File deleted or inaccessible
                    if (cb) {
                        cb(path, USNReasons::FileDelete);
                    }
                    m_invalidationsTriggered++;
                }
                m_fallbackPolls++;
            }

            // Poll every 2 seconds in fallback mode
            for (uint32_t w = 0; w < 2000 && m_monitorRunning.load(); w += 100) {
                ::Sleep(100);
            }
        }
    }

    // ── Members ─────────────────────────────────────────────────────────────

    SRWLOCK m_srwLock{};
    HANDLE m_volumeHandle = INVALID_HANDLE_VALUE;
    wchar_t m_volumeLetter = L'C';
    bool m_initialized = false;
    bool m_fallbackMode = false;

    uint64_t m_journalId = 0;
    USN m_nextUsn = 0;

    std::unordered_map<uint64_t, TrackedFile> m_trackedByFRN;
    std::unordered_map<std::wstring, TrackedFile> m_trackedByPath;

    std::function<void(const std::wstring&, uint32_t)> m_invalidationCallback;

    std::atomic<bool> m_monitorRunning{ false };
    std::thread m_monitorThread;

    std::atomic<uint64_t> m_entriesProcessed{ 0 };
    std::atomic<uint64_t> m_invalidationsTriggered{ 0 };
    std::atomic<uint64_t> m_journalReads{ 0 };
    std::atomic<uint64_t> m_fallbackPolls{ 0 };
    std::chrono::steady_clock::time_point m_startTime = std::chrono::steady_clock::now();
};

} // namespace USNCache
} // namespace ExplorerLens
