// OffDeviceInferenceRouter.cpp — Off-device AI inference routing controller
// Copyright (c) 2026 ExplorerLens Project
//
#include "OffDeviceInferenceRouter.h"

namespace ExplorerLens { namespace Engine {

OffDeviceInferenceRouter OffDeviceInferenceRouter::s_instance;

OffDeviceInferenceRouter::OffDeviceInferenceRouter()  = default;
OffDeviceInferenceRouter::~OffDeviceInferenceRouter() { Shutdown(); }

OffDeviceInferenceRouter& OffDeviceInferenceRouter::Instance() noexcept { return s_instance; }

bool OffDeviceInferenceRouter::Initialize()
{
    m_stats  = {};
    m_target = IsNPUAvailable() ? InferenceTarget::IntelNPU : InferenceTarget::CPU;
    return true;
}

void OffDeviceInferenceRouter::Shutdown()
{
    m_stats = {};
}

InferenceTarget OffDeviceInferenceRouter::SelectTarget() const noexcept
{
    return m_target;
}

const char* OffDeviceInferenceRouter::TargetName() const noexcept
{
    switch (m_target)
    {
    case InferenceTarget::IntelNPU:    return "Intel-NPU";
    case InferenceTarget::DirectML:    return "DirectML-GPU";
    case InferenceTarget::ONNXRuntime: return "ONNX-Runtime";
    case InferenceTarget::CPU:         return "CPU";
    default:                           return "Unknown";
    }
}

bool OffDeviceInferenceRouter::RouteInference(uint32_t /*workloadTokens*/)
{
    switch (m_target)
    {
    case InferenceTarget::IntelNPU: ++m_stats.npuRouted; break;
    case InferenceTarget::DirectML: ++m_stats.gpuRouted; break;
    default:                        ++m_stats.cpuRouted; break;
    }
    return true;
}

void OffDeviceInferenceRouter::ResetStats() noexcept
{
    m_stats = {};
}

}} // namespace ExplorerLens::Engine
