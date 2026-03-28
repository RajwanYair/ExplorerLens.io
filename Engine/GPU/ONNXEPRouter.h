// ONNXEPRouter.h — ONNX Runtime Execution Provider Router
// Copyright (c) 2026 ExplorerLens Project
//
// Dynamically selects the best available ONNX Execution Provider at runtime
// — NPU (OpenVINO), GPU (DirectML/CUDA), or CPU — based on hardware capability.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ONNXExecutionProvider { DirectML, CUDA, OpenVINO_NPU, QNN, CPU };
enum class EPSelectionPolicy     { HighestThroughput, LowestLatency, LowestPower, Manual };

struct EPCapability {
    ONNXExecutionProvider ep;
    bool     available    = false;
    uint32_t scorePoints  = 0;
    std::string deviceName;
};

struct EPRouterConfig {
    EPSelectionPolicy   policy       = EPSelectionPolicy::HighestThroughput;
    bool                allowCPUFallback = true;
    bool                warmupOnInit    = false;
    std::vector<ONNXExecutionProvider> preferenceOrder;
};

class ONNXEPRouter {
public:
    explicit ONNXEPRouter(const EPRouterConfig& cfg = {}) : m_cfg(cfg) {}

    void  RegisterEP(const EPCapability& cap) { m_eps.push_back(cap); }

    ONNXExecutionProvider Select() const {
        for (const auto& ep : m_eps)
            if (ep.available) return ep.ep;
        return ONNXExecutionProvider::CPU;
    }

    EPCapability* FindEP(ONNXExecutionProvider ep) {
        for (auto& c : m_eps)
            if (c.ep == ep) return &c;
        return nullptr;
    }

    bool   IsEPAvailable(ONNXExecutionProvider ep) const {
        for (const auto& c : m_eps)
            if (c.ep == ep) return c.available;
        return false;
    }

    size_t          RegisteredCount() const { return m_eps.size(); }
    EPSelectionPolicy GetPolicy()     const { return m_cfg.policy; }
    void            SetPolicy(EPSelectionPolicy p) { m_cfg.policy = p; }
    const EPRouterConfig& GetConfig() const { return m_cfg; }
    void            Clear()                 { m_eps.clear(); }

private:
    EPRouterConfig           m_cfg;
    std::vector<EPCapability> m_eps;
};

} // namespace Engine
} // namespace ExplorerLens
