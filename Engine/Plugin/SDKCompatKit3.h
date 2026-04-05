// SDKCompatKit3.h — SDK Compatibility Kit v3 for legacy plugin support
// Copyright (c) 2026 ExplorerLens Project
//
// Provides ABI-stable shims and adapter wrappers allowing plugins built against
// ExplorerLens SDK v1 and v2 to run on the v3+ plugin host without recompilation.
// The compat kit intercepts legacy plugin calls and translates to new ABI.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens { namespace Engine {

enum class SDKVersion : uint8_t
{
    V1   = 1,
    V2   = 2,
    V3   = 3,
    Unknown = 255,
};

struct CompatAdapterStats
{
    uint64_t  v1Calls    = 0;
    uint64_t  v2Calls    = 0;
    uint64_t  v3Calls    = 0;
    uint32_t  shimErrors = 0;
};

class SDKCompatKit3
{
public:
    SDKCompatKit3();
    ~SDKCompatKit3();

    SDKCompatKit3(const SDKCompatKit3&)            = delete;
    SDKCompatKit3& operator=(const SDKCompatKit3&) = delete;

    bool              Initialize();
    void              Shutdown();
    SDKVersion        DetectPluginSDKVersion(const std::string& pluginPath) const;
    bool              AdaptPlugin(const std::string& pluginPath, SDKVersion fromVersion);
    CompatAdapterStats GetStats()  const noexcept { return m_stats; }
    bool              IsCompatible(SDKVersion ver) const noexcept;

    static SDKCompatKit3& Instance() noexcept;

private:
    CompatAdapterStats  m_stats;
    bool                m_initialized = false;
    static SDKCompatKit3 s_instance;
};

}} // namespace ExplorerLens::Engine
