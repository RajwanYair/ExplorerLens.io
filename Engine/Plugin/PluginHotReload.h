// PluginHotReload.h — Plugin Hot-Reload System
// Copyright (c) 2026 ExplorerLens Project
//
// Monitor plugin DLLs for changes and reload them without restarting Explorer,
// with configurable policies and state tracking.
//
#pragma once
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class HotReloadTrigger : uint8_t {
    FileChanged,
    FileChange = FileChanged,
    Manual,
    Scheduled,
    EventBased,
    COUNT = 4
};

enum class HotReloadState : uint8_t {
    Idle,
    Watching,
    Loading,
    Ready,
    Failed,
    COUNT = 5
};

enum class HotReloadPolicy : uint8_t {
    Automatic,
    Manual,
    Supervised,
    Disabled,
    COUNT = 4
};

class PluginHotReload
{
  public:
    static const wchar_t* TriggerName(HotReloadTrigger t) noexcept
    {
        switch (t) {
            case HotReloadTrigger::FileChanged:
                return L"File Changed";
            case HotReloadTrigger::Manual:
                return L"Manual";
            case HotReloadTrigger::Scheduled:
                return L"Scheduled";
            case HotReloadTrigger::EventBased:
                return L"Event-Based";
            default:
                return L"Unknown";
        }
    }
    static const wchar_t* StateName(HotReloadState s) noexcept
    {
        switch (s) {
            case HotReloadState::Idle:
                return L"Idle";
            case HotReloadState::Watching:
                return L"Watching";
            case HotReloadState::Loading:
                return L"Loading";
            case HotReloadState::Ready:
                return L"Ready";
            case HotReloadState::Failed:
                return L"Failed";
            default:
                return L"Unknown";
        }
    }
    static const wchar_t* PolicyName(HotReloadPolicy p) noexcept
    {
        switch (p) {
            case HotReloadPolicy::Automatic:
                return L"Automatic";
            case HotReloadPolicy::Manual:
                return L"Manual";
            case HotReloadPolicy::Supervised:
                return L"Supervised";
            case HotReloadPolicy::Disabled:
                return L"Disabled";
            default:
                return L"Unknown";
        }
    }
    static size_t TriggerCount() noexcept
    {
        return static_cast<size_t>(HotReloadTrigger::COUNT);
    }
    static size_t StateCount() noexcept
    {
        return static_cast<size_t>(HotReloadState::COUNT);
    }
    static size_t PolicyCount() noexcept
    {
        return static_cast<size_t>(HotReloadPolicy::COUNT);
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
