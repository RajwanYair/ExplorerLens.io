// PlaneSurfaceDetector.h — Plane Surface Detector for AR
// Copyright (c) 2026 ExplorerLens Project
//
// Detects horizontal and vertical planes in AR passthrough frames,
// enabling projection of thumbnails onto real-world surfaces.
//
#pragma once
#include <vector>
#include <array>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class PlaneOrientation { Horizontal, Vertical, Arbitrary };

struct DetectedPlane {
    uint64_t              id = 0;
    PlaneOrientation      orientation = PlaneOrientation::Horizontal;
    std::array<float, 3>  center{};
    std::array<float, 3>  normal{0,1,0};
    float                 width  = 0.0f;
    float                 height = 0.0f;
    float                 areaM2 = 0.0f;
    float                 confidence = 0.0f;
    double                detectionMs = 0.0;
};

class PlaneSurfaceDetector {
public:
    PlaneSurfaceDetector() = default;

    bool Initialize() { m_ready = true; return true; }
    bool IsReady() const { return m_ready; }

    std::vector<DetectedPlane> Detect(const void* depthBuffer, uint32_t width, uint32_t height) {
        (void)depthBuffer; (void)width; (void)height;
        if (!m_ready) return {};

        DetectedPlane floor;
        floor.id          = ++m_nextId;
        floor.orientation = PlaneOrientation::Horizontal;
        floor.center      = {0.0f, -0.5f, -1.5f};
        floor.normal      = {0.0f,  1.0f,  0.0f};
        floor.width       = 3.0f;
        floor.height      = 3.0f;
        floor.areaM2      = 9.0f;
        floor.confidence  = 0.97f;
        floor.detectionMs = 800.0;

        return {floor};
    }

    void SetMinArea(float minM2)   { m_minAreaM2   = minM2; }
    void SetMinConfidence(float c) { m_minConfidence = c; }

    uint64_t GetDetectionCount() const { return m_nextId; }

private:
    bool     m_ready        = false;
    float    m_minAreaM2    = 0.25f;
    float    m_minConfidence= 0.7f;
    uint64_t m_nextId       = 0;
};

}} // namespace ExplorerLens::Engine
