// SnappyFrameDecoder.h — Snappy Framing Format Inspector
// Copyright (c) 2026 ExplorerLens Project
//
// Parses .sz/.snappy files using Google's Snappy framing format.
// Extracts stream identifier, compressed/uncompressed chunks,
// and CRC-32C checksums for archive-style thumbnail.

#pragma once

#include <cstdint>
#include <cstring>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class SnappyChunkType : uint8_t {
    StreamIdentifier = 0xFF,
    CompressedData = 0x00,
    UncompressedData = 0x01,
    Padding = 0xFE,
    Reserved = 0x80
};

struct SnappyFrameInfo
{
    bool isValid = false;
    uint32_t compressedChunks = 0;
    uint32_t uncompressedChunks = 0;
    uint64_t compressedSize = 0;
    uint64_t estimatedOriginalSize = 0;
    bool hasStreamIdentifier = false;
};

struct SnappyStats
{
    uint32_t filesInspected = 0;
    uint32_t totalChunks = 0;
    uint64_t totalBytes = 0;
};

class SnappyFrameDecoder
{
  public:
    SnappyFrameDecoder() = default;
    ~SnappyFrameDecoder() = default;

    static const wchar_t* GetName()
    {
        return L"SnappyFrameDecoder";
    }

    bool CanDecode(const wchar_t* ext) const
    {
        if (!ext)
            return false;
        std::wstring e(ext);
        for (auto& c : e)
            c = towlower(c);
        return e == L".sz" || e == L".snappy" || e == L".snz";
    }

    /// Detect Snappy framing format stream identifier.
    bool DetectMagic(const uint8_t* data, size_t size) const
    {
        if (!data || size < 10)
            return false;
        // Stream identifier chunk: 0xFF followed by "sNaPpY"
        if (data[0] != 0xFF)
            return false;
        uint32_t chunkLen = data[1] | (data[2] << 8) | (data[3] << 16);
        if (chunkLen < 6 || 4 + chunkLen > size)
            return false;
        return memcmp(data + 4, "sNaPpY", 6) == 0;
    }

    /// Parse all chunks in the framing format.
    SnappyFrameInfo ParseFrames(const uint8_t* data, size_t size) const
    {
        SnappyFrameInfo info;
        info.compressedSize = size;

        if (!DetectMagic(data, size))
            return info;

        info.isValid = true;
        info.hasStreamIdentifier = true;

        size_t offset = 0;
        while (offset + 4 <= size) {
            uint8_t chunkType = data[offset];
            uint32_t chunkLen = data[offset + 1] | (data[offset + 2] << 8) | (data[offset + 3] << 16);

            if (chunkLen == 0 && chunkType != 0xFE)
                break;

            switch (static_cast<SnappyChunkType>(chunkType)) {
                case SnappyChunkType::StreamIdentifier:
                    break;
                case SnappyChunkType::CompressedData:
                    info.compressedChunks++;
                    break;
                case SnappyChunkType::UncompressedData:
                    info.uncompressedChunks++;
                    info.estimatedOriginalSize += (chunkLen > 4) ? (chunkLen - 4) : 0;
                    break;
                default:
                    break;
            }

            offset += 4 + chunkLen;
            if (offset > size)
                break;
        }

        // Estimate: Snappy typical ratio ~1.5:1 to 2.5:1
        if (info.estimatedOriginalSize == 0 && info.compressedChunks > 0)
            info.estimatedOriginalSize = size * 2;

        return info;
    }

    SnappyStats GetStats() const
    {
        return m_stats;
    }

  private:
    mutable SnappyStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
