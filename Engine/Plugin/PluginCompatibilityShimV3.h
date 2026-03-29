// PluginCompatibilityShimV3.h — Plugin Compatibility Shim v3
// Copyright (c) 2026 ExplorerLens Project
//
// Wraps legacy SDK v1/v2 plugins for transparent execution in the SDK v3 host.
//
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

enum class PCShimSourceSDK { SDKv1, SDKv2, SDKv2_5 };

struct PCShimLoadResult {
    bool            success        = false;
    std::string     shimmedPluginId;
    PCShimSourceSDK detectedSDK    = PCShimSourceSDK::SDKv2;
    std::string     errorMsg;
};

class PluginCompatibilityShimV3 {
public:
    PCShimLoadResult Load(const std::string& dllPath) {
        PCShimLoadResult r;
        if (dllPath.empty()) { r.errorMsg = "Empty path"; return r; }
        r.detectedSDK = (dllPath.find("v1")   != std::string::npos) ? PCShimSourceSDK::SDKv1   :
                        (dllPath.find("v2.5") != std::string::npos) ? PCShimSourceSDK::SDKv2_5
                                                                     : PCShimSourceSDK::SDKv2;
        size_t slash      = dllPath.rfind('\\');
        std::string leaf  = (slash != std::string::npos) ? dllPath.substr(slash + 1) : dllPath;
        r.shimmedPluginId = "shim-" + leaf;
        m_loaded[r.shimmedPluginId] = r.detectedSDK;
        r.success = true;
        return r;
    }

    bool IsLoaded(const std::string& shimId) const {
        return m_loaded.count(shimId) > 0;
    }

    bool Unload(const std::string& shimId) {
        return m_loaded.erase(shimId) > 0;
    }

    static bool IsCompatibleSDK(PCShimSourceSDK sdk) {
        // SDKv1 requires full shim; SDKv2 and SDKv2_5 pass through
        return sdk != PCShimSourceSDK::SDKv1;
    }

private:
    std::unordered_map<std::string, PCShimSourceSDK> m_loaded;
};

}} // namespace ExplorerLens::Engine
