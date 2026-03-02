// ZstdFrameDecoder.h — Standalone Zstandard Frame Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Parses standalone .zst files to inspect Zstandard frame headers,
// extract original size, window size, and compression parameters.
// Provides archive-like thumbnail showing compression ratio.

#pragma once

#include <cstdint>
#include <cstring>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct ZstdFrameInfo {
    uint32_t magicNumber = 0;
    uint64_t frameContentSize = 0;
    uint32_t windowSize = 0;
    uint8_t  dictionaryId = 0;
    bool     hasChecksum = false;
    bool     singleSegment = false;
    double   compressionRatio = 0.0;
};

struct ZstdStats {
    uint32_t framesInspected = 0;
    uint64_t totalOriginalSize = 0;
    uint64_t totalCompressedSize = 0;
    double   avgCompressionRatio = 0.0;
};

class ZstdFrameDecoder {
public:
    ZstdFrameDecoder() = default;
    ~ZstdFrameDecoder() = default;

    static const wchar_t* GetName() { return L"ZstdFrameDecoder"; }

    bool CanDecode(const wchar_t* ext) const {
        if (!ext) return false;
        std::wstring e(ext);
        for (auto& c : e) c = towlower(c);
        return e == L".zst" || e == L".zstd";
    }

    /// Detect Zstandard magic: 0xFD2FB528
    bool DetectMagic(const uint8_t* data, size_t size) const {
        if (!data || size < 4) return false;
        uint32_t magic = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
        return magic == 0xFD2FB528;
    }

    /// Parse Zstandard frame header.
    ZstdFrameInfo ParseFrame(const uint8_t* data, size_t size) const {
        ZstdFrameInfo info;
        if (!DetectMagic(data, size) || size < 6) return info;

        info.magicNumber = 0xFD2FB528;
        uint8_t desc = data[4];
        info.hasChecksum = (desc & 0x04) != 0;
        info.singleSegment = (desc & 0x20) != 0;
        uint8_t fcsFlag = (desc >> 6) & 0x03;

        size_t offset = 5;
        if (!info.singleSegment && offset < size) {
            uint8_t wdByte = data[offset++];
            uint8_t exponent = wdByte >> 3;
            uint8_t mantissa = wdByte & 0x07;
            info.windowSize = (1ull << (10 + exponent)) + (static_cast<uint64_t>(mantissa) << (7 + exponent));
        }

        uint8_t didFlag = desc & 0x03;
        if (didFlag > 0 && offset < size) {
            info.dictionaryId = data[offset];
            offset += (1ull << (didFlag - 1));
        }

        if (fcsFlag > 0 && offset < size) {
            switch (fcsFlag) {
            case 1: info.frameContentSize = data[offset] + 256; break;
            case 2: if (offset + 1 < size) info.frameContentSize = data[offset] | (data[offset + 1] << 8); break;
            case 3: if (offset + 3 < size) info.frameContentSize = data[offset] | (data[offset + 1] << 8) |
                (static_cast<uint64_t>(data[offset + 2]) << 16) | (static_cast<uint64_t>(data[offset + 3]) << 24); break;
            }
        }

        if (info.frameContentSize > 0 && size > 0)
            info.compressionRatio = static_cast<double>(info.frameContentSize) / size;

        return info;
    }

    ZstdStats GetStats() const { return m_stats; }

private:
    mutable ZstdStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
