/******************************************************************************
 * ExplorerLens Plugin Host Server
 * Copyright (c) 2026 - ExplorerLens Project
 * 
 * Server-side implementation for PluginHost process. Manages plugin loading,
 * IPC communication, and request processing.
 *****************************************************************************/

#pragma once

#include "../Plugin/IPC/PluginIPCProtocol.h"
#include "../Plugin/PluginManager.h"
#include <Windows.h>
#include <string>
#include <memory>
#include <atomic>
#include <thread>

namespace ExplorerLens {
namespace PluginHost {

//============================================================================
// Plugin Host Server
//============================================================================

class PluginHostServer {
public:
 PluginHostServer();
 ~PluginHostServer();
 
 // Initialize the server with plugin and pipe name
 bool Initialize(const std::wstring& plugin_path,
 const std::wstring& pipe_name,
 uint32_t timeout_ms);
 
 // Run the message processing loop
 void Run();
 
 // Shutdown the server
 void Shutdown();
 
 // Check if server is running
 bool IsRunning() const { return running_; }

private:
 // IPC Methods
 bool CreatePipe();
 bool ConnectToPipe();
 bool ReadMessage(IPC::MessageHeader& header, std::vector<uint8_t>& payload);
 bool WriteMessage(const IPC::MessageHeader& header, const void* payload, uint32_t size);
 
 // Message Handlers
 void HandleRequestThumbnail(uint64_t correlation_id, const std::vector<uint8_t>& payload);
 void HandleCanDecodeQuery(uint64_t correlation_id, const std::vector<uint8_t>& payload);
 void HandleGetPluginInfo(uint64_t correlation_id);
 void HandleShutdown();
 void HandleHeartbeatPing(uint64_t correlation_id);
 
 // Error Response
 void SendErrorResponse(uint64_t correlation_id, IPC::IPCErrorCode error_code, const std::string& message);
 
 // Plugin Management
 bool LoadPlugin();
 void UnloadPlugin();
 
 // Heartbeat Management
 void StartHeartbeatThread();
 void StopHeartbeatThread();
 void HeartbeatThreadProc();
 
 // Configuration
 std::wstring plugin_path_;
 std::wstring pipe_name_;
 uint32_t timeout_ms_ = 30000;
 
 // IPC
 HANDLE pipe_handle_ = INVALID_HANDLE_VALUE;
 
 // Plugin
 std::unique_ptr<PluginHandle> plugin_;
 
 // State
 std::atomic<bool> running_{false};
 std::atomic<bool> shutdown_requested_{false};
 
 // Heartbeat
 std::thread heartbeat_thread_;
 std::atomic<uint64_t> last_heartbeat_time_{0};
};

} // namespace PluginHost
} // namespace ExplorerLens

