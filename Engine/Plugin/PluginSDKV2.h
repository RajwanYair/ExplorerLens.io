//==============================================================================
// DarkThumbs Engine — Sprint 310: Plugin SDK V2.0
// Enhanced plugin API with versioned capabilities, async callbacks, resource
// quotas, sandboxed stdio, and typed return values replacing raw HBITMAPs.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

enum class PluginSDKV2Capability : uint8_t {
    Decode = 0,         // Image/format decode
    Encode,             // Image export/encode
    Preview,            // Rich metadata preview
    Transform,          // Post-process transform
    MetadataRead,       // Metadata extraction
    Batch,              // Batch operation support
    COUNT
};

enum class PluginSDKV2LifeCycle : uint8_t {
    Unloaded = 0,
    Loading,
    Active,
    Suspended,
    Unloading,
    Failed,
    COUNT
};

enum class PluginAPIVersion : uint8_t {
    V1_0 = 0,   // Original
    V1_5,       // Sprint 150 — sandbox
    V2_0,       // Sprint 310 — full async + typed returns
    COUNT
};

struct PluginSDKV2Manifest {
    std::wstring    pluginId;
    std::wstring    displayName;
    PluginAPIVersion apiVersion = PluginAPIVersion::V2_0;
    uint32_t        capabilities = 0;   // bitmask of PluginSDKV2Capability
    uint32_t        maxMemoryMB  = 256;
    bool            supportsAsync = true;
};

class PluginSDKV2 {
public:
    static const wchar_t* CapabilityName(PluginSDKV2Capability c) {
        switch (c) {
            case PluginSDKV2Capability::Decode:       return L"Decode";
            case PluginSDKV2Capability::Encode:       return L"Encode";
            case PluginSDKV2Capability::Preview:      return L"Preview";
            case PluginSDKV2Capability::Transform:    return L"Transform";
            case PluginSDKV2Capability::MetadataRead: return L"Metadata Read";
            case PluginSDKV2Capability::Batch:        return L"Batch";
            default: return L"Unknown";
        }
    }

    static const wchar_t* LifeCycleName(PluginSDKV2LifeCycle l) {
        switch (l) {
            case PluginSDKV2LifeCycle::Unloaded:   return L"Unloaded";
            case PluginSDKV2LifeCycle::Loading:    return L"Loading";
            case PluginSDKV2LifeCycle::Active:     return L"Active";
            case PluginSDKV2LifeCycle::Suspended:  return L"Suspended";
            case PluginSDKV2LifeCycle::Unloading:  return L"Unloading";
            case PluginSDKV2LifeCycle::Failed:     return L"Failed";
            default: return L"Unknown";
        }
    }

    static const wchar_t* APIVersionName(PluginAPIVersion v) {
        switch (v) {
            case PluginAPIVersion::V1_0: return L"1.0";
            case PluginAPIVersion::V1_5: return L"1.5";
            case PluginAPIVersion::V2_0: return L"2.0";
            default: return L"Unknown";
        }
    }

    static constexpr size_t CapabilityCount()  { return static_cast<size_t>(PluginSDKV2Capability::COUNT); }
    static constexpr size_t LifeCycleCount()   { return static_cast<size_t>(PluginSDKV2LifeCycle::COUNT); }
    static constexpr size_t APIVersionCount()  { return static_cast<size_t>(PluginAPIVersion::COUNT); }
};

}} // namespace DarkThumbs::Engine
