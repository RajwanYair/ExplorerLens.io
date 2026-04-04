// JXLDecoder.cpp - JPEG XL (JXL) format decoder implementation
// Part of ExplorerLens Engine v5.3.0+

#include "JXLDecoder.h"
#include <algorithm>
#include <fstream>
#include <vector>

// libjxl integration - conditionally included when HAS_LIBJXL is defined
#ifdef HAS_LIBJXL
    #include <jxl/decode.h>
    #include <jxl/decode_cxx.h>
    #pragma comment(lib, "jxl.lib")
    #pragma comment(lib, "jxl_threads.lib")
#endif

namespace ExplorerLens {
namespace Engine {

// Static extension array defined in header

JXLDecoder::JXLDecoder() {}

JXLDecoder::~JXLDecoder() = default;

bool JXLDecoder::CanDecode(const wchar_t* filePath)
{
    if (!filePath)
        return false;

    // Check file extension
    const wchar_t* ext = wcsrchr(filePath, L'.');
    if (!ext)
        return false;

    return (_wcsicmp(ext, L".jxl") == 0);
}

const wchar_t** JXLDecoder::GetSupportedExtensions() const
{
    return const_cast<const wchar_t**>(s_extensions);
}

DecoderInfo JXLDecoder::GetInfo() const
{
    DecoderInfo info;
    info.name = L"JXLDecoder";
    info.version = L"1.0.0";
    info.supportedExtensions = const_cast<const wchar_t**>(s_extensions);
    info.extensionCount = 1;
    info.supportsGPU = false;
    info.isArchiveDecoder = false;
    return info;
}

HRESULT JXLDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result)
{
    // Initialize result structure
    result.hBitmap = nullptr;
    result.width = 0;
    result.height = 0;
    result.status = E_FAIL;
    result.usedGPU = false;

    // Validate input
    if (!request.filePath) {
        result.status = E_INVALIDARG;
        return E_INVALIDARG;
    }

    // Verify file exists
    DWORD attrs = GetFileAttributesW(request.filePath);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        result.status = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        return result.status;
    }

    // Read file data
    size_t fileSize = 0;
    auto fileData = ReadFileData(request.filePath, fileSize);
    if (!fileData || fileSize == 0) {
        result.status = HRESULT_FROM_WIN32(ERROR_READ_FAULT);
        return result.status;
    }

    // Verify JXL signature
    if (!VerifyJXLSignature(fileData.get(), fileSize)) {
        result.status = E_FAIL;  // Invalid format
        return result.status;
    }

#ifdef HAS_LIBJXL
    // Decode JXL image using libjxl
    uint32_t decodedWidth = 0;
    uint32_t decodedHeight = 0;
    uint32_t channels = 0;

    uint8_t* pixels =
        DecodeJXLImage(fileData.get(), fileSize, request.width, request.height, decodedWidth, decodedHeight, channels);

    if (!pixels) {
        result.status = E_FAIL;
        return result.status;
    }

    // Create HBITMAP from decoded pixels
    result.hBitmap = CreateHBITMAPFromRGBA(pixels, decodedWidth, decodedHeight, channels);
    delete[] pixels;

    if (result.hBitmap) {
        result.status = S_OK;
        result.width = decodedWidth;
        result.height = decodedHeight;
        return S_OK;
    } else {
        result.status = E_OUTOFMEMORY;
        return result.status;
    }
#else
    // Placeholder until libjxl is integrated
    // Return E_NOTIMPL to indicate decoder not yet implemented
    result.status = E_NOTIMPL;
    return E_NOTIMPL;
#endif
}

bool JXLDecoder::VerifyJXLSignature(const uint8_t* data, size_t size) const
{
    if (!data || size < 12) {
        return false;
    }

    // JXL has two container formats:
    // 1. Bare codestream: starts with 0xFF 0x0A
    // 2. JXL container: starts with "JXL " (0x4A 0x58 0x4C 0x20) followed by 0x0D 0x0A 0x87 0x0A

    // Check for bare codestream signature
    if (data[0] == 0xFF && data[1] == 0x0A) {
        return true;
    }

    // Check for JXL container signature
    if (size >= 12 && data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x0C && data[4] == 0x4A
        && data[5] == 0x58 && data[6] == 0x4C && data[7] == 0x20 && data[8] == 0x0D && data[9] == 0x0A
        && data[10] == 0x87 && data[11] == 0x0A) {
        return true;
    }

    return false;
}

