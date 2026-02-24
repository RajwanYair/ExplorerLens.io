// PSDDecoder.cpp - Adobe Photoshop PSD/PSB Decoder Implementation
// ExplorerLens Engine v6.1.0+
// Specification: Adobe Photoshop File Formats Specification (2024)

#include "PSDDecoder.h"
#include <fstream>
#include <algorithm>
#include <cstring>
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

namespace ExplorerLens {
namespace Engine {

// Static members
const wchar_t* PSDDecoder::m_extensions[] = {
    L".psd",
    L".psb"
};

const uint32_t PSDDecoder::m_extensionCount =
    static_cast<uint32_t>(sizeof(m_extensions) / sizeof(m_extensions[0]));

PSDDecoder::PSDDecoder() {}
PSDDecoder::~PSDDecoder() {}

bool PSDDecoder::CanDecode(const wchar_t* filePath) {
    if (!filePath || filePath[0] == L'\0') return false;
    const wchar_t* ext = wcsrchr(filePath, L'.');
    if (!ext) return false;
    for (uint32_t i = 0; i < m_extensionCount; ++i) {
        if (_wcsicmp(ext, m_extensions[i]) == 0)
            return IsPSDFormat(filePath);
    }
    return false;
}

HRESULT PSDDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
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

DecoderInfo PSDDecoder::GetInfo() const {
    DecoderInfo info;
    info.name = L"PSDDecoder";
    info.version = L"1.0.0";
    info.supportedExtensions = m_extensions;
    info.extensionCount = m_extensionCount;
    info.supportsGPU = false;
    info.isArchiveDecoder = false;
    return info;
}

const wchar_t** PSDDecoder::GetSupportedExtensions() const {
    return m_extensions;
}

bool PSDDecoder::IsPSDFormat(const wchar_t* path) {
    size_t fileSize = 0;
    auto data = ReadFileData(path, fileSize);
    if (!data || fileSize < sizeof(PSDHeader)) return false;
    return memcmp(data.get(), "8BPS", 4) == 0;
}

HRESULT PSDDecoder::DecodeFromFile(const wchar_t* path, HBITMAP* phBitmap) {
    if (!phBitmap) return E_POINTER;
    *phBitmap = nullptr;

    size_t fileSize = 0;
    auto fileData = ReadFileData(path, fileSize);
    if (!fileData || fileSize < sizeof(PSDHeader)) return E_FAIL;

    const PSDHeader* header = reinterpret_cast<const PSDHeader*>(fileData.get());
    if (memcmp(header->signature, "8BPS", 4) != 0) return E_FAIL;

    uint16_t version = ReadBE16(reinterpret_cast<const uint8_t*>(&header->version));
    if (version != 1 && version != 2) return E_FAIL;

    // Try to extract the embedded JPEG thumbnail from Image Resources first
    HRESULT hr = ExtractThumbnailResource(fileData.get(), fileSize, phBitmap);
    if (SUCCEEDED(hr) && *phBitmap) return S_OK;

    // Fall back to reading the composite (merged) image data
    PSDHeader parsedHeader;
    parsedHeader.version = version;
    parsedHeader.channels = ReadBE16(reinterpret_cast<const uint8_t*>(&header->channels));
    parsedHeader.height = ReadBE32(reinterpret_cast<const uint8_t*>(&header->height));
    parsedHeader.width = ReadBE32(reinterpret_cast<const uint8_t*>(&header->width));
    parsedHeader.depth = ReadBE16(reinterpret_cast<const uint8_t*>(&header->depth));
    parsedHeader.colorMode = ReadBE16(reinterpret_cast<const uint8_t*>(&header->colorMode));

    return ExtractCompositeImage(fileData.get(), fileSize, parsedHeader, phBitmap);
}

HRESULT PSDDecoder::ExtractThumbnailResource(const uint8_t* data, size_t size,
                                              HBITMAP* phBitmap) {
    // Skip header (26 bytes)
    size_t pos = 26;
    if (pos >= size) return E_FAIL;

    // Skip Color Mode Data section
    if (pos + 4 > size) return E_FAIL;
    uint32_t colorModeLen = ReadBE32(data + pos);
    pos += 4 + colorModeLen;

    // Image Resources section
    if (pos + 4 > size) return E_FAIL;
    uint32_t resourcesLen = ReadBE32(data + pos);
    pos += 4;
    size_t resourcesEnd = pos + resourcesLen;
    if (resourcesEnd > size) resourcesEnd = size;

    // Scan image resource blocks for thumbnail (ID 1036)
    while (pos + 12 <= resourcesEnd) {
        // Signature "8BIM"
        if (memcmp(data + pos, "8BIM", 4) != 0) break;
        pos += 4;

        uint16_t resourceId = ReadBE16(data + pos);
        pos += 2;

        // Pascal string (name) - padded to even
        uint8_t nameLen = data[pos];
        pos += 1 + nameLen;
        if (pos % 2 != 0) pos++; // pad to even

        if (pos + 4 > resourcesEnd) break;
        uint32_t dataLen = ReadBE32(data + pos);
        pos += 4;

        if (resourceId == THUMBNAIL_RESOURCE_ID && dataLen > 28) {
            // Thumbnail resource: 28-byte header + JFIF data
            // Header: format(4) + width(4) + height(4) + widthBytes(4) +
            //         totalSize(4) + compressedSize(4) + bpp(2) + planes(2)
            uint32_t format = ReadBE32(data + pos);
            if (format == 1 && pos + 28 + 2 <= resourcesEnd) {
                // kJpegRGB format - embedded JPEG thumbnail
                const uint8_t* jpegData = data + pos + 28;
                uint32_t jpegSize = dataLen - 28;
                if (jpegSize > 2 && jpegData[0] == 0xFF && jpegData[1] == 0xD8) {
                    // Decode JPEG via WIC
                    IStream* pStream = nullptr;
                    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, jpegSize);
                    if (hMem) {
                        void* pMem = GlobalLock(hMem);
                        if (pMem) {
                            memcpy(pMem, jpegData, jpegSize);
                            GlobalUnlock(hMem);
                            if (SUCCEEDED(CreateStreamOnHGlobal(hMem, TRUE, &pStream))) {
                                // Use GDI+ to decode the JPEG
                                Gdiplus::GdiplusStartupInput gpsi;
                                ULONG_PTR gdipToken = 0;
                                if (Gdiplus::GdiplusStartup(&gdipToken, &gpsi, nullptr) == Gdiplus::Ok) {
                                    Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromStream(pStream);
                                    if (bmp && bmp->GetLastStatus() == Gdiplus::Ok) {
                                        bmp->GetHBITMAP(Gdiplus::Color(255, 255, 255), phBitmap);
                                    }
                                    delete bmp;
                                    Gdiplus::GdiplusShutdown(gdipToken);
                                }
                                pStream->Release();
                            } else {
                                GlobalFree(hMem);
                            }
                        } else {
                            GlobalFree(hMem);
                        }
                    }
                    if (*phBitmap) return S_OK;
                }
            }
        }

        pos += dataLen;
        if (pos % 2 != 0) pos++; // pad to even
    }

