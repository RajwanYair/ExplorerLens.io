// PQToSDRToneMapper.h — SMPTE ST.2084 (PQ) to sRGB Tone Mapper
// Copyright (c) 2026 ExplorerLens Project
//
// Converts HDR10 / PQ-encoded pixel data (SMPTE ST.2084) to sRGB for
// thumbnail display on standard 8-bit monitors. Applies a Hable filmic
// tone-map curve or ACES RRT+ODT depending on content type.
// P50 target: < 0.5 ms (LUT + GPU compute). Used by ExrDecoder, JXLDecoder.
//
#pragma once
#include <cstdint>
#include <array>

namespace ExplorerLens { namespace Engine {

enum class PQToneMapOp : uint8_t {
    Hable       = 0,   // Filmic Hable "Uncharted 2" curve — warm, filmic
    ACES        = 1,   // ACES RRT + sRGB ODT — standard for film/VFX
    Reinhard    = 2,   // Classic Reinhard; low contrast but artifact-free
    AgX         = 3,   // AgX — modern perceptually uniform curve
};

struct PQToneMapParams {
    PQToneMapOp op          = PQToneMapOp::ACES;
    float           peakNits    = 1000.0f;  // Peak luminance of source content (nits)
    float           displayNits = 100.0f;   // Display peak luminance for mapping
    float           exposure    = 0.0f;     // EV offset applied before tonemapping
    bool            applyBT709  = true;     // Final BT.709 primaries clamp
};

struct PQToneMapResult {
    uint8_t* pixelsBGRA = nullptr;  // Caller-owned output; width × height × 4 bytes
    uint32_t width      = 0;
    uint32_t height     = 0;
    bool     success    = false;
    float    processMs  = 0.0f;
};

class PQToSDRToneMapper {
public:
    PQToSDRToneMapper();
    ~PQToSDRToneMapper();

    // Tone-map `srcFP16` (half-float RGBA, PQ-encoded) to BGRA8 sRGB.
    // srcFP16 must be width × height × 8 bytes (4× fp16 channels).
    PQToneMapResult ToneMap(
        const uint16_t*       srcFP16,
        uint32_t              width,
        uint32_t              height,
        const PQToneMapParams& params) const noexcept;

    // Tone-map from BGRA10 (HDR10 10-bit packed) to BGRA8 sRGB.
    PQToneMapResult ToneMapBGRA10(
        const uint32_t*       srcBGRA10,
        uint32_t              width,
        uint32_t              height,
        const PQToneMapParams& params) const noexcept;

    // Build a 1024-entry LUT for fast CPU-side PQ→sRGB mapping.
    static std::array<uint8_t, 1024> BuildPQToSRGBLUT(const PQToneMapParams& params) noexcept;

    // Convert a single normalised PQ value [0, 1] to linear photometric light.
    static float PQToLinear(float pqValue) noexcept;

    // Apply Hable filmic curve to a linear light value.
    static float HableCurve(float x) noexcept;

    // Apply ACES RRT approximation to a linear light value.
    static float ACESCurve(float x) noexcept;
};

}} // namespace ExplorerLens::Engine
