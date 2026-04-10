// DeviceCapabilityAdvertiser.cpp — Local Hardware Capability Publication
// Copyright (c) 2026 ExplorerLens Project
//
#include "DeviceCapabilityAdvertiser.h"
#include <algorithm>
#include <sstream>

namespace ExplorerLens {
namespace Engine {

void DeviceCapabilityAdvertiser::Probe()
{
    m_profile = {};
    m_profile.capturedAt = 0; // stub; real impl would call time()

    // Stub hardware detection — real impl queries DXGI / Vulkan / CPU info.
    m_profile.gpuTier        = DeviceGPUTier::DX11;
    m_profile.logicalCores   = 4;
    m_profile.availableMemMB = 4096;
    m_profile.canDecode4K    = true;
    m_profile.canDecodeHDR   = false;
    m_probed = true;
}

std::string DeviceCapabilityAdvertiser::Serialize() const
{
    std::ostringstream ss;
    ss << "{\"deviceId\":\"" << m_profile.deviceId << "\","
       << "\"hostname\":\"" << m_profile.hostname << "\","
       << "\"gpuTier\":"    << static_cast<int>(m_profile.gpuTier) << ","
       << "\"cores\":"      << m_profile.logicalCores << ","
       << "\"memMB\":"      << m_profile.availableMemMB << ","
       << "\"4k\":"         << (m_profile.canDecode4K  ? "true" : "false") << ","
       << "\"hdr\":"        << (m_profile.canDecodeHDR ? "true" : "false") << ","
       << "\"ts\":"         << m_profile.capturedAt << "}";
    return ss.str();
}

bool DeviceCapabilityAdvertiser::Deserialize(const std::string& json)
{
    if (json.empty() || json.front() != '{') return false;
    // Stub: in production use a proper JSON parser.
    m_probed = true;
    return true;
}

void DeviceCapabilityAdvertiser::LoadPeerProfiles(
    const std::vector<DeviceCapabilityProfile>& peers)
{
    m_peers = peers;
}

const DeviceCapabilityProfile* DeviceCapabilityAdvertiser::FindBestDecoder() const
{
    const DeviceCapabilityProfile* best = nullptr;
    for (const auto& p : m_peers) {
        if (!p.canDecode4K) continue;
        if (!best ||
            static_cast<int>(p.gpuTier) > static_cast<int>(best->gpuTier))
        {
            best = &p;
        }
    }
    return best;
}

} // namespace Engine
} // namespace ExplorerLens
