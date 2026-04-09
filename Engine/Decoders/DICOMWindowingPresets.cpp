// DICOMWindowingPresets.cpp — DICOM Window/Level Clinical Presets
// Copyright (c) 2026 ExplorerLens Project
//
#include "Decoders/DICOMWindowingPresets.h"
#include <algorithm>
#include <cmath>

namespace ExplorerLens { namespace Engine {

DICOMWindowParams DICOMWindowingPresets::GetPreset(DICOMWindowPreset preset) noexcept
{
    switch (preset) {
    case DICOMWindowPreset::CTLung:      return { 1500, -500 };
    case DICOMWindowPreset::CTBone:      return { 2500,  500 };
    case DICOMWindowPreset::Brain:       return {   80,   40 };
    case DICOMWindowPreset::Abdomen:     return {  400,   40 };
    case DICOMWindowPreset::Angiography: return {  600,  300 };
    case DICOMWindowPreset::Spine:       return {  250,   50 };
    case DICOMWindowPreset::SoftTissue:  return {  350,   50 };
    default:                             return {  400,   40 };
    }
}

std::vector<uint8_t> DICOMWindowingPresets::BuildLinearLUT(
    int32_t windowWidth, int32_t windowCentre, bool invert) noexcept
{
    // Build a 65536-entry LUT for the full 16-bit HU range [-32768, 32767].
    // Index into the LUT = HU + 32768.
    std::vector<uint8_t> lut(65536, 0);
    const float ww = static_cast<float>(std::max(1, windowWidth));
    const float wc = static_cast<float>(windowCentre);
    const float lo = wc - ww * 0.5f;
    const float hi = wc + ww * 0.5f;

    for (int32_t hu = -32768; hu <= 32767; ++hu) {
        float v = (static_cast<float>(hu) - lo) / (hi - lo);
        v = std::clamp(v, 0.0f, 1.0f);
        if (invert) v = 1.0f - v;
        lut[static_cast<uint32_t>(hu + 32768)] = static_cast<uint8_t>(v * 255.0f + 0.5f);
    }
    return lut;
}

DICOMApplyResult DICOMWindowingPresets::Apply(
    const int16_t*           huPixels,
    uint32_t                 width,
    uint32_t                 height,
    const DICOMWindowParams& params) const noexcept
{
    DICOMApplyResult result{};
    if (!huPixels || width == 0 || height == 0) return result;

    result.width  = width;
    result.height = height;
    result.pixelsBGRA.resize(static_cast<size_t>(width) * height * 4u);

    const auto lut = params.sigmoid
        ? BuildLinearLUT(params.windowWidth, params.windowCentre, params.invert)
        : BuildLinearLUT(params.windowWidth, params.windowCentre, params.invert);

    uint8_t* dst = result.pixelsBGRA.data();
    for (uint32_t i = 0; i < width * height; ++i) {
        const uint32_t idx = static_cast<uint32_t>(static_cast<int32_t>(huPixels[i]) + 32768);
        const uint8_t  g   = lut[idx];
        dst[0] = g; dst[1] = g; dst[2] = g; dst[3] = 0xFF;
        dst += 4;
    }
    result.success = true;
    return result;
}

DICOMApplyResult DICOMWindowingPresets::ApplyPreset(
    const int16_t*  huPixels,
    uint32_t        width,
    uint32_t        height,
    DICOMWindowPreset preset) const noexcept
{
    return Apply(huPixels, width, height, GetPreset(preset));
}

}} // namespace ExplorerLens::Engine
