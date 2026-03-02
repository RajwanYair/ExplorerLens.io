//==============================================================================
// ExplorerLens Engine — Plugin Hot Reload Manager (Sprint 580)
//
// Watches plugin DLL directories for changes using ReadDirectoryChangesW and
// hot-reloads modified plugins without restarting Explorer. Features:
//   - Filesystem monitoring via ReadDirectoryChangesW on a background thread
//   - Debounce logic (500ms) to handle multi-write file operations
//   - SHA-256 hash comparison via Windows CNG (BCrypt) to filter false positives
//   - Callback-based reload notification for caller-managed unload/load cycle
//   - Thread-safe with SRWLOCK
//
// Header-only, C++20, MSVC /W4 clean.
//==============================================================================
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <bcrypt.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <chrono>
#include <cstdint>

#pragma comment(lib, "bcrypt.lib")

namespace ExplorerLens {
namespace Engine {

struct HotReloadStats {
    uint64_t filesWatched         = 0;
    uint64_t reloadsTriggered     = 0;
    uint64_t hashMismatches       = 0;
    uint64_t falsePositivesFiltered = 0;
};

class PluginHotReloadManager {
public:
    PluginHotReloadManager() {
        InitializeSRWLock(&m_lock);
    }

    ~PluginHotReloadManager() {
        StopWatching();
    }

    PluginHotReloadManager(const PluginHotReloadManager&) = delete;
    PluginHotReloadManager& operator=(const PluginHotReloadManager&) = delete;

    inline void SetPluginDirectory(const std::wstring& dir) {
        AcquireSRWLockExclusive(&m_lock);
        m_pluginDirectory = dir;
        if (!m_pluginDirectory.empty() && m_pluginDirectory.back() != L'\\') {
            m_pluginDirectory += L'\\';
        }
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline void SetReloadCallback(std::function<void(const std::wstring&)> fn) {
        AcquireSRWLockExclusive(&m_lock);
        m_reloadCallback = std::move(fn);
        ReleaseSRWLockExclusive(&m_lock);
    }

    inline void StartWatching() {
        if (m_watching.exchange(true, std::memory_order_acq_rel)) {
            return; // Already watching
        }

        m_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!m_stopEvent) {
            m_watching.store(false, std::memory_order_release);
            return;
        }

        m_watchThread = std::thread([this]() { WatchThreadProc(); });
    }

    inline void StopWatching() {
        if (!m_watching.exchange(false, std::memory_order_acq_rel)) {
            return; // Not watching
        }

        if (m_stopEvent) {
            SetEvent(m_stopEvent);
        }

        if (m_watchThread.joinable()) {
            m_watchThread.join();
        }

        if (m_stopEvent) {
            CloseHandle(m_stopEvent);
            m_stopEvent = nullptr;
        }
    }

    inline void RegisterPluginHash(const std::wstring& dllPath) {
        std::vector<uint8_t> hash = ComputeSHA256(dllPath);
        if (!hash.empty()) {
            AcquireSRWLockExclusive(&m_lock);
            m_knownHashes[dllPath] = std::move(hash);
            m_stats.filesWatched = m_knownHashes.size();
            ReleaseSRWLockExclusive(&m_lock);
        }
    }

    inline bool HasChanged(const std::wstring& dllPath) {
        std::vector<uint8_t> currentHash = ComputeSHA256(dllPath);
        if (currentHash.empty()) return false;

        AcquireSRWLockShared(&m_lock);
        auto it = m_knownHashes.find(dllPath);
        if (it == m_knownHashes.end()) {
            ReleaseSRWLockShared(&m_lock);
            return true; // Unknown plugin, treat as changed
        }
        bool changed = (it->second != currentHash);
        ReleaseSRWLockShared(&m_lock);
        return changed;
    }

    inline HotReloadStats GetStats() const {
        HotReloadStats s;
        AcquireSRWLockShared(&m_lock);
        s = m_stats;
        ReleaseSRWLockShared(&m_lock);
        return s;
    }

private:
    inline void WatchThreadProc() {
        std::wstring dirCopy;
        AcquireSRWLockShared(&m_lock);
        dirCopy = m_pluginDirectory;
        ReleaseSRWLockShared(&m_lock);

        if (dirCopy.empty()) {
            m_watching.store(false, std::memory_order_release);
            return;
        }

        HANDLE hDir = CreateFileW(
            dirCopy.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            nullptr);

        if (hDir == INVALID_HANDLE_VALUE) {
            m_watching.store(false, std::memory_order_release);
            return;
        }

        constexpr DWORD kBufSize = 8192;
        alignas(DWORD) uint8_t buffer[kBufSize]{};
        OVERLAPPED overlapped{};
        overlapped.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

        if (!overlapped.hEvent) {
            CloseHandle(hDir);
            m_watching.store(false, std::memory_order_release);
            return;
        }

        HANDLE waitHandles[2] = { m_stopEvent, overlapped.hEvent };

        while (m_watching.load(std::memory_order_acquire)) {
            ResetEvent(overlapped.hEvent);

            BOOL ok = ReadDirectoryChangesW(
                hDir, buffer, kBufSize, FALSE,
                FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE,
                nullptr, &overlapped, nullptr);

            if (!ok) break;

            DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);
            if (waitResult == WAIT_OBJECT_0) {
                // Stop event signaled
                CancelIoEx(hDir, &overlapped);
                break;
            }

            if (waitResult != WAIT_OBJECT_0 + 1) break;

            DWORD bytesReturned = 0;
            if (!GetOverlappedResult(hDir, &overlapped, &bytesReturned, FALSE) || bytesReturned == 0) {
                continue;
            }

            // Parse FILE_NOTIFY_INFORMATION entries
            DWORD offset = 0;
            do {
                auto* fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer + offset);
                if (fni->Action == FILE_ACTION_MODIFIED) {
                    std::wstring fileName(fni->FileName,
                        fni->FileNameLength / sizeof(wchar_t));

                    // Only process .dll files
                    if (fileName.size() >= 4) {
                        std::wstring ext = fileName.substr(fileName.size() - 4);
                        for (auto& c : ext) c = static_cast<wchar_t>(towlower(c));
                        if (ext == L".dll") {
                            std::wstring fullPath = dirCopy + fileName;
                            HandleDllChanged(fullPath);
                        }
                    }
                }

                if (fni->NextEntryOffset == 0) break;
                offset += fni->NextEntryOffset;
            } while (offset < bytesReturned);
        }

