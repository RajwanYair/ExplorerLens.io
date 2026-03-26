// SemanticColorPalette.h — AI Dominant Color Palette Extractor
// Copyright (c) 2026 ExplorerLens Project
//
// Extracts a semantically meaningful dominant color palette from thumbnail
// pixels for folder mosaic tiles, color-based search, and visual grouping
// in the LENSManager gallery view.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---- Color Entry ------------------------------------------------------------

struct PaletteColor {
    uint8_t  r, g, b;
    float    coverage     = 0.0f;  // Fraction of pixels (0.0-1.0)
    bool     isDominant   = false;
    bool     isBackground = false;
    std::string name;              // Optional: "Navy Blue", "Forest Green", ...
};

// ---- Extraction Options -----------------------------------------------------

struct PaletteOptions {
    uint32_t maxColors          = 6;
    bool     excludeWhiteBlack  = true;   // Ignore near-white/black background
    bool     quantizeToMaterial = true;   // Snap to Material Design 500-level hues
    bool     labelColors        = false;  // Fill PaletteColor::name (costs ~2ms)
    float    minCoverage        = 0.03f;  // Drop colors below 3% coverage
};

// ---- Result -----------------------------------------------------------------

struct SemanticPaletteResult {
    bool                      success   = false;
    std::vector<PaletteColor> colors;
    PaletteColor              dominant;   // Highest coverage non-background color
    float                     extractionMs = 0.0f;
};

// ---- SemanticColorPalette ---------------------------------------------------

class SemanticColorPalette {
public:
    SemanticColorPalette()  = default;
    ~SemanticColorPalette() = default;

    // Extract palette from BGRA thumbnail pixels.
    SemanticPaletteResult Extract(
        const uint8_t*       pixels,
        uint32_t             width,
        uint32_t             height,
        const PaletteOptions& opts = {}) const;

    // Encode palette as a compact hex string for cache storage.
    // e.g. "2c3e50:0.42,3498db:0.28,e74c3c:0.15"
    static std::string Encode(const std::vector<PaletteColor>& colors);

    // Decode a hex string back to PaletteColor vector.
    static std::vector<PaletteColor> Decode(const std::string& encoded);

    // Color distance in CIE Lab (perceptual).
    static float CIELabDistance(const PaletteColor& a, const PaletteColor& b);

private:
    // k-means++ initialisation + Lloyd's algorithm, max 20 iterations.
    std::vector<PaletteColor> KMeansQuantize(
        const uint8_t* pixels,
        uint32_t w, uint32_t h,
        uint32_t k) const;
};

} // namespace Engine
} // namespace ExplorerLens
