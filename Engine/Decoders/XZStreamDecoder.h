// XZStreamDecoder.h — XZ/LZMA Compressed Stream Inspector
// Copyright (c) 2026 ExplorerLens Project
//
// Parses .xz files using the XZ container format specification.
// Extracts stream header magic, flags, check type, and block info.
// Also handles raw .lzma files via LZMA alone header detection.

#pragma once

#include <cstdint>
#include <cstring>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class XZCheckType : uint8_t {
    None = 0x00,
    CRC32 = 0x01,
    CRC64 = 0x04,
    SHA256 = 0x0A,
    Unknown = 0xFF
};

struct XZStreamInfo
{
    bool isXZ = false;
    bool isLZMA = false;
    XZCheckType checkType = XZCheckType::Unknown;
    uint64_t compressedSize = 0;
    uint64_t uncompressedSize = 0;
    uint32_t blockCount = 0;
    uint8_t dictionaryBits = 0;
    bool isValid = false;
};

struct XZStats
{
    uint32_t streamsInspected = 0;
    uint64_t totalCompressed = 0;
    uint64_t totalUncompressed = 0;
};

class XZStreamDecoder
{
  public:
    XZStreamDecoder() = default;
    ~XZStreamDecoder() = default;

    static const wchar_t* GetName()
    {
        return L"XZStreamDecoder";
    }

    bool CanDecode(const wchar_t* ext) const
    {
        if (!ext)
            return false;
        std::wstring e(ext);
        for (auto& c : e)
            c = towlower(c);
        return e == L".xz" || e == L".lzma" || e == L".txz";
    }

    /// Detect XZ magic: FD 37 7A 58 5A 00 (6 bytes)
    bool DetectXZMagic(const uint8_t* data, size_t size) const
    {
        if (!data || size < 6)
            return false;
        static const uint8_t xzMagic[] = {0xFD, 0x37, 0x7A, 0x58, 0x5A, 0x00};
        return memcmp(data, xzMagic, 6) == 0;
    }

    /// Detect raw LZMA magic (properties byte + dictionary size).
    bool DetectLZMAMagic(const uint8_t* data, size_t size) const
    {
        if (!data || size < 13)
            return false;
        // LZMA properties byte: lc + lp*9 + pb*9*5, typically 0x5D
        uint8_t props = data[0];
        return (props <= 0xE1);  // Valid range for LZMA properties
    }

    /// Parse XZ stream header.
    XZStreamInfo ParseStream(const uint8_t* data, size_t size) const
    {
        XZStreamInfo info;
        info.compressedSize = size;

        if (DetectXZMagic(data, size)) {
            info.isXZ = true;
            info.isValid = true;
            // Stream flags at bytes 6-7
            if (size >= 8) {
                uint8_t checkByte = data[7] & 0x0F;
                switch (checkByte) {
                    case 0x00:
                        info.checkType = XZCheckType::None;
                        break;
                    case 0x01:
                        info.checkType = XZCheckType::CRC32;
                        break;
                    case 0x04:
                        info.checkType = XZCheckType::CRC64;
                        break;
                    case 0x0A:
                        info.checkType = XZCheckType::SHA256;
                        break;
                    default:
                        info.checkType = XZCheckType::Unknown;
                        break;
                }
            }
            info.blockCount = 1;
        } else if (DetectLZMAMagic(data, size)) {
            info.isLZMA = true;
            info.isValid = true;
            // Dictionary size at bytes 1-4
            if (size >= 5) {
                uint32_t dictSize = data[1] | (data[2] << 8) | (data[3] << 16) | (data[4] << 24);
                uint8_t bits = 0;
                while ((1u << bits) < dictSize && bits < 32)
                    bits++;
                info.dictionaryBits = bits;
            }
            // Uncompressed size at bytes 5-12
            if (size >= 13) {
                memcpy(&info.uncompressedSize, data + 5, 8);
            }
        }
        return info;
    }

    XZStats GetStats() const
    {
        return m_stats;
    }

  private:
    mutable XZStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
