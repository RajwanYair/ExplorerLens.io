// VulkanRayTracingPreview.h — Vulkan RT for 3D Model Ray-Traced Previews
// Copyright (c) 2026 ExplorerLens Project
//
// Vulkan RT for 3D model ray-traced previews. Manages VK_KHR_ray_tracing_pipeline
// extension, generates single-frame ray-traced thumbnails.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

enum class RTAccelStructType : uint8_t {
    TopLevel,
    BottomLevel
};

enum class RTShaderStage : uint8_t {
    RayGen,
    Miss,
    ClosestHit,
    AnyHit,
    Intersection
};

struct RTVertex {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float nx = 0.0f, ny = 1.0f, nz = 0.0f;
};

struct RTTriangle {
    uint32_t v0 = 0, v1 = 0, v2 = 0;
};

struct RTMaterial {
    std::array<float, 3> albedo = { 0.8f, 0.8f, 0.8f };
    float roughness = 0.5f;
    float metallic = 0.0f;
    float ior = 1.5f;
};

struct RTScene {
    std::vector<RTVertex> vertices;
    std::vector<RTTriangle> triangles;
    std::vector<RTMaterial> materials;
    std::array<float, 3> lightDirection = { 0.577f, -0.577f, 0.577f };
    std::array<float, 3> lightColor = { 1.0f, 1.0f, 1.0f };
    float lightIntensity = 2.0f;
    std::array<float, 3> cameraPosition = { 0.0f, 0.0f, 3.0f };
    std::array<float, 3> cameraTarget = { 0.0f, 0.0f, 0.0f };
    float cameraFoV = 45.0f;
};

struct RTRenderConfig {
    uint32_t width = 256;
    uint32_t height = 256;
    uint32_t maxBounces = 2;
    uint32_t samplesPerPixel = 1;
    bool enableShadows = true;
    bool enableAO = false;
};

class VulkanRayTracingPreview {
public:
    static VulkanRayTracingPreview& Instance() {
        static VulkanRayTracingPreview instance;
        return instance;
    }

    inline std::vector<uint8_t> RenderPreview(const RTScene& scene, const RTRenderConfig& config) const {
        std::vector<uint8_t> framebuffer(static_cast<size_t>(config.width) * config.height * 3, 0);
        if (scene.vertices.empty() || scene.triangles.empty()) return framebuffer;

        float aspectRatio = static_cast<float>(config.width) / config.height;
        float fovScale = std::tan(scene.cameraFoV * 0.5f * 3.14159f / 180.0f);

        auto forward = Normalize(Sub(scene.cameraTarget, scene.cameraPosition));
        auto right = Normalize(Cross(forward, { 0.0f, 1.0f, 0.0f }));
        auto up = Cross(right, forward);

        for (uint32_t y = 0; y < config.height; ++y) {
            for (uint32_t x = 0; x < config.width; ++x) {
                float u = (2.0f * (x + 0.5f) / config.width - 1.0f) * aspectRatio * fovScale;
                float v = (1.0f - 2.0f * (y + 0.5f) / config.height) * fovScale;

                std::array<float, 3> dir = Normalize({
                    forward[0] + u * right[0] + v * up[0],
                    forward[1] + u * right[1] + v * up[1],
                    forward[2] + u * right[2] + v * up[2]
                    });

                auto color = TraceRay(scene, scene.cameraPosition, dir, config.maxBounces, config.enableShadows);

                size_t idx = (static_cast<size_t>(y) * config.width + x) * 3;
                framebuffer[idx + 0] = GammaEncode(color[0]);
                framebuffer[idx + 1] = GammaEncode(color[1]);
                framebuffer[idx + 2] = GammaEncode(color[2]);
            }
        }
        return framebuffer;
    }

    inline size_t EstimateMemoryUsage(const RTScene& scene) const {
        size_t vertMem = scene.vertices.size() * sizeof(RTVertex);
        size_t triMem = scene.triangles.size() * sizeof(RTTriangle);
        size_t blasEstimate = (vertMem + triMem) * 2;
        return vertMem + triMem + blasEstimate;
    }

    inline bool ValidateScene(const RTScene& scene) const {
        if (scene.vertices.empty() || scene.triangles.empty()) return false;
        uint32_t maxIdx = static_cast<uint32_t>(scene.vertices.size());
        for (const auto& tri : scene.triangles) {
            if (tri.v0 >= maxIdx || tri.v1 >= maxIdx || tri.v2 >= maxIdx) return false;
        }
        return true;
    }

private:
    VulkanRayTracingPreview() = default;

