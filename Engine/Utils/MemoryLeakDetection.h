// ============================================================================
// MemoryLeakDetection.h
// CRT Debug Heap for memory leak detection
// Automatically included in Debug builds
// ============================================================================

#pragma once

#ifdef _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#endif

#include <crtdbg.h>
#include <cstdlib>

// ============================================================================
// Automatic Memory Leak Detection on Exit
// ============================================================================

class MemoryLeakDetector {
public:
    MemoryLeakDetector() {
        // Enable leak checking on program exit
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        
        // Report leaks to Output window
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
        
        OutputDebugStringA("[ExplorerLens] Memory leak detection ENABLED\n");
    }
    
    ~MemoryLeakDetector() {
        // Report any leaks found
        if (_CrtDumpMemoryLeaks()) {
            OutputDebugStringA("[ExplorerLens] WARNING: Memory leaks detected!\n");
        } else {
            OutputDebugStringA("[ExplorerLens] No memory leaks detected\n");
        }
    }
    
    // Take a memory snapshot
    void Snapshot() {
        _CrtMemCheckpoint(&m_memState);
    }
    
    // Check for leaks since last snapshot
    bool CheckLeaksSinceSnapshot() {
        _CrtMemState currentState, diff;
        _CrtMemCheckpoint(&currentState);
        
        if (_CrtMemDifference(&diff, &m_memState, &currentState)) {
            _CrtMemDumpStatistics(&diff);
            return true;
        }
        return false;
    }
    
private:
    _CrtMemState m_memState;
};

// Global instance - automatically enabled in Debug builds
static MemoryLeakDetector g_memoryLeakDetector;

// ============================================================================
// RAII Handle Wrappers
// ============================================================================

// HBITMAP wrapper
class BitmapHandle {
public:
    BitmapHandle() : m_hBitmap(nullptr) {}
    explicit BitmapHandle(HBITMAP hBitmap) : m_hBitmap(hBitmap) {}
    
    ~BitmapHandle() {
        if (m_hBitmap) {
            DeleteObject(m_hBitmap);
        }
    }
    
    // No copy, move only
    BitmapHandle(const BitmapHandle&) = delete;
    BitmapHandle& operator=(const BitmapHandle&) = delete;
    
    BitmapHandle(BitmapHandle&& other) noexcept : m_hBitmap(other.m_hBitmap) {
        other.m_hBitmap = nullptr;
    }
    
    BitmapHandle& operator=(BitmapHandle&& other) noexcept {
        if (this != &other) {
            if (m_hBitmap) {
                DeleteObject(m_hBitmap);
            }
            m_hBitmap = other.m_hBitmap;
            other.m_hBitmap = nullptr;
        }
        return *this;
    }
    
    HBITMAP Get() const { return m_hBitmap; }
    HBITMAP* GetAddressOf() { return &m_hBitmap; }
    
    HBITMAP Release() {
        HBITMAP temp = m_hBitmap;
        m_hBitmap = nullptr;
        return temp;
    }
    
    void Reset(HBITMAP hBitmap = nullptr) {
        if (m_hBitmap) {
            DeleteObject(m_hBitmap);
        }
        m_hBitmap = hBitmap;
    }
    
    explicit operator bool() const { return m_hBitmap != nullptr; }
    
private:
    HBITMAP m_hBitmap;
};

// HICON wrapper
class IconHandle {
public:
    IconHandle() : m_hIcon(nullptr) {}
    explicit IconHandle(HICON hIcon) : m_hIcon(hIcon) {}
    
    ~IconHandle() {
        if (m_hIcon) {
            DestroyIcon(m_hIcon);
        }
    }
    
    // No copy, move only
    IconHandle(const IconHandle&) = delete;
    IconHandle& operator=(const IconHandle&) = delete;
    
    IconHandle(IconHandle&& other) noexcept : m_hIcon(other.m_hIcon) {
        other.m_hIcon = nullptr;
    }
    
    IconHandle& operator=(IconHandle&& other) noexcept {
        if (this != &other) {
            if (m_hIcon) {
                DestroyIcon(m_hIcon);
            }
            m_hIcon = other.m_hIcon;
            other.m_hIcon = nullptr;
        }
        return *this;
    }
    
    HICON Get() const { return m_hIcon; }
    HICON* GetAddressOf() { return &m_hIcon; }
    
    HICON Release() {
        HICON temp = m_hIcon;
        m_hIcon = nullptr;
        return temp;
    }
    
    void Reset(HICON hIcon = nullptr) {
        if (m_hIcon) {
            DestroyIcon(m_hIcon);
        }
        m_hIcon = hIcon;
    }
    
    explicit operator bool() const { return m_hIcon != nullptr; }
    
private:
    HICON m_hIcon;
};

// HANDLE wrapper
class HandleWrapper {
public:
    HandleWrapper() : m_hHandle(nullptr) {}
    explicit HandleWrapper(HANDLE hHandle) : m_hHandle(hHandle) {}
    
    ~HandleWrapper() {
        if (m_hHandle && m_hHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_hHandle);
        }
    }
    
    // No copy, move only
    HandleWrapper(const HandleWrapper&) = delete;
    HandleWrapper& operator=(const HandleWrapper&) = delete;
    
    HandleWrapper(HandleWrapper&& other) noexcept : m_hHandle(other.m_hHandle) {
        other.m_hHandle = nullptr;
    }
    
    HandleWrapper& operator=(HandleWrapper&& other) noexcept {
        if (this != &other) {
            if (m_hHandle && m_hHandle != INVALID_HANDLE_VALUE) {
                CloseHandle(m_hHandle);
            }
            m_hHandle = other.m_hHandle;
            other.m_hHandle = nullptr;
        }
        return *this;
    }
    
    HANDLE Get() const { return m_hHandle; }
    HANDLE* GetAddressOf() { return &m_hHandle; }
    
    HANDLE Release() {
        HANDLE temp = m_hHandle;
        m_hHandle = nullptr;
        return temp;
    }
    
    void Reset(HANDLE hHandle = nullptr) {
        if (m_hHandle && m_hHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_hHandle);
        }
        m_hHandle = hHandle;
    }
    
    explicit operator bool() const { 
        return m_hHandle != nullptr && m_hHandle != INVALID_HANDLE_VALUE; 
    }
    
private:
    HANDLE m_hHandle;
};

#else
// Release build - no-op stubs
class MemoryLeakDetector {
public:
    void Snapshot() {}
    bool CheckLeaksSinceSnapshot() { return false; }
};

// Define handle wrappers even in Release for consistent API
// (but without leak detection overhead)
#include "../Engine/Utils/HandleWrappers.h"

#endif // _DEBUG

