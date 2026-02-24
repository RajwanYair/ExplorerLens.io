/******************************************************************************
 * ExplorerLens Plugin Host Client Implementation
 * Copyright (c) 2026 - ExplorerLens Project
 *****************************************************************************/

#include "PluginHostClient.h"
#include <sstream>
#include <iomanip>
#include <iostream>

namespace ExplorerLens {

//============================================================================
// PluginHostClient Implementation
//============================================================================

PluginHostClient::PluginHostClient() {
    last_heartbeat_ = std::chrono::steady_clock::now();
}

PluginHostClient::~PluginHostClient() {
    Shutdown();
}

bool PluginHostClient::StartPluginHost(const std::wstring& plugin_path, uint32_t timeout_ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (is_alive_) {
        return false;  // Already running
    }
    
    plugin_path_ = plugin_path;
    timeout_ms_ = timeout_ms;
    
    // Generate unique pipe name
    std::wostringstream oss;
    oss << L"ExplorerLens-PluginHost-" << GetCurrentProcessId() 
        << L"-" << std::hex << std::setw(16) << std::setfill(L'0')
        << GenerateCorrelationId();
    pipe_name_ = oss.str();
    
    // Spawn PluginHost process
    if (!SpawnPluginHostProcess()) {
        return false;
    }
    
    // Connect to pipe
    if (!ConnectToPipe()) {
        Terminate();
        return false;
    }
    
    is_alive_ = true;
    last_heartbeat_ = std::chrono::steady_clock::now();
    
    return true;
}

IPC::IPCErrorCode PluginHostClient::RequestThumbnail(const DecodeRequest& request,
                                                     DecodeResult& result) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_alive_) {
        return IPC::IPCErrorCode::PROCESS_TERMINATED;
    }
    
    uint64_t correlation_id = GenerateCorrelationId();
    
    // Convert file path from UTF-8 to wide for IPC
    std::wstring file_path_wide;
    if (request.file_path) {
        int len = MultiByteToWideChar(CP_UTF8, 0, request.file_path, -1, nullptr, 0);
        if (len > 0) {
            file_path_wide.resize(len - 1);
            MultiByteToWideChar(CP_UTF8, 0, request.file_path, -1, &file_path_wide[0], len);
        }
    }
    
    // Build request message
    IPC::ThumbnailRequest ipc_request;
    ipc_request.filePathLength = static_cast<uint32_t>(file_path_wide.length() + 1);
    ipc_request.targetWidth = request.target_width;
    ipc_request.targetHeight = request.target_height;
    ipc_request.flags = 0;
    ipc_request.timeoutMs = timeout_ms_;
    
    // Determine if we need shared memory (not yet implemented)
    ipc_request.useSharedMemory = 0;
    ipc_request.inlineDataSize = 0;
    
    // Build payload: request struct + file path
    std::vector<uint8_t> payload;
    payload.resize(sizeof(ipc_request) + ipc_request.filePathLength * sizeof(wchar_t));
    memcpy(payload.data(), &ipc_request, sizeof(ipc_request));
    memcpy(payload.data() + sizeof(ipc_request), file_path_wide.c_str(), 
           ipc_request.filePathLength * sizeof(wchar_t));
    
    // Send request
    IPC::MessageHeader header;
    header.messageType = static_cast<uint32_t>(IPC::MessageType::REQUEST_THUMBNAIL);
    header.correlationId = correlation_id;
    header.dataSize = static_cast<uint32_t>(payload.size());
    
    if (!SendMessage(header, payload.data(), header.dataSize)) {
        return IPC::IPCErrorCode::PIPE_WRITE_FAILED;
    }
    
    // Receive response
    IPC::MessageHeader response_header;
    std::vector<uint8_t> response_payload;
    
    if (!ReceiveMessage(response_header, response_payload, timeout_ms_)) {
        return IPC::IPCErrorCode::REQUEST_TIMEOUT;
    }
    
    // Validate correlation ID
    if (response_header.correlationId != correlation_id) {
        return IPC::IPCErrorCode::MALFORMED_MESSAGE;
    }
    
    // Handle error response
    if (response_header.messageType == static_cast<uint32_t>(IPC::MessageType::ERROR_RESPONSE)) {
        if (response_payload.size() >= sizeof(IPC::ErrorResponse)) {
            const auto* error = reinterpret_cast<const IPC::ErrorResponse*>(response_payload.data());
            return static_cast<IPC::IPCErrorCode>(error->errorCode);
        }
        return IPC::IPCErrorCode::PLUGIN_DECODE_FAILED;
    }
    
    // Parse thumbnail response
    if (response_payload.size() < sizeof(IPC::ThumbnailResponse)) {
        return IPC::IPCErrorCode::MALFORMED_MESSAGE;
    }
    
    const auto* ipc_response = reinterpret_cast<const IPC::ThumbnailResponse*>(response_payload.data());
    
    // Convert to DecodeResult
    result.error_code = static_cast<PluginErrorCode>(ipc_response->resultCode);
    result.width = ipc_response->width;
    result.height = ipc_response->height;
    
    // Convert bitmap data to HBITMAP
    if (ipc_response->bitmapDataSize > 0 && ipc_response->resultCode == static_cast<uint32_t>(IPC::IPCErrorCode::SUCCESS)) {
        const uint8_t* pixel_data = response_payload.data() + sizeof(IPC::ThumbnailResponse);
        
        // Create BITMAPINFO structure
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = static_cast<LONG>(ipc_response->width);
        bmi.bmiHeader.biHeight = -static_cast<LONG>(ipc_response->height);  // Negative for top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;  // BGRA32
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = ipc_response->bitmapDataSize;
        
        // Allocate result pixel buffer
        result.pixels = static_cast<uint8_t*>(malloc(ipc_response->bitmapDataSize));
        result.buffer_size = ipc_response->bitmapDataSize;
        result.stride = ipc_response->stride;
        result.width = ipc_response->width;
        result.height = ipc_response->height;
        result.pixel_format = static_cast<PixelFormat>(ipc_response->pixelFormat);
        result.error_code = PLUGIN_SUCCESS;
        result.error_message = nullptr;
        result.metadata = nullptr;
        
        if (result.pixels) {
            // Copy pixel data
            memcpy(result.pixels, pixel_data, ipc_response->bitmapDataSize);
        } else {
            result.error_code = PLUGIN_ERROR_OUT_OF_MEMORY;
        }
    } else {
        result.pixels = nullptr;
        result.error_code = PLUGIN_ERROR_DECODE_ERROR;
    }
    
    return static_cast<IPC::IPCErrorCode>(ipc_response->resultCode);
}