        CloseHandle(overlapped.hEvent);
        CloseHandle(hDir);
    }

    inline void HandleDllChanged(const std::wstring& fullPath) {
        // Debounce: wait 500ms for writes to settle
        Sleep(500);

        // Verify file is accessible (not locked by another process)
        HANDLE hTest = CreateFileW(fullPath.c_str(), GENERIC_READ,
            FILE_SHARE_READ, nullptr, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hTest == INVALID_HANDLE_VALUE) {
            return; // File still locked
        }
        CloseHandle(hTest);

        // Compute new hash
        std::vector<uint8_t> newHash = ComputeSHA256(fullPath);
        if (newHash.empty()) return;

        // Compare with stored hash
        AcquireSRWLockExclusive(&m_lock);
        auto it = m_knownHashes.find(fullPath);
        if (it != m_knownHashes.end() && it->second == newHash) {
            // Hash unchanged — false positive (metadata change only)
            m_stats.falsePositivesFiltered++;
            ReleaseSRWLockExclusive(&m_lock);
            return;
        }

        // Update stored hash
        m_knownHashes[fullPath] = newHash;
        m_stats.hashMismatches++;
        m_stats.reloadsTriggered++;

        auto callback = m_reloadCallback;
        ReleaseSRWLockExclusive(&m_lock);

        // Trigger reload callback outside lock
        if (callback) {
            callback(fullPath);
        }
    }

    inline std::vector<uint8_t> ComputeSHA256(const std::wstring& filePath) {
        std::vector<uint8_t> hashResult;

        HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ,
            FILE_SHARE_READ, nullptr, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return hashResult;

        BCRYPT_ALG_HANDLE hAlg = nullptr;
        BCRYPT_HASH_HANDLE hHash = nullptr;

        NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg,
            BCRYPT_SHA256_ALGORITHM, nullptr, 0);
        if (status < 0) {
            CloseHandle(hFile);
            return hashResult;
        }

        DWORD hashLength = 0;
        DWORD cbData = 0;
        status = BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH,
            reinterpret_cast<PUCHAR>(&hashLength), sizeof(hashLength), &cbData, 0);
        if (status < 0 || hashLength == 0) {
            BCryptCloseAlgorithmProvider(hAlg, 0);
            CloseHandle(hFile);
            return hashResult;
        }

        DWORD hashObjectSize = 0;
        status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH,
            reinterpret_cast<PUCHAR>(&hashObjectSize), sizeof(hashObjectSize), &cbData, 0);
        if (status < 0 || hashObjectSize == 0) {
            BCryptCloseAlgorithmProvider(hAlg, 0);
            CloseHandle(hFile);
            return hashResult;
        }

        std::vector<uint8_t> hashObject(hashObjectSize);
        status = BCryptCreateHash(hAlg, &hHash, hashObject.data(),
            hashObjectSize, nullptr, 0, 0);
        if (status < 0) {
            BCryptCloseAlgorithmProvider(hAlg, 0);
            CloseHandle(hFile);
            return hashResult;
        }

        // Read file in 64KB chunks and hash
        constexpr DWORD kChunkSize = 65536;
        std::vector<uint8_t> readBuf(kChunkSize);
        DWORD bytesRead = 0;
        while (ReadFile(hFile, readBuf.data(), kChunkSize, &bytesRead, nullptr) && bytesRead > 0) {
            BCryptHashData(hHash, readBuf.data(), bytesRead, 0);
        }

        hashResult.resize(hashLength);
        status = BCryptFinishHash(hHash, hashResult.data(), hashLength, 0);
        if (status < 0) {
            hashResult.clear();
        }

        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        CloseHandle(hFile);

        return hashResult;
    }

    // Members
    mutable SRWLOCK m_lock{};
    std::wstring    m_pluginDirectory;
    std::function<void(const std::wstring&)> m_reloadCallback;
    std::unordered_map<std::wstring, std::vector<uint8_t>> m_knownHashes;
    std::thread     m_watchThread;
    HANDLE          m_stopEvent = nullptr;
    std::atomic<bool> m_watching{false};
    HotReloadStats  m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
