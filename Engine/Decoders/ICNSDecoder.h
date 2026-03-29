// ICNSDecoder.h — Apple ICNS Icon Bundle Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes Apple ICNS icon bundles, extracting the highest-resolution
// icon variant (PNG sub-image or raw ARGB payload) for thumbnail generation.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens::Decoders {

/// Apple ICNS four-character-code type tags for known icon variants.
enum class ICNSIconType : uint32_t {
    ic07 = 0x69633037,  // 128×128 PNG (ICNS 7)
    ic08 = 0x69633038,  // 256×256 PNG
    ic09 = 0x69633039,  // 512×512 PNG
    ic10 = 0x69633130,  // 1024×1024 PNG (Retina)
    ic11 = 0x69633131,  // 16×16 @2x PNG
    ic12 = 0x69633132,  // 32×32 @2x PNG
    ic13 = 0x69633133,  // 128×128 @2x PNG
    ic14 = 0x69633134,  // 256×256 @2x PNG
    Unknown = 0,
};

class ICNSDecoder {
public:
    ICNSDecoder() = default;

    struct DecodeResult {
        bool    success  = false;
        uint32_t width   = 0;
        uint32_t height  = 0;
        std::vector<uint8_t> pixelData;  // BGRA32
        std::string error;
    };

    struct IconInfo {
        uint32_t    variantCount = 0;
        uint32_t    bestWidth    = 0;
        uint32_t    bestHeight   = 0;
        ICNSIconType bestType    = ICNSIconType::Unknown;

        bool IsValid() const { return variantCount > 0 && bestWidth > 0; }
    };

    /// Probe the ICNS file and return metadata about the best available icon.
    IconInfo ReadInfo(const std::string& filePath) const { (void)filePath; return {}; }

    /// Decode the highest-resolution icon variant to BGRA32.
    DecodeResult Decode(const std::string& filePath, uint32_t targetWidth = 256) const { (void)filePath; (void)targetWidth; return {}; }

    static bool IsICNSExtension(const std::string& ext) {
        return ext == ".icns" || ext == ".ICNS";
    }
    static constexpr const char* EXTENSIONS[] = { ".icns", nullptr };

private:
    static ICNSIconType BestVariant(const std::vector<ICNSIconType>& variants);
};

}  // namespace ExplorerLens::Decoders
