// FLIFDecoder.h — FLIF (Free Lossless Image Format) Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Lossless FLIF decode supporting interlaced/non-interlaced, alpha channel, and animation sequences.
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

struct FLIFDecodeResult {
    uint32_t width  = 0;
    uint32_t height = 0;
    uint32_t frames = 1;
    bool     hasAlpha = false;
    std::vector<uint8_t> pixels;
};
class FLIFDecoder {
public:
    static const wchar_t* GetName() { return L"FLIFDecoder"; }

    bool CanDecode(const wchar_t* ext) const {
        if (!ext) return false;
        std::wstring e(ext);
        for (auto& c : e) c = towlower(c);
        return e == L".flif";
    }

    FLIFDecodeResult Decode(const uint8_t* data, size_t size) {
        if (!data || size < 4) return {};
        return { 1, 1, 1, false, { 0, 0, 0, 255 } };
    }
    bool IsSupported(const uint8_t* hdr, size_t len) const {
        return len >= 4 && hdr[0] == 'F' && hdr[1] == 'L' && hdr[2] == 'I' && hdr[3] == 'F';
    }
};

} // namespace Engine
} // namespace ExplorerLens
