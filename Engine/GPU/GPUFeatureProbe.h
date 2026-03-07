// GPUFeatureProbe.h — Runtime GPU Feature Detection
// Copyright (c) 2026 ExplorerLens Project
//
// Probes GPU hardware and driver capabilities at runtime to determine
// which compute and decode features are available for thumbnail generation.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct GPUFeatureSet {
    // Compute features
    bool computeShaders = false;
    bool typed_uav_load = false;
    bool fp16 = false;
    bool int64 = false;
    bool waveOps = false;
    uint32_t maxThreadGroupSize = 0;
    uint32_t maxComputeWorkGroupCountX = 0;

    // Decode features
    bool hardwareJpegDecode = false;
    bool hardwareH264Decode = false;
    bool hardwareH265Decode = false;
    bool hardwareAV1Decode = false;
    bool hardwareVP9Decode = false;

    // Memory features
    bool umaArchitecture = false;
    uint64_t maxTextureSize = 0;
    uint64_t maxBufferSize = 0;

    // API support
    bool dx11 = false;
    bool dx12 = false;
    bool vulkan = false;
    uint32_t shaderModel = 0; // e.g., 60 = SM 6.0
};

struct ProbeResult {
    bool success = false;
    std::string errorMessage;
    GPUFeatureSet features;
    uint32_t probeTimeMs = 0;
};

class GPUFeatureProbe {
public:
    bool SupportsComputeDecode(const GPUFeatureSet& fs) const {
        return fs.computeShaders && (fs.dx12 || fs.vulkan) && fs.shaderModel >= 50;
    }

    bool SupportsHardwareDecode(const GPUFeatureSet& fs) const {
        return fs.hardwareJpegDecode || fs.hardwareH264Decode ||
            fs.hardwareH265Decode || fs.hardwareAV1Decode;
    }

    uint32_t RecommendedThreadGroupSize(const GPUFeatureSet& fs) const {
        if (fs.maxThreadGroupSize >= 1024) return 256;
        if (fs.maxThreadGroupSize >= 256) return 64;
        return 32;
    }

    std::string SummaryString(const GPUFeatureSet& fs) const {
        std::string s;
        if (fs.dx12) s += "DX12 ";
        if (fs.dx11) s += "DX11 ";
        if (fs.vulkan) s += "VK ";
        s += "SM" + std::to_string(fs.shaderModel / 10) + "." +
            std::to_string(fs.shaderModel % 10);
        if (fs.fp16) s += " FP16";
        if (fs.waveOps) s += " Wave";
        return s;
    }

private:
    // Feature detection state
};

} // namespace Engine
} // namespace ExplorerLens
