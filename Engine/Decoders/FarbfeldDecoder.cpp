//==============================================================================
// FarbfeldDecoder.cpp — Farbfeld Image Decoder
//==============================================================================

#include "FarbfeldDecoder.h"
#include "DecoderSecurityHardening.h"
#include <fstream>
#include <algorithm>
#include <cstring>

namespace ExplorerLens {
namespace Engine {

bool FarbfeldDecoder::CanDecode(const wchar_t* filePath) const {
    if (!filePath) return false;
    std::wstring path(filePath);
    std::transform(path.begin(), path.end(), path.begin(), ::towlower);
    return path.length() >= 3 &&
        path.compare(path.length() - 3, 3, L".ff") == 0;
}

HRESULT FarbfeldDecoder::Decode(const wchar_t* filePath, uint32_t requestedSize,
    HBITMAP& hBitmap) {
    if (!filePath || requestedSize == 0) return E_INVALIDARG;

    // Read entire file
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    size_t fileSize = static_cast<size_t>(file.tellg());
    if (fileSize < 16) return E_FAIL; // Magic (8) + width (4) + height (4)

    file.seekg(0);
    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    file.close();

    // Security: File size limit
    if (!Security::ValidateFileSize(fileSize, Security::MAX_IMAGE_FILE_SIZE)) return E_FAIL;

    // Validate magic
    if (!Security::ValidateMagic(data.data(), fileSize, MAGIC, 8)) return E_FAIL;

    // Read dimensions (big-endian)
    uint32_t width = ReadBE32(data.data() + 8);
    uint32_t height = ReadBE32(data.data() + 12);

    // Security: Dimension validation
    if (!Security::ValidateDimensions(width, height)) return E_FAIL;

    // Validate data size: 16 (header) + width * height * 8 (16-bit RGBA)
    // Security: Use safe multiplication to avoid overflow
    size_t pixelDataSize = 0;
    if (!Security::SafeMulTriple(static_cast<size_t>(width), static_cast<size_t>(height),
        8, pixelDataSize)) return E_FAIL;
    uint64_t expectedSize64 = 0;
    if (!Security::SafeAdd64(16, pixelDataSize, expectedSize64)) return E_FAIL;
    if (fileSize < static_cast<size_t>(expectedSize64)) return E_FAIL;

    // Security: Safe pixel buffer allocation
    size_t bgraSize = 0;
    if (!Security::ValidatePixelAllocation(width, height, 4, bgraSize)) return E_FAIL;

    // Convert 16-bit RGBA (big-endian) to 8-bit BGRA
    std::vector<uint8_t> pixels(bgraSize);
    const uint8_t* src = data.data() + 16;

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            size_t srcOffset = (static_cast<size_t>(y) * width + x) * 8;
            size_t dstOffset = (static_cast<size_t>(y) * width + x) * 4;

            // 16-bit channels → 8-bit (take high byte)
            uint8_t r = src[srcOffset + 0]; // High byte of R16
            uint8_t g = src[srcOffset + 2]; // High byte of G16
            uint8_t b = src[srcOffset + 4]; // High byte of B16
            uint8_t a = src[srcOffset + 6]; // High byte of A16

            // Store as BGRA
            pixels[dstOffset + 0] = b;
            pixels[dstOffset + 1] = g;
            pixels[dstOffset + 2] = r;
            pixels[dstOffset + 3] = a;
        }
    }

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

} // namespace Engine
} // namespace ExplorerLens
