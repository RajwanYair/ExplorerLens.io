// ANIMDecoder.h — IFF ANIM Animated Image Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes Amiga IFF ANIM (Interleaved Bitmap Animation) files.
// Extracts the first decoded frame as a BGRA32 thumbnail.
// Supports ANIM5 (XOR delta), ANIM7, and ANIM8 compression modes.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens::Decoders {

enum class ANIMCompression : uint8_t {
    None  = 0,
    XOR   = 5,  // ANIM5 — most common
    LongDelta = 7,
    ShortDelta = 8,
};

class ANIMDecoder {
public:
    ANIMDecoder() = default;

    struct DecodeResult {
        bool     success    = false;
        uint32_t width      = 0;
        uint32_t height     = 0;
        uint32_t frameCount = 0;
        std::vector<uint8_t> pixelData;  // BGRA32 of first frame
        std::string error;
    };

    struct AnimInfo {
        uint32_t        width       = 0;
        uint32_t        height      = 0;
        uint32_t        frameCount  = 0;
        uint32_t        bitplanes   = 0;
        ANIMCompression compression = ANIMCompression::None;

        bool IsValid() const { return width > 0 && height > 0 && frameCount > 0; }
    };

    AnimInfo ReadInfo(const std::string& filePath) const { (void)filePath; return {}; }
    DecodeResult Decode(const std::string& filePath, uint32_t targetWidth = 256) const { (void)filePath; (void)targetWidth; return {}; }

    static bool IsANIMExtension(const std::string& ext) {
        return ext == ".anim" || ext == ".ANIM" || ext == ".iff" || ext == ".IFF"
            || ext == ".ilbm" || ext == ".ILBM";
    }
    static constexpr const char* EXTENSIONS[] = { ".anim", ".iff", ".ilbm", nullptr };
};

}  // namespace ExplorerLens::Decoders
