// HDRToneMapper.h — HDR to SDR Tone Mapping for Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Converts HDR image data (HDR10, Dolby Vision, scRGB) to SDR-range
// thumbnails using perceptual tone mapping operators. Preserves detail
// in both highlights and shadows for accurate thumbnail representation.
//
#pragma once

#include <cstdint>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

enum class HDRToneMappingOp : uint8_t {
    Reinhard, ACES, Filmic, Hable, AgX, None, COUNT
};

enum class HDRSourceFormat : uint8_t {
    HDR10_PQ, HLG, scRGB, OpenEXR, RadianceHDR, COUNT
};

struct HDRMapperConfig {
    HDRToneMappingOp op = HDRToneMappingOp::ACES;
    float exposure = 1.0f;
    float whitePoint = 1.0f;
    float gamma = 2.2f;
    float saturation = 1.0f;
    bool autoExposure = true;
};

struct ToneMappingResult {
    HDRSourceFormat sourceFormat = HDRSourceFormat::HDR10_PQ;
    float peakLuminance = 0.0f;
    float avgLuminance = 0.0f;
    float exposureApplied = 1.0f;
    double processingMs = 0.0;
    bool clipped = false;
};

class HDRToneMapper {
public:
    void Configure(const HDRMapperConfig& cfg) { m_config = cfg; }
    const HDRMapperConfig& GetConfig() const { return m_config; }

    float ToneMap(float hdrValue) const {
        float v = hdrValue * m_config.exposure;
        switch (m_config.op) {
        case HDRToneMappingOp::Reinhard:
            return v / (1.0f + v);
        case HDRToneMappingOp::ACES: {
            float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
            float mapped = (v * (a * v + b)) / (v * (c * v + d) + e);
            return (mapped < 0.0f) ? 0.0f : ((mapped > 1.0f) ? 1.0f : mapped);
        }
        case HDRToneMappingOp::Filmic: {
            float x = (v - 0.004f > 0.0f) ? (v - 0.004f) : 0.0f;
            return (x * (6.2f * x + 0.5f)) / (x * (6.2f * x + 1.7f) + 0.06f);
        }
        case HDRToneMappingOp::None:
            return (v > 1.0f) ? 1.0f : ((v < 0.0f) ? 0.0f : v);
        default:
            return v / (1.0f + v);
        }
    }

    float ApplyGamma(float linear) const {
        return std::pow(linear, 1.0f / m_config.gamma);
    }

    ToneMappingResult Process(HDRSourceFormat fmt, float peakNits, float avgNits) {
        ToneMappingResult r;
        r.sourceFormat = fmt;
        r.peakLuminance = peakNits;
        r.avgLuminance = avgNits;
        r.exposureApplied = m_config.autoExposure ? (0.18f / (avgNits + 0.001f)) : m_config.exposure;
        r.clipped = (peakNits > 1000.0f);
        return r;
    }

    static size_t OperatorCount() { return static_cast<size_t>(HDRToneMappingOp::COUNT); }
    static size_t FormatCount() { return static_cast<size_t>(HDRSourceFormat::COUNT); }

private:
    HDRMapperConfig m_config;
};

} // namespace Engine
} // namespace ExplorerLens
