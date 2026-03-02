// LZ4FrameDecoder.h — LZ4 Frame Format Inspector
// Copyright (c) 2026 ExplorerLens Project
//
// Parses standalone .lz4 files using LZ4 Frame Format specification.
// Extracts magic number, frame descriptor, block size, content size,
// and checksum info for archive thumbnail display.

#pragma once

#include <cstdint>
#include <cstring>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct LZ4FrameInfo {
    uint32_t magicNumber = 0;
    uint8_t  version = 0;
    bool     blockIndependence = false;
    bool     blockChecksum = false;
    bool     contentSize = false;
    bool     contentChecksum = false;
    uint8_t  blockMaxSize = 0; // 4=64KB, 5=256KB, 6=1MB, 7=4MB
    uint64_t originalContentSize = 0;
    uint32_t dictionaryId = 0;
    bool     isValid = false;
};

struct LZ4Stats {
    uint32_t framesInspected = 0;
    uint64_t totalOriginalBytes = 0;
    uint64_t totalCompressedBytes = 0;
};

class LZ4FrameDecoder {
public:
    LZ4FrameDecoder() = default;
    ~LZ4FrameDecoder() = default;

    static const wchar_t* GetName() { return L"LZ4FrameDecoder"; }

    bool CanDecode(const wchar_t* ext) const {
        if (!ext) return false;
        std::wstring e(ext);
        for (auto& c : e) c = towlower(c);
        return e == L".lz4";
    }

    /// Detect LZ4 frame magic: 0x184D2204
    bool DetectMagic(const uint8_t* data, size_t size) const {
        if (!data || size < 4) return false;
        uint32_t magic = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
        return magic == 0x184D2204;
    }

    /// Parse LZ4 frame descriptor.
    LZ4FrameInfo ParseFrame(const uint8_t* data, size_t size) const {
        LZ4FrameInfo info;
        if (!DetectMagic(data, size) || size < 7) return info;

        info.magicNumber = 0x184D2204;
        uint8_t flg = data[4];
        uint8_t bd = data[5];

        info.version = (flg >> 6) & 0x03;
        info.blockIndependence = (flg & 0x20) != 0;
        info.blockChecksum = (flg & 0x10) != 0;
        info.contentSize = (flg & 0x08) != 0;
        info.contentChecksum = (flg & 0x04) != 0;

        info.blockMaxSize = (bd >> 4) & 0x07;

        size_t offset = 6;
        if (info.contentSize && offset + 8 <= size) {
            memcpy(&info.originalContentSize, data + offset, 8);
            offset += 8;
        }

        info.isValid = (info.version == 1);
        return info;
    }

    /// Get human-readable block size string.
    const wchar_t* GetBlockSizeString(uint8_t blockMaxSize) const {
        switch (blockMaxSize) {
        case 4: return L"64 KB";
        case 5: return L"256 KB";
        case 6: return L"1 MB";
        case 7: return L"4 MB";
        default: return L"Unknown";
        }
    }

    LZ4Stats GetStats() const { return m_stats; }

private:
    mutable LZ4Stats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
