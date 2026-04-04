// PluginHostSupervisor.h — Out-of-Process Plugin Host Management
// Copyright (c) 2026 ExplorerLens Project
//
// Manages lifecycle of plugin host processes, including launch, monitoring,
// restart-on-crash, and graceful shutdown with configurable policies.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SupervisorHostState : uint8_t {
    NotStarted = 0,
    Starting = 1,
    Running = 2,
    Suspended = 3,
    Crashed = 4,
    Terminated = 5
};

struct HostProcessInfo
{
    DWORD processId = 0;
    HANDLE processHandle = nullptr;
    SupervisorHostState state = SupervisorHostState::NotStarted;
    std::wstring exePath;
    std::wstring arguments;
    uint32_t restartCount = 0;
    uint32_t maxRestarts = 3;
    uint64_t startTimeMs = 0;
    uint64_t uptimeMs = 0;
    int32_t exitCode = 0;
};

struct PluginHostPolicy
{
    uint32_t maxRestarts = 3;
    uint32_t restartDelayMs = 1000;
    uint32_t healthCheckIntervalMs = 5000;
    uint32_t gracefulShutdownMs = 3000;
    bool autoRestart = true;
    bool isolateMemory = true;
};

class PluginHostSupervisor
{
  public:
    static PluginHostSupervisor& Instance()
    {
        static PluginHostSupervisor s;
        return s;
    }

    void SetPolicy(const PluginHostPolicy& policy)
    {
        m_policy = policy;
    }
    const PluginHostPolicy& GetPolicy() const
    {
        return m_policy;
    }

    bool Launch(const std::wstring& exePath, const std::wstring& args = L"")
    {
        HostProcessInfo info{};
        info.exePath = exePath;
        info.arguments = args;
        info.state = SupervisorHostState::Starting;
        info.maxRestarts = m_policy.maxRestarts;

        STARTUPINFOW si{};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi{};
        std::wstring cmdLine = exePath + L" " + args;
        BOOL ok = ::CreateProcessW(nullptr, &cmdLine[0], nullptr, nullptr, FALSE,
                                   CREATE_NEW_PROCESS_GROUP | CREATE_SUSPENDED, nullptr, nullptr, &si, &pi);
        if (!ok) {
            info.state = SupervisorHostState::Terminated;
            m_hosts.push_back(info);
            return false;
        }
        info.processId = pi.dwProcessId;
        info.processHandle = pi.hProcess;
        info.startTimeMs = GetTickCount64();
        ::ResumeThread(pi.hThread);
        ::CloseHandle(pi.hThread);
        info.state = SupervisorHostState::Running;
        m_hosts.push_back(info);
        return true;
    }

    SupervisorHostState Monitor(size_t index)
    {
        if (index >= m_hosts.size())
            return SupervisorHostState::NotStarted;
        auto& host = m_hosts[index];
        if (host.processHandle == nullptr)
            return host.state;
        DWORD exitCode = 0;
        if (::GetExitCodeProcess(host.processHandle, &exitCode)) {
            if (exitCode == STILL_ACTIVE) {
                host.state = SupervisorHostState::Running;
                host.uptimeMs = GetTickCount64() - host.startTimeMs;
            } else {
                host.exitCode = static_cast<int32_t>(exitCode);
                host.state = (exitCode == 0) ? SupervisorHostState::Terminated : SupervisorHostState::Crashed;
                ::CloseHandle(host.processHandle);
                host.processHandle = nullptr;
            }
        }
        return host.state;
    }

    bool Restart(size_t index)
    {
        if (index >= m_hosts.size())
            return false;
        auto& host = m_hosts[index];
        if (host.restartCount >= host.maxRestarts)
            return false;
        Kill(index);
        ++host.restartCount;
        return Launch(host.exePath, host.arguments);
    }

    bool Kill(size_t index)
    {
        if (index >= m_hosts.size())
            return false;
        auto& host = m_hosts[index];
        if (host.processHandle) {
            ::TerminateProcess(host.processHandle, 1);
            ::WaitForSingleObject(host.processHandle, m_policy.gracefulShutdownMs);
            ::CloseHandle(host.processHandle);
            host.processHandle = nullptr;
        }
        host.state = SupervisorHostState::Terminated;
        return true;
    }

    size_t HostCount() const
    {
        return m_hosts.size();
    }

    const HostProcessInfo* GetHostInfo(size_t index) const
    {
        return (index < m_hosts.size()) ? &m_hosts[index] : nullptr;
    }

    uint32_t CountByState(SupervisorHostState state) const
    {
        uint32_t count = 0;
        for (const auto& h : m_hosts) {
            if (h.state == state)
                ++count;
        }
        return count;
    }

    bool Validate() const
    {
        if (m_policy.maxRestarts > 100)
            return false;
        if (m_policy.gracefulShutdownMs > 60000)
            return false;
        for (const auto& h : m_hosts) {
            if (h.restartCount > h.maxRestarts + 1)
                return false;
        }
        return true;
    }

  private:
    PluginHostSupervisor() = default;
    ~PluginHostSupervisor()
    {
        for (auto& h : m_hosts) {
            if (h.processHandle) {
                ::TerminateProcess(h.processHandle, 1);
                ::CloseHandle(h.processHandle);
            }
        }
    }
    PluginHostSupervisor(const PluginHostSupervisor&) = delete;
    PluginHostSupervisor& operator=(const PluginHostSupervisor&) = delete;

    PluginHostPolicy m_policy;
    std::vector<HostProcessInfo> m_hosts;
};

}  // namespace Engine
}  // namespace ExplorerLens
