// QOIDecoder.cpp - Quite OK Image Decoder Implementation
// ExplorerLens Engine v5.3.0+
// QOI Specification: https://qoiformat.org/

#include "QOIDecoder.h"
#include <fstream>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// Static members
const wchar_t* QOIDecoder::m_extensions[] = {
 L".qoi"
};

const uint32_t QOIDecoder::m_extensionCount = 1;

QOIDecoder::QOIDecoder() {
}

QOIDecoder::~QOIDecoder() {
}

bool QOIDecoder::CanDecode(const wchar_t* filePath) {
 if (!filePath || filePath[0] == L'\0') {
 return false;
 }
 
 const wchar_t* ext = wcsrchr(filePath, L'.');
 if (!ext) {
 return false;
 }
 
 if (_wcsicmp(ext, L".qoi") == 0) {
 return IsQOIFormat(filePath);
 }
 
 return false;
}

HRESULT QOIDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
 result.hBitmap = nullptr;
 result.width = 0;
 result.height = 0;
 
 if (!request.filePath) {
 return E_INVALIDARG;
 }
 
 HRESULT hr = DecodeFromFile(request.filePath, &result.hBitmap);
 
 if (SUCCEEDED(hr) && result.hBitmap) {
 BITMAP bm;
 if (GetObject(result.hBitmap, sizeof(bm), &bm)) {
 result.width = bm.bmWidth;
 result.height = bm.bmHeight;
 }
 }
 
 return hr;
}

DecoderInfo QOIDecoder::GetInfo() const {
 DecoderInfo info;
 info.name = L"QOIDecoder";
 info.version = L"1.0.0";
 info.supportedExtensions = m_extensions;
 info.extensionCount = m_extensionCount;
 info.supportsGPU = false;
 info.isArchiveDecoder = false;
 return info;
}

const wchar_t** QOIDecoder::GetSupportedExtensions() const {
 return m_extensions;
}

HRESULT QOIDecoder::DecodeFromFile(const wchar_t* path, HBITMAP* phBitmap) {
 if (!phBitmap) {
 return E_POINTER;
 }
 
 *phBitmap = nullptr;
 
 // Read file
 size_t fileSize = 0;
 auto fileData = ReadFileData(path, fileSize);
 
 if (!fileData || fileSize < sizeof(QOIHeader) + 8) {
 return E_FAIL;
 }
 
 // Decode QOI
 uint8_t* pixels = nullptr;
 uint32_t width = 0, height = 0;
 uint8_t channels = 0;
 
 HRESULT hr = DecodeQOI(fileData.get(), fileSize, &pixels, &width, &height, &channels);
 
 if (FAILED(hr) || !pixels) {
 return hr;
 }
 
 // Create bitmap
 *phBitmap = CreateBitmapFromRGBA(pixels, width, height, channels);
 
 delete[] pixels;
 
 return (*phBitmap) ? S_OK : E_FAIL;
}

HRESULT QOIDecoder::DecodeQOI(const uint8_t* data, size_t size, uint8_t** ppPixels,
 uint32_t* pWidth, uint32_t* pHeight, uint8_t* pChannels) {
 if (!ppPixels || !pWidth || !pHeight || !pChannels) {
 return E_POINTER;
 }
 
 if (size < sizeof(QOIHeader) + 8) {
 return E_FAIL;
 }
 
 // Read header
 const QOIHeader* header = reinterpret_cast<const QOIHeader*>(data);
 
 if (memcmp(header->magic, "qoif", 4) != 0) {
 return E_FAIL;
 }
 
 uint32_t width = ReadBE32(reinterpret_cast<const uint8_t*>(&header->width));
 uint32_t height = ReadBE32(reinterpret_cast<const uint8_t*>(&header->height));
 uint8_t channels = header->channels;
 
 if (width == 0 || height == 0 || width > 16384 || height > 16384) {
 return E_FAIL;
 }
 
 if (channels != 3 && channels != 4) {
 return E_FAIL;
 }
 
 // Allocate pixel buffer (always expand to RGBA for consistency)
 uint32_t pixelCount = width * height;
 uint8_t* pixels = new uint8_t[pixelCount * 4];
 
 // QOI decoding
 const uint8_t* p = data + sizeof(QOIHeader);
 const uint8_t* end = data + size - 8; // QOI ends with 8-byte marker
 
 uint8_t px[4] = { 0, 0, 0, 255 }; // Current pixel (RGBA)
 uint8_t index[64][4] = {}; // Color index array
 
 uint32_t pixelIndex = 0;
 
 while (pixelIndex < pixelCount && p < end) {
 uint8_t b1 = *p++;
 
 if (b1 == QOI_OP_RGB) {
 // RGB: 3 bytes follow
 px[0] = *p++;
 px[1] = *p++;
 px[2] = *p++;
 }
 else if (b1 == QOI_OP_RGBA) {
 // RGBA: 4 bytes follow
 px[0] = *p++;
 px[1] = *p++;
 px[2] = *p++;
 px[3] = *p++;
 }
 else if ((b1 & 0xC0) == QOI_OP_INDEX) {
 // Index: use cached color
 uint8_t idx = b1 & 0x3F;
 px[0] = index[idx][0];
 px[1] = index[idx][1];
 px[2] = index[idx][2];
 px[3] = index[idx][3];
 }
 else if ((b1 & 0xC0) == QOI_OP_DIFF) {
 // Diff: small RGB difference
 px[0] += ((b1 >> 4) & 0x03) - 2;
 px[1] += ((b1 >> 2) & 0x03) - 2;
 px[2] += (b1 & 0x03) - 2;
 }
 else if ((b1 & 0xC0) == QOI_OP_LUMA) {
 // Luma: green difference + RB offsets
 uint8_t b2 = *p++;
 int8_t vg = (b1 & 0x3F) - 32;
 px[0] += vg - 8 + ((b2 >> 4) & 0x0F);
 px[1] += vg;
 px[2] += vg - 8 + (b2 & 0x0F);
 }
 else if ((b1 & 0xC0) == QOI_OP_RUN) {
 // Run: repeat current pixel
 // This is handled by not reading new data
 // The pixel copy loop below handles it
 }
 
 // Update index
 uint8_t hash = (px[0] * 3 + px[1] * 5 + px[2] * 7 + px[3] * 11) % 64;
 index[hash][0] = px[0];
 index[hash][1] = px[1];
 index[hash][2] = px[2];
 index[hash][3] = px[3];
 
 // Handle run-length
 uint32_t run = 1;
 if ((b1 & 0xC0) == QOI_OP_RUN) {
 run = (b1 & 0x3F) + 1;
 }
 
 // Write pixels
 for (uint32_t r = 0; r < run && pixelIndex < pixelCount; ++r) {
 pixels[pixelIndex * 4 + 0] = px[0]; // R
 pixels[pixelIndex * 4 + 1] = px[1]; // G
 pixels[pixelIndex * 4 + 2] = px[2]; // B
 pixels[pixelIndex * 4 + 3] = px[3]; // A
 pixelIndex++;
 }
 }
 
 *ppPixels = pixels;
 *pWidth = width;
 *pHeight = height;
 *pChannels = 4; // Always RGBA
 
 return S_OK;
}

