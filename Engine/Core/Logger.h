//==============================================================================
// DarkThumbs Engine - Simple Logging Utility
// Version: 1.0.0
// Copyright (c) 2026 - DarkThumbs Project
//
// Provides lightweight logging macros for debug and diagnostic output.
// All logs output to OutputDebugString for viewing in DebugView or VS debugger.
//
// THREAD SAFETY: All macros are thread-safe (OutputDebugStringW is atomic)
//
// PERFORMANCE:
//   - Negligible in Release builds (can be disabled with #define DISABLE_LOGGING)
//   - ~10-50μs per log call in Debug
//   - Logs are buffered by the OS, non-blocking
//
// BEST PRACTICES:
//   - Use LOG_ERROR for failures that users/devs should know about
//   - Use LOG_WARN for recoverable issues or deprecations
//   - Use LOG_INFO for important state changes or milestones
//   - Use LOG_DEBUG for verbose diagnostic info (disabled in Release)
//   - Use LOG_PERF for performance-critical timing measurements
//   - Use LOG_HRESULT for Windows API failures
//
// ERROR CODE DOCUMENTATION:
//   HRESULT values logged by LOG_HRESULT:
//   - 0x80004001 (E_NOTIMPL): Feature not  implemented
//   - 0x80004005 (E_FAIL): Generic failure
//   - 0x80070057 (E_INVALIDARG): Invalid parameter
//   - 0x8007000E (E_OUTOFMEMORY): Out of memory
//   - 0x80004002 (E_NOINTERFACE): Interface not supported
//   - 0x80004004 (E_ABORT): Operation aborted
//   - 0x80070002 (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)): File not found
//==============================================================================

#pragma once

#include <windows.h>
#include <stdio.h>
#include <time.h>

namespace DarkThumbs {
namespace Engine {

//==============================================================================
// Simple Debug Logging Macros
//==============================================================================

/// Internal helper to format timestamp
inline void _LogTimestamp(wchar_t* buffer, size_t size) {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    wcsftime(buffer, size, L"%H:%M:%S", &timeinfo);
}

/// Log informational message
#define LOG_INFO(fmt, ...)  do { \
    wchar_t _log_time[32]; \
    DarkThumbs::Engine::_LogTimestamp(_log_time, _countof(_log_time)); \
    wchar_t _log_buf[2048]; \
    swprintf_s(_log_buf, L"[%s][INFO] " fmt L"\n", _log_time, ##__VA_ARGS__); \
    OutputDebugStringW(_log_buf); \
} while(0)

/// Log error message
#define LOG_ERROR(fmt, ...) do { \
    wchar_t _log_time[32]; \
    DarkThumbs::Engine::_LogTimestamp(_log_time, _countof(_log_time)); \
    wchar_t _log_buf[2048]; \
    swprintf_s(_log_buf, L"[%s][ERROR] " fmt L"\n", _log_time, ##__VA_ARGS__); \
    OutputDebugStringW(_log_buf); \
} while(0)

/// Log debug message
#define LOG_DEBUG(fmt, ...) do { \
    wchar_t _log_time[32]; \
    DarkThumbs::Engine::_LogTimestamp(_log_time, _countof(_log_time)); \
    wchar_t _log_buf[2048]; \
    swprintf_s(_log_buf, L"[%s][DEBUG] " fmt L"\n", _log_time, ##__VA_ARGS__); \
    OutputDebugStringW(_log_buf); \
} while(0)

/// Log warning message
#define LOG_WARN(fmt, ...) do { \
    wchar_t _log_time[32]; \
    DarkThumbs::Engine::_LogTimestamp(_log_time, _countof(_log_time)); \
    wchar_t _log_buf[2048]; \
    swprintf_s(_log_buf, L"[%s][WARN] " fmt L"\n", _log_time, ##__VA_ARGS__); \
    OutputDebugStringW(_log_buf); \
} while(0)

//==============================================================================
// Advanced Logging Features
//==============================================================================

/// Log with HRESULT
#define LOG_HRESULT(hr, fmt, ...) do { \
    wchar_t _log_time[32]; \
    DarkThumbs::Engine::_LogTimestamp(_log_time, _countof(_log_time)); \
    wchar_t _log_buf[2048]; \
    swprintf_s(_log_buf, L"[%s][ERROR] " fmt L" (HRESULT=0x%08X)\n", \
                _log_time, ##__VA_ARGS__, (hr)); \
    OutputDebugStringW(_log_buf); \
} while(0)

/// Log performance timing
#define LOG_PERF(operation, time_ms) do { \
    wchar_t _log_time[32]; \
    DarkThumbs::Engine::_LogTimestamp(_log_time, _countof(_log_time)); \
    wchar_t _log_buf[2048]; \
    swprintf_s(_log_buf, L"[%s][PERF] %s took %llu ms\n", \
                _log_time, (operation), (time_ms)); \
    OutputDebugStringW(_log_buf); \
} while(0)

//==============================================================================
// Input Validation Helper Macros
//==============================================================================

/// Check pointer and return E_POINTER if null
#define CHECK_POINTER(ptr) do { \
    if (!(ptr)) { \
        LOG_ERROR(L"Null pointer: " L"" #ptr); \
        return E_POINTER; \
    } \
} while(0)

/// Check pointer and return NULL if null
#define CHECK_POINTER_NULL(ptr) do { \
    if (!(ptr)) { \
        LOG_ERROR(L"Null pointer: " L"" #ptr); \
        return NULL; \
    } \
} while(0)

/// Check HR and return on failure
#define CHECK_HR(hr) do { \
    HRESULT _hr = (hr); \
    if (FAILED(_hr)) { \
        LOG_HRESULT(_hr, L"Failed: " L"" #hr); \
        return _hr; \
    } \
} while(0)

/// Validate dimensions
#define CHECK_DIMENSIONS(w, h) do { \
    if ((w) < 1 || (w) > 65535 || (h) < 1 || (h) > 65535) { \
        LOG_ERROR(L"Invalid dimensions: %ux%u", (w), (h)); \
        return E_INVALIDARG; \
    } \
} while(0)

//==============================================================================
// Performance Profiling Macro
//==============================================================================
 
/// Auto-log function execution time
/// Usage: PROFILE_SCOPE(L"MyFunction");
#define PROFILE_SCOPE(name) \
    auto _profile_start = std::chrono::high_resolution_clock::now(); \
    struct _ProfileGuard { \
        const wchar_t* _name; \
        decltype(_profile_start) _start; \
        ~_ProfileGuard() { \
            auto _end = std::chrono::high_resolution_clock::now(); \
            auto _dur = std::chrono::duration_cast<std::chrono::milliseconds>(_end - _start).count(); \
            LOG_PERF(_name, _dur); \
        } \
    } _profile_guard{(name), _profile_start};

} // namespace Engine
} // namespace DarkThumbs
