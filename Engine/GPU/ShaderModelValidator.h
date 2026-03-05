// ShaderModelValidator.h — Runtime Shader Model Compatibility Checking
// Copyright (c) 2026 ExplorerLens Project
//
// Runtime shader model compatibility checking. Queries D3D12 feature levels,
// validates shader bytecode compatibility against device capabilities.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

enum class ShaderModelLevel : uint8_t {
    SM_5_0,
    SM_5_1,
    SM_6_0,
    SM_6_1,
    SM_6_2,
    SM_6_3,
    SM_6_4,
    SM_6_5,
    SM_6_6,
    SM_6_7,
    SM_6_8,
    Unknown
};

enum class ShaderFeature : uint32_t {
    WaveOps = 1 << 0,
    Int64 = 1 << 1,
    Float16 = 1 << 2,
    RayTracing = 1 << 3,
    MeshShaders = 1 << 4,
    SamplerFeedback = 1 << 5,
    AtomicInt64OnGroupShared = 1 << 6,
    DerivativesInMeshShaders = 1 << 7,
    WorkGraphs = 1 << 8
};

struct ShaderBytecodeInfo {
    ShaderModelLevel requiredModel = ShaderModelLevel::Unknown;
    uint32_t requiredFeatures = 0;
    uint32_t registerCount = 0;
    uint32_t threadGroupSizeX = 1;
    uint32_t threadGroupSizeY = 1;
    uint32_t threadGroupSizeZ = 1;
    size_t bytecodeSizeBytes = 0;
    bool isValid = false;
};

struct ShaderGPUCaps {
    ShaderModelLevel maxShaderModel = ShaderModelLevel::SM_5_0;
    uint32_t supportedFeatures = 0;
    uint32_t maxComputeThreadGroupSize = 1024;
    uint32_t maxRootSignatureDwords = 64;
    uint64_t maxGPUMemoryBytes = 0;
    std::string adapterName;
    uint32_t vendorId = 0;
    uint32_t deviceId = 0;
};

struct ShaderValidationResult {
    bool compatible = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    ShaderModelLevel requiredModel = ShaderModelLevel::Unknown;
    ShaderModelLevel deviceModel = ShaderModelLevel::Unknown;
};

class ShaderModelValidator {
public:
    static ShaderModelValidator& Instance() {
        static ShaderModelValidator instance;
        return instance;
    }

    inline ShaderValidationResult Validate(const ShaderBytecodeInfo& shader, const ShaderGPUCaps& gpu) const {
        ShaderValidationResult result;
        result.requiredModel = shader.requiredModel;
        result.deviceModel = gpu.maxShaderModel;

        if (!shader.isValid) {
            result.errors.push_back("Invalid shader bytecode");
            return result;
        }

        if (static_cast<uint8_t>(shader.requiredModel) > static_cast<uint8_t>(gpu.maxShaderModel)) {
            result.errors.push_back("Shader requires " + ShaderModelToString(shader.requiredModel) +
                " but device supports " + ShaderModelToString(gpu.maxShaderModel));
        }

        uint32_t missingFeatures = shader.requiredFeatures & ~gpu.supportedFeatures;
        if (missingFeatures != 0) {
            if (missingFeatures & static_cast<uint32_t>(ShaderFeature::WaveOps))
                result.errors.push_back("Missing required feature: Wave Operations");
            if (missingFeatures & static_cast<uint32_t>(ShaderFeature::RayTracing))
                result.errors.push_back("Missing required feature: Ray Tracing");
            if (missingFeatures & static_cast<uint32_t>(ShaderFeature::MeshShaders))
                result.errors.push_back("Missing required feature: Mesh Shaders");
            if (missingFeatures & static_cast<uint32_t>(ShaderFeature::Float16))
                result.warnings.push_back("Float16 not natively supported, will use emulation");
        }

        uint32_t totalThreads = shader.threadGroupSizeX * shader.threadGroupSizeY * shader.threadGroupSizeZ;
        if (totalThreads > gpu.maxComputeThreadGroupSize) {
            result.errors.push_back("Thread group size " + std::to_string(totalThreads) +
                " exceeds max " + std::to_string(gpu.maxComputeThreadGroupSize));
        }

        result.compatible = result.errors.empty();
        return result;
    }

    inline ShaderBytecodeInfo AnalyzeBytecode(const uint8_t* data, size_t size) const {
        ShaderBytecodeInfo info;
        info.bytecodeSizeBytes = size;

        if (!data || size < 20) return info;

        uint32_t magic = static_cast<uint32_t>(data[0]) |
            (static_cast<uint32_t>(data[1]) << 8) |
            (static_cast<uint32_t>(data[2]) << 16) |
            (static_cast<uint32_t>(data[3]) << 24);

        if (magic == 0x43425844) {
            info.isValid = true;
            uint8_t majorVersion = data[4];
            uint8_t minorVersion = data[5];
            if (majorVersion >= 6) {
                info.requiredModel = static_cast<ShaderModelLevel>(
                    static_cast<uint8_t>(ShaderModelLevel::SM_6_0) + (std::min)(static_cast<uint8_t>(8), minorVersion));
            }
            else {
                info.requiredModel = minorVersion >= 1 ? ShaderModelLevel::SM_5_1 : ShaderModelLevel::SM_5_0;
            }
        }
        return info;
    }

    inline std::string ShaderModelToString(ShaderModelLevel sm) const {
        switch (sm) {
        case ShaderModelLevel::SM_5_0: return "SM 5.0";
        case ShaderModelLevel::SM_5_1: return "SM 5.1";
        case ShaderModelLevel::SM_6_0: return "SM 6.0";
        case ShaderModelLevel::SM_6_1: return "SM 6.1";
        case ShaderModelLevel::SM_6_2: return "SM 6.2";
        case ShaderModelLevel::SM_6_3: return "SM 6.3";
        case ShaderModelLevel::SM_6_4: return "SM 6.4";
        case ShaderModelLevel::SM_6_5: return "SM 6.5";
        case ShaderModelLevel::SM_6_6: return "SM 6.6";
        case ShaderModelLevel::SM_6_7: return "SM 6.7";
        case ShaderModelLevel::SM_6_8: return "SM 6.8";
        default:                  return "Unknown";
        }
    }

private:
    ShaderModelValidator() = default;
};

}
} // namespace ExplorerLens::Engine
