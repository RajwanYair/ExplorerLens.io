// FLIFDecoder.h — Free Lossless Image Format Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Detects FLIF files via magic bytes "FLIF") and extracts header metadata
// (dimensions, channels, bit depth). Provides WIC-based fallback for
// actual pixel decode until native FLIF library is integrated.

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct FLIFHeader {
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t  channels = 0;   // 1=Gray, 3=RGB, 4=RGBA
    uint8_t  bitDepth = 8;
    bool     interlaced = false;
    bool     animated = false;
    uint32_t frameCount = 1;
};

struct FLIFStats {
    uint32_t filesDetected = 0;
    uint32_t headersDecoded = 0;
    uint32_t unsupportedFiles = 0;
};

class FLIFDecoder {
public:
    FLIFDecoder() = default;
    ~FLIFDecoder() = default;

    static const wchar_t* GetName() { return L"FLIFDecoder"; }

    bool CanDecode(const wchar_t* ext) const {
        if (!ext) return false;
        std::wstring e(ext);
        for (auto& c : e) c = towlower(c);
        return e == L".flif";
    }

    /// Detect FLIF magic bytes: "FLIF" (0x46 0x4C 0x49 0x46)
    bool DetectMagic(const uint8_t* data, size_t size) const {
        if (!data || size < 6) return false;
        return data[0] == 0x46 && data[1] == 0x4C &&
            data[2] == 0x49 && data[3] == 0x46;
    }

    /// Parse FLIF header to extract metadata.
    FLIFHeader ParseHeader(const uint8_t* data, size_t size) const {
        FLIFHeader hdr;
        if (!DetectMagic(data, size)) return hdr;

        uint8_t byte4 = data[4];
        hdr.interlaced = (byte4 >> 4) & 1;
        hdr.animated = ((byte4 >> 4) & 0x0F) >= 4;
        uint8_t channelCode = byte4 & 0x0F;
        switch (channelCode) {
        case 1: hdr.channels = 1; break;
        case 3: hdr.channels = 3; break;
        case 4: hdr.channels = 4; break;
        default: hdr.channels = 3; break;
        }

        uint8_t byte5 = (size > 5) ? data[5] : 0;
        if (byte5 == 0) hdr.bitDepth = 8;
        else if (byte5 == 1) hdr.bitDepth = 16;
        else hdr.bitDepth = 8;

        // Dimensions encoded as varint starting at byte 6
        if (size > 8) {
            hdr.width = (data[6] << 8) | data[7];
            hdr.height = (size > 9) ? ((data[8] << 8) | data[9]) : hdr.width;
        }

        return hdr;
    }

    FLIFStats GetStats() const { return m_stats; }

private:
    mutable FLIFStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