bool PluginHostClient::CanDecode(const std::wstring& file_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_alive_) {
        return false;
    }
    
    uint64_t correlation_id = GenerateCorrelationId();
    
    // Build query message
    IPC::CanDecodeQuery query;
    query.filePathLength = static_cast<uint32_t>(file_path.length() + 1);
    query.hasHeader = 0;
    query.headerSize = 0;
    
    // Build payload
    std::vector<uint8_t> payload;
    payload.resize(sizeof(query) + query.filePathLength * sizeof(wchar_t));
    memcpy(payload.data(), &query, sizeof(query));
    memcpy(payload.data() + sizeof(query), file_path.c_str(),
           query.filePathLength * sizeof(wchar_t));
    
    // Send query
    IPC::MessageHeader header;
    header.messageType = static_cast<uint32_t>(IPC::MessageType::CAN_DECODE_QUERY);
    header.correlationId = correlation_id;
    header.dataSize = static_cast<uint32_t>(payload.size());
    
    if (!SendMessage(header, payload.data(), header.dataSize)) {
        return false;
    }
    
    // Receive response
    IPC::MessageHeader response_header;
    std::vector<uint8_t> response_payload;
    
    if (!ReceiveMessage(response_header, response_payload, timeout_ms_)) {
        return false;
    }
    
    // Parse response
    if (response_payload.size() < sizeof(IPC::CanDecodeResponse)) {
        return false;
    }
    
    const auto* response = reinterpret_cast<const IPC::CanDecodeResponse*>(response_payload.data());
    return response->canDecode != 0;
}

bool PluginHostClient::CanDecode(const uint8_t* header_data, size_t header_size) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_alive_) {
        return false;
    }
    
    if (!header_data || header_size == 0) {
        return false;
    }
    
    uint64_t correlation_id = GenerateCorrelationId();
    
    // Build CanDecodeQuery with header data
    IPC::CanDecodeQuery query;
    query.filePathLength = 0;  // No file path when using header data
    query.hasHeader = 1;
    query.headerSize = static_cast<uint32_t>(header_size);
    
    // Build payload: query structure + header data
    std::vector<uint8_t> payload;
    payload.resize(sizeof(query) + header_size);
    memcpy(payload.data(), &query, sizeof(query));
    memcpy(payload.data() + sizeof(query), header_data, header_size);
    
    // Send request
    IPC::MessageHeader header;
    header.messageType = static_cast<uint32_t>(IPC::MessageType::CAN_DECODE_QUERY);
    header.correlationId = correlation_id;
    header.dataSize = static_cast<uint32_t>(payload.size());
    
    if (!SendMessage(header, payload.data(), header.dataSize)) {
        return false;
    }
    
    // Receive response
    IPC::MessageHeader response_header;
    std::vector<uint8_t> response_payload;
    
    if (!ReceiveMessage(response_header, response_payload, timeout_ms_)) {
        return false;
    }
    
    // Parse response
    if (response_payload.size() < sizeof(IPC::CanDecodeResponse)) {
        return false;
    }
    
    const auto* response = reinterpret_cast<const IPC::CanDecodeResponse*>(response_payload.data());
    return response->canDecode != 0;
}

