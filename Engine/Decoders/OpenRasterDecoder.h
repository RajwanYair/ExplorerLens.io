#pragma once
//==============================================================================
// OpenRaster (.ora) Decoder
// Sprint 185: Open image editor format support
// OpenRaster is a ZIP archive containing a merged PNG image.
// Used by GIMP, Krita, MyPaint, and other editors.
// Copyright (c) 2026 - DarkThumbs Project
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace DarkThumbs::Decoders {

    class OpenRasterDecoder {
    public:
        OpenRasterDecoder() = default;

        struct DecodeResult {
            bool     success = false;
            uint32_t width = 0;
            uint32_t height = 0;
            std::vector<uint8_t> pixelData; // BGRA32
            std::string error;
        };

        struct ImageInfo {
            uint32_t width = 0;
            uint32_t height = 0;
            bool     hasThumbnail = false;
            bool     hasMergedImage = false;
            std::string version;

            bool IsValid() const { return width > 0 && height > 0; }
        };

        /// Read OpenRaster metadata from the stack.xml inside the ZIP.
        ImageInfo ReadInfo(const std::string& filePath) const;

        /// Decode OpenRaster to BGRA32 pixels.
        /// Extracts Thumbnails/thumbnail.png or mergedimage.png from the ZIP.
        DecodeResult Decode(const std::string& filePath, uint32_t targetWidth = 256) const;

        /// Check if extension is OpenRaster.
        static bool IsORAExtension(const std::string& ext);

        /// Supported extensions
        static constexpr const char* EXTENSIONS[] = { ".ora", nullptr };
    };

} // namespace DarkThumbs::Decoders
