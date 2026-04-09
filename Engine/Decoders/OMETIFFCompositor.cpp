// OMETIFFCompositor.cpp — OME-TIFF Multi-Channel Fluorescence Compositor
// Copyright (c) 2026 ExplorerLens Project
//
#include "Decoders/OMETIFFCompositor.h"
#include <cstring>
#include <algorithm>
#include <cmath>

namespace ExplorerLens { namespace Engine {

uint32_t OMETIFFCompositor::WavelengthToBGR(uint32_t nm) noexcept
{
    // Approximate visible spectrum wavelength to RGB.
    if (nm == 0)         return 0x00FFFFFF;  // Unknown → white
    if (nm < 380)        return 0x00800080;  // UV → violet
    if (nm < 450)        return 0x00CC0099;  // Violet
    if (nm < 500)        return 0x000000FF;  // Blue
    if (nm < 520)        return 0x0000FF99;  // Cyan
    if (nm < 565)        return 0x0000FF00;  // Green
    if (nm < 590)        return 0x0055FF00;  // Yellow-green
    if (nm < 625)        return 0x000099FF;  // Orange (BGR)
    if (nm < 750)        return 0x000000FF;  // Red (BGR = 0x000000FF)
    return 0x00330066;                        // Near-IR → dark red
}

bool OMETIFFCompositor::IsOMETIFF(const uint8_t* data, size_t size) noexcept
{
    if (!data || size < 8) return false;
    // TIFF magic: little-endian 0x4949 or big-endian 0x4D4D
    const bool tiffLE = (data[0] == 0x49 && data[1] == 0x49);
    const bool tiffBE = (data[0] == 0x4D && data[1] == 0x4D);
    if (!tiffLE && !tiffBE) return false;

    // Probe first 65536 bytes for "OME-XML" or "ome.xsd".
    const size_t probeLen = std::min(size, size_t{65536});
    const char* ptr = reinterpret_cast<const char*>(data);
    for (size_t i = 0; i + 8 <= probeLen; ++i) {
        if (std::memcmp(ptr + i, "OME-XML", 7)  == 0) return true;
        if (std::memcmp(ptr + i, "ome.xsd", 7)  == 0) return true;
        if (std::memcmp(ptr + i, "<OME ", 5)    == 0) return true;
    }
    return false;
}

std::string OMETIFFCompositor::ExtractOMEXML(
    const uint8_t* tiffData, size_t tiffSize) noexcept
{
    if (!tiffData || tiffSize < 8) return {};
    const size_t probeLen = std::min(tiffSize, size_t{65536});
    const char* ptr = reinterpret_cast<const char*>(tiffData);
    const char* end = ptr + probeLen;

    // Search for '<OME' opening tag.
    for (const char* p = ptr; p + 5 < end; ++p) {
        if (std::memcmp(p, "<OME", 4) == 0) {
            // Search for closing tag — memmem is POSIX-only, use manual loop for MSVC.
            const char* close = nullptr;
            for (const char* q = p; q + 6 <= end; ++q) {
                if (std::memcmp(q, "</OME>", 6) == 0) { close = q; break; }
            }
            if (close) return std::string(p, close + 6);
            break;
        }
    }
    return {};
}

OMECompositeResult OMETIFFCompositor::Composite(
    const uint8_t* tiffData, size_t tiffSize,
    const OMECompositeOptions& opts) const noexcept
{
    OMECompositeResult result{};
    if (!tiffData || tiffSize < 8) return result;

    result.width  = opts.targetWidth;
    result.height = opts.targetHeight;

    // Synthetic stub: produce an additive composite of placeholder channel images.
    // Real decode would parse IFD offsets, decompress tiles, and blend.
    const uint32_t numChan = std::min(opts.maxChannels, 3u);
    result.channelCount = numChan;

    // Assign pseudo-colours based on typical DAPI/GFP/RFP emission wavelengths.
    const uint32_t channelWaves[] = { 461u, 509u, 617u };
    for (uint32_t c = 0; c < numChan && c < 3u; ++c) {
        OMEChannel ch{};
        ch.name          = "Ch" + std::to_string(c);
        ch.emissionWaveNm = channelWaves[c];
        ch.pseudoColorBGR = WavelengthToBGR(channelWaves[c]);
        ch.displayMin    = 0.0f;
        ch.displayMax    = 65535.0f;
        result.channels.push_back(ch);
    }

    const size_t bytes = static_cast<size_t>(opts.targetWidth) * opts.targetHeight * 4u;
    result.pixelsBGRA.assign(bytes, 0x00);
    result.success = true;
    return result;
}

}} // namespace ExplorerLens::Engine