bool PluginHostClient::GetPluginInfo(PluginInfo& info) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_alive_) {
        return false;
    }
    
    uint64_t correlation_id = GenerateCorrelationId();
    
    // Send request
    IPC::MessageHeader header;
    header.messageType = static_cast<uint32_t>(IPC::MessageType::GET_PLUGIN_INFO);
    header.correlationId = correlation_id;
    header.dataSize = 0;
    
    if (!SendMessage(header, nullptr, 0)) {
        return false;
    }
    
    // Receive response
    IPC::MessageHeader response_header;
    std::vector<uint8_t> response_payload;
    
    if (!ReceiveMessage(response_header, response_payload, timeout_ms_)) {
        return false;
    }
    
    // Parse response
    if (response_payload.size() < sizeof(IPC::PluginInfoResponse)) {
        return false;
    }
    
    const auto* response = reinterpret_cast<const IPC::PluginInfoResponse*>(response_payload.data());
    
    // Copy to PluginInfo (SDK structure uses plugin_name, plugin_version, etc.)
    info.api_version = response->apiVersion;
    info.plugin_name = nullptr;       // Would need to allocate storage
    info.plugin_version = nullptr;    // Would need to allocate storage  
    info.plugin_author = nullptr;     // Would need to allocate storage
    info.plugin_description = nullptr; // Would need to allocate storage
    info.plugin_license = nullptr;    // Would need to allocate storage
    info.supported_extensions = nullptr;
    info.mime_types = nullptr;
    info.capabilities = response->capabilities;
    info.max_threads = 0;
    info.requires_gpu = false;
    info.supports_background_loading = true;
    
    // Note: This function has design issues - PluginInfo has const char* fields
    // that expect static string constants, not dynamically allocated strings.
    // A proper implementation would need to store these in a persistent buffer.
    
    return true;
}

bool PluginHostClient::IsAlive() const {
    if (!is_alive_) {
        return false;
    }
    
    if (!process_handle_) {
        return false;
    }
    
    DWORD exit_code = 0;
    if (GetExitCodeProcess(process_handle_, &exit_code)) {
        return exit_code == STILL_ACTIVE;
    }
    
    return false;
}

bool PluginHostClient::HasCrashed() const {
    if (!process_handle_) {
        return false;
    }
    
    DWORD exit_code = 0;
    if (GetExitCodeProcess(process_handle_, &exit_code)) {
        if (exit_code != STILL_ACTIVE && exit_code != 0) {
            // Check for crash exit codes
            return exit_code == STATUS_ACCESS_VIOLATION ||
                   exit_code == STATUS_STACK_OVERFLOW ||
                   exit_code == STATUS_INTEGER_DIVIDE_BY_ZERO ||
                   exit_code == STATUS_ILLEGAL_INSTRUCTION;
        }
    }
    
    return false;
}

DWORD PluginHostClient::GetExitCode() const {
    if (!process_handle_) {
        return 0;
    }
    
    DWORD exit_code = 0;
    GetExitCodeProcess(process_handle_, &exit_code);
    return exit_code;
}

void PluginHostClient::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!is_alive_) {
        return;
    }
    
    // Send shutdown message
    IPC::MessageHeader header;
    header.messageType = static_cast<uint32_t>(IPC::MessageType::SHUTDOWN);
    header.correlationId = GenerateCorrelationId();
    header.dataSize = 0;
    
    SendMessage(header, nullptr, 0);
    
    // Wait for process to exit (max 5 seconds)
    if (process_handle_) {
        WaitForSingleObject(process_handle_, 5000);
    }
    
    is_alive_ = false;
    CloseHandles();
}

void PluginHostClient::Terminate() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (process_handle_) {
        TerminateProcess(process_handle_, 1);
    }
    
    is_alive_ = false;
    CloseHandles();
}

//============================================================================
// Private Methods
//============================================================================

