/******************************************************************************
 * ExplorerLens Crash Handler Implementation
 * Copyright (c) 2026 - ExplorerLens Project
 *****************************************************************************/

#include "CrashHandler.h"
#include <mutex>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <ShlObj.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

namespace ExplorerLens {

//============================================================================
// CrashHandler Implementation
//============================================================================

CrashHandler::CrashHandler() {
    LoadDisabledPlugins();
}

CrashHandler::~CrashHandler() {
    SaveDisabledPlugins();
}

bool CrashHandler::DetectCrash(HANDLE process_handle,
                               const std::wstring& plugin_id,
                               const std::wstring& plugin_path,
                               uint64_t correlation_id,
                               const std::wstring& file_path) {
    if (!process_handle) {
        return false;
    }
    
    // Get exit code
    DWORD exit_code = 0;
    if (!GetExitCodeProcess(process_handle, &exit_code)) {
        return false;
    }
    
    // Check if process is still running
    if (exit_code == STILL_ACTIVE) {
        return false;
    }
    
    // Check if it's a crash exit code
    if (!IsCrashExitCode(exit_code)) {
        return false;  // Normal exit or non-crash error
    }
    
    // Build crash info
    CrashInfo crash_info;
    crash_info.plugin_id = plugin_id;
    crash_info.plugin_path = plugin_path;
    crash_info.exit_code = exit_code;
    crash_info.correlation_id = correlation_id;
    crash_info.file_path = file_path;
    crash_info.timestamp = std::chrono::system_clock::now();
    
    // Record crash
    RecordCrash(crash_info);
    
    // Disable plugin
    DisablePlugin(plugin_id, crash_info);
    
    // Notify
    NotifyCrash(crash_info);
    
    return true;
}

bool CrashHandler::IsPluginDisabled(const std::wstring& plugin_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return disabled_plugins_.find(plugin_id) != disabled_plugins_.end();
}

void CrashHandler::DisablePlugin(const std::wstring& plugin_id, const CrashInfo& crash_info) {
    std::lock_guard<std::mutex> lock(mutex_);
    disabled_plugins_[plugin_id] = crash_info;
    SaveDisabledPlugins();
}

void CrashHandler::EnablePlugin(const std::wstring& plugin_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    disabled_plugins_.erase(plugin_id);
    SaveDisabledPlugins();
}

uint32_t CrashHandler::GetCrashCount(const std::wstring& plugin_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = crash_counts_.find(plugin_id);
    return (it != crash_counts_.end()) ? it->second : 0;
}

bool CrashHandler::GetLastCrash(const std::wstring& plugin_id, CrashInfo& info) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = last_crashes_.find(plugin_id);
    if (it != last_crashes_.end()) {
        info = it->second;
        return true;
    }
    return false;
}

void CrashHandler::ClearHistory() {
    std::lock_guard<std::mutex> lock(mutex_);
    crash_counts_.clear();
    last_crashes_.clear();
    disabled_plugins_.clear();
    SaveDisabledPlugins();
}

void CrashHandler::LoadDisabledPlugins() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Load from registry: HKCU\Software\ExplorerLens\DisabledPlugins
    HKEY key;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\ExplorerLens\\DisabledPlugins",
                     0, KEY_READ, &key) != ERROR_SUCCESS) {
        return;
    }
    
    // Enumerate values
    wchar_t value_name[256];
    DWORD value_name_size = sizeof(value_name) / sizeof(wchar_t);
    DWORD index = 0;
    
    while (RegEnumValueW(key, index++, value_name, &value_name_size,
                        nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
        // Each value is a disabled plugin ID
        CrashInfo crash_info;
        crash_info.plugin_id = value_name;
        crash_info.exit_code = 0;  // Unknown (loaded from registry)
        crash_info.timestamp = std::chrono::system_clock::now();
        
        disabled_plugins_[crash_info.plugin_id] = crash_info;
        
        value_name_size = sizeof(value_name) / sizeof(wchar_t);
    }
    
    RegCloseKey(key);
}

void CrashHandler::SaveDisabledPlugins() {
    // Save to registry: HKCU\Software\ExplorerLens\DisabledPlugins
    HKEY key;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\ExplorerLens\\DisabledPlugins",
                       0, nullptr, 0, KEY_WRITE, nullptr, &key, nullptr) != ERROR_SUCCESS) {
        return;
    }
    
    // Clear existing values
    wchar_t value_name[256];
    DWORD value_name_size = sizeof(value_name) / sizeof(wchar_t);
    
    while (RegEnumValueW(key, 0, value_name, &value_name_size,
                        nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
        RegDeleteValueW(key, value_name);
        value_name_size = sizeof(value_name) / sizeof(wchar_t);
    }
    
    // Write disabled plugins
    for (const auto& pair : disabled_plugins_) {
        const std::wstring& plugin_id = pair.first;
        DWORD value = 1;
        RegSetValueExW(key, plugin_id.c_str(), 0, REG_DWORD,
                      reinterpret_cast<const BYTE*>(&value), sizeof(value));
    }
    
    RegCloseKey(key);
}

