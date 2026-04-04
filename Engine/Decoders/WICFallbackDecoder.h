// WICFallbackDecoder.h — Windows Imaging Component Fallback Path
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a WIC-based fallback decoder for formats when specialized
// decoders fail, leveraging system-installed WIC codecs.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class WICCodecType : uint8_t {
    BMP = 0,
    PNG = 1,
    JPEG = 2,
    TIFF = 3,
    GIF = 4,
    WMPhoto = 5,
    DDS = 6,
    ICO = 7,
    HEIF = 8,
    WEBP = 9,
    Unknown = 255
};

struct WICDecodeResult
{
    bool success = false;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t bitsPerPixel = 0;
    uint32_t stride = 0;
    std::vector<uint8_t> pixels;
    WICCodecType codecUsed = WICCodecType::Unknown;
    double decodeMs = 0.0;
    std::string errorMessage;
};

struct WICFallbackConfig
{
    bool enabled = true;
    uint32_t maxImageDimension = 16384;
    uint32_t timeoutMs = 5000;
    bool convertTo32bppBGRA = true;
    bool enableColorManagement = false;
};

struct WICFallbackStats
{
    uint64_t totalAttempts = 0;
    uint64_t successCount = 0;
    uint64_t failureCount = 0;
    double avgDecodeMs = 0.0;
    double successRate() const
    {
        return totalAttempts > 0 ? 100.0 * successCount / totalAttempts : 0.0;
    }
};

class WICFallbackDecoder
{
  public:
    void Configure(const WICFallbackConfig& config)
    {
        m_config = config;
    }

    bool IsSupported(const std::wstring& extension) const
    {
        if (extension == L".bmp" || extension == L".png" || extension == L".jpg" || extension == L".jpeg"
            || extension == L".tif" || extension == L".tiff" || extension == L".gif" || extension == L".ico"
            || extension == L".dds" || extension == L".wdp" || extension == L".jxr")
            return true;
        return false;
    }

    WICCodecType DetectCodec(const std::wstring& extension) const
    {
        if (extension == L".bmp")
            return WICCodecType::BMP;
        if (extension == L".png")
            return WICCodecType::PNG;
        if (extension == L".jpg" || extension == L".jpeg")
            return WICCodecType::JPEG;
        if (extension == L".tif" || extension == L".tiff")
            return WICCodecType::TIFF;
        if (extension == L".gif")
            return WICCodecType::GIF;
        if (extension == L".ico")
            return WICCodecType::ICO;
        if (extension == L".dds")
            return WICCodecType::DDS;
        if (extension == L".wdp" || extension == L".jxr")
            return WICCodecType::WMPhoto;
        return WICCodecType::Unknown;
    }

    bool ValidateDimensions(uint32_t width, uint32_t height) const
    {
        return width > 0 && height > 0 && width <= m_config.maxImageDimension && height <= m_config.maxImageDimension;
    }

    WICFallbackStats GetStats() const
    {
        return m_stats;
    }

  private:
    WICFallbackConfig m_config;
    WICFallbackStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