bool PluginHostClient::SpawnPluginHostProcess() {
    // Get PluginHost.exe path (same directory as current executable)
    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(nullptr, exe_path, MAX_PATH);
    
    std::wstring exe_dir = exe_path;
    size_t last_slash = exe_dir.find_last_of(L"\\/");
    if (last_slash != std::wstring::npos) {
        exe_dir = exe_dir.substr(0, last_slash + 1);
    }
    
    std::wstring plugin_host_path = exe_dir + L"PluginHost.exe";
    
    // Build command line
    std::wostringstream cmd_line;
    cmd_line << L"\"" << plugin_host_path << L"\" "
             << L"--plugin \"" << plugin_path_ << L"\" "
             << L"--pipe \"" << pipe_name_ << L"\" "
             << L"--timeout " << timeout_ms_;
    
    std::wstring cmd_line_str = cmd_line.str();
    
    // Create process
    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;  // Hidden console
    
    PROCESS_INFORMATION pi = {};
    
    if (!CreateProcessW(
            plugin_host_path.c_str(),
            &cmd_line_str[0],
            nullptr,
            nullptr,
            FALSE,
            CREATE_NEW_CONSOLE | CREATE_SUSPENDED,
            nullptr,
            nullptr,
            &si,
            &pi)) {
        return false;
    }
    
    process_handle_ = pi.hProcess;
    CloseHandle(pi.hThread);
    
    // Create job object and assign process
    job_object_ = std::make_unique<Security::JobObjectManager>();
    Security::JobObjectLimits limits;
    
    if (!job_object_->Create(L"ExplorerLens-Plugin-" + pipe_name_, limits)) {
        TerminateProcess(process_handle_, 1);
        CloseHandle(process_handle_);
        process_handle_ = nullptr;
        return false;
    }
    
    if (!job_object_->AssignProcess(process_handle_)) {
        TerminateProcess(process_handle_, 1);
        CloseHandle(process_handle_);
        process_handle_ = nullptr;
        job_object_.reset();
        return false;
    }
    
    // Resume process
    ResumeThread(pi.hThread);
    
    // Wait a bit for PluginHost to initialize (500ms)
    Sleep(500);
    
    return true;
}

bool PluginHostClient::ConnectToPipe() {
    std::wstring full_pipe_name = L"\\\\.\\pipe\\" + pipe_name_;
    
    // Try to connect (with retries)
    for (int i = 0; i < 10; ++i) {
        pipe_handle_ = CreateFileW(
            full_pipe_name.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr);
        
        if (pipe_handle_ != INVALID_HANDLE_VALUE) {
            // Set pipe to message mode
            DWORD mode = PIPE_READMODE_MESSAGE;
            SetNamedPipeHandleState(pipe_handle_, &mode, nullptr, nullptr);
            return true;
        }
        
        Sleep(100);  // Wait 100ms before retry
    }
    
    return false;
}

void PluginHostClient::CloseHandles() {
    if (pipe_handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(pipe_handle_);
        pipe_handle_ = INVALID_HANDLE_VALUE;
    }
    
    if (process_handle_) {
        CloseHandle(process_handle_);
        process_handle_ = nullptr;
    }
    
    job_object_.reset();
}

bool PluginHostClient::SendMessage(const IPC::MessageHeader& header,
                                   const void* payload,
                                   uint32_t size) {
    if (pipe_handle_ == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    // Write header
    DWORD bytes_written = 0;
    if (!WriteFile(pipe_handle_, &header, sizeof(header), &bytes_written, nullptr)) {
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

bool PluginHostClient::ReceiveMessage(IPC::MessageHeader& header,
                                      std::vector<uint8_t>& payload,
                                      uint32_t timeout_ms) {
    if (pipe_handle_ == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    // Set timeout
    DWORD timeout = timeout_ms;
    if (!SetNamedPipeHandleState(pipe_handle_, nullptr, nullptr, &timeout)) {
        // Non-fatal, continue anyway
    }
    
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
        return false;
    }
    
    // Read payload if present
    if (header.dataSize > 0) {
        payload.resize(header.dataSize);
        if (!ReadFile(pipe_handle_, payload.data(), header.dataSize, &bytes_read, nullptr)) {
            return false;
        }
        
        if (bytes_read != header.dataSize) {
            return false;
        }
    }
    
    return true;
}

uint64_t PluginHostClient::GenerateCorrelationId() {
    return next_correlation_id_.fetch_add(1, std::memory_order_relaxed);
}

void PluginHostClient::SendHeartbeat() {
    IPC::HeartbeatMessage ping;
    ping.timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    ping.processId = GetCurrentProcessId();
    
    IPC::MessageHeader header;
    header.messageType = static_cast<uint32_t>(IPC::MessageType::HEARTBEAT_PING);
    header.correlationId = GenerateCorrelationId();
    header.dataSize = sizeof(ping);
    
    SendMessage(header, &ping, sizeof(ping));
    last_heartbeat_ = std::chrono::steady_clock::now();
}

bool PluginHostClient::CheckHeartbeat() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_heartbeat_).count();
    
    return elapsed < IPC::HEARTBEAT_TIMEOUT;
}

} // namespace ExplorerLens

