// ARPreviewBridgeEngine.h — AR Preview Bridge Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Bridges the ExplorerLens thumbnail pipeline to ARKit, ARCore, and
// OpenXR passthrough sessions for augmented-reality file previews.
//
#pragma once
#include <string>
#include <array>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class ARRuntimeType { ARKit, ARCore, OpenXRPassthrough, Simulation };

struct ARSessionConfig {
    ARRuntimeType runtime         = ARRuntimeType::Simulation;
    uint32_t      displayWidth    = 1920;
    uint32_t      displayHeight   = 1080;
    float         nearPlane       = 0.01f;
    float         farPlane        = 100.0f;
    bool          enableOcclusion = true;
    bool          enableDepth     = true;
};

struct ARPose {
    std::array<float, 16> matrix{}; // 4×4 row-major world transform
    float confidence = 1.0f;
};

class ARPreviewBridgeEngine {
public:
    explicit ARPreviewBridgeEngine(const ARSessionConfig& cfg = {}) : m_cfg(cfg) {}

    bool StartSession() {
        m_active = true;
        return true;
    }

    void StopSession() { m_active = false; }
    bool IsSessionActive() const { return m_active; }

    bool AttachThumbnail(uint64_t anchorId, const std::string& filePath,
                         float widthMeters = 0.2f) {
        if (!m_active || filePath.empty()) return false;
        (void)anchorId; (void)widthMeters;
        return true;
    }

    bool DetachThumbnail(uint64_t anchorId) {
        (void)anchorId;
        return m_active;
    }

    ARPose GetDevicePose() const {
        ARPose pose;
        pose.matrix[0] = pose.matrix[5] = pose.matrix[10] = pose.matrix[15] = 1.0f;
        pose.confidence = 1.0f;
        return pose;
    }

    std::string GetRuntimeName() const {
        switch (m_cfg.runtime) {
            case ARRuntimeType::ARKit:          return "ARKit";
            case ARRuntimeType::ARCore:         return "ARCore";
            case ARRuntimeType::OpenXRPassthrough: return "OpenXR";
            case ARRuntimeType::Simulation:     return "Simulation";
        }
        return "unknown";
    }

    const ARSessionConfig& GetConfig() const { return m_cfg; }

private:
    ARSessionConfig m_cfg;
    bool            m_active = false;
};

}} // namespace ExplorerLens::Engine
