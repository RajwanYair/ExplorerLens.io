//==============================================================================
// PCXDecoder.cpp — ZSoft PCX Image Decoder
//==============================================================================

#include "PCXDecoder.h"
#include <fstream>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

bool PCXDecoder::CanDecode(const wchar_t* filePath) const {
 if (!filePath) return false;
 std::wstring path(filePath);
 std::transform(path.begin(), path.end(), path.begin(), ::towlower);
 return path.length() >= 4 &&
 path.compare(path.length() - 4, 4, L".pcx") == 0;
}

HRESULT PCXDecoder::Decode(const wchar_t* filePath, uint32_t requestedSize,
 HBITMAP& hBitmap) {
 if (!filePath || requestedSize == 0) return E_INVALIDARG;

 // Read entire file
 std::ifstream file(filePath, std::ios::binary | std::ios::ate);
 if (!file.is_open()) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

 size_t fileSize = static_cast<size_t>(file.tellg());
 if (fileSize < sizeof(PCXHeader)) return E_FAIL;

 file.seekg(0);
 std::vector<uint8_t> data(fileSize);
 file.read(reinterpret_cast<char*>(data.data()), fileSize);
 file.close();

 // Decode PCX
 std::vector<uint8_t> pixels;
 uint32_t width = 0, height = 0;
 HRESULT hr = DecodePCXData(data.data(), data.size(), pixels, width, height);
 if (FAILED(hr)) return hr;

 // Create HBITMAP
 BITMAPINFO bmi = {};
 bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
 bmi.bmiHeader.biWidth = static_cast<LONG>(width);
 bmi.bmiHeader.biHeight = -static_cast<LONG>(height); // top-down
 bmi.bmiHeader.biPlanes = 1;
 bmi.bmiHeader.biBitCount = 32;
 bmi.bmiHeader.biCompression = BI_RGB;

 void* bits = nullptr;
 hBitmap = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
 if (!hBitmap || !bits) return E_OUTOFMEMORY;

 memcpy(bits, pixels.data(), width * height * 4);
 return S_OK;
}

size_t PCXDecoder::DecodeRLEScanline(const uint8_t* src, size_t srcSize,
 uint8_t* dst, uint32_t bytesPerLine) {
 size_t srcPos = 0;
 uint32_t dstPos = 0;

 while (dstPos < bytesPerLine && srcPos < srcSize) {
 uint8_t byte = src[srcPos++];
 if ((byte & 0xC0) == 0xC0) {
 // RLE run
 uint32_t count = byte & 0x3F;
 if (srcPos >= srcSize) break;
 uint8_t value = src[srcPos++];
 for (uint32_t i = 0; i < count && dstPos < bytesPerLine; ++i) {
 dst[dstPos++] = value;
 }
 } else {
 // Literal byte
 dst[dstPos++] = byte;
 }
 }
 return srcPos;
}

HRESULT PCXDecoder::DecodePCXData(const uint8_t* data, size_t dataSize,
 std::vector<uint8_t>& pixels,
 uint32_t& width, uint32_t& height) {
 if (dataSize < sizeof(PCXHeader)) return E_FAIL;

 const auto* header = reinterpret_cast<const PCXHeader*>(data);

 // Validate header
 if (header->manufacturer != 0x0A) return E_FAIL;
 if (header->encoding != 1) return E_FAIL; // Must be RLE

 width = header->xMax - header->xMin + 1;
 height = header->yMax - header->yMin + 1;

 if (width == 0 || height == 0 || width > 65535 || height > 65535)
 return E_FAIL;

 uint32_t bpp = header->bitsPerPixel;
 uint32_t planes = header->numPlanes;
 uint32_t bytesPerLine = header->bytesPerLine;
 uint32_t scanlineSize = bytesPerLine * planes;

 pixels.resize(width * height * 4);

 std::vector<uint8_t> scanline(scanlineSize);
 const uint8_t* src = data + 128; // Skip header
 size_t remaining = dataSize - 128;

 for (uint32_t y = 0; y < height; ++y) {
 // Decode one scanline
 size_t consumed = DecodeRLEScanline(src, remaining, scanline.data(), scanlineSize);
 src += consumed;
 remaining -= (std::min)(consumed, remaining);

 uint32_t rowOffset = y * width * 4;

 if (planes == 3 && bpp == 8) {
 // 24-bit RGB (3 planes, 8 bits each)
 for (uint32_t x = 0; x < width; ++x) {
 pixels[rowOffset + x * 4 + 0] = scanline[bytesPerLine * 2 + x]; // B
 pixels[rowOffset + x * 4 + 1] = scanline[bytesPerLine * 1 + x]; // G
 pixels[rowOffset + x * 4 + 2] = scanline[bytesPerLine * 0 + x]; // R
 pixels[rowOffset + x * 4 + 3] = 255; // A
 }
 } else if (planes == 1 && bpp == 8) {
 // 8-bit indexed — palette at end of file
 const uint8_t* palette = nullptr;
 if (dataSize >= 769 && data[dataSize - 769] == 0x0C) {
 palette = data + dataSize - 768;
 }

 for (uint32_t x = 0; x < width; ++x) {
 uint8_t index = scanline[x];
 if (palette) {
 pixels[rowOffset + x * 4 + 0] = palette[index * 3 + 2]; // B
 pixels[rowOffset + x * 4 + 1] = palette[index * 3 + 1]; // G
 pixels[rowOffset + x * 4 + 2] = palette[index * 3 + 0]; // R
 } else {
 // Grayscale fallback
 pixels[rowOffset + x * 4 + 0] = index;
 pixels[rowOffset + x * 4 + 1] = index;
 pixels[rowOffset + x * 4 + 2] = index;
 }
 pixels[rowOffset + x * 4 + 3] = 255;
 }
 } else if (planes == 1 && bpp == 1) {
 // 1-bit monochrome
 for (uint32_t x = 0; x < width; ++x) {
 uint8_t bit = (scanline[x / 8] >> (7 - (x % 8))) & 1;
 uint8_t val = bit ? 255 : 0;
 pixels[rowOffset + x * 4 + 0] = val;
 pixels[rowOffset + x * 4 + 1] = val;
 pixels[rowOffset + x * 4 + 2] = val;
 pixels[rowOffset + x * 4 + 3] = 255;
 }
 } else {
 // Unsupported depth — fill white
 for (uint32_t x = 0; x < width; ++x) {
 pixels[rowOffset + x * 4 + 0] = 255;
 pixels[rowOffset + x * 4 + 1] = 255;
 pixels[rowOffset + x * 4 + 2] = 255;
 pixels[rowOffset + x * 4 + 3] = 255;
 }
 }
 }

 return S_OK;
}

} // namespace Engine
} // namespace ExplorerLens

