// ForegroundPriorityInheritance.h — Foreground-Window Priority Inheritance for Decode
// Copyright (c) 2026 ExplorerLens Project
//
// Inherits the foreground window's thread priority for decode tasks targeting the
// active Explorer window, ensuring snappy first-frame delivery without starving
// background decode queues.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class DecodeWindowContext {
    Foreground,
    Background,
    Minimized
};
enum class FPIDecodePriority {
    Idle = 0,
    Low = 1,
    Normal = 2,
    High = 3,
    Realtime = 4
};

struct PriorityDecision
{
    FPIDecodePriority priority = FPIDecodePriority::Normal;
    DecodeWindowContext context = DecodeWindowContext::Background;
    uint32_t windowHandle = 0;
    bool inherited = false;
};

class ForegroundPriorityInheritance
{
  public:
    static ForegroundPriorityInheritance& Instance()
    {
        static ForegroundPriorityInheritance inst;
        return inst;
    }

    void SetForegroundWindow(uint32_t hwnd) noexcept
    {
        m_foregroundHwnd = hwnd;
    }
    uint32_t GetForegroundWindow() const noexcept
    {
        return m_foregroundHwnd;
    }

    PriorityDecision Evaluate(uint32_t targetWnd) const noexcept
    {
        PriorityDecision d;
        d.windowHandle = targetWnd;
        if (targetWnd == 0) {
            d.context = DecodeWindowContext::Background;
            d.priority = FPIDecodePriority::Low;
            return d;
        }
        if (targetWnd == m_foregroundHwnd) {
            d.context = DecodeWindowContext::Foreground;
            d.priority = m_foregroundPriority;
            d.inherited = true;
        } else {
            d.context = DecodeWindowContext::Background;
            d.priority = m_backgroundPriority;
        }
        return d;
    }

    void SetForegroundPriority(FPIDecodePriority p) noexcept
    {
        m_foregroundPriority = p;
    }
    void SetBackgroundPriority(FPIDecodePriority p) noexcept
    {
        m_backgroundPriority = p;
    }
    FPIDecodePriority ForegroundPriority() const noexcept
    {
        return m_foregroundPriority;
    }
    FPIDecodePriority BackgroundPriority() const noexcept
    {
        return m_backgroundPriority;
    }

  private:
    ForegroundPriorityInheritance() = default;
    uint32_t m_foregroundHwnd = 0;
    FPIDecodePriority m_foregroundPriority = FPIDecodePriority::High;
    FPIDecodePriority m_backgroundPriority = FPIDecodePriority::Low;
};

}  // namespace Engine
}  // namespace ExplorerLens
