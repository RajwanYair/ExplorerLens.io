// PluginDebuggerIntegration.h — Plugin Debugger Integration
// Copyright (c) 2026 ExplorerLens Project
//
// Attach/detach debug sessions to plugins, route plugin log levels,
// and surface plugin lifecycle events for diagnostic instrumentation.
//
#pragma once
#include <cstddef>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class PluginDebugMode : uint8_t {
    Attach,
    Detach,
    StepThrough,
    Inspect,
    COUNT = 4
};

enum class PluginLogLevel : uint8_t {
    Debug,
    Info,
    Warning,
    Error,
    Fatal,
    COUNT = 5
};

enum class PluginDebugEvent : uint8_t {
    Load,
    Unload,
    Crash,
    Freeze,
    Timeout,
    COUNT = 5
};

class PluginDebuggerIntegration {
public:
    static const wchar_t *ModeName(PluginDebugMode m) noexcept {
        switch (m) {
        case PluginDebugMode::Attach:      return L"Attach";
        case PluginDebugMode::Detach:      return L"Detach";
        case PluginDebugMode::StepThrough: return L"Step-Through";
        case PluginDebugMode::Inspect:     return L"Inspect";
        default: return L"Unknown";
        }
    }
    static const wchar_t *LogLevelName(PluginLogLevel l) noexcept {
        switch (l) {
        case PluginLogLevel::Debug:   return L"Debug";
        case PluginLogLevel::Info:    return L"Info";
        case PluginLogLevel::Warning: return L"Warning";
        case PluginLogLevel::Error:   return L"Error";
        case PluginLogLevel::Fatal:   return L"Fatal";
        default: return L"Unknown";
        }
    }
    static const wchar_t *EventName(PluginDebugEvent e) noexcept {
        switch (e) {
        case PluginDebugEvent::Load:    return L"Load";
        case PluginDebugEvent::Unload:  return L"Unload";
        case PluginDebugEvent::Crash:   return L"Crash";
        case PluginDebugEvent::Freeze:  return L"Freeze";
        case PluginDebugEvent::Timeout: return L"Timeout";
        default: return L"Unknown";
        }
    }
    static size_t ModeCount() noexcept {
        return static_cast<size_t>(PluginDebugMode::COUNT);
    }
    static size_t LogLevelCount() noexcept {
        return static_cast<size_t>(PluginLogLevel::COUNT);
    }
};

} // namespace Engine
} // namespace ExplorerLens