uint8_t* JXLDecoder::DecodeJXLImage(const uint8_t* fileData, size_t dataSize, uint32_t targetWidth,
                                    uint32_t targetHeight, uint32_t& outWidth, uint32_t& outHeight,
                                    uint32_t& outChannels)
{
#ifdef HAS_LIBJXL
    // Create JXL decoder using C++ API
    auto decoder = JxlDecoderMake(nullptr);
    if (!decoder) {
        return nullptr;
    }

    // Subscribe to basic info and full image events
    if (JxlDecoderSubscribeEvents(decoder.get(), JXL_DEC_BASIC_INFO | JXL_DEC_FULL_IMAGE) != JXL_DEC_SUCCESS) {
        return nullptr;
    }

    // Set input data
    if (JxlDecoderSetInput(decoder.get(), fileData, dataSize) != JXL_DEC_SUCCESS) {
        return nullptr;
    }

    // Process decoder events
    JxlBasicInfo info;
    uint8_t* pixels = nullptr;

    while (true) {
        JxlDecoderStatus status = JxlDecoderProcessInput(decoder.get());

        if (status == JXL_DEC_BASIC_INFO) {
            if (JxlDecoderGetBasicInfo(decoder.get(), &info) != JXL_DEC_SUCCESS) {
                return nullptr;
            }

            // Calculate thumbnail dimensions
            outWidth = info.xsize;
            outHeight = info.ysize;
            outChannels = info.num_color_channels + (info.alpha_bits > 0 ? 1 : 0);

            // Scale to target size while maintaining aspect ratio
            if (outWidth > targetWidth || outHeight > targetHeight) {
                float scaleW = static_cast<float>(targetWidth) / outWidth;
                float scaleH = static_cast<float>(targetHeight) / outHeight;
                float scale = (scaleW < scaleH) ? scaleW : scaleH;  // Use ternary instead of std::min
                outWidth = static_cast<uint32_t>(outWidth * scale);
                outHeight = static_cast<uint32_t>(outHeight * scale);
            }

            // Allocate pixel buffer (always RGBA for consistency)
            outChannels = 4;  // Force RGBA
            size_t pixelSize = static_cast<size_t>(outWidth) * outHeight * outChannels;
            pixels = new (std::nothrow) uint8_t[pixelSize];
            if (!pixels) {
                return nullptr;
            }

            // Set output buffer with RGBA format
            JxlPixelFormat format = {4,  // RGBA
                                     JXL_TYPE_UINT8, JXL_NATIVE_ENDIAN, 0};

            if (JxlDecoderSetImageOutBuffer(decoder.get(), &format, pixels, pixelSize) != JXL_DEC_SUCCESS) {
                delete[] pixels;
                return nullptr;
            }
        } else if (status == JXL_DEC_FULL_IMAGE) {
            // Image decoded successfully
            return pixels;
        } else if (status == JXL_DEC_SUCCESS) {
            // Decoding finished successfully
            return pixels;
        } else if (status == JXL_DEC_NEED_MORE_INPUT) {
            // Should not happen with complete input
            if (pixels) {
                delete[] pixels;
            }
            return nullptr;
        } else {
            // Error occurred during decoding
            if (pixels) {
                delete[] pixels;
            }
            return nullptr;
        }
    }

    // Should not reach here
    if (pixels) {
        delete[] pixels;
    }
    return nullptr;
#else
    // libjxl not available - return nullptr
    (void)fileData;
    (void)dataSize;
    (void)targetWidth;
    (void)targetHeight;
    outWidth = 0;
    outHeight = 0;
    outChannels = 0;
    return nullptr;
#endif
}

std::unique_ptr<uint8_t[]> JXLDecoder::ReadFileData(const wchar_t* filePath, size_t& outSize)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        outSize = 0;
        return nullptr;
    }

    outSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    auto buffer = std::make_unique<uint8_t[]>(outSize);
    file.read(reinterpret_cast<char*>(buffer.get()), outSize);

    if (!file) {
        outSize = 0;
        return nullptr;
    }

    return buffer;
}

HBITMAP JXLDecoder::CreateHBITMAPFromRGBA(const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t channels)
{
    if (!pixels || width == 0 || height == 0) {
        return nullptr;
    }

    // Create DIB section
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -static_cast<int>(height);  // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HDC hdc = GetDC(NULL);
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    ReleaseDC(NULL, hdc);

    if (!hBitmap || !pBits) {
        return nullptr;
    }

    // Convert RGBA to BGRA (Windows DIB format)
    uint32_t* dest = static_cast<uint32_t*>(pBits);
    for (uint32_t i = 0; i < width * height; ++i) {
        uint8_t r = pixels[i * channels + 0];
        uint8_t g = pixels[i * channels + 1];
        uint8_t b = pixels[i * channels + 2];
        uint8_t a = (channels == 4) ? pixels[i * channels + 3] : 255;

        // Premultiply alpha if present
        if (channels == 4 && a < 255) {
            r = (r * a) / 255;
            g = (g * a) / 255;
            b = (b * a) / 255;
        }

        dest[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }

    return hBitmap;
}

}  // namespace Engine
}  // namespace ExplorerLens
