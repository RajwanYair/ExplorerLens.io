// RealTimeCodecTranscoder.h — On-the-Fly Format Transcoding
// Copyright (c) 2026 ExplorerLens Project
//
// On-the-fly format transcoding for preview. Converts between image formats
// in-memory (e.g., HEIF to BGRA, AVIF to BGRA) with codec negotiation.
//
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class CodecPixelFormat : uint8_t {
    RGBA8,
    BGRA8,
    RGB8,
    BGR8,
    Gray8,
    Gray16,
    RGBAFloat,
    Unknown
};

enum class TranscodeQuality : uint8_t {
    Fastest,
    Balanced,
    HighQuality
};

struct TranscodeRequest
{
    CodecPixelFormat sourceFormat = CodecPixelFormat::Unknown;
    CodecPixelFormat targetFormat = CodecPixelFormat::BGRA8;
    uint32_t width = 0;
    uint32_t height = 0;
    TranscodeQuality quality = TranscodeQuality::Balanced;
};

struct TranscodeResult
{
    std::vector<uint8_t> data;
    CodecPixelFormat format = CodecPixelFormat::Unknown;
    uint32_t width = 0;
    uint32_t height = 0;
    bool success = false;
    std::string errorMessage;
};

class RealTimeCodecTranscoder
{
  public:
    static RealTimeCodecTranscoder& Instance()
    {
        static RealTimeCodecTranscoder instance;
        return instance;
    }

    inline TranscodeResult Transcode(const uint8_t* srcData, size_t srcSize, const TranscodeRequest& request) const
    {
        TranscodeResult result;
        result.width = request.width;
        result.height = request.height;

        if (!srcData || srcSize == 0 || request.width == 0 || request.height == 0) {
            result.errorMessage = "Invalid input parameters";
            return result;
        }

        size_t pixelCount = static_cast<size_t>(request.width) * request.height;
        uint32_t srcBpp = GetBytesPerPixel(request.sourceFormat);
        uint32_t dstBpp = GetBytesPerPixel(request.targetFormat);

        if (srcSize < pixelCount * srcBpp) {
            result.errorMessage = "Source buffer too small";
            return result;
        }

        result.data.resize(pixelCount * dstBpp);
        result.format = request.targetFormat;

        for (size_t i = 0; i < pixelCount; ++i) {
            float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;
            UnpackPixel(srcData + i * srcBpp, request.sourceFormat, r, g, b, a);
            PackPixel(result.data.data() + i * dstBpp, request.targetFormat, r, g, b, a);
        }

        result.success = true;
        return result;
    }

    inline uint32_t GetBytesPerPixel(CodecPixelFormat format) const
    {
        switch (format) {
            case CodecPixelFormat::RGBA8:
                return 4;
            case CodecPixelFormat::BGRA8:
                return 4;
            case CodecPixelFormat::RGB8:
                return 3;
            case CodecPixelFormat::BGR8:
                return 3;
            case CodecPixelFormat::Gray8:
                return 1;
            case CodecPixelFormat::Gray16:
                return 2;
            case CodecPixelFormat::RGBAFloat:
                return 16;
            default:
                return 0;
        }
    }

    inline std::string FormatToString(CodecPixelFormat format) const
    {
        switch (format) {
            case CodecPixelFormat::RGBA8:
                return "RGBA8";
            case CodecPixelFormat::BGRA8:
                return "BGRA8";
            case CodecPixelFormat::RGB8:
                return "RGB8";
            case CodecPixelFormat::BGR8:
                return "BGR8";
            case CodecPixelFormat::Gray8:
                return "Gray8";
            case CodecPixelFormat::Gray16:
                return "Gray16";
            case CodecPixelFormat::RGBAFloat:
                return "RGBAFloat";
            default:
                return "Unknown";
        }
    }

    inline bool CanTranscode(CodecPixelFormat source, CodecPixelFormat target) const
    {
        return GetBytesPerPixel(source) > 0 && GetBytesPerPixel(target) > 0;
    }

  private:
    RealTimeCodecTranscoder() = default;

    inline void UnpackPixel(const uint8_t* src, CodecPixelFormat fmt, float& r, float& g, float& b, float& a) const
    {
        switch (fmt) {
            case CodecPixelFormat::RGBA8:
                r = src[0] / 255.0f;
                g = src[1] / 255.0f;
                b = src[2] / 255.0f;
                a = src[3] / 255.0f;
                break;
            case CodecPixelFormat::BGRA8:
                b = src[0] / 255.0f;
                g = src[1] / 255.0f;
                r = src[2] / 255.0f;
                a = src[3] / 255.0f;
                break;
            case CodecPixelFormat::RGB8:
                r = src[0] / 255.0f;
                g = src[1] / 255.0f;
                b = src[2] / 255.0f;
                a = 1.0f;
                break;
            case CodecPixelFormat::BGR8:
                b = src[0] / 255.0f;
                g = src[1] / 255.0f;
                r = src[2] / 255.0f;
                a = 1.0f;
                break;
            case CodecPixelFormat::Gray8:
                r = g = b = src[0] / 255.0f;
                a = 1.0f;
                break;
            case CodecPixelFormat::Gray16: {
                uint16_t val = static_cast<uint16_t>(src[0]) | (static_cast<uint16_t>(src[1]) << 8);
                r = g = b = val / 65535.0f;
                a = 1.0f;
                break;
            }
            case CodecPixelFormat::RGBAFloat: {
                const float* fp = reinterpret_cast<const float*>(src);
                r = fp[0];
                g = fp[1];
                b = fp[2];
                a = fp[3];
                break;
            }
            default:
                break;
        }
    }

    inline void PackPixel(uint8_t* dst, CodecPixelFormat fmt, float r, float g, float b, float a) const
    {
        auto toByte = [](float v) -> uint8_t {
            return static_cast<uint8_t>((std::max)(0.0f, (std::min)(1.0f, v)) * 255.0f + 0.5f);
        };
        switch (fmt) {
            case CodecPixelFormat::RGBA8:
                dst[0] = toByte(r);
                dst[1] = toByte(g);
                dst[2] = toByte(b);
                dst[3] = toByte(a);
                break;
            case CodecPixelFormat::BGRA8:
                dst[0] = toByte(b);
                dst[1] = toByte(g);
                dst[2] = toByte(r);
                dst[3] = toByte(a);
                break;
            case CodecPixelFormat::RGB8:
                dst[0] = toByte(r);
                dst[1] = toByte(g);
                dst[2] = toByte(b);
                break;
            case CodecPixelFormat::BGR8:
                dst[0] = toByte(b);
                dst[1] = toByte(g);
                dst[2] = toByte(r);
                break;
            case CodecPixelFormat::Gray8:
                dst[0] = toByte(0.299f * r + 0.587f * g + 0.114f * b);
                break;
            case CodecPixelFormat::Gray16: {
                uint16_t val = static_cast<uint16_t>(
                    (std::max)(0.0f, (std::min)(1.0f, 0.299f * r + 0.587f * g + 0.114f * b)) * 65535.0f);
                dst[0] = static_cast<uint8_t>(val & 0xFF);
                dst[1] = static_cast<uint8_t>((val >> 8) & 0xFF);
                break;
            }
            case CodecPixelFormat::RGBAFloat: {
                float* fp = reinterpret_cast<float*>(dst);
                fp[0] = r;
                fp[1] = g;
                fp[2] = b;
                fp[3] = a;
                break;
            }
            default:
                break;
        }
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
