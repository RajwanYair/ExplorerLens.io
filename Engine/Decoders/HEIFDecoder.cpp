// HEIFDecoder.cpp - HEIF/HEIC format decoder implementation
// Part of DarkThumbs Engine v5.3.0+

#include "HEIFDecoder.h"
#include <fstream>
#include <vector>
#include <algorithm>

// libheif integration - conditionally included when HAS_LIBHEIF is defined
#ifdef HAS_LIBHEIF
#include <libheif/heif.h>
#pragma comment(lib, "heif.lib")
#endif

namespace DarkThumbs {
namespace Engine {

    HEIFDecoder::HEIFDecoder()
        : m_preferEmbeddedThumbnail(true)
        , m_supportHDR(false) // Disable HDR by default (requires tone mapping)
    {
    }

    HEIFDecoder::~HEIFDecoder() = default;

    bool HEIFDecoder::CanDecode(const wchar_t* filePath) {
        if (!filePath) return false;
        
        const wchar_t* ext = wcsrchr(filePath, L'.');
        if (!ext) return false;

        return (_wcsicmp(ext, L".heif") == 0 ||
                _wcsicmp(ext, L".heic") == 0 ||
                _wcsicmp(ext, L".hif") == 0 ||
                _wcsicmp(ext, L".heifs") == 0 ||
                _wcsicmp(ext, L".heics") == 0 ||
                _wcsicmp(ext, L".avci") == 0 ||
                _wcsicmp(ext, L".avcs") == 0);
    }
    
    const wchar_t** HEIFDecoder::GetSupportedExtensions() const {
        return const_cast<const wchar_t**>(s_extensions);
    }
    
    uint32_t HEIFDecoder::GetExtensionCount() const {
        // Count non-null extensions
        uint32_t count = 0;
        while (s_extensions[count] != nullptr) {
            count++;
        }
        return count;
    }
    
    DecoderInfo HEIFDecoder::GetInfo() const {
        DecoderInfo info;
        info.name = L"HEIFDecoder";
        info.version = L"1.0.0";
        info.supportedExtensions = const_cast<const wchar_t**>(s_extensions);
        info.extensionCount = GetExtensionCount();
        info.supportsGPU = false;
        info.isArchiveDecoder = false;
        return info;
    }

    HRESULT HEIFDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
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

        // Verify HEIF signature
        if (!VerifyHEIFSignature(fileData.get(), fileSize)) {
            result.status = E_FAIL;
            return result.status;
        }

#ifdef HAS_LIBHEIF
        // Decode HEIF image
        uint32_t decodedWidth = 0;
        uint32_t decodedHeight = 0;
        uint32_t channels = 0;

        // Try fast path: extract embedded thumbnail
        uint8_t* pixels = nullptr;
        if (m_preferEmbeddedThumbnail) {
            pixels = ExtractEmbeddedThumbnail(
                fileData.get(),
                fileSize,
                decodedWidth,
                decodedHeight,
                channels
            );
        }

        // Fallback: decode full image
        if (!pixels) {
            pixels = DecodeHEIFImage(
                fileData.get(),
                fileSize,
                request.width,
                request.height,
                decodedWidth,
                decodedHeight,
                channels
            );
        }

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
        // Placeholder until libheif is integrated
        // Return E_NOTIMPL to indicate decoder not yet implemented
        result.status = E_NOTIMPL;
        return E_NOTIMPL;
#endif
    }

