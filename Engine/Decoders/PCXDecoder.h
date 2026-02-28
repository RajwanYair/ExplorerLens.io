#pragma once
//==============================================================================
// PCXDecoder.h — ZSoft PCX Image Decoder
// ExplorerLens Engine v8.4.0 — Easy Format Wins
//
// Decodes PCX (PC Paintbrush) format images. PCX uses RLE compression
// with planar color data. Supports 1/4/8/24-bit color depths.
//==============================================================================

#include "../Core/IThumbnailDecoder.h"
#include <string>
#include <vector>
#include <cstdint>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// PCX (ZSoft Paintbrush) format decoder.
///
/// PCX header is 128 bytes with fields:
/// - Manufacturer (0x0A), Version, Encoding (RLE), BitsPerPixel
/// - Window: Xmin, Ymin, Xmax, Ymax
/// - HRes, VRes (DPI)
/// - Palette (16 colors, 48 bytes)
/// - NPlanes, BytesPerLine, PaletteInfo
///
/// For 256-color PCX, the VGA palette is appended at EOF (769 bytes: 0x0C + 768).
class PCXDecoder {
public:
 PCXDecoder() = default;
 ~PCXDecoder() = default;

 // IThumbnailDecoder interface
 bool CanDecode(const wchar_t* filePath) const;
 HRESULT Decode(const wchar_t* filePath, uint32_t requestedSize, HBITMAP& hBitmap);
 const wchar_t* GetName() const { return L"PCXDecoder"; }
 bool SupportsGPU() const { return false; }
 bool IsArchiveDecoder() const { return false; }

 static constexpr const wchar_t* s_extensions[] = { L".pcx" };
 static constexpr uint32_t s_extensionCount = 1;

 const wchar_t** GetSupportedExtensions() const {
 return const_cast<const wchar_t**>(s_extensions);
 }
 uint32_t GetExtensionCount() const { return s_extensionCount; }

private:
#pragma pack(push, 1)
 struct PCXHeader {
 uint8_t manufacturer; // Must be 0x0A
 uint8_t version; // 0=v2.5, 2=v2.8+pal, 3=v2.8-pal, 4=Paintbrush Win, 5=v3.0+
 uint8_t encoding; // 1 = RLE
 uint8_t bitsPerPixel; // Bits per pixel per plane
 uint16_t xMin, yMin;
 uint16_t xMax, yMax;
 uint16_t hDPI, vDPI;
 uint8_t egaPalette[48]; // 16-color EGA palette
 uint8_t reserved;
 uint8_t numPlanes;
 uint16_t bytesPerLine; // Bytes per scanline per plane (must be even)
 uint16_t paletteInfo; // 1 = color/BW, 2 = grayscale
 uint16_t hScreenSize;
 uint16_t vScreenSize;
 uint8_t padding[54];
 };
#pragma pack(pop)

 static_assert(sizeof(PCXHeader) == 128, "PCXHeader must be 128 bytes");

 /// Decode PCX data to raw BGRA pixel buffer
 HRESULT DecodePCXData(const uint8_t* data, size_t dataSize,
 std::vector<uint8_t>& pixels,
 uint32_t& width, uint32_t& height);

 /// RLE decode a single scanline
 static size_t DecodeRLEScanline(const uint8_t* src, size_t srcSize,
 uint8_t* dst, uint32_t bytesPerLine);
};

} // namespace Engine
} // namespace ExplorerLens

