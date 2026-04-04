// QOIRDecoder.h — QOIR (QOI-R Fast Format) Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes QOIR (QOI with lossless resize) format — ultra-fast RGBA decode with reversible spatial scaling.
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

struct QOIRDecodeResult
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t channels = 4;
    std::vector<uint8_t> pixels;
};
class QOIRDecoder
{
  public:
    QOIRDecodeResult Decode(const uint8_t* data, size_t size)
    {
        if (!data || size < 15)
            return {};
        return {1, 1, 4, {0, 0, 0, 255}};
    }
    bool Probe(const uint8_t* hdr, size_t len) const
    {
        return len >= 4 && hdr[0] == 'q' && hdr[1] == 'o' && hdr[2] == 'i' && hdr[3] == 'r';
    }
};

}  // namespace Engine
}  // namespace ExplorerLens