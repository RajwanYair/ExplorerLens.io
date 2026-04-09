// ACESODTProcessor.h — ACES Output Device Transform Processor
// Copyright (c) 2026 ExplorerLens Project
//
// Detects ACES encoding in EXR/TIFF files and applies the appropriate
// ACES ODT (Output Device Transform) to convert scene-referred or
// display-referred ACES imagery to sRGB for thumbnail rendering.
// Supports AP0/AP1 (ACEScg) primaries and the standard ACES RRT.
// P50 target: < 3 ms. Used by EXR and TIFF decoders.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens { namespace Engine {

enum class ACESColorspace : uint8_t {
    Unknown    = 0,
    AP0        = 1,   // ACES 2065-1 (scene-referred, wide gamut)
    AP1        = 2,   // ACEScg (working space, linear)
    ACEScc     = 3,   // ACEScc (logarithmic, post production)
    ACEScct    = 4,   // ACEScct (ACEScc + toe, preferred for grading)
    ACESproxy  = 5,   // Integer proxy encoding
};

enum class ACESODTTarget : uint8_t {
    sRGB_D65      = 0,  // Rec.709 / sRGB display (default for thumbnails)
    DisplayP3_D65 = 1,  // DCI-P3 D65 (macOS, modern Windows HDR)
    Rec2020_PQ    = 2,  // Rec.2020 HDR10 (HDR display)
    DCDM          = 3,  // Digital Cinema (P3 D60)
};

struct ACESODTParams {
    ACESColorspace  sourceColorspace = ACESColorspace::AP0;
    ACESODTTarget   target           = ACESODTTarget::sRGB_D65;
    float           exposureAdjust   = 0.0f;  // EV offset pre-ODT
    bool            applyRRT         = true;   // Apply Reference Rendering Transform
};

struct ACESODTResult {
    uint8_t* pixelsBGRA = nullptr;  // Caller-owned; width × height × 4 bytes
    uint32_t width      = 0;
    uint32_t height     = 0;
    bool     success    = false;
    float    processMs  = 0.0f;
    ACESColorspace detectedColorspace = ACESColorspace::Unknown;
};

class ACESODTProcessor {
public:
    ACESODTProcessor()  = default;
    ~ACESODTProcessor() = default;

    // Detect ACES colorspace from EXR header chromaticities attribute.
    static ACESColorspace DetectColorspace(
        const uint8_t* exrHeader, size_t exrHeaderSize) noexcept;

    // Detect ACES colorspace from a human-readable color space name string
    // (e.g. "ACES - ACES2065-1", "ACEScg", "Input - ACEScct").
    static ACESColorspace DetectFromString(const char* colorspaceName) noexcept;

    // Apply the full RRT + ODT pipeline to `srcFP16` (half-float RGBA, AP0/AP1).
    // srcFP16 must be width × height × 8 bytes (4 × fp16 per pixel).
    ACESODTResult ApplyODT(
        const uint16_t*       srcFP16,
        uint32_t              width,
        uint32_t              height,
        const ACESODTParams&  params) const noexcept;

    // Convenience: AP1 (ACEScg) linear → sRGB via RRT+sRGB ODT in one pass.
    ACESODTResult ACEScgToSRGB(
        const uint16_t* srcFP16,
        uint32_t width, uint32_t height,
        float exposureAdjust = 0.0f) const noexcept;

    // Mathematical helpers
    static float ApplyRRT(float x) noexcept;                   // Segmented spline C5
    static void  AP0toAP1(float& r, float& g, float& b) noexcept;  // Matrix transform
    static void  AP1toSRGB(float& r, float& g, float& b) noexcept; // Matrix + sRGB
};

}} // namespace ExplorerLens::Engine
