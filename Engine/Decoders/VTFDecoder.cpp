//==============================================================================
// VTF (Valve Texture Format) Decoder — Implementation
// Game Texture Format Support
// Handles VTF v7.0-7.5, DXT1/DXT5/RGB888/BGRA8888 formats
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include "VTFDecoder.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cctype>

namespace ExplorerLens::Decoders {

 //==========================================================================
 // Extension check
 //==========================================================================
 bool VTFDecoder::IsVTFExtension(const std::string& ext)
 {
 std::string lower = ext;
 std::transform(lower.begin(), lower.end(), lower.begin(),
 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
 return lower == ".vtf";
 }

 //==========================================================================
 // Read VTF header
 //==========================================================================
 VTFDecoder::TextureInfo VTFDecoder::ReadInfo(const std::string& filePath) const
 {
 TextureInfo info;
 std::ifstream file(filePath, std::ios::binary);
 if (!file.is_open()) return info;

 VTFHeader hdr;
 memset(&hdr, 0, sizeof(hdr));
 file.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
 if (!file.good()) return info;

 // Validate signature "VTF\0"
 if (hdr.signature[0] != 'V' || hdr.signature[1] != 'T' ||
 hdr.signature[2] != 'F' || hdr.signature[3] != '\0') {
 return info;
 }

 // Validate version (7.0 - 7.5)
 if (hdr.versionMajor != 7 || hdr.versionMinor > 5) {
 return info;
 }

 info.width = hdr.width;
 info.height = hdr.height;
 info.versionMajor = hdr.versionMajor;
 info.versionMinor = hdr.versionMinor;
 info.mipmapCount = hdr.mipmapCount;
 info.frames = hdr.frames;
 info.format = static_cast<VTFImageFormat>(hdr.highResImageFormat);
 info.hasLowResImage = (hdr.lowResImageFormat != static_cast<int32_t>(VTFImageFormat::NONE));
 info.lowResWidth = hdr.lowResImageWidth;
 info.lowResHeight = hdr.lowResImageHeight;

 return info;
 }

 //==========================================================================
 // Compute image size for a given format and dimensions
 //==========================================================================
 size_t VTFDecoder::ComputeImageSize(uint32_t width, uint32_t height, VTFImageFormat format)
 {
 uint32_t w = (width > 0) ? width : 1;
 uint32_t h = (height > 0) ? height : 1;

 switch (format) {
 case VTFImageFormat::RGBA8888:
 case VTFImageFormat::ABGR8888:
 case VTFImageFormat::ARGB8888:
 case VTFImageFormat::BGRA8888:
 case VTFImageFormat::BGRX8888:
 case VTFImageFormat::UVWQ8888:
 case VTFImageFormat::UVLX8888:
 return static_cast<size_t>(w) * h * 4;

 case VTFImageFormat::RGB888:
 case VTFImageFormat::BGR888:
 case VTFImageFormat::RGB888_BLUESCREEN:
 case VTFImageFormat::BGR888_BLUESCREEN:
 return static_cast<size_t>(w) * h * 3;

 case VTFImageFormat::RGB565:
 case VTFImageFormat::BGR565:
 case VTFImageFormat::BGRA4444:
 case VTFImageFormat::BGRX5551:
 case VTFImageFormat::BGRA5551:
 case VTFImageFormat::UV88:
 case VTFImageFormat::IA88:
 return static_cast<size_t>(w) * h * 2;

 case VTFImageFormat::I8:
 case VTFImageFormat::A8:
 return static_cast<size_t>(w) * h;

 case VTFImageFormat::DXT1:
 case VTFImageFormat::DXT1_ONEBITALPHA: {
 uint32_t blocksW = (w + 3) / 4;
 uint32_t blocksH = (h + 3) / 4;
 return static_cast<size_t>(blocksW) * blocksH * 8;
 }

 case VTFImageFormat::DXT3:
 case VTFImageFormat::DXT5: {
 uint32_t blocksW = (w + 3) / 4;
 uint32_t blocksH = (h + 3) / 4;
 return static_cast<size_t>(blocksW) * blocksH * 16;
 }

 case VTFImageFormat::RGBA16161616F:
 case VTFImageFormat::RGBA16161616:
 return static_cast<size_t>(w) * h * 8;

 default:
 return static_cast<size_t>(w) * h * 4;
 }
 }

 //==========================================================================
 // Decode VTF to BGRA32
 //==========================================================================
 VTFDecoder::DecodeResult VTFDecoder::Decode(const std::string& filePath,
 uint32_t targetWidth) const
 {
 DecodeResult result;
 TextureInfo info = ReadInfo(filePath);
 if (!info.IsValid()) {
 result.error = "Invalid VTF header";
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

 // Read header to get headerSize
 const VTFHeader* hdr = reinterpret_cast<const VTFHeader*>(fileData.data());
 size_t dataOffset = hdr->headerSize;

 // Skip low-res thumbnail if present
 if (info.hasLowResImage) {
 VTFImageFormat lowFmt = static_cast<VTFImageFormat>(hdr->lowResImageFormat);
 size_t lowResSize = ComputeImageSize(info.lowResWidth, info.lowResHeight, lowFmt);
 dataOffset += lowResSize;
 }

 // Find the best mip level for target size
 // VTF stores mipmaps smallest-first, then largest
 // Mip 0 is the largest image
 uint32_t bestMip = 0;
 {
 uint32_t w = info.width, h = info.height;
 const uint32_t maxMip = (info.mipmapCount > 0)
 ? static_cast<uint32_t>(info.mipmapCount - 1)
 : 0u;
 while (bestMip < maxMip) {
 uint32_t nextW = (w > 1) ? w / 2 : 1;
 uint32_t nextH = (h > 1) ? h / 2 : 1;
 if (nextW <= targetWidth && nextH <= targetWidth) break;
 w = nextW;
 h = nextH;
 bestMip++;
 }
 }

 uint32_t mipW = info.width >> bestMip;
 uint32_t mipH = info.height >> bestMip;
 if (mipW == 0) mipW = 1;
 if (mipH == 0) mipH = 1;

 result.width = mipW;
 result.height = mipH;

 // Calculate offset to the target mip level
 // VTF stores mipmaps from smallest to largest
 // We need to skip all smaller mipmaps and all frames
 size_t mipOffset = dataOffset;
 for (int m = static_cast<int>(info.mipmapCount) - 1; m > static_cast<int>(bestMip); --m) {
 uint32_t mw = info.width >> m;
 uint32_t mh = info.height >> m;
 if (mw == 0) mw = 1;
 if (mh == 0) mh = 1;
 size_t mipSize = ComputeImageSize(mw, mh, info.format);
 mipOffset += mipSize * info.frames; // Skip all frames for this mip
 }

 size_t imageSize = ComputeImageSize(mipW, mipH, info.format);
 if (mipOffset + imageSize > fileSize) {
 result.error = "VTF data truncated";
 return result;
 }

 const uint8_t* imageData = fileData.data() + mipOffset;

 // Decode based on format
 switch (info.format) {
 case VTFImageFormat::BGRA8888:
 case VTFImageFormat::BGRX8888:
 result.pixelData.assign(imageData, imageData + imageSize);
 break;

 case VTFImageFormat::RGBA8888: {
 result.pixelData.resize(static_cast<size_t>(mipW) * mipH * 4);
 for (uint32_t i = 0; i < mipW * mipH; ++i) {
 result.pixelData[i * 4 + 0] = imageData[i * 4 + 2]; // B
 result.pixelData[i * 4 + 1] = imageData[i * 4 + 1]; // G
 result.pixelData[i * 4 + 2] = imageData[i * 4 + 0]; // R
 result.pixelData[i * 4 + 3] = imageData[i * 4 + 3]; // A
 }
 break;
 }

 case VTFImageFormat::BGR888:
 case VTFImageFormat::BGR888_BLUESCREEN: {
 result.pixelData.resize(static_cast<size_t>(mipW) * mipH * 4);
 for (uint32_t i = 0; i < mipW * mipH; ++i) {
 result.pixelData[i * 4 + 0] = imageData[i * 3 + 0]; // B
 result.pixelData[i * 4 + 1] = imageData[i * 3 + 1]; // G
 result.pixelData[i * 4 + 2] = imageData[i * 3 + 2]; // R
 result.pixelData[i * 4 + 3] = 255;
 }
 break;
 }

 case VTFImageFormat::RGB888:
 case VTFImageFormat::RGB888_BLUESCREEN: {
 result.pixelData.resize(static_cast<size_t>(mipW) * mipH * 4);
 for (uint32_t i = 0; i < mipW * mipH; ++i) {
 result.pixelData[i * 4 + 0] = imageData[i * 3 + 2]; // B
 result.pixelData[i * 4 + 1] = imageData[i * 3 + 1]; // G
 result.pixelData[i * 4 + 2] = imageData[i * 3 + 0]; // R
 result.pixelData[i * 4 + 3] = 255;
 }
 break;
 }

 case VTFImageFormat::DXT1:
 case VTFImageFormat::DXT1_ONEBITALPHA: {
 result.pixelData.resize(static_cast<size_t>(mipW) * mipH * 4);
 DecompressDXT1(imageData, result.pixelData.data(), mipW, mipH);
 break;
 }

 case VTFImageFormat::DXT5: {
 result.pixelData.resize(static_cast<size_t>(mipW) * mipH * 4);
 DecompressDXT5(imageData, result.pixelData.data(), mipW, mipH);
 break;
 }

 case VTFImageFormat::I8: {
 result.pixelData.resize(static_cast<size_t>(mipW) * mipH * 4);
 for (uint32_t i = 0; i < mipW * mipH; ++i) {
 uint8_t v = imageData[i];
 result.pixelData[i * 4 + 0] = v;
 result.pixelData[i * 4 + 1] = v;
 result.pixelData[i * 4 + 2] = v;
 result.pixelData[i * 4 + 3] = 255;
 }
 break;
 }

 default:
 result.error = "Unsupported VTF format";
 return result;
 }

 result.success = true;
 return result;
 }

 //==========================================================================
 // BC1 (DXT1) block decompression
 //==========================================================================
 void VTFDecoder::DecompressBC1Block(const uint8_t* block, uint8_t* output,
 uint32_t outputStride)
 {
 uint16_t c0 = block[0] | (block[1] << 8);
 uint16_t c1 = block[2] | (block[3] << 8);

 uint8_t colors[4][4]; // [index][BGRA]
 auto rgb565 = [](uint16_t c, uint8_t out[4]) {
 out[2] = static_cast<uint8_t>(((c >> 11) & 0x1F) * 255 / 31); // R
 out[1] = static_cast<uint8_t>(((c >> 5) & 0x3F) * 255 / 63); // G
 out[0] = static_cast<uint8_t>((c & 0x1F) * 255 / 31); // B
 out[3] = 255;
 };

 rgb565(c0, colors[0]);
 rgb565(c1, colors[1]);

 if (c0 > c1) {
 for (int i = 0; i < 3; ++i) {
 colors[2][i] = static_cast<uint8_t>((2 * colors[0][i] + colors[1][i]) / 3);
 colors[3][i] = static_cast<uint8_t>((colors[0][i] + 2 * colors[1][i]) / 3);
 }
 colors[2][3] = colors[3][3] = 255;
 }
 else {
 for (int i = 0; i < 3; ++i) {
 colors[2][i] = static_cast<uint8_t>((colors[0][i] + colors[1][i]) / 2);
 }
 colors[2][3] = 255;
 colors[3][0] = colors[3][1] = colors[3][2] = 0;
 colors[3][3] = 0;
 }

 uint32_t indices = block[4] | (block[5] << 8) | (block[6] << 16) | (block[7] << 24);
 for (int y = 0; y < 4; ++y) {
 for (int x = 0; x < 4; ++x) {
 int idx = (indices >> (2 * (y * 4 + x))) & 0x3;
 uint8_t* dest = output + y * outputStride + x * 4;
 memcpy(dest, colors[idx], 4);
 }
 }
 }

 //==========================================================================
 // DXT1 full image decompression
 //==========================================================================
 void VTFDecoder::DecompressDXT1(const uint8_t* src, uint8_t* dst,
 uint32_t width, uint32_t height)
 {
 uint32_t blocksW = (width + 3) / 4;
 uint32_t blocksH = (height + 3) / 4;

 for (uint32_t by = 0; by < blocksH; ++by) {
 for (uint32_t bx = 0; bx < blocksW; ++bx) {
 uint8_t blockPixels[4 * 16]; // 4x4 * BGRA
 DecompressBC1Block(src, blockPixels, 4 * 4);

 for (int py = 0; py < 4 && (by * 4 + py) < height; ++py) {
 for (int px = 0; px < 4 && (bx * 4 + px) < width; ++px) {
 size_t dstOff = (static_cast<size_t>(by * 4 + py) * width + bx * 4 + px) * 4;
 size_t srcOff = static_cast<size_t>(py * 4 + px) * 4;
 memcpy(dst + dstOff, blockPixels + srcOff, 4);
 }
 }
 src += 8; // BC1 block = 8 bytes
 }
 }
 }

 //==========================================================================
 // DXT5 full image decompression
 //==========================================================================
 void VTFDecoder::DecompressDXT5(const uint8_t* src, uint8_t* dst,
 uint32_t width, uint32_t height)
 {
 uint32_t blocksW = (width + 3) / 4;
 uint32_t blocksH = (height + 3) / 4;

 for (uint32_t by = 0; by < blocksH; ++by) {
 for (uint32_t bx = 0; bx < blocksW; ++bx) {
 // DXT5: 8 bytes alpha block + 8 bytes color block
 // Alpha block
 uint8_t a0 = src[0];
 uint8_t a1 = src[1];
 uint8_t alphas[8];
 alphas[0] = a0;
 alphas[1] = a1;
 if (a0 > a1) {
 for (int i = 1; i < 7; ++i)
 alphas[i + 1] = static_cast<uint8_t>(((7 - i) * a0 + i * a1) / 7);
 }
 else {
 for (int i = 1; i < 5; ++i)
 alphas[i + 1] = static_cast<uint8_t>(((5 - i) * a0 + i * a1) / 5);
 alphas[6] = 0;
 alphas[7] = 255;
 }

 // Read 48-bit alpha index table
 uint64_t alphaBits = 0;
 for (int i = 0; i < 6; ++i)
 alphaBits |= static_cast<uint64_t>(src[2 + i]) << (8 * i);

 uint8_t alphaIndices[16];
 for (int i = 0; i < 16; ++i) {
 alphaIndices[i] = static_cast<uint8_t>((alphaBits >> (3 * i)) & 0x7);
 }

 // Color block (same as BC1)
 uint8_t blockPixels[4 * 16];
 DecompressBC1Block(src + 8, blockPixels, 4 * 4);

 // Apply alpha
 for (int i = 0; i < 16; ++i) {
 blockPixels[i * 4 + 3] = alphas[alphaIndices[i]];
 }

 // Copy to destination
 for (int py = 0; py < 4 && (by * 4 + py) < height; ++py) {
 for (int px = 0; px < 4 && (bx * 4 + px) < width; ++px) {
 size_t dstOff = (static_cast<size_t>(by * 4 + py) * width + bx * 4 + px) * 4;
 size_t srcOff = static_cast<size_t>(py * 4 + px) * 4;
 memcpy(dst + dstOff, blockPixels + srcOff, 4);
 }
 }
 src += 16; // DXT5 block = 16 bytes
 }
 }
 }

} // namespace ExplorerLens::Decoders

