/******************************************************************************
 * jxl_decoder.cpp
 * JPEG XL Image Decoder Implementation for DarkThumbs
 * 
 * Sprint 3 - Modern Image Formats (JPEG XL 0.11.1)
 * Supports: .jxl files (naked and containerized formats)
 ******************************************************************************/

#include "StdAfx.h"
#include "jxl_decoder.h"

// libjxl headers
#include <jxl/decode.h>
#include <jxl/decode_cxx.h>
#include <jxl/resizable_parallel_runner.h>
#include <jxl/resizable_parallel_runner_cxx.h>

#pragma comment(lib, "jxl.lib")
#pragma comment(lib, "jxl_threads.lib")

// Delay-load for optimal performance (only load when .jxl files are encountered)
#pragma comment(linker, "/DELAYLOAD:jxl.dll")
#pragma comment(linker, "/DELAYLOAD:jxl_threads.dll")

namespace DarkThumbs {

/******************************************************************************
 * IsNakedJXL - Check for naked JXL signature
 * 
 * Naked JXL files start with: 0xFF 0x0A or 0x00 0x00 0x00 0x0C 0x4A 0x58 0x4C 0x20 0x0D 0x0A 0x87 0x0A
 ******************************************************************************/
bool JXLDecoder::IsNakedJXL(const BYTE* data, size_t size) {
    if (size < 2) return false;

    // Check 0xFF 0x0A (most common naked JXL signature)
    if (data[0] == 0xFF && data[1] == 0x0A) {
        return true;
    }

    // Check full naked signature (12 bytes)
    if (size >= 12) {
        const BYTE nakedSig[] = {
            0x00, 0x00, 0x00, 0x0C, 0x4A, 0x58, 0x4C, 0x20, 0x0D, 0x0A, 0x87, 0x0A
        };
        if (memcmp(data, nakedSig, 12) == 0) {
            return true;
        }
    }

    return false;
}

/******************************************************************************
 * IsContainerizedJXL - Check for JXL in ISO BMFF container
 * 
 * Containerized JXL uses ISO Base Media File Format (like AVIF/HEIF)
 * Format: "....ftypjxl "
 ******************************************************************************/
bool JXLDecoder::IsContainerizedJXL(const BYTE* data, size_t size) {
    if (size < 12) return false;

    // Check for ISO BMFF "ftyp" box
    if (memcmp(data + 4, "ftyp", 4) != 0) {
        return false;
    }

    // Check for "jxl " brand
    const char* brand = reinterpret_cast<const char*>(data + 8);
    if (memcmp(brand, "jxl ", 4) == 0) {
        return true;
    }

    return false;
}

/******************************************************************************
 * IsJXLFormat - Public format checker
 ******************************************************************************/
bool JXLDecoder::IsJXLFormat(const BYTE* data, size_t size) {
    if (!data || size < 12) {
        return false;
    }

    return IsNakedJXL(data, size) || IsContainerizedJXL(data, size);
}

/******************************************************************************
 * GetDimensions - Fast dimension extraction without full decode
 ******************************************************************************/
bool JXLDecoder::GetDimensions(const BYTE* data, size_t size, int* width, int* height) {
    if (!IsJXLFormat(data, size) || !width || !height) {
        return false;
    }

    // Create decoder
    auto decoder = JxlDecoderMake(nullptr);
    if (!decoder) {
        return false;
    }

    // Subscribe to basic info only
    if (JxlDecoderSubscribeEvents(decoder.get(), JXL_DEC_BASIC_INFO) != JXL_DEC_SUCCESS) {
        return false;
    }

    // Set input
    if (JxlDecoderSetInput(decoder.get(), data, size) != JXL_DEC_SUCCESS) {
        return false;
    }

    // Process to get basic info
    JxlDecoderStatus status = JxlDecoderProcessInput(decoder.get());
    if (status != JXL_DEC_BASIC_INFO) {
        return false;
    }

    // Get basic info
    JxlBasicInfo info;
    if (JxlDecoderGetBasicInfo(decoder.get(), &info) != JXL_DEC_SUCCESS) {
        return false;
    }

    *width = static_cast<int>(info.xsize);
    *height = static_cast<int>(info.ysize);

    return true;
}

/******************************************************************************
 * DecodeToHBITMAP - Full decode to Windows bitmap
 ******************************************************************************/
HRESULT JXLDecoder::DecodeToHBITMAP(const BYTE* data, size_t size, HBITMAP* phBitmap) {
    if (!data || size == 0 || !phBitmap) {
        return E_INVALIDARG;
    }

    *phBitmap = nullptr;

    // Verify format
    if (!IsJXLFormat(data, size)) {
        return E_FAIL;
    }

    // Create decoder with parallel runner for performance
    auto runner = JxlResizableParallelRunnerMake(nullptr);
    auto decoder = JxlDecoderMake(nullptr);

    if (!decoder || !runner) {
        return E_OUTOFMEMORY;
    }

    // Set parallel runner
    if (JxlDecoderSetParallelRunner(decoder.get(), JxlResizableParallelRunner, runner.get()) != JXL_DEC_SUCCESS) {
        return E_FAIL;
    }

    // Subscribe to basic info and full image events
    const int events = JXL_DEC_BASIC_INFO | JXL_DEC_FULL_IMAGE;
    if (JxlDecoderSubscribeEvents(decoder.get(), events) != JXL_DEC_SUCCESS) {
        return E_FAIL;
    }

    // Set input
    if (JxlDecoderSetInput(decoder.get(), data, size) != JXL_DEC_SUCCESS) {
        return E_FAIL;
    }

    // Process decoder
    JxlBasicInfo info;
    JxlPixelFormat format = { 4, JXL_TYPE_UINT8, JXL_NATIVE_ENDIAN, 0 };
    std::vector<BYTE> pixels;
    size_t buffer_size = 0;

    bool success = false;
    JxlDecoderStatus status;

    for (;;) {
        status = JxlDecoderProcessInput(decoder.get());

        if (status == JXL_DEC_ERROR) {
            return E_FAIL;
        }

        if (status == JXL_DEC_NEED_MORE_INPUT) {
            return E_FAIL;  // Unexpected - we provided all data
        }

        if (status == JXL_DEC_BASIC_INFO) {
            // Get image info
            if (JxlDecoderGetBasicInfo(decoder.get(), &info) != JXL_DEC_SUCCESS) {
                return E_FAIL;
            }

            // Set thread count based on image size
            JxlResizableParallelRunnerSetThreads(
                runner.get(),
                JxlResizableParallelRunnerSuggestThreads(info.xsize, info.ysize)
            );
        }

        if (status == JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
            // Get required buffer size
            if (JxlDecoderImageOutBufferSize(decoder.get(), &format, &buffer_size) != JXL_DEC_SUCCESS) {
                return E_FAIL;
            }

            // Allocate pixel buffer
            pixels.resize(buffer_size);

            // Set output buffer
            if (JxlDecoderSetImageOutBuffer(decoder.get(), &format, pixels.data(), buffer_size) != JXL_DEC_SUCCESS) {
                return E_FAIL;
            }
        }

        if (status == JXL_DEC_FULL_IMAGE) {
            // Image decoded successfully
            success = true;
            continue;  // May have more frames, but we only need first
        }

        if (status == JXL_DEC_SUCCESS) {
            // All done
            break;
        }
    }

    if (!success || pixels.empty()) {
        return E_FAIL;
    }

    // Convert to HBITMAP
    HBITMAP hBitmap = CreateBitmapFromRGBA(
        pixels.data(),
        static_cast<int>(info.xsize),
        static_cast<int>(info.ysize)
    );

    if (!hBitmap) {
        return E_FAIL;
    }

    *phBitmap = hBitmap;
    return S_OK;
}

/******************************************************************************
 * CreateBitmapFromRGBA - Convert RGBA buffer to Windows DIB
 ******************************************************************************/
HBITMAP JXLDecoder::CreateBitmapFromRGBA(const BYTE* rgba, int width, int height) {
    if (!rgba || width <= 0 || height <= 0) {
        return nullptr;
    }

    // Setup DIB header
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;  // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    // Create DIB section
    void* pBits = nullptr;
    HDC hdc = GetDC(nullptr);
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    ReleaseDC(nullptr, hdc);

    if (!hBitmap || !pBits) {
        return nullptr;
    }

    // Convert RGBA to BGRA (Windows DIB format)
    const BYTE* src = rgba;
    BYTE* dst = static_cast<BYTE*>(pBits);
    const size_t pixelCount = static_cast<size_t>(width) * height;

    for (size_t i = 0; i < pixelCount; ++i) {
        dst[0] = src[2];  // B
        dst[1] = src[1];  // G
        dst[2] = src[0];  // R
        dst[3] = src[3];  // A
        src += 4;
        dst += 4;
    }

    return hBitmap;
}

} // namespace DarkThumbs
