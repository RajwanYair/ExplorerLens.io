// PluginCapabilityMatrixV3.h — Plugin Capability Matrix v3
// Copyright (c) 2026 ExplorerLens Project
//
// Declares and validates per-plugin capability bitmasks for SDK v3 gate decisions.
//
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

enum class PCMv3Capability : uint32_t {
    None           = 0,
    Decode         = 1u << 0,
    Encode         = 1u << 1,
    GPU_Accelerate = 1u << 2,
    Network        = 1u << 3,
    FileSystem     = 1u << 4,
};

struct PCMv3PluginProfile {
    std::string pluginId;
    uint32_t    capabilityMask = 0;
    std::string sdkVersion;
};

struct PCMv3GateResult {
    bool     granted     = false;
    uint32_t grantedMask = 0;
    uint32_t deniedMask  = 0;
};

class PluginCapabilityMatrixV3 {
public:
    void Register(const PCMv3PluginProfile& profile) {
        m_profiles[profile.pluginId] = profile;
    }

    PCMv3GateResult Evaluate(const std::string& pluginId, uint32_t requestedMask) {
        PCMv3GateResult r;
        auto it = m_profiles.find(pluginId);
        if (it == m_profiles.end()) return r;
        r.grantedMask = it->second.capabilityMask & requestedMask;
        r.deniedMask  = requestedMask & ~r.grantedMask;
        r.granted     = (r.grantedMask == requestedMask);
        return r;
    }

    uint32_t RegisteredCount() const {
        return static_cast<uint32_t>(m_profiles.size());
    }

private:
    std::unordered_map<std::string, PCMv3PluginProfile> m_profiles;
};

}} // namespace ExplorerLens::Engine