    bool HEIFDecoder::VerifyHEIFSignature(const uint8_t* data, size_t size) const {
        if (!data || size < 12) {
            return false;
        }

        // HEIF files are ISO Base Media Format (ISOBMFF) containers
        // They start with an 'ftyp' box at offset 4
        // Format: [4 bytes size][4 bytes 'ftyp'][4 bytes brand][...]

        // Check for 'ftyp' box
        if (data[4] != 'f' || data[5] != 't' || data[6] != 'y' || data[7] != 'p') {
            return false;
        }

        // Check for HEIF-compatible brands (major brand at offset 8-11)
        // Common brands: heic, heix, hevc, heim, heis, mif1, msf1
        const char* brand = reinterpret_cast<const char*>(data + 8);
        
        if (size >= 12) {
            // Check major brand
            if (strncmp(brand, "heic", 4) == 0 ||  // HEIC image
                strncmp(brand, "heix", 4) == 0 ||  // HEIC image with alpha
                strncmp(brand, "hevc", 4) == 0 ||  // HEVC image
                strncmp(brand, "heim", 4) == 0 ||  // HEIC multiple images
                strncmp(brand, "heis", 4) == 0 ||  // HEIC image sequence
                strncmp(brand, "mif1", 4) == 0 ||  // HEIF base
                strncmp(brand, "msf1", 4) == 0) {  // HEIF sequence
                return true;
            }

            // Check compatible brands (after major brand)
            // Many HEIF files have mif1 as major brand, heic as compatible
            for (size_t i = 16; i < size - 4 && i < 64; i += 4) {
                const char* compatBrand = reinterpret_cast<const char*>(data + i);
                if (strncmp(compatBrand, "heic", 4) == 0 ||
                    strncmp(compatBrand, "heix", 4) == 0 ||
                    strncmp(compatBrand, "hevc", 4) == 0) {
                    return true;
                }
            }
        }

        return false;
    }

    uint8_t* HEIFDecoder::DecodeHEIFImage(
        const uint8_t* fileData,
        size_t dataSize,
        uint32_t targetWidth,
        uint32_t targetHeight,
        uint32_t& outWidth,
        uint32_t& outHeight,
        uint32_t& outChannels)
    {
#ifdef HAS_LIBHEIF
        // Initialize HEIF context
        struct heif_context* ctx = heif_context_alloc();
        if (!ctx) {
            return nullptr;
        }

        // Read HEIF data
        struct heif_error err = heif_context_read_from_memory_without_copy(
            ctx, fileData, dataSize, nullptr);
        if (err.code != heif_error_Ok) {
            heif_context_free(ctx);
            return nullptr;
        }

        // Get primary image handle
        struct heif_image_handle* handle = nullptr;
        err = heif_context_get_primary_image_handle(ctx, &handle);
        if (err.code != heif_error_Ok) {
            heif_context_free(ctx);
            return nullptr;
        }

        // Get image dimensions
        int imageWidth = heif_image_handle_get_width(handle);
        int imageHeight = heif_image_handle_get_height(handle);
        bool hasAlpha = heif_image_handle_has_alpha_channel(handle);

        // Calculate thumbnail dimensions (maintain aspect ratio)
        float aspectRatio = static_cast<float>(imageWidth) / imageHeight;
        outWidth = targetWidth;
        outHeight = static_cast<uint32_t>(targetWidth / aspectRatio);
        
        if (outHeight > targetHeight) {
            outHeight = targetHeight;
            outWidth = static_cast<uint32_t>(targetHeight * aspectRatio);
        }

        outChannels = hasAlpha ? 4 : 3;

        // Decode image to RGBA/RGB
        struct heif_image* image = nullptr;
        err = heif_decode_image(handle, &image,
            heif_colorspace_RGB,
            hasAlpha ? heif_chroma_interleaved_RGBA : heif_chroma_interleaved_RGB,
            nullptr);

        if (err.code != heif_error_Ok) {
            heif_image_handle_release(handle);
            heif_context_free(ctx);
            return nullptr;
        }

        // Get pixel data
        int stride = 0;
        const uint8_t* srcData = heif_image_get_plane_readonly(
            image, heif_channel_interleaved, &stride);

        if (!srcData) {
            heif_image_release(image);
            heif_image_handle_release(handle);
            heif_context_free(ctx);
            return nullptr;
        }

        // Allocate output buffer
        size_t pixelSize = static_cast<size_t>(imageWidth) * imageHeight * outChannels;
        uint8_t* pixels = new (std::nothrow) uint8_t[pixelSize];
        if (!pixels) {
            heif_image_release(image);
            heif_image_handle_release(handle);
            heif_context_free(ctx);
            return nullptr;
        }

        // Copy pixel data
        for (int y = 0; y < imageHeight; ++y) {
            memcpy(pixels + y * imageWidth * outChannels,
                   srcData + y * stride,
                   imageWidth * outChannels);
        }

        // Update output dimensions to actual decoded size
        outWidth = imageWidth;
        outHeight = imageHeight;

        // Cleanup
        heif_image_release(image);
        heif_image_handle_release(handle);
        heif_context_free(ctx);

        return pixels;
#else
        // libheif not available
        (void)fileData; (void)dataSize;
        (void)targetWidth; (void)targetHeight;
        outWidth = 0; outHeight = 0; outChannels = 0;
        return nullptr;
#endif
    }

