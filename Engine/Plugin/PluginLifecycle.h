#pragma once
//=============================================================================
// PluginLifecycle.h — Unified Plugin Lifecycle Management Umbrella
//
// Consolidates plugin activation and runtime lifecycle components:
//   - PluginActivation.h   (Feature flags, discovery, IPC channel,
//                            lifecycle manager — register/activate/suspend/unload)
//   - PluginHotReload.h    (Hot-reload triggers, states, policies)
//
// Usage: #include "PluginLifecycle.h" for all lifecycle-related plugin APIs.
//
// Namespace note:
//   PluginActivation uses ExplorerLens::Engine::Plugin (3-level)
//   PluginHotReload uses ExplorerLens::Engine
//   Both coexist without collision.
//=============================================================================

// Plugin activation: feature flags, discovery, IPC, lifecycle manager
#include "PluginActivation.h"

// Hot-reload: triggers, states, policies
#include "PluginHotReloadManager.h"

namespace ExplorerLens {
namespace Engine {

/// Unified lifecycle phase encompassing both activation and hot-reload
enum class PluginLifecyclePhase : uint32_t {
    /// Plugin discovery and manifest parsing
    Discovery = 0,
    /// Feature flag evaluation
    FlagEvaluation = 1,
    /// IPC channel establishment
    IPCConnect = 2,
    /// Plugin activation (DLL load / process spawn)
    Activation = 3,
    /// Plugin running normally
    Running = 4,
    /// Hot-reload triggered (file change / user request)
    HotReloading = 5,
    /// Plugin suspended (low memory / policy)
    Suspended = 6,
    /// Plugin unloading / teardown
    Unloading = 7,
    /// Plugin fully unloaded
    Terminated = 8,
    /// Error state
    Faulted = 9,

    COUNT = 10
};

/// Returns human-readable name for a lifecycle phase
inline const wchar_t* LifecyclePhaseName(PluginLifecyclePhase phase)
{
    switch (phase) {
        case PluginLifecyclePhase::Discovery:
            return L"Discovery";
        case PluginLifecyclePhase::FlagEvaluation:
            return L"FlagEvaluation";
        case PluginLifecyclePhase::IPCConnect:
            return L"IPCConnect";
        case PluginLifecyclePhase::Activation:
            return L"Activation";
        case PluginLifecyclePhase::Running:
            return L"Running";
        case PluginLifecyclePhase::HotReloading:
            return L"HotReloading";
        case PluginLifecyclePhase::Suspended:
            return L"Suspended";
        case PluginLifecyclePhase::Unloading:
            return L"Unloading";
        case PluginLifecyclePhase::Terminated:
            return L"Terminated";
        case PluginLifecyclePhase::Faulted:
            return L"Faulted";
        default:
            return L"Unknown";
    }
}

/// Returns the total number of lifecycle phases
inline constexpr uint32_t LifecyclePhaseCount()
{
    return static_cast<uint32_t>(PluginLifecyclePhase::COUNT);
}

/// Check if a phase represents an active (loaded) plugin
inline bool IsActivePhase(PluginLifecyclePhase phase)
{
    return phase == PluginLifecyclePhase::Running || phase == PluginLifecyclePhase::HotReloading
           || phase == PluginLifecyclePhase::Suspended;
}

/// Check if a phase represents a terminal state
inline bool IsTerminalPhase(PluginLifecyclePhase phase)
{
    return phase == PluginLifecyclePhase::Terminated || phase == PluginLifecyclePhase::Faulted;
}

}  // namespace Engine
}  // namespace ExplorerLens
