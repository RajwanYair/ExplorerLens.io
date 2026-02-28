//==============================================================================
// SGI Image Decoder — Implementation
// SGI/RGB & Legacy Format Support
// Handles SGI image format (IRIS RGB) with RLE and verbatim encoding.
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include "SGIDecoder.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cctype>

namespace ExplorerLens::Decoders {

 //==========================================================================
 // SGI file header (big-endian, 512 bytes)
 //==========================================================================
 struct SGIHeader {
 uint16_t magic; // 474 (0x01DA)
 uint8_t storage; // 0=verbatim, 1=RLE
 uint8_t bpc; // bytes per channel (1 or 2)
 uint16_t dimension; // 1=1D, 2=2D, 3=3D (channels)
 uint16_t xsize;
 uint16_t ysize;
 uint16_t zsize; // Number of channels
 int32_t pixmin;
 int32_t pixmax;
 uint8_t dummy[4];
 char imagename[80];
 uint32_t colormap; // 0=normal, 1=dithered, 2=screen, 3=colormap
 uint8_t padding[404]; // Pad to 512 bytes
 };

 //==========================================================================
 // Big-endian read helpers
 //==========================================================================
 static uint16_t ReadBE16(const uint8_t* p) {
 return (static_cast<uint16_t>(p[0]) << 8) | p[1];
 }

 static uint32_t ReadBE32(const uint8_t* p) {
 return (static_cast<uint32_t>(p[0]) << 24) |
 (static_cast<uint32_t>(p[1]) << 16) |
 (static_cast<uint32_t>(p[2]) << 8) | p[3];
 }

