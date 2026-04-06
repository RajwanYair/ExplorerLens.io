// TGADecoder.cpp - Targa Image Decoder Implementation
// ExplorerLens Engine v5.3.0+

#include "TGADecoder.h"
#include <algorithm>
#include <fstream>
#include "DecoderSecurityHardening.h"

namespace ExplorerLens {
namespace Engine {

// Static members
const wchar_t* TGADecoder::m_extensions[] = {L".tga", L".tpic"};

const uint32_t TGADecoder::m_extensionCount = static_cast<uint32_t>(sizeof(m_extensions) / sizeof(m_extensions[0]));

TGADecoder::TGADecoder() {}

TGADecoder::~TGADecoder() {}

bool TGADecoder::CanDecode(const wchar_t* filePath)
{
    if (!filePath || filePath[0] == L'\0') {
        return false;
    }

    const wchar_t* ext = wcsrchr(filePath, L'.');
    if (!ext) {
        return false;
    }

    for (uint32_t i = 0; i < m_extensionCount; ++i) {
        if (_wcsicmp(ext, m_extensions[i]) == 0) {
            return true;
        }
    }

    return false;
}

HRESULT TGADecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result)
{
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

DecoderInfo TGADecoder::GetInfo() const
{
    DecoderInfo info;
    info.name = L"TGADecoder";
    info.version = L"1.0.0";
    info.supportedExtensions = m_extensions;
    info.extensionCount = m_extensionCount;
    info.supportsGPU = false;
    info.isArchiveDecoder = false;
    return info;
}

const wchar_t** TGADecoder::GetSupportedExtensions() const
{
    return m_extensions;
}

HRESULT TGADecoder::DecodeFromFile(const wchar_t* path, HBITMAP* phBitmap)
{
    if (!phBitmap) {
        return E_POINTER;
    }

    *phBitmap = nullptr;

    // Read file
    size_t fileSize = 0;
    auto fileData = ReadFileData(path, fileSize);

    if (!fileData || fileSize < sizeof(TGAHeader)) {
        return E_FAIL;
    }

    // Security: File size limit (2 GB)
    if (!Security::ValidateFileSize(fileSize, Security::MAX_IMAGE_FILE_SIZE)) {
        return E_FAIL;
    }

    // Parse header
    const TGAHeader* header = reinterpret_cast<const TGAHeader*>(fileData.get());

    // Validate header
    if (header->width == 0 || header->height == 0) {
        return E_FAIL;
    }

    // Security: Dimension validation
    if (!Security::ValidateDimensions(header->width, header->height)) {
        return E_FAIL;
    }

    if (header->bitsPerPixel != 16 && header->bitsPerPixel != 24 && header->bitsPerPixel != 32) {
        return E_FAIL;  // Unsupported bit depth
    }

    // Security: Validate pixel buffer allocation won't overflow
    size_t pixBufSize = 0;
    if (!Security::ValidatePixelAllocation(header->width, header->height, header->bitsPerPixel / 8, pixBufSize)) {
        return E_FAIL;
    }

    // Security: Validate ID field offset is within file bounds
    size_t dataOffset = sizeof(TGAHeader) + header->idLength;
    if (!Security::ValidateBufferAccess(dataOffset, 1, fileSize)) {
        return E_FAIL;
    }

    // Skip ID field if present
    const uint8_t* imageData = fileData.get() + dataOffset;

    // Decode based on image type
    uint8_t* pixels = nullptr;
    uint32_t stride = 0;
    HRESULT hr;

    switch (header->imageType) {
        case TGA_RGB:
            hr = DecodeUncompressed(imageData, *header, &pixels, &stride);
            break;

        case TGA_RLE_RGB:
            hr = DecodeRLE(imageData, *header, &pixels, &stride);
            break;

        default:
            return E_FAIL;  // Unsupported image type
    }

    if (FAILED(hr) || !pixels) {
        return hr;
    }

    // Check orientation (bit 5 of imageDescriptor)
    bool flipY = !(header->imageDescriptor & 0x20);

    // Create bitmap
    *phBitmap = CreateBitmapFromRGB(pixels, header->width, header->height, header->bitsPerPixel, flipY);

    delete[] pixels;

    return (*phBitmap) ? S_OK : E_FAIL;
}

HRESULT TGADecoder::DecodeUncompressed(const uint8_t* data, const TGAHeader& header, uint8_t** ppPixels,
                                       uint32_t* pStride)
{
    if (!Security::ValidatePtr(ppPixels, pStride, data)) {
        return E_POINTER;
    }

    uint32_t bytesPerPixel = header.bitsPerPixel / 8;

    // Security: Safe pixel buffer size calculation
    size_t pixelDataSize = 0;
    if (!Security::ValidatePixelAllocation(header.width, header.height, bytesPerPixel, pixelDataSize)) {
        return E_FAIL;
    }

    uint32_t stride = header.width * bytesPerPixel;

    *ppPixels = new (std::nothrow) uint8_t[pixelDataSize];
    if (!*ppPixels)
        return E_OUTOFMEMORY;
    *pStride = stride;

    // Simple memcpy for uncompressed data
    memcpy(*ppPixels, data, pixelDataSize);

    return S_OK;
}

HRESULT TGADecoder::DecodeRLE(const uint8_t* data, const TGAHeader& header, uint8_t** ppPixels, uint32_t* pStride)
{
    if (!Security::ValidatePtr(ppPixels, pStride, data)) {
        return E_POINTER;
    }

    uint32_t bytesPerPixel = header.bitsPerPixel / 8;

    // Security: Safe pixel buffer size calculation
    size_t pixelDataSize = 0;
    if (!Security::ValidatePixelAllocation(header.width, header.height, bytesPerPixel, pixelDataSize)) {
        return E_FAIL;
    }

    uint32_t stride = header.width * bytesPerPixel;

    *ppPixels = new (std::nothrow) uint8_t[pixelDataSize];
    if (!*ppPixels)
        return E_OUTOFMEMORY;
    *pStride = stride;

    uint8_t* dest = *ppPixels;
    uint8_t* destEnd = dest + pixelDataSize;
    const uint8_t* src = data;
    uint32_t pixelsDecoded = 0;
    uint32_t totalPixels = header.width * header.height;

    while (pixelsDecoded < totalPixels) {
        uint8_t packetHeader = *src++;
        uint8_t packetType = packetHeader & 0x80;
        uint32_t pixelCount = (packetHeader & 0x7F) + 1;

        // Security: Prevent writing past buffer end
        if (pixelsDecoded + pixelCount > totalPixels) {
            pixelCount = totalPixels - pixelsDecoded;
        }

        if (packetType) {
            // RLE packet (repeat pixel)
            for (uint32_t i = 0; i < pixelCount; ++i) {
                if (dest + bytesPerPixel > destEnd)
                    break;
                memcpy(dest, src, bytesPerPixel);
                dest += bytesPerPixel;
            }
            src += bytesPerPixel;
        } else {
            // Raw packet (copy pixels)
            uint32_t byteCount = pixelCount * bytesPerPixel;
            if (dest + byteCount > destEnd)
                break;
            memcpy(dest, src, byteCount);
            dest += byteCount;
            src += byteCount;
        }

        pixelsDecoded += pixelCount;
    }

    return S_OK;
}

bool TGADecoder::IsTGAFormat(const wchar_t* path)
{
    size_t fileSize = 0;
    auto data = ReadFileData(path, fileSize);

    return data && IsTGAFormat(data.get(), fileSize);
}

bool TGADecoder::IsTGAFormat(const uint8_t* data, size_t size)
{
    if (!data || size < sizeof(TGAHeader)) {
        return false;
    }

    const TGAHeader* header = reinterpret_cast<const TGAHeader*>(data);

    // Basic validation
    if (header->colorMapType > 1)
        return false;
    if (header->imageType > 11)
        return false;
    if (header->bitsPerPixel != 8 && header->bitsPerPixel != 16 && header->bitsPerPixel != 24
        && header->bitsPerPixel != 32) {
        return false;
    }

    // Check for reasonable dimensions
    if (header->width == 0 || header->height == 0 || header->width > 16384 || header->height > 16384) {
        return false;
    }

    return true;
}

HBITMAP TGADecoder::CreateBitmapFromRGB(const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t bpp, bool flipY)
{
    if (!pixels || width == 0 || height == 0) {
        return nullptr;
    }

    // Create DIB section
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = flipY ? height : -(LONG)height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HDC hdcScreen = GetDC(nullptr);
    HBITMAP hBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    ReleaseDC(nullptr, hdcScreen);

    if (!hBitmap || !pBits) {
        return nullptr;
    }

    // Convert to BGRA
    uint8_t* dest = static_cast<uint8_t*>(pBits);
    uint32_t bytesPerPixel = bpp / 8;

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            const uint8_t* src = pixels + (y * width + x) * bytesPerPixel;

            if (bytesPerPixel == 3) {
                // 24-bit BGR -> BGRA
                dest[0] = src[0];  // B
                dest[1] = src[1];  // G
                dest[2] = src[2];  // R
                dest[3] = 255;     // A
            } else if (bytesPerPixel == 4) {
                // 32-bit BGRA
                dest[0] = src[0];  // B
                dest[1] = src[1];  // G
                dest[2] = src[2];  // R
                dest[3] = src[3];  // A
            } else if (bytesPerPixel == 2) {
                // 16-bit RGB555 or RGB565
                uint16_t color = *reinterpret_cast<const uint16_t*>(src);
                dest[0] = static_cast<uint8_t>((color & 0x001F) << 3);  // B
                dest[1] = static_cast<uint8_t>((color & 0x03E0) >> 2);  // G
                dest[2] = static_cast<uint8_t>((color & 0x7C00) >> 7);  // R
                dest[3] = 255;                                          // A
            }

            dest += 4;
        }
    }

    return hBitmap;
}

std::unique_ptr<uint8_t[]> TGADecoder::ReadFileData(const wchar_t* path, size_t& fileSize)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        fileSize = 0;
        return nullptr;
    }

    fileSize = static_cast<size_t>(file.tellg());

    // Security: Reject files exceeding 2 GB
    if (!Security::ValidateFileSize(fileSize, Security::MAX_IMAGE_FILE_SIZE)) {
        fileSize = 0;
        return nullptr;
    }

    file.seekg(0, std::ios::beg);

    auto data = std::make_unique<uint8_t[]>(fileSize);
    file.read(reinterpret_cast<char*>(data.get()), fileSize);

    return data;
}

}  // namespace Engine
}  // namespace ExplorerLens
