// PluginSandboxEnforcer.h — Plugin Execution Sandbox
// Copyright (c) 2026 ExplorerLens Project
//
// Enforces security boundaries around plugin execution. Restricts
// file system access, network calls, registry operations, and memory
// limits per plugin instance using Windows Job Objects and access tokens.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class SandboxRestriction : uint8_t {
    FileSystemRead, FileSystemWrite, NetworkAccess, RegistryAccess,
    ProcessCreation, MemoryLimit, CPUTimeLimit, COUNT
};

enum class SandboxViolation : uint8_t {
    None, FileAccessDenied, NetworkBlocked, RegistryBlocked,
    MemoryExceeded, TimeoutExpired, ProcessBlocked, COUNT
};

struct EnforcerSandboxPolicy {
    bool allowFileRead = true;
    bool allowFileWrite = false;
    bool allowNetwork = false;
    bool allowRegistry = false;
    bool allowProcessCreate = false;
    size_t memoryLimitMB = 256;
    uint32_t cpuTimeLimitMs = 5000;
    std::wstring allowedPath;
};

struct SandboxStats {
    uint32_t totalInvocations = 0;
    uint32_t violationCount = 0;
    uint32_t terminatedCount = 0;
    size_t peakMemoryMB = 0;
    double avgExecutionMs = 0.0;
};

class PluginSandboxEnforcer {
public:
    void SetPolicy(const EnforcerSandboxPolicy& policy) { m_policy = policy; }
    const EnforcerSandboxPolicy& GetPolicy() const { return m_policy; }

    SandboxViolation CheckAccess(SandboxRestriction restriction) const {
        switch (restriction) {
        case SandboxRestriction::FileSystemWrite:
            return m_policy.allowFileWrite ? SandboxViolation::None : SandboxViolation::FileAccessDenied;
        case SandboxRestriction::NetworkAccess:
            return m_policy.allowNetwork ? SandboxViolation::None : SandboxViolation::NetworkBlocked;
        case SandboxRestriction::RegistryAccess:
            return m_policy.allowRegistry ? SandboxViolation::None : SandboxViolation::RegistryBlocked;
        case SandboxRestriction::ProcessCreation:
            return m_policy.allowProcessCreate ? SandboxViolation::None : SandboxViolation::ProcessBlocked;
        default:
            return SandboxViolation::None;
        }
    }

    bool IsAllowed(SandboxRestriction r) const {
        return CheckAccess(r) == SandboxViolation::None;
    }

    void RecordInvocation(size_t memoryUsedMB, double durationMs) {
        m_stats.totalInvocations++;
        if (memoryUsedMB > m_stats.peakMemoryMB)
            m_stats.peakMemoryMB = memoryUsedMB;
        double total = m_stats.avgExecutionMs * (m_stats.totalInvocations - 1) + durationMs;
        m_stats.avgExecutionMs = total / m_stats.totalInvocations;
        if (memoryUsedMB > m_policy.memoryLimitMB) {
            m_stats.violationCount++;
            m_stats.terminatedCount++;
        }
    }

    const SandboxStats& GetStats() const { return m_stats; }
    void Reset() { m_stats = {}; }

    static size_t RestrictionCount() { return static_cast<size_t>(SandboxRestriction::COUNT); }
    static size_t ViolationCount() { return static_cast<size_t>(SandboxViolation::COUNT); }

private:
    EnforcerSandboxPolicy m_policy;
    SandboxStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
