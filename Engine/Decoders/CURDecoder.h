// CURDecoder.h — Windows Cursor (.cur / .ani) Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes Windows static cursor (.cur) and animated cursor (.ani) files,
// extracting the best-resolution frame as a BGRA32 thumbnail.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens::Decoders {

enum class CursorFileType : uint8_t {
    Static    = 1,  // .cur  — ICONDIR with type=2
    Animated  = 2,  // .ani  — RIFF container
    Unknown   = 0,
};

class CURDecoder {
public:
    CURDecoder() = default;

    struct DecodeResult {
        bool     success   = false;
        uint32_t width     = 0;
        uint32_t height    = 0;
        uint32_t frameCount = 1;
        std::vector<uint8_t> pixelData;  // BGRA32 of best frame
        std::string error;
    };

    struct CursorInfo {
        CursorFileType type   = CursorFileType::Unknown;
        uint32_t       frames = 0;
        uint32_t       width  = 0;
        uint32_t       height = 0;
        uint32_t       hotspotX = 0;
        uint32_t       hotspotY = 0;

        bool IsValid() const { return type != CursorFileType::Unknown && width > 0; }
    };

    CursorInfo ReadInfo(const std::string& filePath) const;
    DecodeResult Decode(const std::string& filePath, uint32_t targetWidth = 64) const;

    static bool IsCursorExtension(const std::string& ext);
    static constexpr const char* EXTENSIONS[] = { ".cur", ".ani", nullptr };

private:
    DecodeResult DecodeStatic(const std::string& filePath, uint32_t targetWidth) const;
    DecodeResult DecodeAnimated(const std::string& filePath, uint32_t targetWidth) const;
};

}  // namespace ExplorerLens::Decoders
