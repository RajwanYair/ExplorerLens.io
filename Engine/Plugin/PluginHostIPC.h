// PluginHostIPC.h — Named Pipe IPC for Plugin Host Communication
// Copyright (c) 2026 ExplorerLens Project
//
// Implements the named pipe protocol between LENSShell.dll (in-process
// COM shell extension) and PluginHost.exe (out-of-process plugin runner).
// Provides serialization, message framing, and async I/O.

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <vector>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// IPC message types for plugin host protocol
enum class IPCMessageType : uint32_t {
 // Control messages
 Handshake = 0x0001, ///< Initial connection setup
 Heartbeat = 0x0002, ///< Keep-alive ping
 Shutdown = 0x0003, ///< Graceful shutdown request
 Ack = 0x0004, ///< Acknowledgment

 // Plugin management
 LoadPlugin = 0x0100, ///< Load a plugin DLL
 UnloadPlugin = 0x0101, ///< Unload a plugin
 ListPlugins = 0x0102, ///< Enumerate loaded plugins
 PluginInfo = 0x0103, ///< Get plugin metadata

 // Decode operations
 DecodeRequest = 0x0200, ///< Request thumbnail decode
 DecodeResult = 0x0201, ///< Decode result (bitmap data)
 DecodeError = 0x0202, ///< Decode failure report
 DecodeCancel = 0x0203, ///< Cancel pending decode

 // Diagnostics
 GetStats = 0x0300, ///< Request performance stats
 StatsResponse = 0x0301, ///< Performance stats response
 CrashReport = 0x0302, ///< Plugin crash notification
};

/// IPC message header (fixed 16 bytes, little‐endian)
struct IPCMessageHeader {
 uint32_t magic = 0x4C454E53; ///< "LENS" in ASCII
 IPCMessageType type = IPCMessageType::Heartbeat;
 uint32_t payloadSize = 0; ///< Size of payload following header
 uint32_t sequenceId = 0; ///< Request/response correlation ID
};
static_assert(sizeof(IPCMessageHeader) == 16, "IPC header must be 16 bytes");

/// IPC connection state
enum class IPCConnectionState : uint8_t {
 Disconnected = 0,
 Connecting = 1,
 Connected = 2,
 Error = 3
};

/// Named pipe IPC channel
class PluginHostIPC {
public:
 static PluginHostIPC& Instance() {
 static PluginHostIPC instance;
 return instance;
 }

 /// Create and connect to the named pipe
 bool Connect(const wchar_t* pipeName = L"\\\\.\\pipe\\ExplorerLensPluginHost") {
 std::lock_guard<std::mutex> lock(m_mutex);
 if (m_state == IPCConnectionState::Connected) return true;

 m_state = IPCConnectionState::Connecting;
 m_hPipe = CreateFileW(pipeName, GENERIC_READ | GENERIC_WRITE,
 0, nullptr, OPEN_EXISTING,
 FILE_FLAG_OVERLAPPED, nullptr);

 if (m_hPipe == INVALID_HANDLE_VALUE) {
 m_state = IPCConnectionState::Error;
 m_lastError = GetLastError();
 return false;
 }

 // Set pipe to message mode
 DWORD mode = PIPE_READMODE_MESSAGE;
 SetNamedPipeHandleState(m_hPipe, &mode, nullptr, nullptr);

 m_state = IPCConnectionState::Connected;
 m_sequenceCounter = 0;
 return true;
 }

 /// Create named pipe server (called by PluginHost.exe)
 bool CreateServer(const wchar_t* pipeName = L"\\\\.\\pipe\\ExplorerLensPluginHost") {
 std::lock_guard<std::mutex> lock(m_mutex);
 m_hPipe = CreateNamedPipeW(
 pipeName,
 PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
 PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
 1, // Max instances
 65536, // Output buffer
 65536, // Input buffer
 5000, // Default timeout
 nullptr); // Default security

 if (m_hPipe == INVALID_HANDLE_VALUE) {
 m_state = IPCConnectionState::Error;
 return false;
 }
 return true;
 }

