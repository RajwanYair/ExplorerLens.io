/******************************************************************************
 * ExplorerLens Plugin Host Server Implementation
 * Copyright (c) 2026 - ExplorerLens Project
 *****************************************************************************/

#include "PluginHostServer.h"
#include "../Plugin/IPC/SharedMemoryManager.h"
#include <chrono>
#include <iostream>

namespace ExplorerLens {
namespace PluginHost {

//============================================================================
// PluginHostServer Implementation
//============================================================================

PluginHostServer::PluginHostServer() = default;

PluginHostServer::~PluginHostServer() { Shutdown(); }

bool PluginHostServer::Initialize(const std::wstring &plugin_path,
                                  const std::wstring &pipe_name,
                                  uint32_t timeout_ms) {
  plugin_path_ = plugin_path;
  pipe_name_ = pipe_name;
  timeout_ms_ = timeout_ms;

  // Load the plugin
  if (!LoadPlugin()) {
    std::wcerr << L"Failed to load plugin: " << plugin_path << L"\n";
    return false;
  }

  // Create named pipe
  if (!CreatePipe()) {
    std::wcerr << L"Failed to create named pipe\n";
    UnloadPlugin();
    return false;
  }

  running_ = true;

  // Start heartbeat thread
  StartHeartbeatThread();

  return true;
}

void PluginHostServer::Run() {
  if (!running_) {
    return;
  }

  std::wcout << L"Waiting for client connection...\n";

  // Wait for client to connect
  if (!ConnectToPipe()) {
    std::wcerr << L"Failed to connect to client\n";
    running_ = false;
    return;
  }

  std::wcout << L"Client connected, processing messages...\n";

  // Message processing loop
  while (running_ && !shutdown_requested_) {
    IPC::MessageHeader header;
    std::vector<uint8_t> payload;

    if (!ReadMessage(header, payload)) {
      // Pipe broken or read error
      std::wcerr << L"Failed to read message, shutting down\n";
      break;
    }

    // Dispatch message
    auto msg_type = static_cast<IPC::MessageType>(header.messageType);

    switch (msg_type) {
    case IPC::MessageType::REQUEST_THUMBNAIL:
      HandleRequestThumbnail(header.correlationId, payload);
      break;

    case IPC::MessageType::CAN_DECODE_QUERY:
      HandleCanDecodeQuery(header.correlationId, payload);
      break;

    case IPC::MessageType::GET_PLUGIN_INFO:
      HandleGetPluginInfo(header.correlationId);
      break;

    case IPC::MessageType::SHUTDOWN:
      HandleShutdown();
      break;

    case IPC::MessageType::HEARTBEAT_PING:
      HandleHeartbeatPing(header.correlationId);
      break;

    default:
      std::wcerr << L"Unknown message type: " << header.messageType << L"\n";
      SendErrorResponse(header.correlationId,
                        IPC::IPCErrorCode::INVALID_MESSAGE_TYPE,
                        "Unknown message type");
      break;
    }
  }

  std::wcout << L"Message loop exited\n";
}

void PluginHostServer::Shutdown() {
  if (!running_) {
    return;
  }

  shutdown_requested_ = true;
  running_ = false;

  // Stop heartbeat thread
  StopHeartbeatThread();

  // Unload plugin
  UnloadPlugin();

  // Close pipe
  if (pipe_handle_ != INVALID_HANDLE_VALUE) {
    DisconnectNamedPipe(pipe_handle_);
    CloseHandle(pipe_handle_);
    pipe_handle_ = INVALID_HANDLE_VALUE;
  }
}

//============================================================================
// IPC Methods
//============================================================================

bool PluginHostServer::CreatePipe() {
  std::wstring full_pipe_name = L"\\\\.\\pipe\\" + pipe_name_;

  pipe_handle_ =
      CreateNamedPipeW(full_pipe_name.c_str(), PIPE_ACCESS_DUPLEX,
                       PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                       1,                     // Max instances
                       IPC::MAX_MESSAGE_SIZE, // Out buffer size
                       IPC::MAX_MESSAGE_SIZE, // In buffer size
                       0,                     // Default timeout
                       nullptr);              // Default security

  return pipe_handle_ != INVALID_HANDLE_VALUE;
}

bool PluginHostServer::ConnectToPipe() {
  if (pipe_handle_ == INVALID_HANDLE_VALUE) {
    return false;
  }

  return ConnectNamedPipe(pipe_handle_, nullptr) != 0 ||
         GetLastError() == ERROR_PIPE_CONNECTED;
}

bool PluginHostServer::ReadMessage(IPC::MessageHeader &header,
                                   std::vector<uint8_t> &payload) {
  // Read header
  DWORD bytes_read = 0;
  if (!ReadFile(pipe_handle_, &header, sizeof(header), &bytes_read, nullptr)) {
    return false;
  }

  if (bytes_read != sizeof(header)) {
    return false;
  }

  // Validate header
  if (!header.IsValid()) {
    std::wcerr << L"Invalid message header\n";
    return false;
  }

  // Read payload if present
  if (header.dataSize > 0) {
    payload.resize(header.dataSize);
    if (!ReadFile(pipe_handle_, payload.data(), header.dataSize, &bytes_read,
                  nullptr)) {
      return false;
    }

    if (bytes_read != header.dataSize) {
      return false;
    }
  }

  return true;
}

bool PluginHostServer::WriteMessage(const IPC::MessageHeader &header,
                                    const void *payload, uint32_t size) {
  // Write header
  DWORD bytes_written = 0;
  if (!WriteFile(pipe_handle_, &header, sizeof(header), &bytes_written,
                 nullptr)) {
    return false;
  }

  // Write payload if present
  if (size > 0 && payload) {
    if (!WriteFile(pipe_handle_, payload, size, &bytes_written, nullptr)) {
      return false;
    }
  }

  return FlushFileBuffers(pipe_handle_) != 0;
}

//============================================================================
// Message Handlers
//============================================================================

void PluginHostServer::HandleRequestThumbnail(
    uint64_t correlation_id, const std::vector<uint8_t> &payload) {
  std::wcout << L"Processing thumbnail request (ID: " << correlation_id
             << L")\n";

  // Parse request
  if (payload.size() < sizeof(IPC::ThumbnailRequest)) {
    SendErrorResponse(correlation_id, IPC::IPCErrorCode::MALFORMED_MESSAGE,
                      "Thumbnail request too small");
    return;
  }

  const auto *request =
      reinterpret_cast<const IPC::ThumbnailRequest *>(payload.data());

  // Extract file path
  const wchar_t *file_path_ptr = reinterpret_cast<const wchar_t *>(
      payload.data() + sizeof(IPC::ThumbnailRequest));
  std::wstring file_path(file_path_ptr, request->filePathLength);

  std::wcout << L"  File: " << file_path << L"\n";
  std::wcout << L"  Size: " << request->targetWidth << L"x"
             << request->targetHeight << L"\n";

  // Implement actual plugin decoding
  IPC::ThumbnailResponse response;
  std::vector<uint8_t> bitmap_data;

  if (!plugin_) {
    // No plugin loaded
    response.resultCode =
        static_cast<uint32_t>(IPC::IPCErrorCode::PLUGIN_LOAD_FAILED);
  } else {
    // Call plugin decode function
    DecodeRequest decode_req{};
    // Convert wide string to UTF-8 for plugin API
    int utf8_length = WideCharToMultiByte(CP_UTF8, 0, file_path.c_str(), -1,
                                          nullptr, 0, nullptr, nullptr);
    std::vector<char> utf8_path(utf8_length);
    WideCharToMultiByte(CP_UTF8, 0, file_path.c_str(), -1, utf8_path.data(),
                        utf8_length, nullptr, nullptr);

    decode_req.file_path = utf8_path.data();
    decode_req.target_width = request->targetWidth;
    decode_req.target_height = request->targetHeight;

    DecodeResult decode_result{};
    PluginErrorCode err = plugin_->Decode(&decode_req, &decode_result);

    if (err == PluginErrorCode::PLUGIN_SUCCESS && decode_result.pixels) {
      // Success - prepare pixel data for IPC
      response.resultCode = static_cast<uint32_t>(IPC::IPCErrorCode::SUCCESS);
      response.width = decode_result.width;
      response.height = decode_result.height;
      response.pixelFormat = 0; // BGRA32
      response.stride = decode_result.stride;

      // Copy pixel buffer directly
      size_t data_size =
          static_cast<size_t>(decode_result.stride) * decode_result.height;
      if (decode_result.buffer_size > 0 &&
          decode_result.buffer_size < data_size) {
        data_size = decode_result.buffer_size;
      }
      bitmap_data.assign(decode_result.pixels,
                         decode_result.pixels + data_size);

      response.bitmapDataSize = static_cast<uint32_t>(bitmap_data.size());
      response.useSharedMemory = 0; // Use inline for now (TODO: implement
                                    // shared memory for large bitmaps)

      // Clean up plugin result
      plugin_->FreeResult(&decode_result);
    } else {
      // Decode failed
      response.resultCode =
          static_cast<uint32_t>(IPC::IPCErrorCode::PLUGIN_DECODE_FAILED);
      response.width = 0;
      response.height = 0;
    }
  }

  // Send response with bitmap data
  IPC::MessageHeader header;
  header.messageType =
      static_cast<uint32_t>(IPC::MessageType::RESPONSE_THUMBNAIL);
  header.correlationId = correlation_id;
  header.dataSize =
      sizeof(response) + static_cast<uint32_t>(bitmap_data.size());

  // Combine response and bitmap data into single payload
  std::vector<uint8_t> full_payload(sizeof(response) + bitmap_data.size());
  memcpy(full_payload.data(), &response, sizeof(response));
  if (!bitmap_data.empty()) {
    memcpy(full_payload.data() + sizeof(response), bitmap_data.data(),
           bitmap_data.size());
  }

  // Write message
  if (!WriteMessage(header, full_payload.data(),
                    static_cast<uint32_t>(full_payload.size()))) {
    std::wcerr << L"Failed to send thumbnail response\n";
  }
}

void PluginHostServer::HandleCanDecodeQuery(
    uint64_t correlation_id, const std::vector<uint8_t> &payload) {
  // Parse query
  if (payload.size() < sizeof(IPC::CanDecodeQuery)) {
    SendErrorResponse(correlation_id, IPC::IPCErrorCode::MALFORMED_MESSAGE,
                      "CanDecode query too small");
    return;
  }

  const auto *query =
      reinterpret_cast<const IPC::CanDecodeQuery *>(payload.data());

  // Extract file path
  const wchar_t *file_path_ptr = reinterpret_cast<const wchar_t *>(
      payload.data() + sizeof(IPC::CanDecodeQuery));
  std::wstring file_path(file_path_ptr, query->filePathLength);

  // Check with plugin
  bool can_decode = false;
  if (plugin_) {
    can_decode = plugin_->CanDecode(file_path);
  }

  // Send response
  IPC::CanDecodeResponse response;
  response.canDecode = can_decode ? 1 : 0;
  response.confidence = can_decode ? 100 : 0;

  IPC::MessageHeader header;
  header.messageType =
      static_cast<uint32_t>(IPC::MessageType::CAN_DECODE_RESPONSE);
  header.correlationId = correlation_id;
  header.dataSize = sizeof(response);

  WriteMessage(header, &response, sizeof(response));
}

void PluginHostServer::HandleGetPluginInfo(uint64_t correlation_id) {
  if (!plugin_ || !plugin_->GetInfo()) {
    SendErrorResponse(correlation_id, IPC::IPCErrorCode::PLUGIN_LOAD_FAILED,
                      "Plugin not loaded");
    return;
  }

  const auto *info = plugin_->GetInfo();

  IPC::PluginInfoResponse response;
  response.apiVersion = info->api_version;
  strncpy_s(response.pluginId, info->plugin_name,
            sizeof(response.pluginId) - 1);
  strncpy_s(response.pluginName, info->plugin_name,
            sizeof(response.pluginName) - 1);
  strncpy_s(response.pluginVersion, info->plugin_version,
            sizeof(response.pluginVersion) - 1);
  strncpy_s(response.vendor, info->plugin_author, sizeof(response.vendor) - 1);
  response.capabilities = info->capabilities;

  // Count extensions from null-terminated array
  uint32_t extCount = 0;
  if (info->supported_extensions) {
    while (info->supported_extensions[extCount] != nullptr && extCount < 32)
      ++extCount;
  }
  response.extensionCount = extCount;

  // Build full payload with extensions
  std::vector<uint8_t> payload;
  payload.resize(sizeof(response) + extCount * 16);
  memcpy(payload.data(), &response, sizeof(response));

  // Copy extensions
  char *ext_ptr = reinterpret_cast<char *>(payload.data() + sizeof(response));
  for (uint32_t i = 0; i < extCount; ++i) {
    strncpy_s(ext_ptr + i * 16, 16, info->supported_extensions[i], 15);
  }

  IPC::MessageHeader header;
  header.messageType =
      static_cast<uint32_t>(IPC::MessageType::PLUGIN_INFO_RESPONSE);
  header.correlationId = correlation_id;
  header.dataSize = static_cast<uint32_t>(payload.size());

  WriteMessage(header, payload.data(), header.dataSize);
}

void PluginHostServer::HandleShutdown() {
  std::wcout << L"Received shutdown request\n";
  shutdown_requested_ = true;
  running_ = false;
}

void PluginHostServer::HandleHeartbeatPing(uint64_t correlation_id) {
  // Update last heartbeat time
  auto now = std::chrono::high_resolution_clock::now();
  last_heartbeat_time_ = now.time_since_epoch().count();

  // Send pong
  IPC::HeartbeatMessage pong;
  pong.timestamp = last_heartbeat_time_;
  pong.processId = GetCurrentProcessId();

  IPC::MessageHeader header;
  header.messageType = static_cast<uint32_t>(IPC::MessageType::HEARTBEAT_PONG);
  header.correlationId = correlation_id;
  header.dataSize = sizeof(pong);

  WriteMessage(header, &pong, sizeof(pong));
}

void PluginHostServer::SendErrorResponse(uint64_t correlation_id,
                                         IPC::IPCErrorCode error_code,
                                         const std::string &message) {
  IPC::ErrorResponse error;
  error.errorCode = static_cast<uint32_t>(error_code);
  error.messageLength = static_cast<uint32_t>(message.size());

  // Build payload
  std::vector<uint8_t> payload;
  payload.resize(sizeof(error) + message.size());
  memcpy(payload.data(), &error, sizeof(error));
  memcpy(payload.data() + sizeof(error), message.data(), message.size());

  IPC::MessageHeader header;
  header.messageType = static_cast<uint32_t>(IPC::MessageType::ERROR_RESPONSE);
  header.correlationId = correlation_id;
  header.dataSize = static_cast<uint32_t>(payload.size());

  WriteMessage(header, payload.data(), header.dataSize);
}

//============================================================================
// Plugin Management
//============================================================================

bool PluginHostServer::LoadPlugin() {
  try {
    plugin_ = std::make_unique<PluginHandle>(plugin_path_);

    if (!plugin_->IsLoaded()) {
      std::wcerr << L"Plugin failed to load\n";
      plugin_.reset();
      return false;
    }

    const auto *info = plugin_->GetInfo();
    if (info) {
      std::wcout << L"Plugin loaded: " << info->plugin_name << L" v"
                 << info->plugin_version << L"\n";
      std::wcout << L"  Author: " << info->plugin_author << L"\n";
    }

    return true;
  } catch (const std::exception &ex) {
    std::cerr << "Exception loading plugin: " << ex.what() << "\n";
    plugin_.reset();
    return false;
  }
}

void PluginHostServer::UnloadPlugin() { plugin_.reset(); }

//============================================================================
// Heartbeat Management
//============================================================================

void PluginHostServer::StartHeartbeatThread() {
  heartbeat_thread_ = std::thread(&PluginHostServer::HeartbeatThreadProc, this);
}

void PluginHostServer::StopHeartbeatThread() {
  if (heartbeat_thread_.joinable()) {
    heartbeat_thread_.join();
  }
}

void PluginHostServer::HeartbeatThreadProc() {
  while (running_ && !shutdown_requested_) {
    // Sleep for heartbeat interval
    std::this_thread::sleep_for(
        std::chrono::milliseconds(IPC::HEARTBEAT_INTERVAL));

    // Check if we've received a heartbeat recently
    auto now = std::chrono::high_resolution_clock::now();
    auto now_count = now.time_since_epoch().count();

    uint64_t last_hb = last_heartbeat_time_.load();
    if (last_hb > 0) {
      uint64_t elapsed_ns = now_count - last_hb;
      uint64_t elapsed_ms = elapsed_ns / 1000000;

      if (elapsed_ms > IPC::HEARTBEAT_TIMEOUT) {
        std::wcerr << L"Heartbeat timeout, shutting down\n";
        shutdown_requested_ = true;
        running_ = false;
        break;
      }
    }
  }
}

} // namespace PluginHost
} // namespace ExplorerLens
