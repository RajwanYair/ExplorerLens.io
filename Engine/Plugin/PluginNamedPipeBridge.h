//==============================================================================
// ExplorerLens Engine — Plugin Named Pipe Bridge (Sprint 578)
//
// Purpose:
//   Real named-pipe IPC bridge for out-of-process plugin communication.
//   Creates a duplex overlapped named pipe with a length-prefixed binary
//   protocol (MessageType + PayloadSize + Payload), supports asynchronous
//   message callbacks via a background reader thread, and restricts pipe
//   access to the current user via an explicit DACL.
//
// Classes:
//   PluginNamedPipeBridge — Full server lifecycle (start, wait-for-client,
//   send/receive, disconnect, stop) with statistics tracking.
//
// Inputs:
//   - Plugin identifier string (used to form the pipe name)
//   - Binary message payloads (std::vector<uint8_t>)
//
// Outputs:
//   - Success / failure booleans for each operation
//   - Incoming messages delivered via callback
//   - PipeStats (messages sent/received, bytes, latency, uptime)
//
// Protocol:
//   | MessageType (4 B) | PayloadSize (4 B) | Payload (PayloadSize B) |
//   MessageType: Query=1, Response=2, Heartbeat=3, Shutdown=4
//
// Security:
//   Pipe DACL allows only the current user SID (via ConvertStringSecurityDescriptorToSecurityDescriptorW).
//
// Thread Safety:
//   All mutable state protected by SRWLOCK. Background reader thread is
//   joined on StopServer().
//
// Build:
//   Header-only, C++20, MSVC /W4 clean, no external dependencies.
//==============================================================================
#pragma once

#include <windows.h>
#include <sddl.h>
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstdint>

#pragma comment(lib, "advapi32.lib")

namespace ExplorerLens {
namespace Engine {

/// Message types in the named-pipe protocol.
enum class PipeMessageType : uint32_t {
    Query     = 1,
    Response  = 2,
    Heartbeat = 3,
    Shutdown  = 4
};

/// Cumulative pipe I/O statistics.
struct PipeStats {
    uint64_t messagesSent      = 0;
    uint64_t messagesReceived  = 0;
    uint64_t bytesSent         = 0;
    uint64_t bytesReceived     = 0;
    double   totalLatencyMs    = 0.0;
    double   connectionUptimeMs = 0.0;
    double AvgLatencyMs() const {
        uint64_t total = messagesSent + messagesReceived;
        return total ? totalLatencyMs / static_cast<double>(total) : 0.0;
    }
};

/// Named-pipe IPC bridge for plugin communication.
class PluginNamedPipeBridge {
public:
    PluginNamedPipeBridge() {
        InitializeSRWLock(&m_statsLock);
    }

    ~PluginNamedPipeBridge() {
        StopServer();
    }

    // Non-copyable
    PluginNamedPipeBridge(const PluginNamedPipeBridge&) = delete;
    PluginNamedPipeBridge& operator=(const PluginNamedPipeBridge&) = delete;

    // ---- Server lifecycle --------------------------------------------------

    /// Start the named pipe server.  Pipe name: \\.\pipe\ExplorerLens_Plugin_{pluginId}
    inline bool StartServer(const std::wstring& pluginId) {
        if (m_serverRunning.load()) return false;

        m_pipeName = L"\\\\.\\pipe\\ExplorerLens_Plugin_" + pluginId;

        // Build a SECURITY_ATTRIBUTES restricting to the current user
        SECURITY_ATTRIBUTES sa{};
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = FALSE;
        PSECURITY_DESCRIPTOR pSD = nullptr;
        bool hasSA = BuildCurrentUserDACL(&sa, &pSD);

        m_hPipe = CreateNamedPipeW(
            m_pipeName.c_str(),
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            1,                // max instances
            kBufferSize,      // out buffer
            kBufferSize,      // in buffer
            5000,             // default timeout
            hasSA ? &sa : nullptr);

        if (pSD) LocalFree(pSD);

        if (m_hPipe == INVALID_HANDLE_VALUE) {
            m_hPipe = nullptr;
            return false;
        }

        m_hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!m_hEvent) {
            CloseHandle(m_hPipe);
            m_hPipe = nullptr;
            return false;
        }

