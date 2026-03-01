#pragma once
// ============================================================================
// DecoderSandboxPolicy.h — Security sandbox configuration for format decoders
//
// Purpose:   Security sandbox configuration for format decoders
// Provides:  DecoderSandboxLevel, SandboxResourceLimit enums,
//            DecoderSandboxRule, DecoderSandboxViolation structs,
//            and DecoderSandboxPolicy class
// Used by:   Decoder execution context
// ============================================================================

#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// DecoderSandboxPolicy — Security sandbox policies for each decoder
// ============================================================================

enum class DecoderSandboxLevel {
    None,
    Basic,
    Standard,
    Strict,
    Paranoid
};

inline const char* DecoderSandboxLevelName(DecoderSandboxLevel value) {
    switch (value) {
    case DecoderSandboxLevel::None:     return "None";
    case DecoderSandboxLevel::Basic:    return "Basic";
    case DecoderSandboxLevel::Standard: return "Standard";
    case DecoderSandboxLevel::Strict:   return "Strict";
    case DecoderSandboxLevel::Paranoid: return "Paranoid";
    default:                     return "Unknown";
    }
}

enum class SandboxResourceLimit {
    Memory,
    CPU,
    Disk,
    Network,
    GPU
};

inline const char* SandboxResourceLimitName(SandboxResourceLimit value) {
    switch (value) {
    case SandboxResourceLimit::Memory:  return "Memory";
    case SandboxResourceLimit::CPU:     return "CPU";
    case SandboxResourceLimit::Disk:    return "Disk";
    case SandboxResourceLimit::Network: return "Network";
    case SandboxResourceLimit::GPU:     return "GPU";
    default:                     return "Unknown";
    }
}

struct DecoderSandboxRule {
    DecoderSandboxLevel level = DecoderSandboxLevel::Standard;
    uint32_t     memoryLimitMB = 256;
    uint32_t     cpuTimeMs = 5000;
    bool         allowNetwork = false;
    bool         allowGPU = true;
    bool         allowDiskWrite = false;
    uint32_t     maxThreads = 4;
    std::string  decoderName;

    bool IsRestricted() const {
        return level != DecoderSandboxLevel::None;
    }

    bool IsResourceAllowed(SandboxResourceLimit res) const {
        switch (res) {
        case SandboxResourceLimit::Network: return allowNetwork;
        case SandboxResourceLimit::GPU:     return allowGPU;
        case SandboxResourceLimit::Disk:    return !allowDiskWrite ? false : true;
        default:                     return true;
        }
    }
};

struct DecoderSandboxViolation {
    std::string  decoderName;
    SandboxResourceLimit resource = SandboxResourceLimit::Memory;
    uint64_t     requestedAmount = 0;
    uint64_t     limitAmount = 0;
    uint64_t     timestampMs = 0;
};

class DecoderSandboxPolicy {
public:
    static constexpr uint32_t MAX_MEMORY_MB = 512;
    static constexpr uint32_t MAX_CPU_TIME_MS = 30000;
    static constexpr uint32_t MAX_THREADS = 16;
    static constexpr uint32_t DEFAULT_MEMORY_MB = 256;

    DecoderSandboxPolicy() = default;
    ~DecoderSandboxPolicy() = default;

    DecoderSandboxRule GetPolicy(const std::string& decoderName) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_policies.find(decoderName);
        if (it != m_policies.end()) {
            return it->second;
        }
        return GetDefaultForDecoder(decoderName);
    }

    void SetPolicy(const std::string& decoderName, const DecoderSandboxRule& policy) {
        std::lock_guard<std::mutex> lock(m_mutex);
        DecoderSandboxRule clamped = policy;
        clamped.memoryLimitMB = (clamped.memoryLimitMB > MAX_MEMORY_MB) ? MAX_MEMORY_MB : clamped.memoryLimitMB;
        clamped.cpuTimeMs = (clamped.cpuTimeMs > MAX_CPU_TIME_MS) ? MAX_CPU_TIME_MS : clamped.cpuTimeMs;
        clamped.maxThreads = (clamped.maxThreads > MAX_THREADS) ? MAX_THREADS : clamped.maxThreads;
        clamped.decoderName = decoderName;
        m_policies[decoderName] = clamped;
    }

    DecoderSandboxRule GetDefaultForDecoder(const std::string& decoderName) const {
        DecoderSandboxRule policy;
        policy.decoderName = decoderName;

        // High-risk decoders get stricter defaults
        if (decoderName == "RAR" || decoderName == "7z" || decoderName == "CAB") {
            policy.level = DecoderSandboxLevel::Strict;
            policy.memoryLimitMB = 128;
            policy.allowNetwork = false;
            policy.allowDiskWrite = false;
        }
        else if (decoderName == "PDF" || decoderName == "SVG") {
            policy.level = DecoderSandboxLevel::Strict;
            policy.memoryLimitMB = 192;
            policy.allowNetwork = false;
        }
        else if (decoderName == "glTF" || decoderName == "FBX") {
            policy.level = DecoderSandboxLevel::Standard;
            policy.memoryLimitMB = 384;
            policy.allowGPU = true;
        }
        else {
            policy.level = DecoderSandboxLevel::Basic;
            policy.memoryLimitMB = DEFAULT_MEMORY_MB;
        }
        return policy;
    }

    size_t GetPolicyCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_policies.size();
    }

    void RecordViolation(const DecoderSandboxViolation& violation) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_violations.push_back(violation);
    }

    size_t GetViolationCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_violations.size();
    }

    void ClearViolations() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_violations.clear();
    }

private:
    mutable std::mutex                                 m_mutex;
    std::unordered_map<std::string, DecoderSandboxRule>     m_policies;
    std::vector<DecoderSandboxViolation>                      m_violations;
};

} // namespace Engine
} // namespace ExplorerLens
