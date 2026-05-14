// AdaptiveShaderSelection.h — Dynamic Shader Variant Chooser
// Copyright (c) 2026 ExplorerLens Project
//
// Selects optimal shader variants at runtime based on GPU capabilities,
// image characteristics, and performance history.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class ShaderVariant : uint8_t {
    BasicResize = 0,    // Bilinear resize (SM 5.0)
    LanczosResize = 1,  // Lanczos3 (SM 5.0)
    ComputeResize = 2,  // Compute shader resize (SM 6.0)
    WaveResize = 3,     // Wave intrinsics resize (SM 6.0 + WaveOps)
    FP16Resize = 4      // Half precision path (SM 6.2 + FP16)
};

struct ShaderCapabilityReq
{
    uint32_t minShaderModel = 50;
    bool requiresWaveOps = false;
    bool requiresFP16 = false;
    bool requiresComputeShaders = false;
    uint32_t minThreadGroupSize = 32;
};

struct ShaderSelectionResult
{
    ShaderVariant variant = ShaderVariant::BasicResize;
    std::string shaderName;
    std::string reason;
    double estimatedMs = 0.0;
};

struct ShaderPerfRecord
{
    ShaderVariant variant = ShaderVariant::BasicResize;
    uint64_t invocations = 0;
    double totalMs = 0.0;
    double bestMs = 1e9;
    double worstMs = 0.0;
    double AvgMs() const
    {
        return invocations > 0 ? totalMs / invocations : 0.0;
    }
};

class AdaptiveShaderSelection
{
  public:
    ShaderSelectionResult Select(uint32_t shaderModel, bool hasWaveOps, bool hasFP16, uint32_t imageWidth) const
    {
        ShaderSelectionResult r;
        if (shaderModel >= 62 && hasFP16 && imageWidth > 2048) {
            r.variant = ShaderVariant::FP16Resize;
            r.shaderName = "ResizeFP16CS";
            r.reason = "Large image + FP16 support";
        } else if (shaderModel >= 60 && hasWaveOps) {
            r.variant = ShaderVariant::WaveResize;
            r.shaderName = "ResizeWaveCS";
            r.reason = "Wave operations available";
        } else if (shaderModel >= 60) {
            r.variant = ShaderVariant::ComputeResize;
            r.shaderName = "ResizeComputeCS";
            r.reason = "Compute shader path";
        } else if (imageWidth > 1024) {
            r.variant = ShaderVariant::LanczosResize;
            r.shaderName = "ResizeLanczosPS";
            r.reason = "Quality resize for large image";
        } else {
            r.variant = ShaderVariant::BasicResize;
            r.shaderName = "ResizeBilinearPS";
            r.reason = "Default fast path";
        }
        return r;
    }

    void RecordPerformance(ShaderVariant variant, double ms)
    {
        auto& rec = m_perfHistory[static_cast<uint8_t>(variant)];
        rec.variant = variant;
        rec.invocations++;
        rec.totalMs += ms;
        if (ms < rec.bestMs)
            rec.bestMs = ms;
        if (ms > rec.worstMs)
            rec.worstMs = ms;
    }

  private:
    std::unordered_map<uint8_t, ShaderPerfRecord> m_perfHistory;
};

}  // namespace Engine
}  // namespace ExplorerLens
