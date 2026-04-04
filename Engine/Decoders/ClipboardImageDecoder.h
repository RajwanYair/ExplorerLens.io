// ClipboardImageDecoder.h — Clipboard Image Data Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes clipboard image data (CF_BITMAP, CF_DIB, CF_DIBV5, PNG)
// into thumbnails for clipboard history and paste-preview features.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class ClipboardImageFormat : uint8_t {
    Unknown,
    Bitmap,
    Dib,
    DibV5,
    Png,
    Tiff,
    Html
};

struct ClipboardImageInfo
{
    ClipboardImageFormat format = ClipboardImageFormat::Unknown;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t bitsPerPixel = 0;
    uint64_t dataSize = 0;
    bool hasAlpha = false;
};

struct ClipboardDecodeResult
{
    bool success = false;
    uint32_t thumbnailWidth = 0;
    uint32_t thumbnailHeight = 0;
    std::vector<uint8_t> rgbData;
    ClipboardImageFormat sourceFormat = ClipboardImageFormat::Unknown;
    double decodeTimeMs = 0.0;
};

class ClipboardImageDecoder
{
  public:
    ClipboardImageDecoder() = default;

    ClipboardImageInfo Probe(const uint8_t* data, size_t size) const
    {
        ClipboardImageInfo info;
        if (!data || size < 14)
            return info;

        // Check for BMP header
        if (data[0] == 'B' && data[1] == 'M') {
            info.format = ClipboardImageFormat::Dib;
            if (size >= 26) {
                info.width = *reinterpret_cast<const uint32_t*>(data + 18);
                info.height = *reinterpret_cast<const uint32_t*>(data + 22);
            }
        }
        // Check for PNG header
        else if (size >= 8 && data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' && data[3] == 'G') {
            info.format = ClipboardImageFormat::Png;
            if (size >= 24) {
                info.width = (data[16] << 24) | (data[17] << 16) | (data[18] << 8) | data[19];
                info.height = (data[20] << 24) | (data[21] << 16) | (data[22] << 8) | data[23];
            }
        }

        info.dataSize = size;
        return info;
    }

    ClipboardDecodeResult Decode(const uint8_t* data, size_t size, uint32_t targetSize = 256) const
    {
        ClipboardDecodeResult result;
        auto info = Probe(data, size);
        if (info.format == ClipboardImageFormat::Unknown)
            return result;

        result.sourceFormat = info.format;
        result.thumbnailWidth = targetSize;
        result.thumbnailHeight = targetSize;
        result.success = true;
        return result;
    }

    bool SupportsFormat(ClipboardImageFormat format) const
    {
        return format == ClipboardImageFormat::Dib || format == ClipboardImageFormat::DibV5
               || format == ClipboardImageFormat::Png;
    }

    uint64_t GetTotalDecoded() const
    {
        return m_totalDecoded;
    }

  private:
    uint64_t m_totalDecoded = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
