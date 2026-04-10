// DeviceCapabilityAdvertiser.h — Local Hardware Capability Publication
// Copyright (c) 2026 ExplorerLens Project
//
// Publishes the local device's GPU/CPU decode capabilities to the cloud
// sync manifest so that peer devices can decide whether to offload
// expensive decode operations to this machine instead of decoding locally.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// GPU decoding tier supported by this device.
enum class DeviceGPUTier : uint8_t {
    NONE,       // CPU-only decode
    DX11,       // DirectX 11 hardware decode
    DX12,       // DirectX 12 hardware decode
    VULKAN,     // Vulkan compute decode
    NVDEC,      // NVIDIA hardware decode
    QUICKSYNC,  // Intel Quick Sync
    AMF         // AMD AMF
};

// Capability snapshot for a single device.
struct DeviceCapabilityProfile {
    std::string     deviceId;
    std::string     hostname;
    DeviceGPUTier   gpuTier         = DeviceGPUTier::NONE;
    uint32_t        logicalCores    = 0;
    uint32_t        availableMemMB  = 0;    // Available system memory in MB
    bool            canDecode4K     = false;
    bool            canDecodeHDR    = false;
    uint64_t        capturedAt      = 0;    // Unix epoch seconds
};

// Advertises local hardware capabilities into the sync manifest cloud bucket.
class DeviceCapabilityAdvertiser {
public:
    DeviceCapabilityAdvertiser() = default;

    // Probe local hardware and build the capability profile.
    void Probe();

    // Return the probed profile (after Probe() has been called).
    const DeviceCapabilityProfile& Profile() const { return m_profile; }

    // Serialise the profile to a compact JSON string for cloud upload.
    std::string Serialize() const;

    // Deserialise from JSON. Returns false on parse error.
    bool Deserialize(const std::string& json);

    // Whether Probe() has been called and succeeded.
    bool IsProbed() const { return m_probed; }

    // List capabilities advertised by all known peers (loaded from cloud).
    void LoadPeerProfiles(const std::vector<DeviceCapabilityProfile>& peers);
    const std::vector<DeviceCapabilityProfile>& PeerProfiles() const { return m_peers; }

    // Find the best peer capable of decoding 4 K HDR (for decode offload).
    const DeviceCapabilityProfile* FindBestDecoder() const;

private:
    DeviceCapabilityProfile              m_profile;
    std::vector<DeviceCapabilityProfile> m_peers;
    bool                                 m_probed = false;
};

} // namespace Engine
} // namespace ExplorerLens
