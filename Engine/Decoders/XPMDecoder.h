#pragma once
//==============================================================================
// XPM (X PixMap) Decoder
// SGI/RGB & Legacy Format Support
// Parses XPM2 and XPM3 format files (ASCII-based pixel art format).
// Used in X11/Unix UI systems for icons and cursors.
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens::Decoders {

class XPMDecoder
{
  public:
    XPMDecoder() = default;

    struct DecodeResult
    {
        bool success = false;
        uint32_t width = 0;
        uint32_t height = 0;
        std::vector<uint8_t> pixelData;  // BGRA32
        std::string error;
    };

    struct ImageInfo
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t numColors = 0;
        uint32_t charsPerPixel = 0;

        bool IsValid() const
        {
            return width > 0 && height > 0 && numColors > 0 && charsPerPixel > 0;
        }
    };

    /// Read XPM header metadata.
    ImageInfo ReadInfo(const std::string& filePath) const;

    /// Decode XPM image to BGRA32 pixels.
    DecodeResult Decode(const std::string& filePath, uint32_t targetWidth = 256) const;

    /// Check if extension is XPM.
    static bool IsXPMExtension(const std::string& ext);

    /// Supported extensions
    static constexpr const char* EXTENSIONS[] = {".xpm", nullptr};

  private:
    /// Parse a hex color string (#RRGGBB or named color) to BGRA.
    static uint32_t ParseColor(const std::string& colorStr);

    /// Extract quoted string from XPM line.
    static std::string ExtractQuoted(const std::string& line);
};

}  // namespace ExplorerLens::Decoders
