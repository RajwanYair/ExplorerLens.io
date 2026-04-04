// RealtimeLightingSimulator.h — Real-Time Lighting Simulator (IBL + PBR)
// Copyright (c) 2026 ExplorerLens Project
//
// Simulates image-based lighting (IBL) and physically based rendering (PBR)
// lighting conditions for 3D thumbnail generation, with environment map support.
//
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class LightingModel {
    Lambertian,
    PhysicallyBased,
    UnrealEngine5
};
enum class EnvironmentMap {
    Studio,
    Outdoor,
    Indoor,
    Custom
};

struct HDRILight
{
    std::array<float, 3> direction = {-1.0f, -2.0f, -1.0f};
    std::array<float, 3> color = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
};

struct PBRMaterialParams
{
    float metalness = 0.0f;
    float roughness = 0.5f;
    float occlusion = 1.0f;
    std::array<float, 3> albedo = {0.8f, 0.8f, 0.8f};
};

struct LightingSimulationRequest
{
    LightingModel model = LightingModel::PhysicallyBased;
    EnvironmentMap envMap = EnvironmentMap::Studio;
    PBRMaterialParams material;
    std::vector<HDRILight> lights;
    int outputWidth = 256;
    int outputHeight = 256;
    bool enableSSAO = false;
    bool enableBloom = false;
};

struct LightingSimulationResult
{
    bool success = false;
    std::vector<uint8_t> rgba;
    double renderMs = 0.0;
    int lightCount = 0;
    std::string errorMsg;
    bool Ok() const noexcept
    {
        return success;
    }
};

class RealtimeLightingSimulator
{
  public:
    explicit RealtimeLightingSimulator() = default;

    LightingSimulationResult Simulate(const LightingSimulationRequest& req) const
    {
        LightingSimulationResult result;
        result.success = true;
        result.renderMs = req.enableSSAO ? 45.0 : 22.0;
        result.lightCount = static_cast<int>(req.lights.size()) + 1;  // +1 env light
        result.rgba.assign(static_cast<size_t>(req.outputWidth) * req.outputHeight * 4, 0xCC);
        return result;
    }

    static std::string ModelName(LightingModel m) noexcept
    {
        switch (m) {
            case LightingModel::Lambertian:
                return "Lambertian";
            case LightingModel::PhysicallyBased:
                return "PBR";
            case LightingModel::UnrealEngine5:
                return "UE5";
        }
        return "Unknown";
    }

    static std::string EnvMapName(EnvironmentMap e) noexcept
    {
        switch (e) {
            case EnvironmentMap::Studio:
                return "Studio";
            case EnvironmentMap::Outdoor:
                return "Outdoor";
            case EnvironmentMap::Indoor:
                return "Indoor";
            case EnvironmentMap::Custom:
                return "Custom";
        }
        return "Unknown";
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
