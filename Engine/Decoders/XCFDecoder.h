#pragma once
//==============================================================================
// XCF (GIMP Native) Decoder
// Open image editor format support
// Parses GIMP XCF file headers to extract canvas info.
// Renders flattened composites from the first visible layer.
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens::Decoders {

 //==========================================================================
 // XCF color modes
 //==========================================================================
 enum class XCFColorMode : uint32_t {
 RGB = 0,
 Grayscale = 1,
 Indexed = 2
 };

 //==========================================================================
 // XCF compression types
 //==========================================================================
 enum class XCFCompression : uint8_t {
 None = 0,
 RLE = 1,
 Zlib = 2,
 Fractal = 3 // Unused in practice
 };

 //==========================================================================
 // XCF Decoder
 //==========================================================================
 class XCFDecoder {
 public:
 XCFDecoder() = default;

 struct DecodeResult {
 bool success = false;
 uint32_t width = 0;
 uint32_t height = 0;
 std::vector<uint8_t> pixelData; // BGRA32
 std::string error;
 };

 struct ImageInfo {
 uint32_t width = 0;
 uint32_t height = 0;
 uint32_t version = 0; // XCF file version (0-14+)
 XCFColorMode colorMode = XCFColorMode::RGB;
 uint32_t precision = 8; // bits per channel (8 or 16 or 32)
 std::string versionString; // "gimp xcf v011" etc.

 bool IsValid() const { return width > 0 && height > 0; }
 };

 /// Read XCF header metadata.
 ImageInfo ReadInfo(const std::string& filePath) const;

 /// Decode XCF to BGRA32 pixels.
 /// Extracts from the first visible layer or generates placeholder.
 DecodeResult Decode(const std::string& filePath, uint32_t targetWidth = 256) const;

 /// Check if extension is XCF.
 static bool IsXCFExtension(const std::string& ext);

 /// Supported extensions
 static constexpr const char* EXTENSIONS[] = { ".xcf", nullptr };

 private:
 /// Read a big-endian uint32 from a byte buffer.
 static uint32_t ReadBE32(const uint8_t* data);
 };

} // namespace ExplorerLens::Decoders