 //==========================================================================
 // Extension check
 //==========================================================================
 bool SGIDecoder::IsSGIExtension(const std::string& ext)
 {
 std::string lower = ext;
 std::transform(lower.begin(), lower.end(), lower.begin(),
 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
 return lower == ".sgi" || lower == ".rgb" || lower == ".rgba" ||
 lower == ".bw" || lower == ".int" || lower == ".inta";
 }

 //==========================================================================
 // Read SGI header
 //==========================================================================
 SGIDecoder::ImageInfo SGIDecoder::ReadInfo(const std::string& filePath) const
 {
 ImageInfo info;
 std::ifstream file(filePath, std::ios::binary);
 if (!file.is_open()) return info;

 uint8_t headerBuf[512];
 file.read(reinterpret_cast<char*>(headerBuf), 512);
 if (!file.good()) return info;

 // Check magic (0x01DA, big-endian)
 uint16_t magic = ReadBE16(headerBuf);
 if (magic != 0x01DA) return info;

 info.storage = (headerBuf[2] == 1) ? SGIStorageType::RLE : SGIStorageType::Verbatim;
 info.bytesPerChannel = headerBuf[3];
 if (info.bytesPerChannel != 1 && info.bytesPerChannel != 2) return info;

 uint16_t dimension = ReadBE16(headerBuf + 4);
 info.width = ReadBE16(headerBuf + 6);
 info.height = ReadBE16(headerBuf + 8);
 info.channels = ReadBE16(headerBuf + 10);

 // Validate ranges
 if (info.width == 0 || info.width > 16384) return info;
 if (info.height == 0 || info.height > 16384) return info;
 if (info.channels == 0 || info.channels > 4) return info;

 // 1D images have height=1
 if (dimension == 1) info.height = 1;

 // Extract image name (null-terminated)
 char name[81] = {};
 memcpy(name, headerBuf + 20, 80);
 info.imageName = name;

 return info;
 }

 //==========================================================================
 // RLE scanline decompression
 //==========================================================================
 size_t SGIDecoder::DecodeRLEScanline(const uint8_t* src, size_t srcLen,
 uint8_t* dst, size_t dstLen)
 {
 size_t si = 0, di = 0;
 while (si < srcLen && di < dstLen) {
 uint8_t pixel = src[si++];
 uint8_t count = pixel & 0x7F;
 if (count == 0) break; // End of scanline

 if (pixel & 0x80) {
 // Literal run
 for (uint8_t i = 0; i < count && si < srcLen && di < dstLen; ++i)
 dst[di++] = src[si++];
 }
 else {
 // Repeated value
 if (si >= srcLen) break;
 uint8_t value = src[si++];
 for (uint8_t i = 0; i < count && di < dstLen; ++i)
 dst[di++] = value;
 }
 }
 return di;
 }

 //==========================================================================
 // Decode SGI image to BGRA32
 //==========================================================================
 SGIDecoder::DecodeResult SGIDecoder::Decode(const std::string& filePath,
 uint32_t targetWidth) const
 {
 (void)targetWidth;
 DecodeResult result;
 ImageInfo info = ReadInfo(filePath);
 if (!info.IsValid()) {
 result.error = "Invalid SGI header";
 return result;
 }

 std::ifstream file(filePath, std::ios::binary | std::ios::ate);
 if (!file.is_open()) {
 result.error = "Cannot open file";
 return result;
 }

 size_t fileSize = static_cast<size_t>(file.tellg());
 file.seekg(0);
 std::vector<uint8_t> fileData(fileSize);
 file.read(reinterpret_cast<char*>(fileData.data()), fileSize);

 uint32_t w = info.width, h = info.height, ch = info.channels;
 result.width = w;
 result.height = h;
 result.pixelData.resize(static_cast<size_t>(w) * h * 4);

 if (info.storage == SGIStorageType::Verbatim && info.bytesPerChannel == 1) {
 // Uncompressed 8-bit — channels are stored as separate planes
 size_t planeSize = static_cast<size_t>(w) * h;
 size_t dataOff = 512; // After header

 for (uint32_t y = 0; y < h; ++y) {
 // SGI images are stored bottom-up
 uint32_t flippedY = h - 1 - y;
 for (uint32_t x = 0; x < w; ++x) {
 size_t pixOff = (static_cast<size_t>(y) * w + x) * 4;
 size_t srcIdx = static_cast<size_t>(flippedY) * w + x;

 uint8_t r = 0, g = 0, b = 0, a = 255;
 if (ch >= 1 && dataOff + srcIdx < fileSize)
 r = fileData[dataOff + srcIdx];
 if (ch >= 2 && dataOff + planeSize + srcIdx < fileSize)
 g = fileData[dataOff + planeSize + srcIdx];
 if (ch >= 3 && dataOff + 2 * planeSize + srcIdx < fileSize)
 b = fileData[dataOff + 2 * planeSize + srcIdx];
 if (ch >= 4 && dataOff + 3 * planeSize + srcIdx < fileSize)
 a = fileData[dataOff + 3 * planeSize + srcIdx];

 if (ch == 1) { g = r; b = r; } // Grayscale
 if (ch == 2) { g = r; b = r; } // Grayscale + alpha

 result.pixelData[pixOff + 0] = b; // B
 result.pixelData[pixOff + 1] = g; // G
 result.pixelData[pixOff + 2] = r; // R
 result.pixelData[pixOff + 3] = a; // A
 }
 }
 }
 else if (info.storage == SGIStorageType::RLE && info.bytesPerChannel == 1) {
 // RLE compressed — offset/length tables follow header
 size_t tabLen = static_cast<size_t>(h) * ch;
 if (512 + tabLen * 8 > fileSize) {
 result.error = "RLE offset table truncated";
 return result;
 }

 // Read offset and length tables (big-endian uint32)
 std::vector<uint32_t> offsets(tabLen), lengths(tabLen);
 for (size_t i = 0; i < tabLen; ++i) {
 offsets[i] = ReadBE32(fileData.data() + 512 + i * 4);
 lengths[i] = ReadBE32(fileData.data() + 512 + tabLen * 4 + i * 4);
 }

 // Decode each channel scanline
 std::vector<uint8_t> channelData(static_cast<size_t>(w) * h * ch);
 for (uint32_t c = 0; c < ch; ++c) {
 for (uint32_t y = 0; y < h; ++y) {
 size_t tabIdx = c * h + y;
 if (tabIdx >= tabLen) continue;

 uint32_t off = offsets[tabIdx];
 uint32_t len = lengths[tabIdx];
 if (off + len > fileSize) continue;

 uint8_t* scanDst = channelData.data() + (c * static_cast<size_t>(w) * h + static_cast<size_t>(y) * w);
 DecodeRLEScanline(fileData.data() + off, len, scanDst, w);
 }
 }

 // Interleave channels into BGRA
 size_t planeSize = static_cast<size_t>(w) * h;
 for (uint32_t y = 0; y < h; ++y) {
 uint32_t flippedY = h - 1 - y;
 for (uint32_t x = 0; x < w; ++x) {
 size_t pixOff = (static_cast<size_t>(y) * w + x) * 4;
 size_t srcIdx = static_cast<size_t>(flippedY) * w + x;

 uint8_t r = 0, g = 0, b = 0, a = 255;
 if (ch >= 1) r = channelData[srcIdx];
 if (ch >= 2) g = channelData[planeSize + srcIdx];
 if (ch >= 3) b = channelData[2 * planeSize + srcIdx];
 if (ch >= 4) a = channelData[3 * planeSize + srcIdx];

 if (ch == 1) { g = r; b = r; }
 if (ch == 2) { g = r; b = r; }

 result.pixelData[pixOff + 0] = b;
 result.pixelData[pixOff + 1] = g;
 result.pixelData[pixOff + 2] = r;
 result.pixelData[pixOff + 3] = a;
 }
 }
 }
 else {
 result.error = "Unsupported SGI format (16-bit channels not yet implemented)";
 return result;
 }

 result.success = true;
 return result;
 }

} // namespace ExplorerLens::Decoders

