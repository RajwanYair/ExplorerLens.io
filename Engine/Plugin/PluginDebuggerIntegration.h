//==============================================================================
// ExplorerLens Engine — Plugin Debugger Integration
// VS Code / Visual Studio debugger attach, log streaming, breakpoint-in-
// plugin support, and crash minidump with plugin call stack symbolization.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PluginDebugMode : uint8_t {
 Disabled = 0,
 LogOnly, // Capture logs, no debugger
 AttachOnFault, // Attach debugger on first fault
 AlwaysAttach, // Launch with debugger attached
 Attach = AlwaysAttach, // compat alias
 COUNT = AlwaysAttach + 1
};

enum class PluginLogLevel : uint8_t {
 Trace = 0,
 Debug,
 Info,
 Warning,
 Error,
 Fatal,
 COUNT
};

enum class PluginDebugEvent : uint8_t {
 Load = 0,
 Decode,
 MemAlloc,
 Timeout,
 Fault,
 Unload,
 COUNT
};

struct PluginDebugSession {
 std::wstring pluginId;
 PluginDebugMode mode = PluginDebugMode::LogOnly;
 uint32_t processId = 0;
 bool attached = false;
};

struct PluginDebugLogEntry {
 PluginLogLevel level = PluginLogLevel::Info;
 PluginDebugEvent event = PluginDebugEvent::Decode;
 std::wstring message;
 uint64_t timestampMs = 0;
};

class PluginDebuggerIntegration {
public:
 static const wchar_t *DebugModeName(PluginDebugMode m) {
 switch (m) {
 case PluginDebugMode::Disabled:
 return L"Disabled";
 case PluginDebugMode::LogOnly:
 return L"Log Only";
 case PluginDebugMode::AttachOnFault:
 return L"Attach On Fault";
 case PluginDebugMode::AlwaysAttach:
 return L"Always Attach";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *LogLevelName(PluginLogLevel l) {
 switch (l) {
 case PluginLogLevel::Trace:
 return L"TRACE";
 case PluginLogLevel::Debug:
 return L"DEBUG";
 case PluginLogLevel::Info:
 return L"INFO";
 case PluginLogLevel::Warning:
 return L"WARN";
 case PluginLogLevel::Error:
 return L"ERROR";
 case PluginLogLevel::Fatal:
 return L"FATAL";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *DebugEventName(PluginDebugEvent e) {
 switch (e) {
 case PluginDebugEvent::Load:
 return L"Load";
 case PluginDebugEvent::Decode:
 return L"Decode";
 case PluginDebugEvent::MemAlloc:
 return L"MemAlloc";
 case PluginDebugEvent::Timeout:
 return L"Timeout";
 case PluginDebugEvent::Fault:
 return L"Fault";
 case PluginDebugEvent::Unload:
 return L"Unload";
 default:
 return L"Unknown";
 }
 }

 static constexpr size_t DebugModeCount() {
 return static_cast<size_t>(PluginDebugMode::COUNT);
 }
 static constexpr size_t LogLevelCount() {
 return static_cast<size_t>(PluginLogLevel::COUNT);
 }
 static constexpr size_t DebugEventCount() {
 return static_cast<size_t>(PluginDebugEvent::COUNT);
 }

 // Compatibility aliases (tests)
 static const wchar_t *ModeName(PluginDebugMode m) { return DebugModeName(m); }
 static const wchar_t *EventName(PluginDebugEvent e) {
 return DebugEventName(e);
 }
 static constexpr size_t ModeCount() { return DebugModeCount(); }
};

} // namespace Engine
} // namespace ExplorerLens