        m_serverRunning.store(true);
        return true;
    }

    /// Wait for a client to connect (blocking up to timeoutMs).
    inline bool WaitForClient(uint32_t timeoutMs = 5000) {
        if (!m_serverRunning.load() || !m_hPipe) return false;

        OVERLAPPED ol{};
        ol.hEvent = m_hEvent;
        ResetEvent(m_hEvent);

        BOOL connected = ConnectNamedPipe(m_hPipe, &ol);
        if (!connected) {
            DWORD err = GetLastError();
            if (err == ERROR_IO_PENDING) {
                DWORD waitResult = WaitForSingleObject(m_hEvent, timeoutMs);
                if (waitResult != WAIT_OBJECT_0) {
                    CancelIoEx(m_hPipe, &ol);
                    return false;
                }
                DWORD transferred = 0;
                if (!GetOverlappedResult(m_hPipe, &ol, &transferred, FALSE)) {
                    return false;
                }
            } else if (err != ERROR_PIPE_CONNECTED) {
                return false;
            }
        }

        m_clientConnected.store(true);
        m_connectTime = std::chrono::high_resolution_clock::now();

        // Start background reader if callback is set
        if (m_messageCallback) {
            StartReaderThread();
        }

        return true;
    }

    /// Send a length-prefixed message.
    inline bool SendMessage(const std::vector<uint8_t>& data) {
        return SendRaw(static_cast<uint32_t>(PipeMessageType::Response),
                       data.data(), static_cast<uint32_t>(data.size()));
    }

    /// Send a typed message with message type.
    inline bool SendTypedMessage(PipeMessageType type,
                                 const std::vector<uint8_t>& data) {
        return SendRaw(static_cast<uint32_t>(type),
                       data.data(), static_cast<uint32_t>(data.size()));
    }

    /// Receive a length-prefixed message (blocking up to timeoutMs).
    inline bool ReceiveMessage(std::vector<uint8_t>& outData,
                               uint32_t timeoutMs = 5000) {
        uint32_t msgType = 0;
        return ReceiveRaw(msgType, outData, timeoutMs);
    }

    /// Receive typed message with message type.
    inline bool ReceiveTypedMessage(uint32_t& outMsgType,
                                    std::vector<uint8_t>& outData,
                                    uint32_t timeoutMs = 5000) {
        return ReceiveRaw(outMsgType, outData, timeoutMs);
    }

    /// Set an asynchronous message callback.  If the server is already
    /// connected, the reader thread starts immediately.
    inline void SetMessageCallback(
        std::function<void(uint32_t msgType, const std::vector<uint8_t>& payload)> fn) {
        m_messageCallback = std::move(fn);
        if (m_clientConnected.load() && !m_readerRunning.load()) {
            StartReaderThread();
        }
    }

    /// Disconnect current client (pipe stays open for next client).
    inline void Disconnect() {
        StopReaderThread();
        if (m_hPipe && m_clientConnected.load()) {
            FlushFileBuffers(m_hPipe);
            DisconnectNamedPipe(m_hPipe);
        }
        UpdateUptimeStats();
        m_clientConnected.store(false);
    }

    /// Stop the server entirely and close the pipe handle.
    inline void StopServer() {
        Disconnect();
        m_serverRunning.store(false);
        if (m_hEvent) { CloseHandle(m_hEvent); m_hEvent = nullptr; }
        if (m_hPipe)  { CloseHandle(m_hPipe);  m_hPipe  = nullptr; }
    }

    // ---- Status queries ----------------------------------------------------

    inline bool IsConnected()    const { return m_clientConnected.load(); }
    inline bool IsServerRunning() const { return m_serverRunning.load(); }