 /// Disconnect
 void Disconnect() {
 std::lock_guard<std::mutex> lock(m_mutex);
 if (m_hPipe != INVALID_HANDLE_VALUE) {
 CloseHandle(m_hPipe);
 m_hPipe = INVALID_HANDLE_VALUE;
 }
 m_state = IPCConnectionState::Disconnected;
 }

 /// Send a message
 bool Send(IPCMessageType type, const void* payload = nullptr, uint32_t payloadSize = 0) {
 if (m_state != IPCConnectionState::Connected) return false;

 IPCMessageHeader header;
 header.type = type;
 header.payloadSize = payloadSize;
 header.sequenceId = m_sequenceCounter++;

 DWORD written = 0;
 if (!WriteFile(m_hPipe, &header, sizeof(header), &written, nullptr) ||
 written != sizeof(header)) {
 return false;
 }

 if (payload && payloadSize > 0) {
 if (!WriteFile(m_hPipe, payload, payloadSize, &written, nullptr) ||
 written != payloadSize) {
 return false;
 }
 }
 return true;
 }

 /// Receive a message (blocking)
 bool Receive(IPCMessageHeader& header, std::vector<uint8_t>& payload) {
 if (m_state != IPCConnectionState::Connected) return false;

 DWORD bytesRead = 0;
 if (!ReadFile(m_hPipe, &header, sizeof(header), &bytesRead, nullptr) ||
 bytesRead != sizeof(header)) {
 return false;
 }

 if (header.magic != 0x4C454E53) return false; // Invalid magic

 if (header.payloadSize > 0) {
 payload.resize(header.payloadSize);
 if (!ReadFile(m_hPipe, payload.data(), header.payloadSize, &bytesRead, nullptr) ||
 bytesRead != header.payloadSize) {
 return false;
 }
 }
 return true;
 }

 /// Get connection state
 IPCConnectionState GetState() const { return m_state; }
 DWORD GetLastError() const { return m_lastError; }

 /// Message type name lookup
 static const char* MessageTypeName(IPCMessageType t) {
 switch (t) {
 case IPCMessageType::Handshake: return "Handshake";
 case IPCMessageType::Heartbeat: return "Heartbeat";
 case IPCMessageType::Shutdown: return "Shutdown";
 case IPCMessageType::Ack: return "Ack";
 case IPCMessageType::LoadPlugin: return "LoadPlugin";
 case IPCMessageType::UnloadPlugin: return "UnloadPlugin";
 case IPCMessageType::ListPlugins: return "ListPlugins";
 case IPCMessageType::PluginInfo: return "PluginInfo";
 case IPCMessageType::DecodeRequest: return "DecodeRequest";
 case IPCMessageType::DecodeResult: return "DecodeResult";
 case IPCMessageType::DecodeError: return "DecodeError";
 case IPCMessageType::DecodeCancel: return "DecodeCancel";
 case IPCMessageType::GetStats: return "GetStats";
 case IPCMessageType::StatsResponse: return "StatsResponse";
 case IPCMessageType::CrashReport: return "CrashReport";
 default: return "Unknown";
 }
 }

 /// Get total number of message types
 static constexpr uint32_t GetMessageTypeCount() { return 15; }

 /// Connection state name lookup
 static const char* ConnectionStateName(IPCConnectionState s) {
 switch (s) {
 case IPCConnectionState::Disconnected: return "Disconnected";
 case IPCConnectionState::Connecting: return "Connecting";
 case IPCConnectionState::Connected: return "Connected";
 case IPCConnectionState::Error: return "Error";
 default: return "Unknown";
 }
 }

private:
 PluginHostIPC() = default;
 ~PluginHostIPC() { Disconnect(); }
 PluginHostIPC(const PluginHostIPC&) = delete;
 PluginHostIPC& operator=(const PluginHostIPC&) = delete;

 HANDLE m_hPipe = INVALID_HANDLE_VALUE;
 IPCConnectionState m_state = IPCConnectionState::Disconnected;
 DWORD m_lastError = 0;
 std::atomic<uint32_t> m_sequenceCounter{0};
 std::mutex m_mutex;
};

} // namespace Engine
} // namespace ExplorerLens
