// JXLDecoder.cpp - JPEG XL (JXL) format decoder implementation
// Part of DarkThumbs Engine v5.3.0+

#include "JXLDecoder.h"
#include <fstream>
#include <vector>
#include <algorithm>

// TODO: Uncomment when libjxl is built
// #include <jxl/decode.h>
// #include <jxl/decode_cxx.h>
// #pragma comment(lib, "jxl.lib")
// #pragma comment(lib, "jxl_threads.lib")

namespace DarkThumbs {
namespace Engine {

    JXLDecoder::JXLDecoder()
        : m_useMultithreading(true)
        , m_maxThreads(4)
    {
    }

    JXLDecoder::~JXLDecoder() = default;

    bool JXLDecoder::CanDecode(const std::wstring& filePath) {
        // Check file extension
        size_t dotPos = filePath.find_last_of(L'.');
        if (dotPos == std::wstring::npos) {
            return false;
        }

        std::wstring ext = filePath.substr(dotPos);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

        return (ext == L".jxl");
    }

    ThumbnailResult JXLDecoder::Decode(const ThumbnailRequest& request) {
        ThumbnailResult result;
        result.Success = false;

        // Verify file exists
        DWORD attrs = GetFileAttributesW(request.FilePath.c_str());
        if (attrs == INVALID_FILE_ATTRIBUTES) {
            result.ErrorMessage = L"File not found";
            return result;
        }

        // Read file data
        size_t fileSize = 0;
        auto fileData = ReadFileData(request.FilePath, fileSize);
        if (!fileData || fileSize == 0) {
            result.ErrorMessage = L"Failed to read file";
            return result;
        }

        // Verify JXL signature
        if (!VerifyJXLSignature(fileData.get(), fileSize)) {
            result.ErrorMessage = L"Invalid JXL signature";
            return result;
        }

// TODO: Implement actual JXL decoding when libjxl is built
#if 0
        // Decode JXL image
        uint32_t decodedWidth = 0;
        uint32_t decodedHeight = 0;
        uint32_t channels = 0;

        uint8_t* pixels = DecodeJXLImage(
            fileData.get(),
            fileSize,
            request.Width,
            request.Height,
            decodedWidth,
            decodedHeight,
            channels
        );

        if (!pixels) {
            result.ErrorMessage = L"JXL decoding failed";
            return result;
        }

        // Create HBITMAP from decoded pixels
        result.Bitmap = CreateHBITMAPFromRGBA(pixels, decodedWidth, decodedHeight, channels);
        delete[] pixels;

        if (result.Bitmap) {
            result.Success = true;
            result.Width = decodedWidth;
            result.Height = decodedHeight;
        } else {
            result.ErrorMessage = L"Failed to create HBITMAP";
        }
#else
        // Placeholder until libjxl is integrated
        result.ErrorMessage = L"JXL decoder not yet implemented (libjxl library required)";
        result.Success = false;
#endif

        return result;
    }

    bool JXLDecoder::VerifyJXLSignature(const uint8_t* data, size_t size) const {
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
        if (size >= 12 &&
            data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x0C &&
            data[4] == 0x4A && data[5] == 0x58 && data[6] == 0x4C && data[7] == 0x20 &&
            data[8] == 0x0D && data[9] == 0x0A && data[10] == 0x87 && data[11] == 0x0A) {
            return true;
        }

        return false;
    }

    uint8_t* JXLDecoder::DecodeJXLImage(
        const uint8_t* fileData,
        size_t dataSize,
        uint32_t targetWidth,
        uint32_t targetHeight,
        uint32_t& outWidth,
        uint32_t& outHeight,
        uint32_t& outChannels)
    {
// TODO: Implement when libjxl is available
#if 0
        // Create JXL decoder
        auto decoder = JxlDecoderMake(nullptr);
        if (!decoder) {
            return nullptr;
        }

        // Subscribe to basic info and full image events
        if (JxlDecoderSubscribeEvents(decoder.get(),
            JXL_DEC_BASIC_INFO | JXL_DEC_FULL_IMAGE) != JXL_DEC_SUCCESS) {
            return nullptr;
        }

        // Set input data
        if (JxlDecoderSetInput(decoder.get(), fileData, dataSize) != JXL_DEC_SUCCESS) {
            return nullptr;
        }

        // Process decoder events
        JxlBasicInfo info;
        bool gotInfo = false;
        uint8_t* pixels = nullptr;

        while (true) {
            JxlDecoderStatus status = JxlDecoderProcessInput(decoder.get());

            if (status == JXL_DEC_BASIC_INFO) {
                if (JxlDecoderGetBasicInfo(decoder.get(), &info) != JXL_DEC_SUCCESS) {
                    return nullptr;
                }
                gotInfo = true;

                // Calculate thumbnail dimensions
                outWidth = info.xsize;
                outHeight = info.ysize;
                outChannels = info.num_color_channels + (info.alpha_bits > 0 ? 1 : 0);

                // Scale to target size while maintaining aspect ratio
                if (outWidth > targetWidth || outHeight > targetHeight) {
                    float scaleW = static_cast<float>(targetWidth) / outWidth;
                    float scaleH = static_cast<float>(targetHeight) / outHeight;
                    float scale = min(scaleW, scaleH);
                    outWidth = static_cast<uint32_t>(outWidth * scale);
                    outHeight = static_cast<uint32_t>(outHeight * scale);
                }

                // Allocate pixel buffer
                size_t pixelSize = outWidth * outHeight * outChannels;
                pixels = new (std::nothrow) uint8_t[pixelSize];
                if (!pixels) {
                    return nullptr;
                }

                // Set output buffer
                JxlPixelFormat format = {
                    outChannels,
                    JXL_TYPE_UINT8,
                    JXL_NATIVE_ENDIAN,
                    0
                };

                if (JxlDecoderSetImageOutBuffer(decoder.get(), &format,
                    pixels, pixelSize) != JXL_DEC_SUCCESS) {
                    delete[] pixels;
                    return nullptr;
                }
            }
            else if (status == JXL_DEC_FULL_IMAGE) {
                // Image decoded successfully
                return pixels;
            }
            else if (status == JXL_DEC_SUCCESS) {
                // Decoding finished
                return pixels;
            }
            else {
                // Error occurred
                if (pixels) {
                    delete[] pixels;
                }
                return nullptr;
            }
        }
#else
        // Placeholder
        outWidth = 256;
        outHeight = 256;
        outChannels = 4;
        return nullptr;
#endif
    }

    std::unique_ptr<uint8_t[]> JXLDecoder::ReadFileData(const std::wstring& filePath, size_t& outSize) {
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

    HBITMAP JXLDecoder::CreateHBITMAPFromRGBA(
        const uint8_t* pixels,
        uint32_t width,
        uint32_t height,
        uint32_t channels)
    {
        if (!pixels || width == 0 || height == 0) {
            return nullptr;
        }

        // Create DIB section
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -static_cast<int>(height); // Top-down
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

} // namespace Engine
} // namespace DarkThumbs