bool QOIDecoder::IsQOIFormat(const wchar_t* path) {
 size_t fileSize = 0;
 auto data = ReadFileData(path, fileSize);
 
 return data && IsQOIFormat(data.get(), fileSize);
}

bool QOIDecoder::IsQOIFormat(const uint8_t* data, size_t size) {
 if (!data || size < sizeof(QOIHeader)) {
 return false;
 }
 
 // Check magic "qoif"
 return (data[0] == 'q' && data[1] == 'o' && data[2] == 'i' && data[3] == 'f');
}

HBITMAP QOIDecoder::CreateBitmapFromRGBA(const uint8_t* pixels, uint32_t width,
 uint32_t height, uint8_t channels) {
 (void)channels; // Always RGBA (4 channels) for QOI format
 
 if (!pixels || width == 0 || height == 0) {
 return nullptr;
 }
 
 // Create DIB section
 BITMAPINFO bmi = {};
 bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
 bmi.bmiHeader.biWidth = width;
 bmi.bmiHeader.biHeight = -(LONG)height; // Top-down
 bmi.bmiHeader.biPlanes = 1;
 bmi.bmiHeader.biBitCount = 32;
 bmi.bmiHeader.biCompression = BI_RGB;
 
 void* pBits = nullptr;
 HDC hdcScreen = GetDC(nullptr);
 HBITMAP hBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS,
 &pBits, nullptr, 0);
 ReleaseDC(nullptr, hdcScreen);
 
 if (!hBitmap || !pBits) {
 return nullptr;
 }
 
 // Convert RGBA to BGRA
 uint8_t* dest = static_cast<uint8_t*>(pBits);
 const uint8_t* src = pixels;
 
 for (uint32_t i = 0; i < width * height; ++i) {
 dest[0] = src[2]; // B
 dest[1] = src[1]; // G
 dest[2] = src[0]; // R
 dest[3] = src[3]; // A
 
 src += 4;
 dest += 4;
 }
 
 return hBitmap;
}

uint32_t QOIDecoder::ReadBE32(const uint8_t* data) {
 return (static_cast<uint32_t>(data[0]) << 24) |
 (static_cast<uint32_t>(data[1]) << 16) |
 (static_cast<uint32_t>(data[2]) << 8) |
 static_cast<uint32_t>(data[3]);
}

std::unique_ptr<uint8_t[]> QOIDecoder::ReadFileData(const wchar_t* path, size_t& fileSize) {
 std::ifstream file(path, std::ios::binary | std::ios::ate);
 
 if (!file.is_open()) {
 fileSize = 0;
 return nullptr;
 }
 
 fileSize = static_cast<size_t>(file.tellg());
 file.seekg(0, std::ios::beg);
 
 auto data = std::make_unique<uint8_t[]>(fileSize);
 file.read(reinterpret_cast<char*>(data.get()), fileSize);
 
 return data;
}

} // namespace Engine
} // namespace ExplorerLens

