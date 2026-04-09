// OMETIFFCompositor.h — OME-TIFF Multi-Channel Fluorescence Compositor
// Copyright (c) 2026 ExplorerLens Project
//
// Reads OME-XML metadata from the ImageDescription tag of an OME-TIFF file
// to identify fluorescence channels and their emission wavelengths. Maps each
// channel to an RGB pseudo-colour (based on emission wavelength) and composites
// up to 8 channels into a single BGRA32 thumbnail using additive blending.
// Supports single-file OME-TIFF and BigTIFF variants.
//
#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace ExplorerLens { namespace Engine {

struct OMEChannel {
    std::string  name;           // Channel name from OME-XML
    uint32_t     emissionWaveNm = 0; // Emission wavelength in nm (0 = unknown)
    uint32_t     pseudoColorBGR = 0; // Derived display colour packed as BGR
    float        displayMin     = 0.0f;
    float        displayMax     = 65535.0f;
};

struct OMECompositeResult {
    std::vector<uint8_t> pixelsBGRA;
    uint32_t width        = 0;
    uint32_t height       = 0;
    uint32_t channelCount = 0;
    std::vector<OMEChannel> channels;
    bool     success      = false;
};

struct OMECompositeOptions {
    uint32_t maxChannels     = 8;
    uint32_t targetWidth     = 256;
    uint32_t targetHeight    = 256;
    bool     autoScale       = true;   // Per-channel auto min/max
    bool     equalizeChannels = false; // Equal weight for all channels
};

class OMETIFFCompositor {
public:
    OMETIFFCompositor()  = default;
    ~OMETIFFCompositor() = default;

    // Composite OME-TIFF from raw bytes.
    OMECompositeResult Composite(
        const uint8_t*            tiffData,
        size_t                    tiffSize,
        const OMECompositeOptions& opts = {}) const noexcept;

    // Map emission wavelength (nm) to a BGR packed display colour.
    static uint32_t WavelengthToBGR(uint32_t wavelengthNm) noexcept;

    // Extract OME-XML string from TIFF ImageDescription tag (first 64 KB).
    static std::string ExtractOMEXML(
        const uint8_t* tiffData, size_t tiffSize) noexcept;

    // Check OME-TIFF signature (TIFF magic + "<?xml" in first 64 KB).
    static bool IsOMETIFF(const uint8_t* data, size_t size) noexcept;
};

}} // namespace ExplorerLens::Engine
