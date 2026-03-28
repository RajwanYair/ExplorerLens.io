// HRZDecoder.h — HRZ / Slow-Scan Television (SSTV) Format Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes HRZ (Slow-Scan Television) image files — raw 256×240 RGB24 images
// historically transmitted over amateur radio. Also handles the broader
// family of SSTV-originated still-image formats.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens::Decoders {

/// HRZ canonical dimensions — defined by the original SSTV standard.
inline constexpr uint32_t HRZ_WIDTH  = 256;
inline constexpr uint32_t HRZ_HEIGHT = 240;
inline constexpr uint32_t HRZ_BYTES  = HRZ_WIDTH * HRZ_HEIGHT * 3;  // RGB24

class HRZDecoder {
public:
    HRZDecoder() = default;

    struct DecodeResult {
        bool     success = false;
        uint32_t width   = HRZ_WIDTH;
        uint32_t height  = HRZ_HEIGHT;
        std::vector<uint8_t> pixelData;  // BGRA32
        std::string error;
    };

    struct HRZInfo {
        uint32_t width  = HRZ_WIDTH;
        uint32_t height = HRZ_HEIGHT;
        uint32_t fileSize = 0;

        bool IsValid() const { return fileSize == HRZ_BYTES; }
    };

    HRZInfo  ReadInfo(const std::string& filePath) const;
    DecodeResult Decode(const std::string& filePath, uint32_t targetWidth = 256) const;

    static bool IsHRZExtension(const std::string& ext);
    static constexpr const char* EXTENSIONS[] = { ".hrz", nullptr };
};

}  // namespace ExplorerLens::Decoders
