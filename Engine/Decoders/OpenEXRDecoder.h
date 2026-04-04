// OpenEXRDecoder.h — ILM OpenEXR Multi-Layer HDR Image Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// ILM OpenEXR multi-layer HDR image decoder. Parses EXR magic bytes, extracts
// first displayable layer, and tonemaps to LDR for thumbnail generation.
//
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class EXRCompression : uint8_t {
    None = 0,
    RLE = 1,
    ZIPS = 2,
    ZIP = 3,
    PIZ = 4,
    PXR24 = 5,
    B44 = 6,
    B44A = 7,
    DWAA = 8,
    DWAB = 9,
    Unknown = 255
};

enum class EXRPixelType : uint8_t {
    UInt = 0,
    Half = 1,
    Float = 2,
    Unknown = 255
};

struct EXRChannelInfo
{
    std::string name;
    EXRPixelType type = EXRPixelType::Half;
    uint32_t xSampling = 1;
    uint32_t ySampling = 1;
};

struct EXRHeader
{
    uint32_t magic = 0;
    uint32_t version = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    EXRCompression compression = EXRCompression::Unknown;
    std::vector<EXRChannelInfo> channels;
    bool isTiled = false;
    bool isMultiPart = false;
    bool isValid = false;
};

class OpenEXRDecoder
{
  public:
    static constexpr uint32_t EXR_MAGIC = 0x01312F76;

    static OpenEXRDecoder& Instance()
    {
        static OpenEXRDecoder instance;
        return instance;
    }

    inline bool IsEXRFile(const uint8_t* data, size_t size) const
    {
        if (!data || size < 4)
            return false;
        uint32_t magic = static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8)
                         | (static_cast<uint32_t>(data[2]) << 16) | (static_cast<uint32_t>(data[3]) << 24);
        return magic == EXR_MAGIC;
    }

    inline EXRHeader ParseHeader(const uint8_t* data, size_t size) const
    {
        EXRHeader header;
        if (!data || size < 8)
            return header;

        header.magic = ReadU32LE(data, 0);
        if (header.magic != EXR_MAGIC)
            return header;

        header.version = ReadU32LE(data, 4);
        header.isTiled = (header.version & 0x200) != 0;
        header.isMultiPart = (header.version & 0x1000) != 0;
        header.isValid = true;

        size_t offset = 8;
        while (offset + 1 < size && data[offset] != 0) {
            auto [attrName, nextPos] = ReadNullTermString(data, size, offset);
            offset = nextPos;
            if (offset >= size)
                break;

            auto [attrType, nextPos2] = ReadNullTermString(data, size, offset);
            offset = nextPos2;
            if (offset + 4 > size)
                break;

            uint32_t attrSize = ReadU32LE(data, offset);
            offset += 4;

            if (attrName == "compression" && attrSize >= 1 && offset < size) {
                header.compression = static_cast<EXRCompression>(data[offset]);
            } else if (attrName == "dataWindow" && attrSize >= 16 && offset + 16 <= size) {
                int32_t xMin = static_cast<int32_t>(ReadU32LE(data, offset));
                int32_t yMin = static_cast<int32_t>(ReadU32LE(data, offset + 4));
                int32_t xMax = static_cast<int32_t>(ReadU32LE(data, offset + 8));
                int32_t yMax = static_cast<int32_t>(ReadU32LE(data, offset + 12));
                header.width = static_cast<uint32_t>(xMax - xMin + 1);
                header.height = static_cast<uint32_t>(yMax - yMin + 1);
            }

            offset += attrSize;
        }
        return header;
    }

    inline std::vector<uint8_t> ToneMapToLDR(const float* hdrData, uint32_t width, uint32_t height,
                                             uint32_t channels) const
    {
        size_t pixelCount = static_cast<size_t>(width) * height;
        std::vector<uint8_t> ldr(pixelCount * 3, 0);
        if (!hdrData || pixelCount == 0)
            return ldr;

        float lumAvg = ComputeLogAverageLuminance(hdrData, width, height, channels);
        float exposure = 0.18f / (lumAvg + 0.001f);

        for (size_t i = 0; i < pixelCount; ++i) {
            size_t src = i * channels;
            float r = hdrData[src] * exposure;
            float g = channels > 1 ? hdrData[src + 1] * exposure : r;
            float b = channels > 2 ? hdrData[src + 2] * exposure : r;

            r = r / (1.0f + r);
            g = g / (1.0f + g);
            b = b / (1.0f + b);

            auto toSRGB = [](float linear) -> uint8_t {
                float srgb = linear <= 0.0031308f ? linear * 12.92f : 1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f;
                return static_cast<uint8_t>((std::max)(0.0f, (std::min)(1.0f, srgb)) * 255.0f + 0.5f);
            };

            size_t dst = i * 3;
            ldr[dst + 0] = toSRGB(r);
            ldr[dst + 1] = toSRGB(g);
            ldr[dst + 2] = toSRGB(b);
        }
        return ldr;
    }

    inline std::string CompressionToString(EXRCompression comp) const
    {
        switch (comp) {
            case EXRCompression::None:
                return "None";
            case EXRCompression::RLE:
                return "RLE";
            case EXRCompression::ZIPS:
                return "ZIPS";
            case EXRCompression::ZIP:
                return "ZIP";
            case EXRCompression::PIZ:
                return "PIZ";
            case EXRCompression::PXR24:
                return "PXR24";
            case EXRCompression::B44:
                return "B44";
            case EXRCompression::B44A:
                return "B44A";
            case EXRCompression::DWAA:
                return "DWAA";
            case EXRCompression::DWAB:
                return "DWAB";
            default:
                return "Unknown";
        }
    }

  private:
    OpenEXRDecoder() = default;

    inline uint32_t ReadU32LE(const uint8_t* data, size_t offset) const
    {
        return static_cast<uint32_t>(data[offset]) | (static_cast<uint32_t>(data[offset + 1]) << 8)
               | (static_cast<uint32_t>(data[offset + 2]) << 16) | (static_cast<uint32_t>(data[offset + 3]) << 24);
    }

    inline std::pair<std::string, size_t> ReadNullTermString(const uint8_t* data, size_t size, size_t offset) const
    {
        std::string result;
        while (offset < size && data[offset] != 0) {
            result.push_back(static_cast<char>(data[offset++]));
        }
        if (offset < size)
            ++offset;
        return {result, offset};
    }

    inline float ComputeLogAverageLuminance(const float* data, uint32_t width, uint32_t height, uint32_t channels) const
    {
        size_t pixelCount = static_cast<size_t>(width) * height;
        double logSum = 0.0;
        size_t count = 0;
        for (size_t i = 0; i < pixelCount; ++i) {
            size_t idx = i * channels;
            float lum = 0.2126f * data[idx];
            if (channels > 1)
                lum += 0.7152f * data[idx + 1];
            if (channels > 2)
                lum += 0.0722f * data[idx + 2];
            if (lum > 0.0001f) {
                logSum += std::log(lum + 0.0001);
                ++count;
            }
        }
        return count > 0 ? static_cast<float>(std::exp(logSum / count)) : 0.18f;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
