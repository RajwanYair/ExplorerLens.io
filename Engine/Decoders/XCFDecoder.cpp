//==============================================================================
// XCF (GIMP Native) Decoder — Implementation
// Open image editor format support
// Parses XCF file headers (versions 0-14+), extracts canvas dimensions,
// color mode, version, and generates thumbnail from header info.
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include "XCFDecoder.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cctype>

namespace ExplorerLens::Decoders {

 //==========================================================================
 // Extension check
 //==========================================================================
 bool XCFDecoder::IsXCFExtension(const std::string& ext)
 {
 std::string lower = ext;
 std::transform(lower.begin(), lower.end(), lower.begin(),
 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
 return lower == ".xcf";
 }

 //==========================================================================
 // Read big-endian uint32
 //==========================================================================
 uint32_t XCFDecoder::ReadBE32(const uint8_t* data)
 {
 return (static_cast<uint32_t>(data[0]) << 24) |
 (static_cast<uint32_t>(data[1]) << 16) |
 (static_cast<uint32_t>(data[2]) << 8) |
 static_cast<uint32_t>(data[3]);
 }

 //==========================================================================
 // Read XCF header
 // XCF format: "gimp xcf " followed by version string, then null byte,
 // then width(4), height(4), base_type(4)
 //==========================================================================
 XCFDecoder::ImageInfo XCFDecoder::ReadInfo(const std::string& filePath) const
 {
 ImageInfo info;
 std::ifstream file(filePath, std::ios::binary);
 if (!file.is_open()) return info;

 // Read header — minimum 26 bytes: "gimp xcf vXXX\0" (14) + width(4) + height(4) + base_type(4)
 uint8_t header[30];
 file.read(reinterpret_cast<char*>(header), 30);
 if (!file.good() && !file.eof()) return info;

 // Validate magic: "gimp xcf "
 if (memcmp(header, "gimp xcf ", 9) != 0)
 return info;

 // Extract version string — find the null terminator
 size_t verEnd = 9;
 while (verEnd < 22 && header[verEnd] != 0) ++verEnd;
 if (verEnd >= 22) return info;

 info.versionString = std::string(reinterpret_cast<const char*>(header), verEnd);

 // Parse version number from "gimp xcf vXXX" or "gimp xcf file"
 std::string verPart(reinterpret_cast<const char*>(header + 9), verEnd - 9);
 if (verPart == "file") {
 info.version = 0;
 }
 else if (verPart.size() > 0 && verPart[0] == 'v') {
 // "v001", "v002", ..., "v014"
 try {
 info.version = std::stoul(verPart.substr(1));
 }
 catch (...) {
 info.version = 0;
 }
 }

 // Width, Height, BaseType follow the null terminator
 size_t dataOff = verEnd + 1;
 if (dataOff + 12 > 30) return info;

 info.width = ReadBE32(header + dataOff);
 info.height = ReadBE32(header + dataOff + 4);
 uint32_t baseType = ReadBE32(header + dataOff + 8);

 if (baseType <= 2) {
 info.colorMode = static_cast<XCFColorMode>(baseType);
 }

 // Version 4+ support higher precision
 if (info.version >= 4) {
 // Read precision property if available
 // For now, default to 8-bit
 info.precision = 8;
 }

 return info;
 }

 //==========================================================================
 // Decode XCF
 // Full XCF layer compositing is extremely complex (hundreds of blend modes,
 // RLE compression, tile format, etc.). For thumbnail generation, we create
 // an informative placeholder showing the image dimensions and metadata.
 // A full implementation would require a GIMP-compatible layer engine.
 //==========================================================================
 XCFDecoder::DecodeResult XCFDecoder::Decode(const std::string& filePath,
 uint32_t targetWidth) const
 {
 DecodeResult result;
 ImageInfo info = ReadInfo(filePath);
 if (!info.IsValid()) {
 result.error = "Invalid XCF header";
 return result;
 }

 // Calculate thumbnail dimensions maintaining aspect ratio
 uint32_t thumbW = targetWidth;
 uint32_t thumbH = targetWidth;
 if (info.width > 0 && info.height > 0) {
 float aspect = static_cast<float>(info.width) / static_cast<float>(info.height);
 if (aspect > 1.0f) {
 thumbH = static_cast<uint32_t>(thumbW / aspect);
 }
 else {
 thumbW = static_cast<uint32_t>(thumbH * aspect);
 }
 }
 if (thumbW == 0) thumbW = 1;
 if (thumbH == 0) thumbH = 1;

 result.width = thumbW;
 result.height = thumbH;

 // Generate a GIMP-branded placeholder
 // Dark background with GIMP orange accent stripes
 result.pixelData.resize(static_cast<size_t>(thumbW) * thumbH * 4);
 for (uint32_t y = 0; y < thumbH; ++y) {
 for (uint32_t x = 0; x < thumbW; ++x) {
 size_t off = (static_cast<size_t>(y) * thumbW + x) * 4;
 // Checkerboard for GIMP transparent background
 bool dark = ((x / 8) + (y / 8)) % 2 == 0;
 uint8_t base = dark ? 180 : 200;

 // Draw border accent
 bool isBorder = (x < 3 || x >= thumbW - 3 || y < 3 || y >= thumbH - 3);

 if (isBorder) {
 // GIMP orange: #FF6600
 result.pixelData[off + 0] = 0x00; // B
 result.pixelData[off + 1] = 0x66; // G
 result.pixelData[off + 2] = 0xFF; // R
 result.pixelData[off + 3] = 255;
 }
 else {
 result.pixelData[off + 0] = base;
 result.pixelData[off + 1] = base;
 result.pixelData[off + 2] = base;
 result.pixelData[off + 3] = 255;
 }
 }
 }

 result.success = true;
 return result;
 }

} // namespace ExplorerLens::Decoders

