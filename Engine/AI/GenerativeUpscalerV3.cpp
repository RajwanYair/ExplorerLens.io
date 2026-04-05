// GenerativeUpscalerV3.cpp — Generative AI Upscaler v3
// Copyright (c) 2026 ExplorerLens Project
//
#include "GenerativeUpscalerV3.h"

namespace ExplorerLens::Engine {

bool GenerativeUpscalerV3::Upscale(const void*, uint32_t, uint32_t, void*, const UpscaleJob&)
{
    return false;
}
std::pair<uint32_t, uint32_t> GenerativeUpscalerV3::GetOutputDimensions(uint32_t w, uint32_t h, UpscaleFactor factor) const
{
    uint32_t scale = 1;
    switch (factor) {
        case UpscaleFactor::x2: scale = 2; break;
        case UpscaleFactor::x3: scale = 3; break;
        case UpscaleFactor::x4: scale = 4; break;
        case UpscaleFactor::x8: scale = 8; break;
        default: break;
    }
    return {w * scale, h * scale};
}
UpscaleModel GenerativeUpscalerV3::SelectBestModel(uint32_t, uint32_t) const
{
    return {};
}
uint64_t GenerativeUpscalerV3::GetVRAMRequiredBytes(const UpscaleJob&) const
{
    return 0;
}
bool GenerativeUpscalerV3::IsModelAvailable(UpscaleModel) const
{
    return false;
}

}  // namespace ExplorerLens::Engine