void CrashHandler::RecordCrash(const CrashInfo& crash_info) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Increment crash count
    crash_counts_[crash_info.plugin_id]++;
    
    // Update last crash
    last_crashes_[crash_info.plugin_id] = crash_info;
}

void CrashHandler::NotifyCrash(const CrashInfo& crash_info) {
    // Call callback if set
    if (crash_callback_) {
        crash_callback_(crash_info);
    }
    
    // Log to debug output
    std::wstring message = FormatCrashInfo(crash_info);
    OutputDebugStringW(message.c_str());
    
    // Log to file
    wchar_t local_app_data[MAX_PATH];
    if (SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, local_app_data) == S_OK) {
        std::wstring log_dir = std::wstring(local_app_data) + L"\\ExplorerLens";
        CreateDirectoryW(log_dir.c_str(), nullptr);
        
        std::wstring log_file = log_dir + L"\\crash-log.txt";
        std::wofstream ofs(log_file, std::ios::app);
        if (ofs.is_open()) {
            ofs << message << L"\n";
            ofs.close();
        }
    }
}

//============================================================================
// Helper Functions
//============================================================================

const wchar_t* GetExitCodeName(DWORD exit_code) {
    switch (exit_code) {
        case STATUS_ACCESS_VIOLATION: return L"Access Violation (0xC0000005)";
        case STATUS_STACK_OVERFLOW: return L"Stack Overflow (0xC00000FD)";
        case STATUS_INTEGER_DIVIDE_BY_ZERO: return L"Divide by Zero (0xC0000094)";
        case STATUS_ILLEGAL_INSTRUCTION: return L"Illegal Instruction (0xC000001D)";
        case STATUS_FLOAT_DIVIDE_BY_ZERO: return L"Float Divide by Zero (0xC000008E)";
        case STATUS_ARRAY_BOUNDS_EXCEEDED: return L"Array Bounds Exceeded (0xC000008C)";
        case STATUS_DATATYPE_MISALIGNMENT: return L"Datatype Misalignment (0x80000002)";
        default: {
            static wchar_t buffer[64];
            swprintf_s(buffer, L"Unknown (0x%08X)", exit_code);
            return buffer;
        }
    }
}

bool IsCrashExitCode(DWORD exit_code) {
    return exit_code == STATUS_ACCESS_VIOLATION ||
           exit_code == STATUS_STACK_OVERFLOW ||
           exit_code == STATUS_INTEGER_DIVIDE_BY_ZERO ||
           exit_code == STATUS_ILLEGAL_INSTRUCTION ||
           exit_code == STATUS_FLOAT_DIVIDE_BY_ZERO ||
           exit_code == STATUS_ARRAY_BOUNDS_EXCEEDED ||
           exit_code == STATUS_DATATYPE_MISALIGNMENT;
}

std::wstring FormatCrashInfo(const CrashInfo& crash_info) {
    std::wostringstream oss;
    
    auto time_t = std::chrono::system_clock::to_time_t(crash_info.timestamp);
    wchar_t time_buffer[64];
    struct tm time_info;
    localtime_s(&time_info, &time_t);
    wcsftime(time_buffer, sizeof(time_buffer) / sizeof(wchar_t),
             L"%Y-%m-%d %H:%M:%S", &time_info);
    
    oss << L"[" << time_buffer << L"] Plugin Crash Detected\n";
    oss << L"  Plugin ID: " << crash_info.plugin_id << L"\n";
    oss << L"  Plugin Path: " << crash_info.plugin_path << L"\n";
    oss << L"  Exit Code: " << GetExitCodeName(crash_info.exit_code) << L"\n";
    oss << L"  Crash Type: " << crash_info.GetCrashTypeName() << L"\n";
    
    if (crash_info.correlation_id != 0) {
        oss << L"  Correlation ID: 0x" << std::hex << std::setw(16) << std::setfill(L'0')
            << crash_info.correlation_id << std::dec << L"\n";
    }
    
    if (!crash_info.file_path.empty()) {
        oss << L"  File: " << crash_info.file_path << L"\n";
    }
    
    oss << L"  Plugin has been automatically disabled.\n";
    
    return oss.str();
}

} // namespace ExplorerLens

