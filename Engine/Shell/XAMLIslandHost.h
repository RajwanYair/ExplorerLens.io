// XAMLIslandHost.h — WinUI 3 XAML Island Hosting for Legacy HWND
// Copyright (c) 2026 ExplorerLens Project
//
// Hosts WinUI 3 XAML Islands within legacy Win32 HWND windows, managing
// the island lifecycle and resize behavior. Conditional feature.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

enum class XAMLIslandState : uint32_t {
    Uninitialized = 0,
    Initializing = 1,
    Ready = 2,
    Active = 3,
    Suspended = 4,
    Error = 5,
    Destroyed = 6
};

struct IslandConfig {
    HWND     parentHwnd = nullptr;
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t width = 800;
    uint32_t height = 600;
    bool     enableInput = true;
    bool     transparentBg = false;
    double   scaleFactor = 1.0;
    std::wstring xamlTypeName;
};

struct XAMLIslandInfo {
    uint64_t        islandId = 0;
    XAMLIslandState state = XAMLIslandState::Uninitialized;
    IslandConfig    config;
    HWND            islandHwnd = nullptr;
    uint64_t        createTimeMs = 0;
    uint64_t        lastResizeMs = 0;
};

class XAMLIslandHost {
public:
    static XAMLIslandHost& Instance() {
        static XAMLIslandHost s;
        return s;
    }

    bool Initialize() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_initialized) return true;

        // Check if WinUI 3 runtime is available
        HMODULE winui = LoadLibraryW(L"Microsoft.ui.xaml.dll");
        if (winui) {
            FreeLibrary(winui);
            m_winuiAvailable = true;
        }

        m_initialized = true;
        return true;
    }

    uint64_t CreateIsland(const IslandConfig& config) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_initialized) return 0;

        XAMLIslandInfo info;
        info.islandId = m_nextIslandId++;
        info.config = config;
        info.state = XAMLIslandState::Initializing;
        info.createTimeMs = GetTickCount64();

        if (config.parentHwnd && IsWindow(config.parentHwnd)) {
            // Create a child window to host the island
            HINSTANCE hInst = reinterpret_cast<HINSTANCE>(
                GetWindowLongPtrW(config.parentHwnd, GWLP_HINSTANCE));

            info.islandHwnd = CreateWindowExW(
                WS_EX_NOREDIRECTIONBITMAP,
                L"Static", L"",
                WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
                config.x, config.y, config.width, config.height,
                config.parentHwnd, nullptr, hInst, nullptr);

            if (info.islandHwnd) {
                info.state = m_winuiAvailable ? XAMLIslandState::Ready : XAMLIslandState::Active;
            }
            else {
                info.state = XAMLIslandState::Error;
            }
        }
        else {
            info.state = XAMLIslandState::Error;
        }

        m_islands.push_back(info);
        return info.islandId;
    }

    bool Resize(uint64_t islandId, uint32_t width, uint32_t height) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = FindIsland(islandId);
        if (it == m_islands.end()) return false;
        if (it->state == XAMLIslandState::Destroyed) return false;

        it->config.width = width;
        it->config.height = height;
        it->lastResizeMs = GetTickCount64();

        if (it->islandHwnd && IsWindow(it->islandHwnd)) {
            return SetWindowPos(it->islandHwnd, nullptr,
                it->config.x, it->config.y, width, height,
                SWP_NOZORDER | SWP_NOACTIVATE) != 0;
        }
        return false;
    }

    bool Destroy(uint64_t islandId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = FindIsland(islandId);
        if (it == m_islands.end()) return false;

        if (it->islandHwnd && IsWindow(it->islandHwnd)) {
            DestroyWindow(it->islandHwnd);
        }

        it->islandHwnd = nullptr;
        it->state = XAMLIslandState::Destroyed;
        return true;
    }

    XAMLIslandState GetState(uint64_t islandId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = FindIslandConst(islandId);
        return it != m_islands.end() ? it->state : XAMLIslandState::Uninitialized;
    }

    bool IsWinUIAvailable() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_winuiAvailable;
    }

    size_t GetActiveIslandCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t count = 0;
        for (const auto& info : m_islands) {
            if (info.state != XAMLIslandState::Destroyed &&
                info.state != XAMLIslandState::Error)
                count++;
        }
        return count;
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& info : m_islands) {
            if (info.islandHwnd && IsWindow(info.islandHwnd)) {
                DestroyWindow(info.islandHwnd);
            }
        }
        m_islands.clear();
        m_nextIslandId = 1;
    }

    bool Validate() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_initialized) return true; // Not yet initialized is valid
        for (const auto& info : m_islands) {
            if (info.islandId == 0) return false;
            if (info.config.width == 0 || info.config.height == 0) return false;
            if (info.config.scaleFactor <= 0.0) return false;
        }
        return true;
    }

private:
    XAMLIslandHost() = default;
    ~XAMLIslandHost() { Reset(); }
    XAMLIslandHost(const XAMLIslandHost&) = delete;
    XAMLIslandHost& operator=(const XAMLIslandHost&) = delete;

    std::vector<XAMLIslandInfo>::iterator FindIsland(uint64_t id) {
        for (auto it = m_islands.begin(); it != m_islands.end(); ++it)
            if (it->islandId == id) return it;
        return m_islands.end();
    }

    std::vector<XAMLIslandInfo>::const_iterator FindIslandConst(uint64_t id) const {
        for (auto it = m_islands.begin(); it != m_islands.end(); ++it)
            if (it->islandId == id) return it;
        return m_islands.end();
    }

    mutable std::mutex m_mutex;
    std::vector<XAMLIslandInfo> m_islands;
    uint64_t m_nextIslandId = 1;
    bool m_initialized = false;
    bool m_winuiAvailable = false;
};

}
} // namespace ExplorerLens::Engine
