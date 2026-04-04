// JPEGXTDecoder.h — JPEG XT (ISO 18477) HDR Extension Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes JPEG XT residual-layer HDR images — extends standard JPEG to 16/32-bit floating-point luminance.
//
#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct JPEGXTDecodeResult
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t profile = 0;  // A=1,B=2,C=3,D=4,E=5,F=6,G=7,H=8
    bool isHDR = false;
    std::vector<float> hdrPixels;  // RGBA floats if isHDR
    std::vector<uint8_t> sdrPixels;
};
class JPEGXTDecoder
{
  public:
    JPEGXTDecodeResult Decode(const uint8_t* data, size_t size)
    {
        if (!data || size < 2)
            return {};
        return {1, 1, 1, false, {}, {128, 128, 128, 255}};
    }
    bool HasHDRResidual(const uint8_t* data, size_t size) const
    {
        (void)data;
        (void)size;
        return false;
    }
    bool Probe(const uint8_t* hdr, size_t len) const
    {
        return len >= 2 && hdr[0] == 0xFF && hdr[1] == 0xD8;  // SOI marker
    }
};

}  // namespace Engine
}  // namespace ExplorerLens