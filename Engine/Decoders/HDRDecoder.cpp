// HDRDecoder.cpp - Radiance RGBE HDR Image Decoder Implementation
// ExplorerLens Engine v6.1.0+
// Specification: Radiance HDR (RGBE) format - Greg Ward

#include "HDRDecoder.h"
#include <fstream>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <smmintrin.h> // SSE4.1

namespace ExplorerLens {
namespace Engine {

const wchar_t* HDRDecoder::m_extensions[] = { L".hdr" };
const uint32_t HDRDecoder::m_extensionCount = 1;

HDRDecoder::HDRDecoder() {}
HDRDecoder::~HDRDecoder() {}

bool HDRDecoder::CanDecode(const wchar_t* filePath) {
 if (!filePath || filePath[0] == L'\0') return false;
 const wchar_t* ext = wcsrchr(filePath, L'.');
 if (!ext) return false;
 return _wcsicmp(ext, L".hdr") == 0 && IsHDRFormat(filePath);
}

bool HDRDecoder::IsHDRFormat(const wchar_t* path) {
 size_t fileSize = 0;
 auto data = ReadFileData(path, fileSize);
 if (!data || fileSize < 11) return false;
 return memcmp(data.get(), "#?RADIANCE", 10) == 0 ||
 memcmp(data.get(), "#?RGBE", 6) == 0;
}

HRESULT HDRDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
 result.hBitmap = nullptr;
 result.width = 0;
 result.height = 0;
 if (!request.filePath) return E_INVALIDARG;
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

DecoderInfo HDRDecoder::GetInfo() const {
 DecoderInfo info;
 info.name = L"HDRDecoder";
 info.version = L"1.0.0";
 info.supportedExtensions = m_extensions;
 info.extensionCount = m_extensionCount;
 info.supportsGPU = false;
 info.isArchiveDecoder = false;
 return info;
}

const wchar_t** HDRDecoder::GetSupportedExtensions() const { return m_extensions; }

HRESULT HDRDecoder::DecodeFromFile(const wchar_t* path, HBITMAP* phBitmap) {
 if (!phBitmap) return E_POINTER;
 *phBitmap = nullptr;

 size_t fileSize = 0;
 auto fileData = ReadFileData(path, fileSize);
 if (!fileData || fileSize < 32) return E_FAIL;

 float* pixels = nullptr;
 uint32_t width = 0, height = 0;

 HRESULT hr = ParseHDR(fileData.get(), fileSize, &pixels, &width, &height);
 if (FAILED(hr) || !pixels) return hr;

 ToneMapReinhard_SSE(pixels, width * height);
 *phBitmap = CreateBitmapFromFloat(pixels, width, height);
 delete[] pixels;
 return (*phBitmap) ? S_OK : E_FAIL;
}

HRESULT HDRDecoder::ParseHDR(const uint8_t* data, size_t size,
 float** ppPixels, uint32_t* pWidth, uint32_t* pHeight) {
 const char* text = reinterpret_cast<const char*>(data);
 const char* end = text + size;

 // Skip header lines until empty line
 const char* p = text;
 while (p < end - 1) {
 if (*p == '\n' && *(p + 1) == '\n') { p += 2; break; }
 if (*p == '\n' && *(p + 1) == '\r' && p + 2 < end && *(p + 2) == '\n') { p += 3; break; }
 p++;
 }
 // Also handle \r\n line endings
 if (p >= end) {
 p = text;
 while (p < end - 1) {
 if (*p == '\n') { p++; break; }
 p++;
 }
 }

 // Parse resolution line: "-Y height +X width"
 uint32_t width = 0, height = 0;
 if (sscanf_s(p, "-Y %u +X %u", &height, &width) != 2) {
 // Try alternative format
 if (sscanf_s(p, "+Y %u +X %u", &height, &width) != 2) {
 return E_FAIL;
 }
 }

 if (width == 0 || height == 0 || width > 16384 || height > 16384) return E_FAIL;

 // Advance past resolution line
 while (p < end && *p != '\n') p++;
 if (p < end) p++;

 const uint8_t* pixData = reinterpret_cast<const uint8_t*>(p);
 const uint8_t* pixEnd = data + size;

 *ppPixels = new float[width * height * 3];
 *pWidth = width;
 *pHeight = height;

 auto scanline = std::make_unique<RGBE[]>(width);

 for (uint32_t y = 0; y < height; ++y) {
 HRESULT hr = DecodeScanlineRLE(pixData, pixEnd, scanline.get(), width);
 if (FAILED(hr)) {
 delete[] *ppPixels;
 *ppPixels = nullptr;
 return hr;
 }
 for (uint32_t x = 0; x < width; ++x) {
 float r, g, b;
 RGBEToFloat(scanline[x], r, g, b);
 uint32_t idx = (y * width + x) * 3;
 (*ppPixels)[idx + 0] = r;
 (*ppPixels)[idx + 1] = g;
 (*ppPixels)[idx + 2] = b;
 }
 }

 return S_OK;
}

HRESULT HDRDecoder::DecodeScanlineRLE(const uint8_t*& p, const uint8_t* end,
 RGBE* scanline, uint32_t width) {
 if (p + 4 > end) return E_FAIL;

 // Check for new-style RLE: byte 0,1 = 0x02,0x02
 if (p[0] == 2 && p[1] == 2 && ((p[2] << 8) | p[3]) == (int)width) {
 p += 4;
 // Decode each of the 4 channels separately
 for (int ch = 0; ch < 4; ++ch) {
 uint32_t x = 0;
 while (x < width) {
 if (p >= end) return E_FAIL;
 uint8_t code = *p++;
 if (code > 128) {
 // RLE run
 uint32_t count = code - 128;
 if (p >= end || x + count > width) return E_FAIL;
 uint8_t val = *p++;
 for (uint32_t i = 0; i < count; ++i) {
 reinterpret_cast<uint8_t*>(&scanline[x + i])[ch] = val;
 }
 x += count;
 } else {
 // Raw
 uint32_t count = code;
 if (p + count > end || x + count > width) return E_FAIL;
 for (uint32_t i = 0; i < count; ++i) {
 reinterpret_cast<uint8_t*>(&scanline[x + i])[ch] = *p++;
 }
 x += count;
 }
 }
 }
 } else {
 // Old-style: raw RGBE pixels
 for (uint32_t x = 0; x < width; ++x) {
 if (p + 4 > end) return E_FAIL;
 scanline[x].r = *p++;
 scanline[x].g = *p++;
 scanline[x].b = *p++;
 scanline[x].e = *p++;
 }
 }
 return S_OK;
}

void HDRDecoder::RGBEToFloat(const RGBE& rgbe, float& r, float& g, float& b) {
 if (rgbe.e == 0) {
 r = g = b = 0.0f;
 } else {
 float f = ldexpf(1.0f, (int)rgbe.e - 128 - 8);
 r = rgbe.r * f;
 g = rgbe.g * f;
 b = rgbe.b * f;
 }
}

void HDRDecoder::ToneMapReinhard(float* pixels, uint32_t pixelCount) {
 // Reinhard global tone mapping with gamma correction
 // L_mapped = L / (1 + L)
 const float gamma = 1.0f / 2.2f;
 for (uint32_t i = 0; i < pixelCount; ++i) {
 float* px = pixels + i * 3;
 for (int c = 0; c < 3; ++c) {
 float v = px[c];
 v = v / (1.0f + v); // Reinhard
 v = powf(v, gamma); // Gamma
 px[c] = (std::max)(0.0f, (std::min)(1.0f, v));
 }
 }
}

void HDRDecoder::ToneMapReinhard_SSE(float* pixels, uint32_t pixelCount) {
 // SSE4.1-accelerated Reinhard tone mapping with gamma correction
 // Process 4 float values at a time (not pixel-aligned, just float stream)
 const __m128 ones = _mm_set1_ps(1.0f);
 const __m128 zeros = _mm_setzero_ps();
 // Approximate gamma 1/2.2 ≈ 0.4545 using fast_pow approximation:
 // v^gamma ≈ exp(gamma * ln(v))
 // For thumbnail quality, we use a piecewise linear approximation:
 // sqrt(x) is gamma 0.5, close enough to 0.4545 for thumbnails 
 // and _mm_sqrt_ps is available in SSE
 
 uint32_t totalFloats = pixelCount * 3;
 uint32_t simdCount = (totalFloats / 4) * 4;
 
 for (uint32_t i = 0; i < simdCount; i += 4) {
 __m128 v = _mm_loadu_ps(pixels + i);
 // Reinhard: v / (1 + v)
 __m128 denom = _mm_add_ps(ones, v);
 v = _mm_div_ps(v, denom);
 // Approximate gamma using sqrt (gamma=0.5 vs 0.4545)
 v = _mm_sqrt_ps(v);
 // Clamp to [0, 1]
 v = _mm_max_ps(zeros, _mm_min_ps(ones, v));
 _mm_storeu_ps(pixels + i, v);
 }
 
 // Handle remaining values with scalar
 const float gamma = 1.0f / 2.2f;
 for (uint32_t i = simdCount; i < totalFloats; ++i) {
 float v = pixels[i];
 v = v / (1.0f + v);
 v = powf(v, gamma);
 pixels[i] = (std::max)(0.0f, (std::min)(1.0f, v));
 }
}

HBITMAP HDRDecoder::CreateBitmapFromFloat(const float* pixels,
 uint32_t width, uint32_t height) {
 BITMAPINFO bmi = {};
 bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
 bmi.bmiHeader.biWidth = width;
 bmi.bmiHeader.biHeight = -(LONG)height;
 bmi.bmiHeader.biPlanes = 1;
 bmi.bmiHeader.biBitCount = 32;
 bmi.bmiHeader.biCompression = BI_RGB;

 void* pBits = nullptr;
 HDC hdc = GetDC(nullptr);
 HBITMAP hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
 ReleaseDC(nullptr, hdc);

 if (!hBmp || !pBits) return nullptr;

 uint8_t* dest = static_cast<uint8_t*>(pBits);
 for (uint32_t i = 0; i < width * height; ++i) {
 const float* px = pixels + i * 3;
 dest[i * 4 + 0] = (uint8_t)(px[2] * 255.0f); // B
 dest[i * 4 + 1] = (uint8_t)(px[1] * 255.0f); // G
 dest[i * 4 + 2] = (uint8_t)(px[0] * 255.0f); // R
 dest[i * 4 + 3] = 255; // A
 }
 return hBmp;
}

std::unique_ptr<uint8_t[]> HDRDecoder::ReadFileData(const wchar_t* path, size_t& fileSize) {
 fileSize = 0;
 std::ifstream file(path, std::ios::binary | std::ios::ate);
 if (!file.is_open()) return nullptr;
 fileSize = static_cast<size_t>(file.tellg());
 if (fileSize > 200 * 1024 * 1024) return nullptr; // 200 MB cap
 file.seekg(0, std::ios::beg);
 auto data = std::make_unique<uint8_t[]>(fileSize);
 file.read(reinterpret_cast<char*>(data.get()), fileSize);
 return data;
}

} // namespace Engine
} // namespace ExplorerLens

