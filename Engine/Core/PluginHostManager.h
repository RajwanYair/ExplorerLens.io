//==============================================================================
// PluginHostManager.h — Plugin Host Manager
// Out-of-process plugin execution with named pipe IPC.
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Plugin execution mode (in-process vs out-of-process)
enum class PluginHostMode : uint8_t {
 InProcess = 0,
 OutOfProcess = 1,
 Sandbox = 2,
 COUNT
};

/// Manages the out-of-process PluginHost for crash isolation.
class PluginHostManager {
public:
 enum class HostState {
 NotStarted,
 Starting,
 Running,
 Suspended,
 Crashed,
 Stopped,
 COUNT
 };

 enum class IPCMethod { NamedPipe, SharedMemory, COM, Socket, COUNT };

 enum class RestartPolicy { Never, OnCrash, Always, Exponential, COUNT };

 struct HostStatus {
 HostState state;
 uint32_t pid;
 uint32_t restartCount;
 uint64_t uptimeMs;
 size_t pluginsLoaded;
 IPCMethod ipcMethod;
 };

 static const wchar_t *StateName(HostState s) {
 switch (s) {
 case HostState::NotStarted:
 return L"NotStarted";
 case HostState::Starting:
 return L"Starting";
 case HostState::Running:
 return L"Running";
 case HostState::Suspended:
 return L"Suspended";
 case HostState::Crashed:
 return L"Crashed";
 case HostState::Stopped:
 return L"Stopped";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *IPCMethodName(IPCMethod m) {
 switch (m) {
 case IPCMethod::NamedPipe:
 return L"NamedPipe";
 case IPCMethod::SharedMemory:
 return L"SharedMemory";
 case IPCMethod::COM:
 return L"COM";
 case IPCMethod::Socket:
 return L"Socket";
 default:
 return L"Unknown";
 }
 }

 static const wchar_t *RestartPolicyName(RestartPolicy p) {
 switch (p) {
 case RestartPolicy::Never:
 return L"Never";
 case RestartPolicy::OnCrash:
 return L"OnCrash";
 case RestartPolicy::Always:
 return L"Always";
 case RestartPolicy::Exponential:
 return L"Exponential";
 default:
 return L"Unknown";
 }
 }

 static size_t StateCount() { return static_cast<size_t>(HostState::COUNT); }
 static size_t IPCMethodCount() {
 return static_cast<size_t>(IPCMethod::COUNT);
 }
 static size_t RestartPolicyCount() {
 return static_cast<size_t>(RestartPolicy::COUNT);
 }

 static HostStatus GetDefaultStatus() {
 return {HostState::NotStarted, 0, 0, 0, 0, IPCMethod::NamedPipe};
 }

 static bool IsHealthy(const HostStatus &s) {
 return s.state == HostState::Running && s.restartCount < 5;
 }

 /// Plugin execution mode queries
 static size_t ModeCount() {
 return static_cast<size_t>(PluginHostMode::COUNT);
 }

 static const wchar_t *ModeName(PluginHostMode m) {
 switch (m) {
 case PluginHostMode::InProcess:
 return L"In-Process";
 case PluginHostMode::OutOfProcess:
 return L"Out-of-Process";
 case PluginHostMode::Sandbox:
 return L"Sandbox";
 default:
 return L"Unknown";
 }
 }
};

} // namespace Engine
} // namespace ExplorerLens
