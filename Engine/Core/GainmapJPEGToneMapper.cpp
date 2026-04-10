// GainmapJPEGToneMapper.cpp — Google Ultra HDR Gainmap JPEG Tone Mapper
// Copyright (c) 2026 ExplorerLens Project
//
#include "Core/GainmapJPEGToneMapper.h"
#include <chrono>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

struct GainmapJPEGToneMapper::Impl {};

GainmapJPEGToneMapper::GainmapJPEGToneMapper()
    : m_impl(std::make_unique<Impl>()) {}
GainmapJPEGToneMapper::~GainmapJPEGToneMapper() = default;

// XMP gainmap marker signature search (ISO 21496-1 draft uses XMP_SIGNATURE in APP1).
static const char k_xmpGainmapSig[] = "http://ns.adobe.com/hdr-gain-map/1.0/";
static const char k_ultraHDRSig[]   = "hdrgm:Version=";

bool GainmapJPEGToneMapper::IsUltraHDR(const uint8_t* jpegData, size_t jpegSize) noexcept
{
    if (!jpegData || jpegSize < 4) return false;
    // Quick magic check — JPEG SOI marker
    if (jpegData[0] != 0xFF || jpegData[1] != 0xD8) return false;

    // Scan APP1 markers for Ultra HDR XMP signature.
    size_t pos = 2;
    while (pos + 4 <= jpegSize) {
        if (jpegData[pos] != 0xFF) break;
        const uint8_t marker  = jpegData[pos + 1];
        const uint16_t segLen = static_cast<uint16_t>(
            (jpegData[pos + 2] << 8) | jpegData[pos + 3]);
        pos += 2;
        if (marker == 0xE1) {  // APP1
            const size_t dataStart = pos + 2;
            const size_t dataEnd   = std::min(dataStart + segLen - 2, jpegSize);
            const char* seg = reinterpret_cast<const char*>(jpegData + dataStart);
            const size_t segSize = dataEnd - dataStart;
            for (size_t i = 0; i + sizeof(k_ultraHDRSig) < segSize; ++i) {
                if (std::memcmp(seg + i, k_ultraHDRSig, sizeof(k_ultraHDRSig) - 1) == 0)
                    return true;
            }
        }
        if (segLen < 2) break;
        pos += segLen;
    }
    return false;
}

bool GainmapJPEGToneMapper::ParseGainmapMetadata(
    const uint8_t* jpegData, size_t jpegSize, GainmapMetadata& out) const noexcept
{
    if (!IsUltraHDR(jpegData, jpegSize)) return false;
    // Populate with typical Android/Pixel default values.
    out.gainmapMin      = 0.0f;
    out.gainmapMax      = 1.0f;
    out.hdrCapacityMin  = 0.0f;
    out.hdrCapacityMax  = std::log2(1000.0f / 100.0f);  // 1000/100 nit ratio
    out.offsetSdr       = 1.0f / 64.0f;
    out.offsetHdr       = 1.0f / 64.0f;
    out.isValid         = true;
    return true;
}

GainmapToneMapResult GainmapJPEGToneMapper::ToneMap(
    const GainmapToneMapRequest& req) const noexcept
{
    GainmapToneMapResult result{};
    if (!req.baseBGRA || req.width == 0 || req.height == 0) return result;

    const auto t0 = std::chrono::high_resolution_clock::now();

    const size_t bufSize = static_cast<size_t>(req.width) * req.height * 4u;
    result.outputBGRA = new (std::nothrow) uint8_t[bufSize];
    if (!result.outputBGRA) return result;

    const float boost = std::max(1.0f, req.displayBoost);
    const GainmapMetadata& meta = req.metadata;

    for (uint32_t i = 0; i < req.width * req.height; ++i) {
        const uint8_t b = req.baseBGRA[i * 4 + 0];
        const uint8_t g = req.baseBGRA[i * 4 + 1];
        const uint8_t r = req.baseBGRA[i * 4 + 2];
        const uint8_t a = req.baseBGRA[i * 4 + 3];

        float gainFactor = 1.0f;
        if (req.gainmapGray) {
            // Map gainmap luma [0,255] → gainmap log2 value.
            const float gmap = req.gainmapGray[i] / 255.0f;
            const float logGain = meta.gainmapMin + gmap * (meta.gainmapMax - meta.gainmapMin);
            gainFactor = std::pow(2.0f, logGain * std::log2(boost));
        }

        auto apply = [&](uint8_t ch) -> uint8_t {
            float v = ch / 255.0f * gainFactor;
            v = std::min(v, 1.0f);
            return static_cast<uint8_t>(v * 255.0f + 0.5f);
        };

        result.outputBGRA[i * 4 + 0] = apply(b);
        result.outputBGRA[i * 4 + 1] = apply(g);
        result.outputBGRA[i * 4 + 2] = apply(r);
        result.outputBGRA[i * 4 + 3] = a;
    }

    result.success = true;
    const auto t1 = std::chrono::high_resolution_clock::now();
    result.processMs = std::chrono::duration<float, std::milli>(t1 - t0).count();
    return result;
}

}} // namespace ExplorerLens::Engine
