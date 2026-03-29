// EyeTrackingFocusOptimizer.h — Eye-Tracking Focus Optimizer
// Copyright (c) 2026 ExplorerLens Project
//
// Accepts raw gaze point data and computes priority thumbnail regions to
// pre-decode, leveraging foveal rendering techniques for latency reduction.
//
#pragma once
#include <cstdint>
#include <vector>

namespace ExplorerLens { namespace Engine {

struct GazePoint {
    float x = 0.0f;  // normalized [0,1]
    float y = 0.0f;
    float confidence = 1.0f;
};

struct FocusRegion {
    uint32_t left   = 0;
    uint32_t top    = 0;
    uint32_t right  = 0;
    uint32_t bottom = 0;
    float    priority = 0.0f;
};

class EyeTrackingFocusOptimizer {
public:
    EyeTrackingFocusOptimizer() = default;

    bool Initialize(uint32_t viewW, uint32_t viewH) {
        m_viewW = viewW; m_viewH = viewH; m_ready = true; return true;
    }
    bool IsReady() const { return m_ready; }

    void SetGazePoint(const GazePoint& gaze) { m_lastGaze = gaze; }

    FocusRegion GetPriorityRegion() const {
        FocusRegion r;
        float cx = m_lastGaze.x * m_viewW;
        float cy = m_lastGaze.y * m_viewH;
        float half = m_fovealRadiusPx;
        r.left   = static_cast<uint32_t>(cx > half ? cx - half : 0.0f);
        r.top    = static_cast<uint32_t>(cy > half ? cy - half : 0.0f);
        r.right  = static_cast<uint32_t>(cx + half < m_viewW ? cx + half : m_viewW);
        r.bottom = static_cast<uint32_t>(cy + half < m_viewH ? cy + half : m_viewH);
        r.priority = m_lastGaze.confidence;
        return r;
    }

    void SetFovealRadius(float px) { m_fovealRadiusPx = px; }

    void Shutdown() { m_ready = false; }

private:
    bool      m_ready          = false;
    GazePoint m_lastGaze;
    uint32_t  m_viewW          = 1920;
    uint32_t  m_viewH          = 1080;
    float     m_fovealRadiusPx = 256.0f;
};

}} // namespace ExplorerLens::Engine
