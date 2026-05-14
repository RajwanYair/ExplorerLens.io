// OffDeviceInferenceRouter.h — Off-device AI inference routing controller
// Copyright (c) 2026 ExplorerLens Project
//
// Routes AI inference workloads (thumbnail synthesis, inpainting, IQA) to
// the optimal compute target: Intel NPU > DirectML GPU > ONNX Runtime > CPU.
// Monitors device capability, thermal state, and memory pressure to select
// the fastest available backend at inference time.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens { namespace Engine {

enum class InferenceTarget : uint8_t
{
    IntelNPU   = 0,
    DirectML   = 1,
    ONNXRuntime = 2,
    CPU        = 3,
};

struct InferenceRouterStats
{
    uint64_t npuRouted   = 0;
    uint64_t gpuRouted   = 0;
    uint64_t cpuRouted   = 0;
    float    avgLatencyMs = 0.0f;
};

class OffDeviceInferenceRouter
{
public:
    OffDeviceInferenceRouter();
    ~OffDeviceInferenceRouter();

    OffDeviceInferenceRouter(const OffDeviceInferenceRouter&)            = delete;
    OffDeviceInferenceRouter& operator=(const OffDeviceInferenceRouter&) = delete;

    bool                   Initialize();
    void                   Shutdown();
    InferenceTarget        SelectTarget()         const noexcept;
    const char*            TargetName()           const noexcept;
    bool                   RouteInference(uint32_t workloadTokens);
    InferenceRouterStats   GetStats()             const noexcept { return m_stats; }
    void                   ResetStats()           noexcept;
    bool                   IsNPUAvailable()       const noexcept { return false; }

    static OffDeviceInferenceRouter& Instance() noexcept;

private:
    InferenceTarget          m_target = InferenceTarget::CPU;
    InferenceRouterStats     m_stats;
    static OffDeviceInferenceRouter s_instance;
};

}} // namespace ExplorerLens::Engine