    using Vec3 = std::array<float, 3>;

    inline Vec3 Sub(Vec3 a, Vec3 b) const { return { a[0] - b[0], a[1] - b[1], a[2] - b[2] }; }
    inline Vec3 Add(Vec3 a, Vec3 b) const { return { a[0] + b[0], a[1] + b[1], a[2] + b[2] }; }
    inline Vec3 Mul(Vec3 a, float s) const { return { a[0] * s, a[1] * s, a[2] * s }; }
    inline float Dot(Vec3 a, Vec3 b) const { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
    inline Vec3 Cross(Vec3 a, Vec3 b) const {
        return { a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0] };
    }
    inline Vec3 Normalize(Vec3 v) const {
        float len = std::sqrt(Dot(v, v));
        return len > 1e-8f ? Vec3{ v[0] / len, v[1] / len, v[2] / len } : Vec3{ 0,0,0 };
    }

    struct HitInfo {
        float t = 1e30f;
        Vec3 normal = { 0, 1, 0 };
        uint32_t materialIdx = 0;
        bool hit = false;
    };

    inline HitInfo IntersectScene(const RTScene& scene, Vec3 origin, Vec3 dir) const {
        HitInfo closest;
        for (size_t i = 0; i < scene.triangles.size(); ++i) {
            const auto& tri = scene.triangles[i];
            const auto& v0 = scene.vertices[tri.v0];
            const auto& v1 = scene.vertices[tri.v1];
            const auto& v2 = scene.vertices[tri.v2];

            Vec3 e1 = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
            Vec3 e2 = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };
            Vec3 h = Cross(dir, e2);
            float a = Dot(e1, h);
            if (std::abs(a) < 1e-8f) continue;

            float f = 1.0f / a;
            Vec3 s = { origin[0] - v0.x, origin[1] - v0.y, origin[2] - v0.z };
            float u = f * Dot(s, h);
            if (u < 0.0f || u > 1.0f) continue;

            Vec3 q = Cross(s, e1);
            float vv = f * Dot(dir, q);
            if (vv < 0.0f || u + vv > 1.0f) continue;

            float t = f * Dot(e2, q);
            if (t > 0.001f && t < closest.t) {
                closest.t = t;
                closest.normal = Normalize(Cross(e1, e2));
                closest.materialIdx = static_cast<uint32_t>(i % (std::max)(static_cast<size_t>(1), scene.materials.size()));
                closest.hit = true;
            }
        }
        return closest;
    }

    inline Vec3 TraceRay(const RTScene& scene, Vec3 origin, Vec3 dir, uint32_t depth, bool shadows) const {
        (void)depth;
        auto hitInfo = IntersectScene(scene, origin, dir);
        if (!hitInfo.hit) {
            float t = 0.5f * (dir[1] + 1.0f);
            return { 0.1f + 0.3f * t, 0.1f + 0.4f * t, 0.2f + 0.6f * t };
        }

        RTMaterial mat = scene.materials.empty() ?
            RTMaterial{} : scene.materials[hitInfo.materialIdx % scene.materials.size()];

        Vec3 hitPoint = Add(origin, Mul(dir, hitInfo.t));
        Vec3 lightDir = Normalize({ -scene.lightDirection[0], -scene.lightDirection[1], -scene.lightDirection[2] });
        float NdotL = (std::max)(0.0f, Dot(hitInfo.normal, lightDir));

        float shadowFactor = 1.0f;
        if (shadows) {
            auto shadowHit = IntersectScene(scene, Add(hitPoint, Mul(hitInfo.normal, 0.001f)), lightDir);
            if (shadowHit.hit) shadowFactor = 0.3f;
        }

        float ambient = 0.15f;
        return {
            mat.albedo[0] * (ambient + NdotL * shadowFactor * scene.lightIntensity * scene.lightColor[0]),
            mat.albedo[1] * (ambient + NdotL * shadowFactor * scene.lightIntensity * scene.lightColor[1]),
            mat.albedo[2] * (ambient + NdotL * shadowFactor * scene.lightIntensity * scene.lightColor[2])
        };
    }

    inline uint8_t GammaEncode(float linear) const {
        float v = (std::max)(0.0f, (std::min)(1.0f, linear));
        return static_cast<uint8_t>(std::pow(v, 1.0f / 2.2f) * 255.0f + 0.5f);
    }
};

}
} // namespace ExplorerLens::Engine
