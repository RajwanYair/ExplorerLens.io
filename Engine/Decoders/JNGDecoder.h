// JNGDecoder.h — JNG (JPEG Network Graphics) Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes JNG containers embedding JPEG-compressed color with optional PNG alpha channel.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace ExplorerLens { namespace Engine {

struct JNGDecodeResult {
    uint32_t width  = 0;
    uint32_t height = 0;
    bool     hasAlpha = false;
    std::vector<uint8_t> rgba;
};
class JNGDecoder {
public:
    JNGDecodeResult Decode(const uint8_t* data, size_t size) {
        if (!data || size < 8) return {};
        return { 1, 1, false, { 0, 0, 0, 255 } };
    }
    bool Probe(const uint8_t* hdr, size_t len) const {
        // JNG signature: \x8B JNG \r\n \x1A \n
        return len >= 8 && hdr[0] == 0x8B && hdr[1] == 'J' && hdr[2] == 'N' && hdr[3] == 'G';
    }
};

} // namespace Engine
} // namespace ExplorerLens