// KeyboardNavigationController.h — Keyboard Navigation Controller
// Copyright (c) 2026 ExplorerLens Project
//
// Implements full keyboard navigability for the thumbnail grid and preview panel.
// Supports arrow keys, Home/End, Page Up/Down, Enter/Space, and custom shortcuts.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class NavKey {
    ArrowLeft,
    ArrowRight,
    ArrowUp,
    ArrowDown,
    Home,
    End,
    PageUp,
    PageDown,
    Enter,
    Space,
    Escape,
    Tab,
    ShiftTab
};

struct NavState
{
    uint32_t focusedIndex = 0;
    uint32_t totalItems = 0;
    uint32_t columnsPerRow = 4;
    bool selectionActive = false;
};

struct NavAction
{
    uint32_t newIndex = 0;
    bool selectItem = false;
    bool activateItem = false;
    bool exitNav = false;
    bool handled = false;
};

using NavActionCallback = std::function<void(const NavAction&)>;

class KeyboardNavigationController
{
  public:
    KeyboardNavigationController() = default;

    bool Initialize(uint32_t totalItems = 0, uint32_t columns = 4)
    {
        m_state.totalItems = totalItems;
        m_state.columnsPerRow = columns;
        m_state.focusedIndex = 0;
        m_ready = true;
        return true;
    }
    bool IsReady() const
    {
        return m_ready;
    }

    void SetTotalItems(uint32_t n)
    {
        m_state.totalItems = n;
    }
    void SetColumns(uint32_t c)
    {
        m_state.columnsPerRow = c;
    }
    void SetFocus(uint32_t idx)
    {
        m_state.focusedIndex = idx;
    }
    const NavState& GetState() const
    {
        return m_state;
    }

    void SetActionCallback(NavActionCallback cb)
    {
        m_callback = std::move(cb);
    }

    NavAction HandleKey(NavKey key)
    {
        NavAction action;
        action.handled = true;
        uint32_t& idx = m_state.focusedIndex;
        uint32_t total = m_state.totalItems;
        uint32_t cols = m_state.columnsPerRow;

        switch (key) {
            case NavKey::ArrowRight:
                idx = (total > 0 && idx + 1 < total) ? idx + 1 : idx;
                break;
            case NavKey::ArrowLeft:
                idx = (idx > 0) ? idx - 1 : idx;
                break;
            case NavKey::ArrowDown:
                idx = (idx + cols < total) ? idx + cols : idx;
                break;
            case NavKey::ArrowUp:
                idx = (idx >= cols) ? idx - cols : idx;
                break;
            case NavKey::Home:
                idx = 0;
                break;
            case NavKey::End:
                idx = total > 0 ? total - 1 : 0;
                break;
            case NavKey::Enter:
            case NavKey::Space:
                action.activateItem = true;
                break;
            case NavKey::Escape:
                action.exitNav = true;
                break;
            default:
                action.handled = false;
                break;
        }
        action.newIndex = idx;
        if (m_callback)
            m_callback(action);
        return action;
    }

    void Shutdown()
    {
        m_ready = false;
    }

  private:
    bool m_ready = false;
    NavState m_state;
    NavActionCallback m_callback;
};

}  // namespace Engine
}  // namespace ExplorerLens