    /// Retrieve cumulative statistics (thread-safe).
    inline PipeStats GetStats() const {
        AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_statsLock));
        PipeStats copy = m_stats;
        ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_statsLock));
        return copy;
    }

private:
    static constexpr DWORD kBufferSize = 65536;

    // ---- Low-level send / receive ------------------------------------------

    inline bool SendRaw(uint32_t msgType, const uint8_t* data, uint32_t size) {
        if (!m_hPipe || !m_clientConnected.load()) return false;

        using Clock = std::chrono::high_resolution_clock;
        auto t0 = Clock::now();

        // Header: [msgType:4][payloadSize:4]
        uint8_t header[8];
        std::memcpy(header, &msgType, 4);
        std::memcpy(header + 4, &size, 4);

        OVERLAPPED ol{};
        ol.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!ol.hEvent) return false;

        // Write header
        DWORD written = 0;
        if (!WriteFile(m_hPipe, header, 8, nullptr, &ol)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                CloseHandle(ol.hEvent);
                return false;
            }
            WaitForSingleObject(ol.hEvent, 5000);
        }
        GetOverlappedResult(m_hPipe, &ol, &written, FALSE);
        if (written != 8) { CloseHandle(ol.hEvent); return false; }

        // Write payload
        if (size > 0 && data) {
            ResetEvent(ol.hEvent);
            written = 0;
            if (!WriteFile(m_hPipe, data, size, nullptr, &ol)) {
                if (GetLastError() != ERROR_IO_PENDING) {
                    CloseHandle(ol.hEvent);
                    return false;
                }
                WaitForSingleObject(ol.hEvent, 5000);
            }
            GetOverlappedResult(m_hPipe, &ol, &written, FALSE);
            if (written != size) { CloseHandle(ol.hEvent); return false; }
        }

        CloseHandle(ol.hEvent);

        auto t1 = Clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        AcquireSRWLockExclusive(&m_statsLock);
        m_stats.messagesSent++;
        m_stats.bytesSent += 8 + size;
        m_stats.totalLatencyMs += ms;
        ReleaseSRWLockExclusive(&m_statsLock);

        return true;
    }

    inline bool ReceiveRaw(uint32_t& outMsgType,
                           std::vector<uint8_t>& outData,
                           uint32_t timeoutMs) {
        if (!m_hPipe || !m_clientConnected.load()) return false;

        using Clock = std::chrono::high_resolution_clock;
        auto t0 = Clock::now();

        OVERLAPPED ol{};
        ol.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!ol.hEvent) return false;

        // Read 8-byte header
        uint8_t header[8]{};
        DWORD bytesRead = 0;
        if (!ReadFile(m_hPipe, header, 8, nullptr, &ol)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                CloseHandle(ol.hEvent);
                return false;
            }
            DWORD wait = WaitForSingleObject(ol.hEvent, timeoutMs);
            if (wait != WAIT_OBJECT_0) {
                CancelIoEx(m_hPipe, &ol);
                CloseHandle(ol.hEvent);
                return false;
            }
        }
        GetOverlappedResult(m_hPipe, &ol, &bytesRead, FALSE);
        if (bytesRead != 8) { CloseHandle(ol.hEvent); return false; }

        std::memcpy(&outMsgType, header, 4);
        uint32_t payloadSize = 0;
        std::memcpy(&payloadSize, header + 4, 4);

        // Sanity check
        if (payloadSize > 16 * 1024 * 1024) { CloseHandle(ol.hEvent); return false; }

        outData.resize(payloadSize);
        if (payloadSize > 0) {
            ResetEvent(ol.hEvent);
            bytesRead = 0;
            if (!ReadFile(m_hPipe, outData.data(), payloadSize, nullptr, &ol)) {
                if (GetLastError() != ERROR_IO_PENDING) {
                    CloseHandle(ol.hEvent);
                    return false;
                }
                DWORD wait = WaitForSingleObject(ol.hEvent, timeoutMs);
                if (wait != WAIT_OBJECT_0) {
                    CancelIoEx(m_hPipe, &ol);
                    CloseHandle(ol.hEvent);
                    return false;
                }
            }
            GetOverlappedResult(m_hPipe, &ol, &bytesRead, FALSE);
            if (bytesRead != payloadSize) { CloseHandle(ol.hEvent); return false; }
        }

        CloseHandle(ol.hEvent);

        auto t1 = Clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        AcquireSRWLockExclusive(&m_statsLock);
        m_stats.messagesReceived++;
        m_stats.bytesReceived += 8 + payloadSize;
        m_stats.totalLatencyMs += ms;
        ReleaseSRWLockExclusive(&m_statsLock);

        return true;
    }

    // ---- Background reader thread ------------------------------------------

    inline void StartReaderThread() {
        if (m_readerRunning.load()) return;
        m_readerRunning.store(true);
        m_readerThread = std::thread([this]() {
            while (m_readerRunning.load() && m_clientConnected.load()) {
                uint32_t msgType = 0;
                std::vector<uint8_t> payload;
                if (ReceiveRaw(msgType, payload, 500)) {
                    if (m_messageCallback) {
                        m_messageCallback(msgType, payload);
                    }
                    if (static_cast<PipeMessageType>(msgType) == PipeMessageType::Shutdown) {
                        break;
                    }
                }
            }
            m_readerRunning.store(false);
        });
    }

    inline void StopReaderThread() {
        m_readerRunning.store(false);
        if (m_readerThread.joinable()) {
            m_readerThread.join();
        }
    }

    // ---- Security helpers --------------------------------------------------

    /// Build a SECURITY_DESCRIPTOR with a DACL allowing only the current user.
    static inline bool BuildCurrentUserDACL(SECURITY_ATTRIBUTES* sa,
                                            PSECURITY_DESCRIPTOR* ppSD) {
        // Get current user SID string
        HANDLE hToken = nullptr;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) return false;

        DWORD tokenSize = 0;
        GetTokenInformation(hToken, TokenUser, nullptr, 0, &tokenSize);
        if (tokenSize == 0) { CloseHandle(hToken); return false; }

        std::vector<uint8_t> tokenBuf(tokenSize);
        if (!GetTokenInformation(hToken, TokenUser, tokenBuf.data(), tokenSize, &tokenSize)) {
            CloseHandle(hToken);
            return false;
        }
        CloseHandle(hToken);

        auto* tokenUser = reinterpret_cast<TOKEN_USER*>(tokenBuf.data());
        LPWSTR sidString = nullptr;
        if (!ConvertSidToStringSidW(tokenUser->User.Sid, &sidString)) return false;

        // SDDL: owner=current user, DACL grants full access only to that SID
        std::wstring sddl = L"D:(A;;GA;;;";
        sddl += sidString;
        sddl += L")";
        LocalFree(sidString);

        BOOL ok = ConvertStringSecurityDescriptorToSecurityDescriptorW(
            sddl.c_str(), SDDL_REVISION_1, ppSD, nullptr);
        if (!ok) return false;

        sa->lpSecurityDescriptor = *ppSD;
        return true;
    }

    /// Accumulate connection uptime into stats.
    inline void UpdateUptimeStats() {
        if (m_clientConnected.load()) {
            auto now = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(now - m_connectTime).count();
            AcquireSRWLockExclusive(&m_statsLock);
            m_stats.connectionUptimeMs += ms;
            ReleaseSRWLockExclusive(&m_statsLock);
        }
    }

    // ---- Member state ------------------------------------------------------

    std::wstring    m_pipeName;
    HANDLE          m_hPipe  = nullptr;
    HANDLE          m_hEvent = nullptr;

    std::atomic<bool> m_serverRunning{false};
    std::atomic<bool> m_clientConnected{false};
    std::atomic<bool> m_readerRunning{false};

    std::thread     m_readerThread;
    std::function<void(uint32_t, const std::vector<uint8_t>&)> m_messageCallback;

    std::chrono::high_resolution_clock::time_point m_connectTime{};

    mutable SRWLOCK m_statsLock{};
    PipeStats       m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
