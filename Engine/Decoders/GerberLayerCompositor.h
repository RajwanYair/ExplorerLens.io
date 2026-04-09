// GerberLayerCompositor.h — RS-274X Gerber Layer Compositor
// Copyright (c) 2026 ExplorerLens Project
//
// Parses RS-274X Extended Gerber files and composites copper, soldermask,
// silkscreen, and drill layers into a colour-coded PCB preview thumbnail.
// Uses a rasterisation grid with additive layer compositing.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class GerberLayerType : uint8_t
{
    Unknown      = 0,
    CopperTop    = 1,
    CopperBottom = 2,
    SoldermaskTop,
    SoldermaskBottom,
    SilkscreenTop,
    SilkscreenBottom,
    CopperInner,
    DrillThrough,
    BoardOutline,
};

struct GerberAperture
{
    uint32_t    code;       // D-code number (≥10)
    char        type;       // 'C' circle, 'R' rectangle, 'O' obround
    double      param0;     // diameter or width
    double      param1;     // height (rectangles only)
};

struct GerberLayerInfo
{
    GerberLayerType type;
    uint32_t        flashCount;    // number of flash operations
    uint32_t        drawCount;     // number of draw (line) operations
    std::string     fileExtension; // ".gtl", ".gbr", etc.
    bool            valid;
};

class GerberLayerCompositor
{
public:
    // Returns true if file starts with "G04 " (comment) or "%FS" (format spec).
    static bool IsGerber(const uint8_t* data, size_t size) noexcept;

    // Detect layer type from file extension hint (empty string = auto-detect from content).
    static GerberLayerType DetectLayerType(const char* fileExtension) noexcept;

    // Parse aperture definitions from %ADD block; returns list of defined apertures.
    static std::vector<GerberAperture> ParseApertures(const uint8_t* data, size_t size) noexcept;

    // Scan flash/draw stats without full rasterisation.
    static GerberLayerInfo ProbeLayer(const uint8_t* data, size_t size,
                                       const char* fileExtension = "") noexcept;

    // Rasterise a single Gerber layer to a 256×256 BGRA32 bitmap.
    // Colour is determined by layerType (copper=gold, solder=green, silk=white, etc.)
    static std::vector<uint8_t> RasteriseLayer(const uint8_t* data, size_t size,
                                                GerberLayerType layerType,
                                                uint32_t width  = 256,
                                                uint32_t height = 256);
};

} // namespace Engine
} // namespace ExplorerLens
