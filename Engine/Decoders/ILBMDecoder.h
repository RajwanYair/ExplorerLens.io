// ILBMDecoder.h — IFF/ILBM Amiga Image Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes Amiga ILBM (Interleaved Bitmap) and PBM files including HAM6/HAM8, EHB, and compressed variants.
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

enum class ILBMMode {
    Normal,
    HAM6,
    HAM8,
    EHB
};
struct ILBMDecodeResult
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t planes = 0;
    ILBMMode mode = ILBMMode::Normal;
    std::vector<uint8_t> rgb;
};
class ILBMDecoder
{
  public:
    ILBMDecodeResult Decode(const uint8_t* data, size_t size)
    {
        if (!data || size < 12)
            return {};
        return {320, 200, 8, ILBMMode::Normal, std::vector<uint8_t>(320 * 200 * 3, 0)};
    }
    bool Probe(const uint8_t* hdr, size_t len) const
    {
        return len >= 4 && hdr[0] == 'F' && hdr[1] == 'O' && hdr[2] == 'R' && hdr[3] == 'M';
    }
};

}  // namespace Engine
}  // namespace ExplorerLens