#pragma once
// MemorySafety.h — Consolidated Memory Safety Infrastructure
// Copyright (c) 2026 ExplorerLens Project
//
// Unified header for memory safety concerns:
// - AddressSanitizer and HeapGuard integration
// - Use-after-free detection, buffer overflow probing
// - ASAN build configuration and leak detection
// - Memory-mapped I/O for large files, safe buffer operations
// - CRT Debug Heap / RAII handle wrappers

#include <cstdint>
#include <string>
#include <vector>

// ─── MemorySafetyAuditV2 ───────────────────────────────────────────────────────

namespace ExplorerLens {
namespace Engine {

enum class MemSafetyViolation : uint8_t {
    UseAfterFree = 0,
    BufferOverflow,
    HeapCorruption,
    UninitialisedRead,
    DoubleFree,
    StackOverflow,
    NullDeref,
    COUNT
};
enum class MemSafetyTool : uint8_t {
    ASAN = 0,
    HeapGuard,
    PageHeap,
    DrMemory,
    WinHeap,
    AddressSanitizer = ASAN, // compat alias
    COUNT = WinHeap + 1
};
enum class MemSafetyScope : uint8_t {
    AllDecoders = 0,
    PluginsOnly,
    CoreOnly,
    FullEngine,
    COUNT
};

struct MemSafetyFinding {
    MemSafetyViolation violation = MemSafetyViolation::BufferOverflow;
    MemSafetyTool tool = MemSafetyTool::ASAN;
    std::wstring component;
    std::wstring description;
    bool fixed = false;
};

struct MemSafetyAuditReport {
    MemSafetyScope scope = MemSafetyScope::FullEngine;
    uint32_t findingsCount = 0;
    uint32_t fixedCount = 0;
    uint32_t openCount = 0;
    bool clean = false;
};

class MemorySafetyAuditV2 {
public:
    static const wchar_t* ViolationName(MemSafetyViolation v) {
        switch (v) {
        case MemSafetyViolation::UseAfterFree:
            return L"Use-After-Free";
        case MemSafetyViolation::BufferOverflow:
            return L"Buffer Overflow";
        case MemSafetyViolation::HeapCorruption:
            return L"Heap Corruption";
        case MemSafetyViolation::UninitialisedRead:
            return L"Uninitialised Read";
        case MemSafetyViolation::DoubleFree:
            return L"Double Free";
        case MemSafetyViolation::StackOverflow:
            return L"Stack Overflow";
        case MemSafetyViolation::NullDeref:
            return L"Null Dereference";
        default:
            return L"Unknown";
        }
    }
    static const wchar_t* ToolName(MemSafetyTool t) {
        switch (t) {
        case MemSafetyTool::ASAN:
            return L"AddressSanitizer";
        case MemSafetyTool::HeapGuard:
            return L"HeapGuard";
        case MemSafetyTool::PageHeap:
            return L"PageHeap";
        case MemSafetyTool::DrMemory:
            return L"Dr. Memory";
        case MemSafetyTool::WinHeap:
            return L"Windows Heap";
        default:
            return L"Unknown";
        }
    }
    static const wchar_t* ScopeName(MemSafetyScope s) {
        switch (s) {
        case MemSafetyScope::AllDecoders:
            return L"All Decoders";
        case MemSafetyScope::PluginsOnly:
            return L"Plugins Only";
        case MemSafetyScope::CoreOnly:
            return L"Core Only";
        case MemSafetyScope::FullEngine:
            return L"Full Engine";
        default:
            return L"Unknown";
        }
    }
    static constexpr size_t ViolationCount() {
        return static_cast<size_t>(MemSafetyViolation::COUNT);
    }
    static constexpr size_t ToolCount() {
        return static_cast<size_t>(MemSafetyTool::COUNT);
    }
    static constexpr size_t ScopeCount() {
        return static_cast<size_t>(MemSafetyScope::COUNT);
    }
};

} // namespace Engine
} // namespace ExplorerLens

// ─── MemoryLeakDetection ───────────────────────────────────────────────────────

#ifdef _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#endif

#include <crtdbg.h>
#include <cstdlib>

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
        }
        else {
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

#else
// Release build - no-op stubs
class MemoryLeakDetector {
public:
    void Snapshot() {}
    bool CheckLeaksSinceSnapshot() { return false; }
};

#endif // _DEBUG

// ─── Handle wrappers (available in all build configurations) ───────────────────

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

// ─── End ────────────────────────────────────────────────────────────────────────

#include "MemorySafetyIntegration.h"
