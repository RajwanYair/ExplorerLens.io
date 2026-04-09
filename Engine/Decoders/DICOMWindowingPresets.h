// DICOMWindowingPresets.h — DICOM Window/Level Clinical Presets
// Copyright (c) 2026 ExplorerLens Project
//
// Applies standard clinical DICOM windowing presets (CT Lung, CT Bone, Brain,
// Abdomen, Angio) to raw Hounsfield Unit pixel data, producing a display-ready
// BGRA32 grayscale image. Supports linear, sigmoid, and centre/width look-up
// table (LUT) generation per DICOM PS 3.3 C.7.6.3.1.
//
#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace ExplorerLens { namespace Engine {

enum class DICOMWindowPreset : uint8_t {
    Custom      = 0,
    CTLung      = 1,   // W=1500 L=-500
    CTBone      = 2,   // W=2500 L=500
    Brain       = 3,   // W=80   L=40
    Abdomen     = 4,   // W=400  L=40
    Angiography = 5,   // W=600  L=300
    Spine       = 6,   // W=250  L=50
    SoftTissue  = 7,   // W=350  L=50
};

struct DICOMWindowParams {
    int32_t  windowWidth  = 400;  // HU range
    int32_t  windowCentre = 40;   // HU midpoint
    bool     invert       = false;
    bool     sigmoid      = false; // Sigmoid LUT instead of linear
};

struct DICOMApplyResult {
    std::vector<uint8_t> pixelsBGRA;
    uint32_t width   = 0;
    uint32_t height  = 0;
    bool     success = false;
};

class DICOMWindowingPresets {
public:
    DICOMWindowingPresets()  = default;
    ~DICOMWindowingPresets() = default;

    // Return window params for a named clinical preset.
    static DICOMWindowParams GetPreset(DICOMWindowPreset preset) noexcept;

    // Apply window/level mapping to 16-bit signed HU pixel buffer.
    DICOMApplyResult Apply(
        const int16_t*            huPixels,
        uint32_t                  width,
        uint32_t                  height,
        const DICOMWindowParams&  params) const noexcept;

    // Apply using a named preset.
    DICOMApplyResult ApplyPreset(
        const int16_t*  huPixels,
        uint32_t        width,
        uint32_t        height,
        DICOMWindowPreset preset) const noexcept;

    // Build a linear LUT mapping HU values to [0, 255].
    static std::vector<uint8_t> BuildLinearLUT(
        int32_t windowWidth, int32_t windowCentre, bool invert) noexcept;
};

}} // namespace ExplorerLens::Engine
