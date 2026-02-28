/******************************************************************************
 * ExplorerLens Plugin Host Client
 * Copyright (c) 2026 - ExplorerLens Project
 * 
 * Engine-side client for communicating with PluginHost processes.
 * Manages process spawning, IPC communication, and crash detection.
 *****************************************************************************/

#pragma once

#include "IPC/PluginIPCProtocol.h"
#include "IPC/SharedMemoryManager.h"
#include "Security/JobObjectManager.h"
#include "../SDK/plugin_api.h"
#include <Windows.h>
#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include <chrono>

namespace ExplorerLens {

//============================================================================
// Plugin Host Client
//============================================================================

class PluginHostClient {
public:
 PluginHostClient();
 ~PluginHostClient();
 
 // Non-copyable
 PluginHostClient(const PluginHostClient&) = delete;
 PluginHostClient& operator=(const PluginHostClient&) = delete;
 
 // Start PluginHost process for specific plugin
 bool StartPluginHost(const std::wstring& plugin_path,
 uint32_t timeout_ms = IPC::DEFAULT_REQUEST_TIMEOUT);
 
 // Send decode request
 IPC::IPCErrorCode RequestThumbnail(const DecodeRequest& request,
 DecodeResult& result);
 
 // Query if plugin can decode a file
 bool CanDecode(const std::wstring& file_path);
 bool CanDecode(const uint8_t* header_data, size_t header_size);
 
 // Get plugin information
 bool GetPluginInfo(PluginInfo& info);
 
 // Check if PluginHost is still alive
 bool IsAlive() const;
 
 // Check if PluginHost has crashed
 bool HasCrashed() const;
 
 // Get process exit code (if terminated)
 DWORD GetExitCode() const;
 
 // Gracefully shutdown
 void Shutdown();
 
 // Force terminate (crash recovery)
 void Terminate();
 
 // Get process and pipe handles
 HANDLE GetProcessHandle() const { return process_handle_; }
 HANDLE GetPipeHandle() const { return pipe_handle_; }
 
 // Get plugin path
 const std::wstring& GetPluginPath() const { return plugin_path_; }

private:
 // Process management
 bool SpawnPluginHostProcess();
 bool ConnectToPipe();
 void CloseHandles();
 
 // IPC
 bool SendMessage(const IPC::MessageHeader& header, 
 const void* payload, 
 uint32_t size);
 bool ReceiveMessage(IPC::MessageHeader& header, 
 std::vector<uint8_t>& payload,
 uint32_t timeout_ms);
 
 // Correlation ID generation
 uint64_t GenerateCorrelationId();
 
 // Heartbeat
 void SendHeartbeat();
 bool CheckHeartbeat();
 
 // Configuration
 std::wstring plugin_path_;
 std::wstring pipe_name_;
 uint32_t timeout_ms_ = IPC::DEFAULT_REQUEST_TIMEOUT;
 
 // Process handles
 HANDLE process_handle_ = nullptr;
 HANDLE pipe_handle_ = INVALID_HANDLE_VALUE;
 
 // Job object for resource limits
 std::unique_ptr<Security::JobObjectManager> job_object_;
 
 // State
 std::atomic<bool> is_alive_{false};
 std::atomic<uint64_t> next_correlation_id_{1};
 std::chrono::steady_clock::time_point last_heartbeat_;
 
 // Thread safety
 mutable std::mutex mutex_;
};

} // namespace ExplorerLens

