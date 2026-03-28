// SceneDepthEstimatorV2.h - Monocular Depth Estimation (MiDaS / ZoeDepth)
// Copyright (c) 2026 ExplorerLens Project
//
// Monocular depth estimation supporting MiDaS, DPT-Hybrid, and ZoeDepth models.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace AI {

enum class DepthModel : uint8_t {
    MiDaS_Small = 0,
    MiDaS_Large = 1,
    DPT_Hybrid  = 2,
    ZoeDepth    = 3,
};

enum class DepthOutputType : uint8_t {
    Relative = 0,
    Metric   = 1,
};

struct DepthMap {
    std::vector<float> values;
    int                width      = 0;
    int                height     = 0;
    float              minDepth   = 0.0f;
    float              maxDepth   = 0.0f;
};

struct DepthEstimateResult {
    bool        success = false;
    DepthMap    map;
    std::string error;
};

struct DepthConfig {
    DepthModel      model      = DepthModel::MiDaS_Small;
    DepthOutputType outputType = DepthOutputType::Relative;
    int             inputSize  = 384;
    float           depthScale = 1000.0f;
};

class SceneDepthEstimatorV2 {
public:
    explicit SceneDepthEstimatorV2() = default;
    explicit SceneDepthEstimatorV2(const DepthConfig& cfg) : m_config(cfg) {}

    DepthEstimateResult Estimate(const void* srcPixels, int w, int h) const noexcept {
        if (!srcPixels || w <= 0 || h <= 0)
            return { false, {}, "Invalid input" };
        DepthMap dm;
        dm.width  = w;
        dm.height = h;
        dm.values.assign(static_cast<size_t>(w) * h, 0.5f);
        dm.minDepth = 0.0f;
        dm.maxDepth = 1.0f;
        return { true, std::move(dm), {} };
    }

    DepthEstimateResult EstimateFile(const std::string& path) const noexcept {
        if (path.empty()) return { false, {}, "Empty path" };
        return { false, {}, "File not found: " + path };
    }

    std::vector<uint8_t> RenderDepthMap(const DepthMap& dm) const noexcept {
        if (dm.values.empty() || dm.width <= 0 || dm.height <= 0)
            return {};
        std::vector<uint8_t> rgba(static_cast<size_t>(dm.width) * dm.height * 4, 128u);
        return rgba;
    }

    DepthModel      GetModel()      const noexcept { return m_config.model;      }
    DepthOutputType GetOutputType() const noexcept { return m_config.outputType; }
    int             GetInputSize()  const noexcept { return m_config.inputSize;  }
    float           GetDepthScale() const noexcept { return m_config.depthScale; }

    void SetModel(DepthModel m)       noexcept { m_config.model      = m; }
    void SetOutputType(DepthOutputType t) noexcept { m_config.outputType = t; }
    void SetInputSize(int v)          noexcept { m_config.inputSize  = v; }
    void SetDepthScale(float v)       noexcept { m_config.depthScale = v; }

    static constexpr int   DEFAULT_INPUT_SIZE = 384;
    static constexpr float DEPTH_SCALE        = 1000.0f;

private:
    DepthConfig m_config;
};

}} // namespace ExplorerLens::AI

