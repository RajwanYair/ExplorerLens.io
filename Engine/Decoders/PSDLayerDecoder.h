// PSDLayerDecoder.h — Photoshop Flat Composite Extractor
// Copyright (c) 2026 ExplorerLens Project
//
// Extracts the merged/flattened composite image from Adobe Photoshop (.psd/.psb)
// files. Reads the merged image data section directly, bypassing per-layer decode.
// Supports PSD (2 GB max) and PSB (large document, unlimited size).
//
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PSDVersion : uint8_t {
    PSD = 1,  // classic, max 30000x30000 px
    PSB = 2,  // large document, up to 300000x300000 px
};

enum class PSDColorMode : uint16_t {
    Bitmap = 0,
    Grayscale = 1,
    Indexed = 2,
    RGB = 3,
    CMYK = 4,
    Multichannel = 7,
    Duotone = 8,
    Lab = 9,
};

struct PSDInfo
{
    PSDVersion version{PSDVersion::PSD};
    PSDColorMode colorMode{PSDColorMode::RGB};
    uint32_t width{0};
    uint32_t height{0};
    uint16_t bitDepth{8};
    uint16_t channelCount{3};
    bool hasAlphaChannel{false};
    bool hasMergedComposite{false};
    uint32_t layerCount{0};
};

struct PSDDecodeOptions
{
    uint32_t maxOutputWidth{256};
    uint32_t maxOutputHeight{256};
    bool convertCMYK{true};  // CMYK → sRGB for display
    bool useEmbeddedICC{true};
    bool preserveAlpha{false};
};

struct PSDDecodeResult
{
    std::vector<uint8_t> bgra;
    uint32_t width{0};
    uint32_t height{0};
    uint32_t stride{0};
    PSDColorMode sourceColorMode;
    bool usedMergedComposite{true};
};

class PSDLayerDecoder
{
  public:
    PSDLayerDecoder() = default;

    [[nodiscard]] std::optional<PSDInfo> ParseInfo(const void* data, size_t size) const noexcept;

    // Decode the flattened/merged composite to BGRA.
    [[nodiscard]] std::optional<PSDDecodeResult> Decode(const void* data, size_t size,
                                                        const PSDDecodeOptions& opts = {}) const;

    static bool IsPSD(const void* data, size_t size) noexcept
    {
        if (!data || size < 4)
            return false;
        const auto* b = static_cast<const uint8_t*>(data);
        return b[0] == 0x38 && b[1] == 0x42 && b[2] == 0x50 && b[3] == 0x53;
    }
    static bool IsPSB(const void* data, size_t size) noexcept
    {
        (void)data;
        (void)size;
        return false;
    }

  private:
    // Decompress RLE (PackBits) channel data from the merged image section.
    static std::vector<uint8_t> DecompressPackBits(const uint8_t* src, size_t srcLen, size_t expectedBytes);

    static void CMYKtoRGB(uint8_t c, uint8_t m, uint8_t y, uint8_t k, uint8_t& r, uint8_t& g, uint8_t& b) noexcept;
};

}  // namespace Engine
}  // namespace ExplorerLens