    uint8_t* HEIFDecoder::ExtractEmbeddedThumbnail(
        const uint8_t* fileData,
        size_t dataSize,
        uint32_t& outWidth,
        uint32_t& outHeight,
        uint32_t& outChannels)
    {
#ifdef HAS_LIBHEIF
        // Initialize HEIF context
        struct heif_context* ctx = heif_context_alloc();
        if (!ctx) {
            return nullptr;
        }

        // Read HEIF data
        struct heif_error err = heif_context_read_from_memory_without_copy(
            ctx, fileData, dataSize, nullptr);
        if (err.code != heif_error_Ok) {
            heif_context_free(ctx);
            return nullptr;
        }

        // Get primary image handle
        struct heif_image_handle* handle = nullptr;
        err = heif_context_get_primary_image_handle(ctx, &handle);
        if (err.code != heif_error_Ok) {
            heif_context_free(ctx);
            return nullptr;
        }

        // Check if thumbnail exists
        int numThumbnails = heif_image_handle_get_number_of_thumbnails(handle);
        if (numThumbnails == 0) {
            // No embedded thumbnail
            heif_image_handle_release(handle);
            heif_context_free(ctx);
            return nullptr;
        }

        // Get first thumbnail handle
        heif_item_id thumbnailId;
        heif_image_handle_get_list_of_thumbnail_IDs(handle, &thumbnailId, 1);
        
        struct heif_image_handle* thumbHandle = nullptr;
        err = heif_image_handle_get_thumbnail(handle, thumbnailId, &thumbHandle);
        if (err.code != heif_error_Ok) {
            heif_image_handle_release(handle);
            heif_context_free(ctx);
            return nullptr;
        }

        // Get thumbnail dimensions
        outWidth = heif_image_handle_get_width(thumbHandle);
        outHeight = heif_image_handle_get_height(thumbHandle);
        bool hasAlpha = heif_image_handle_has_alpha_channel(thumbHandle);
        outChannels = hasAlpha ? 4 : 3;

        // Decode thumbnail
        struct heif_image* thumbImage = nullptr;
        err = heif_decode_image(thumbHandle, &thumbImage,
            heif_colorspace_RGB,
            hasAlpha ? heif_chroma_interleaved_RGBA : heif_chroma_interleaved_RGB,
            nullptr);

        if (err.code != heif_error_Ok) {
            heif_image_handle_release(thumbHandle);
            heif_image_handle_release(handle);
            heif_context_free(ctx);
            return nullptr;
        }

        // Get pixel data
        int stride = 0;
        const uint8_t* srcData = heif_image_get_plane_readonly(
            thumbImage, heif_channel_interleaved, &stride);

        if (!srcData) {
            heif_image_release(thumbImage);
            heif_image_handle_release(thumbHandle);
            heif_image_handle_release(handle);
            heif_context_free(ctx);
            return nullptr;
        }

        // Allocate output buffer
        size_t pixelSize = static_cast<size_t>(outWidth) * outHeight * outChannels;
        uint8_t* pixels = new (std::nothrow) uint8_t[pixelSize];
        if (!pixels) {
            heif_image_release(thumbImage);
            heif_image_handle_release(thumbHandle);
            heif_image_handle_release(handle);
            heif_context_free(ctx);
            return nullptr;
        }

        // Copy pixel data
        for (uint32_t y = 0; y < outHeight; ++y) {
            memcpy(pixels + y * outWidth * outChannels,
                   srcData + y * stride,
                   outWidth * outChannels);
        }

        // Cleanup
        heif_image_release(thumbImage);
        heif_image_handle_release(thumbHandle);
        heif_image_handle_release(handle);
        heif_context_free(ctx);

        return pixels;
#else
        // libheif not available
        (void)fileData; (void)dataSize;
        outWidth = 0; outHeight = 0; outChannels = 0;
        return nullptr;
#endif
    }

    std::unique_ptr<uint8_t[]> HEIFDecoder::ReadFileData(const std::wstring& filePath, size_t& outSize) {
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

    HBITMAP HEIFDecoder::CreateHBITMAPFromRGBA(
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