    return E_FAIL;
}

HRESULT PSDDecoder::ExtractCompositeImage(const uint8_t* data, size_t size,
                                           const PSDHeader& header, HBITMAP* phBitmap) {
    // Only support 8-bit RGB/Grayscale for composite extraction
    if (header.depth != 8) return E_FAIL;
    if (header.colorMode != 3 && header.colorMode != 1) return E_FAIL; // RGB or Grayscale
    if (header.width == 0 || header.height == 0) return E_FAIL;
    if (header.width > 30000 || header.height > 30000) return E_FAIL;

    uint16_t channels = (std::min)(header.channels, (uint16_t)4);

    // Navigate to composite image data (at the very end of the file)
    // Skip: Header(26) + ColorMode + ImageResources + LayerMask + then ImageData
    size_t pos = 26;
    if (pos + 4 > size) return E_FAIL;
    uint32_t colorModeLen = ReadBE32(data + pos); pos += 4 + colorModeLen;
    if (pos + 4 > size) return E_FAIL;
    uint32_t resourcesLen = ReadBE32(data + pos); pos += 4 + resourcesLen;
    if (pos + 4 > size) return E_FAIL;
    uint32_t layerMaskLen = ReadBE32(data + pos); pos += 4 + layerMaskLen;

    // Image data section: compression(2) + pixel data
    if (pos + 2 > size) return E_FAIL;
    uint16_t compression = ReadBE16(data + pos);
    pos += 2;

    if (compression == 0) {
        // Raw uncompressed
        uint32_t planeSize = header.width * header.height;
        uint32_t neededSize = planeSize * channels;
        if (pos + neededSize > size) return E_FAIL;

        // Interleave planar to BGRA
        uint32_t pixelCount = header.width * header.height;
        auto pixels = std::make_unique<uint8_t[]>(pixelCount * 4);

        for (uint32_t i = 0; i < pixelCount; ++i) {
            if (header.colorMode == 3) { // RGB
                pixels[i * 4 + 2] = data[pos + i];                     // R → B
                pixels[i * 4 + 1] = data[pos + planeSize + i];         // G
                pixels[i * 4 + 0] = data[pos + planeSize * 2 + i];     // B → R (BGR)
                // Wait, HBITMAP is BGRA: B=0,G=1,R=2,A=3
                // PSD planes: R,G,B,A
                pixels[i * 4 + 0] = data[pos + planeSize * 2 + i]; // B
                pixels[i * 4 + 1] = data[pos + planeSize + i];     // G
                pixels[i * 4 + 2] = data[pos + i];                 // R
                pixels[i * 4 + 3] = (channels >= 4) ? data[pos + planeSize * 3 + i] : 255;
            } else { // Grayscale
                uint8_t gray = data[pos + i];
                pixels[i * 4 + 0] = gray;
                pixels[i * 4 + 1] = gray;
                pixels[i * 4 + 2] = gray;
                pixels[i * 4 + 3] = 255;
            }
        }

        *phBitmap = CreateBitmapFromRGB(pixels.get(), header.width, header.height, 4);
        return (*phBitmap) ? S_OK : E_FAIL;
    }

    if (compression == 1) {
        // RLE (PackBits) compression
        // After the compression type, there are (channels * height) row byte counts
        uint32_t rowCountEntries = channels * header.height;
        size_t countSize = (header.version == 2) ? 4 : 2; // PSB uses 4-byte counts
        size_t rowCountsSize = rowCountEntries * countSize;
        if (pos + rowCountsSize > size) return E_FAIL;

        // Read row byte counts (skip them, we'll decompress sequentially)
        pos += rowCountsSize;

        uint32_t planeSize = header.width * header.height;
        auto decompressed = std::make_unique<uint8_t[]>((size_t)planeSize * channels);
        size_t outPos = 0;
        size_t totalOut = (size_t)planeSize * channels;

        // PackBits decompression
        while (pos < size && outPos < totalOut) {
            int8_t n = static_cast<int8_t>(data[pos++]);
            if (n >= 0) {
                // Copy next (n+1) bytes literally
                uint32_t count = static_cast<uint32_t>(n) + 1;
                if (pos + count > size || outPos + count > totalOut) break;
                memcpy(decompressed.get() + outPos, data + pos, count);
                pos += count;
                outPos += count;
            } else if (n != -128) {
                // Repeat next byte (1 - n) times
                uint32_t count = static_cast<uint32_t>(1 - n);
                if (pos >= size || outPos + count > totalOut) break;
                uint8_t val = data[pos++];
                memset(decompressed.get() + outPos, val, count);
                outPos += count;
            }
            // n == -128: no-op
        }

        // Interleave planar to BGRA
        uint32_t pixelCount = header.width * header.height;
        auto pixels = std::make_unique<uint8_t[]>(pixelCount * 4);

        for (uint32_t i = 0; i < pixelCount; ++i) {
            if (header.colorMode == 3) { // RGB
                pixels[i * 4 + 0] = decompressed[planeSize * 2 + i]; // B
                pixels[i * 4 + 1] = decompressed[planeSize + i];     // G
                pixels[i * 4 + 2] = decompressed[i];                 // R
                pixels[i * 4 + 3] = (channels >= 4) ? decompressed[planeSize * 3 + i] : 255;
            } else { // Grayscale
                uint8_t gray = decompressed[i];
                pixels[i * 4 + 0] = gray;
                pixels[i * 4 + 1] = gray;
                pixels[i * 4 + 2] = gray;
                pixels[i * 4 + 3] = 255;
            }
        }

        *phBitmap = CreateBitmapFromRGB(pixels.get(), header.width, header.height, 4);
        return (*phBitmap) ? S_OK : E_FAIL;
    }

    return E_FAIL; // Unsupported compression (ZIP/2,3)
}

HBITMAP PSDDecoder::CreateBitmapFromRGB(const uint8_t* pixels, uint32_t width,
                                         uint32_t height, uint16_t channels) {
    (void)channels; // Always outputs 32bpp BGRA
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -(LONG)height; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HDC hdc = GetDC(nullptr);
    HBITMAP hBmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    ReleaseDC(nullptr, hdc);

    if (hBmp && pBits) {
        memcpy(pBits, pixels, width * height * 4);
    }
    return hBmp;
}

uint16_t PSDDecoder::ReadBE16(const uint8_t* data) {
    return (uint16_t)((data[0] << 8) | data[1]);
}

uint32_t PSDDecoder::ReadBE32(const uint8_t* data) {
    return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
           ((uint32_t)data[2] << 8) | data[3];
}

std::unique_ptr<uint8_t[]> PSDDecoder::ReadFileData(const wchar_t* path, size_t& fileSize) {
    fileSize = 0;
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return nullptr;
    fileSize = static_cast<size_t>(file.tellg());
    // For thumbnails, cap at 50 MB to avoid huge PSD files
    size_t readSize = (std::min)(fileSize, (size_t)(50 * 1024 * 1024));
    file.seekg(0, std::ios::beg);
    auto data = std::make_unique<uint8_t[]>(readSize);
    file.read(reinterpret_cast<char*>(data.get()), readSize);
    fileSize = readSize;
    return data;
}

} // namespace Engine
} // namespace ExplorerLens

