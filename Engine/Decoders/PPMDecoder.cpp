// PPMDecoder.cpp - Netpbm Format Decoder Implementation
// ExplorerLens Engine v6.1.0+
// Supports P1-P6 magic, PFM float format

#include "PPMDecoder.h"
#include <fstream>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <vector>

namespace ExplorerLens {
namespace Engine {

const wchar_t* PPMDecoder::m_extensions[] = {
 L".ppm", L".pgm", L".pbm", L".pnm", L".pam", L".pfm"
};
const uint32_t PPMDecoder::m_extensionCount = 6;

PPMDecoder::PPMDecoder() {}
PPMDecoder::~PPMDecoder() {}

bool PPMDecoder::CanDecode(const wchar_t* filePath) {
    if (!filePath) return false;
    const wchar_t* ext = wcsrchr(filePath, L'.');
    if (!ext) return false;
    for (uint32_t i = 0; i < m_extensionCount; ++i) {
        if (_wcsicmp(ext, m_extensions[i]) == 0) return true;
    }
    return false;
}

HRESULT PPMDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
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

DecoderInfo PPMDecoder::GetInfo() const {
    DecoderInfo info;
    info.name = L"PPMDecoder";
    info.version = L"1.0.0";
    info.supportedExtensions = m_extensions;
    info.extensionCount = m_extensionCount;
    info.supportsGPU = false;
    info.isArchiveDecoder = false;
    return info;
}

const wchar_t** PPMDecoder::GetSupportedExtensions() const { return m_extensions; }

HRESULT PPMDecoder::DecodeFromFile(const wchar_t* path, HBITMAP* phBitmap) {
    if (!phBitmap) return E_POINTER;
    *phBitmap = nullptr;

    size_t fileSize = 0;
    auto data = ReadFileData(path, fileSize);
    if (!data || fileSize < 3) return E_FAIL;

    // Check magic bytes
    char magic[3] = { (char)data[0], (char)data[1], '\0' };

    // PFM format
    const wchar_t* ext = wcsrchr(path, L'.');
    if (ext && _wcsicmp(ext, L".pfm") == 0) {
        return DecodePFM(data.get(), fileSize, phBitmap);
    }

    if (magic[0] == 'P') {
        switch (magic[1]) {
        case '1': case '4': return DecodePBM(data.get(), fileSize, phBitmap);
        case '2': case '5': return DecodePGM(data.get(), fileSize, phBitmap);
        case '3': case '6': return DecodePPM(data.get(), fileSize, phBitmap);
        }
    }
    return E_FAIL;
}

void PPMDecoder::SkipWhitespaceAndComments(const char*& p, const char* end) {
    while (p < end) {
        if (*p == '#') {
            while (p < end && *p != '\n') p++;
            if (p < end) p++;
        }
        else if (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
            p++;
        }
        else {
            break;
        }
    }
}

int PPMDecoder::ReadASCIIInt(const char*& p, const char* end) {
    SkipWhitespaceAndComments(p, end);
    int val = 0;
    while (p < end && *p >= '0' && *p <= '9') {
        val = val * 10 + (*p - '0');
        p++;
    }
    return val;
}

HRESULT PPMDecoder::DecodePPM(const uint8_t* data, size_t size, HBITMAP* phBitmap) {
    const char* p = reinterpret_cast<const char*>(data);
    const char* end = p + size;
    bool binary = (p[1] == '6');
    p += 2; // Skip magic

    int width = ReadASCIIInt(p, end);
    int height = ReadASCIIInt(p, end);
    int maxval = ReadASCIIInt(p, end);

    if (width <= 0 || height <= 0 || width > 16384 || height > 16384 || maxval <= 0) return E_FAIL;

    // Skip single whitespace after maxval
    if (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++;

    auto bgra = std::make_unique<uint8_t[]>(width * height * 4);

    if (binary) {
        // P6: binary RGB
        for (int i = 0; i < width * height; ++i) {
            if (p + 3 > end) return E_FAIL;
            uint8_t r = (uint8_t)((uint8_t)*p++ * 255 / maxval);
            uint8_t g = (uint8_t)((uint8_t)*p++ * 255 / maxval);
            uint8_t b = (uint8_t)((uint8_t)*p++ * 255 / maxval);
            bgra[i * 4 + 0] = b;
            bgra[i * 4 + 1] = g;
            bgra[i * 4 + 2] = r;
            bgra[i * 4 + 3] = 255;
        }
    }
    else {
        // P3: ASCII RGB
        for (int i = 0; i < width * height; ++i) {
            int r = ReadASCIIInt(p, end) * 255 / (std::max)(1, maxval);
            int g = ReadASCIIInt(p, end) * 255 / (std::max)(1, maxval);
            int b = ReadASCIIInt(p, end) * 255 / (std::max)(1, maxval);
            bgra[i * 4 + 0] = (uint8_t)(std::min)(255, b);
            bgra[i * 4 + 1] = (uint8_t)(std::min)(255, g);
            bgra[i * 4 + 2] = (uint8_t)(std::min)(255, r);
            bgra[i * 4 + 3] = 255;
        }
    }

    *phBitmap = CreateBitmap32(width, height, bgra.get());
    return (*phBitmap) ? S_OK : E_FAIL;
}

HRESULT PPMDecoder::DecodePGM(const uint8_t* data, size_t size, HBITMAP* phBitmap) {
    const char* p = reinterpret_cast<const char*>(data);
    const char* end = p + size;
    bool binary = (p[1] == '5');
    p += 2;

    int width = ReadASCIIInt(p, end);
    int height = ReadASCIIInt(p, end);
    int maxval = ReadASCIIInt(p, end);

    if (width <= 0 || height <= 0 || width > 16384 || height > 16384 || maxval <= 0) return E_FAIL;
    if (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++;

    auto bgra = std::make_unique<uint8_t[]>(width * height * 4);

    if (binary) {
        for (int i = 0; i < width * height; ++i) {
            if (p >= end) return E_FAIL;
            uint8_t v = (uint8_t)((uint8_t)*p++ * 255 / maxval);
            bgra[i * 4 + 0] = v;
            bgra[i * 4 + 1] = v;
            bgra[i * 4 + 2] = v;
            bgra[i * 4 + 3] = 255;
        }
    }
    else {
        for (int i = 0; i < width * height; ++i) {
            int v = ReadASCIIInt(p, end) * 255 / (std::max)(1, maxval);
            uint8_t gray = (uint8_t)(std::min)(255, v);
            bgra[i * 4 + 0] = gray;
            bgra[i * 4 + 1] = gray;
            bgra[i * 4 + 2] = gray;
            bgra[i * 4 + 3] = 255;
        }
    }

    *phBitmap = CreateBitmap32(width, height, bgra.get());
    return (*phBitmap) ? S_OK : E_FAIL;
}

HRESULT PPMDecoder::DecodePBM(const uint8_t* data, size_t size, HBITMAP* phBitmap) {
    const char* p = reinterpret_cast<const char*>(data);
    const char* end = p + size;
    bool binary = (p[1] == '4');
    p += 2;

    int width = ReadASCIIInt(p, end);
    int height = ReadASCIIInt(p, end);
    // PBM has no maxval

    if (width <= 0 || height <= 0 || width > 16384 || height > 16384) return E_FAIL;
    if (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++;

    auto bgra = std::make_unique<uint8_t[]>(width * height * 4);

    if (binary) {
        int rowBytes = (width + 7) / 8;
        for (int y = 0; y < height; ++y) {
            if (p + rowBytes > end) return E_FAIL;
            for (int x = 0; x < width; ++x) {
                int byteIdx = x / 8;
                int bitIdx = 7 - (x % 8);
                bool black = ((uint8_t)p[byteIdx] >> bitIdx) & 1;
                uint8_t v = black ? 0 : 255; // PBM: 1=black, 0=white
                int i = y * width + x;
                bgra[i * 4 + 0] = v;
                bgra[i * 4 + 1] = v;
                bgra[i * 4 + 2] = v;
                bgra[i * 4 + 3] = 255;
            }
            p += rowBytes;
        }
    }
    else {
        for (int i = 0; i < width * height; ++i) {
            int v = ReadASCIIInt(p, end);
            uint8_t color = (v == 0) ? 255 : 0; // PBM: 1=black, 0=white
            bgra[i * 4 + 0] = color;
            bgra[i * 4 + 1] = color;
            bgra[i * 4 + 2] = color;
            bgra[i * 4 + 3] = 255;
        }
    }

    *phBitmap = CreateBitmap32(width, height, bgra.get());
    return (*phBitmap) ? S_OK : E_FAIL;
}

HRESULT PPMDecoder::DecodePFM(const uint8_t* data, size_t size, HBITMAP* phBitmap) {
    const char* p = reinterpret_cast<const char*>(data);
    const char* end = p + size;

    // PFM magic: "PF" (color) or "Pf" (grayscale)
    bool color = (p[1] == 'F');
    p += 2;

    int width = ReadASCIIInt(p, end);
    int height = ReadASCIIInt(p, end);

    SkipWhitespaceAndComments(p, end);
    // Read scale/endian factor
    float scale = 0.0f;
    bool negative = false;
    if (p < end && *p == '-') { negative = true; p++; }
    while (p < end && ((*p >= '0' && *p <= '9') || *p == '.')) {
        // Simple float parse
        scale = scale * 10.0f + (*p - '0');
        p++;
    }
    if (negative) scale = -scale;
    bool littleEndian = (scale < 0);
    scale = fabsf(scale);
    if (scale == 0.0f) scale = 1.0f;

    if (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++;

    if (width <= 0 || height <= 0 || width > 16384 || height > 16384) return E_FAIL;

    int channels = color ? 3 : 1;
    const float* fp = reinterpret_cast<const float*>(p);
    size_t floatsNeeded = (size_t)width * height * channels;
    if ((const char*)(fp + floatsNeeded) > end) return E_FAIL;

    // Byte-swap floats for big-endian PFM files (positive scale indicates big-endian)
    std::vector<float> swapped;
    if (!littleEndian) {
        swapped.resize(floatsNeeded);
        for (size_t i = 0; i < floatsNeeded; ++i) {
            uint32_t bits;
            memcpy(&bits, &fp[i], sizeof(bits));
            bits = ((bits >> 24) & 0xFF) | ((bits >> 8) & 0xFF00) |
                ((bits << 8) & 0xFF0000) | ((bits << 24) & 0xFF000000);
            memcpy(&swapped[i], &bits, sizeof(bits));
        }
        fp = swapped.data();
    }

    auto bgra = std::make_unique<uint8_t[]>(width * height * 4);
    const float gamma = 1.0f / 2.2f;

    // PFM is bottom-to-top
    for (int y = 0; y < height; ++y) {
        int srcY = height - 1 - y;
        for (int x = 0; x < width; ++x) {
            int srcIdx = (srcY * width + x) * channels;
            float r, g, b;
            if (color) {
                r = fp[srcIdx + 0] / scale;
                g = fp[srcIdx + 1] / scale;
                b = fp[srcIdx + 2] / scale;
            }
            else {
                r = g = b = fp[srcIdx + 0] / scale;
            }
            // Tone map and gamma
            r = r / (1.0f + r); g = g / (1.0f + g); b = b / (1.0f + b);
            r = powf((std::max)(0.0f, r), gamma);
            g = powf((std::max)(0.0f, g), gamma);
            b = powf((std::max)(0.0f, b), gamma);

            int i = y * width + x;
            bgra[i * 4 + 0] = (uint8_t)((std::min)(1.0f, b) * 255.0f);
            bgra[i * 4 + 1] = (uint8_t)((std::min)(1.0f, g) * 255.0f);
            bgra[i * 4 + 2] = (uint8_t)((std::min)(1.0f, r) * 255.0f);
            bgra[i * 4 + 3] = 255;
        }
    }

    *phBitmap = CreateBitmap32(width, height, bgra.get());
    return (*phBitmap) ? S_OK : E_FAIL;
}

HBITMAP PPMDecoder::CreateBitmap32(uint32_t width, uint32_t height, const uint8_t* bgra) {
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
    memcpy(pBits, bgra, width * height * 4);
    return hBmp;
}

std::unique_ptr<uint8_t[]> PPMDecoder::ReadFileData(const wchar_t* path, size_t& fileSize) {
    fileSize = 0;
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return nullptr;
    fileSize = static_cast<size_t>(file.tellg());
    if (fileSize > 100 * 1024 * 1024) return nullptr; // 100 MB cap
    file.seekg(0, std::ios::beg);
    auto data = std::make_unique<uint8_t[]>(fileSize);
    file.read(reinterpret_cast<char*>(data.get()), fileSize);
    return data;
}

} // namespace Engine
} // namespace ExplorerLens
