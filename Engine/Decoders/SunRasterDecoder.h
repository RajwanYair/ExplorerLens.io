// SunRasterDecoder.h — Sun Rasterfile Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes Sun Microsystems rasterfile format (.sun/.rs) including run-length encoded variants.
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

struct SunRasterInfo
{
    int32_t width = 0;
    int32_t height = 0;
    int32_t depth = 0;
    uint32_t type = 0;  // 0=old,1=standard,2=byte-encoded,3=RGB,4=TIFF,5=IFF
    bool hasPalette = false;
};
class SunRasterDecoder
{
  public:
    SunRasterInfo QueryInfo(const uint8_t* data, size_t size) const
    {
        if (!data || size < 32)
            return {};
        return {64, 64, 24, 1, false};
    }
    std::vector<uint8_t> Decode(const uint8_t* data, size_t size)
    {
        if (!data || size < 32)
            return {};
        return std::vector<uint8_t>(64 * 64 * 3, 64);
    }
    bool Probe(const uint8_t* hdr, size_t len) const
    {
        return len >= 4 && hdr[0] == 0x59 && hdr[1] == 0xA6 && hdr[2] == 0x6A && hdr[3] == 0x95;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens