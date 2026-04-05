// SDKCompatKit3.cpp — SDK Compatibility Kit v3 for legacy plugin support
// Copyright (c) 2026 ExplorerLens Project
//
#include "SDKCompatKit3.h"

namespace ExplorerLens { namespace Engine {

SDKCompatKit3 SDKCompatKit3::s_instance;

SDKCompatKit3::SDKCompatKit3()  = default;
SDKCompatKit3::~SDKCompatKit3() { Shutdown(); }

SDKCompatKit3& SDKCompatKit3::Instance() noexcept { return s_instance; }

bool SDKCompatKit3::Initialize()
{
    m_stats       = {};
    m_initialized = true;
    return true;
}

void SDKCompatKit3::Shutdown()
{
    m_initialized = false;
    m_stats       = {};
}

SDKVersion SDKCompatKit3::DetectPluginSDKVersion(const std::string& pluginPath) const
{
    if (pluginPath.empty())
        return SDKVersion::Unknown;
    // Stub: would inspect plugin manifest or DLL exports for SDK version markers.
    return SDKVersion::V3;
}

bool SDKCompatKit3::AdaptPlugin(const std::string& pluginPath, SDKVersion fromVersion)
{
    if (!m_initialized || pluginPath.empty())
        return false;
    switch (fromVersion)
    {
    case SDKVersion::V1: ++m_stats.v1Calls; break;
    case SDKVersion::V2: ++m_stats.v2Calls; break;
    case SDKVersion::V3: ++m_stats.v3Calls; break;
    default: ++m_stats.shimErrors; return false;
    }
    return true;
}

bool SDKCompatKit3::IsCompatible(SDKVersion ver) const noexcept
{
    return ver == SDKVersion::V1 || ver == SDKVersion::V2 || ver == SDKVersion::V3;
}

}} // namespace ExplorerLens::Engine
