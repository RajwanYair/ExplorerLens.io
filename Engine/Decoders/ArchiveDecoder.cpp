// ArchiveDecoder.cpp
// Archive (ZIP/CBZ) thumbnail decoder implementation

#include "ArchiveDecoder.h"
#include "../Utils/PerformanceProfiler.h"
#include <algorithm>
#include <shlwapi.h>
#include <wincodec.h>
#include <memory>

// minizip-ng includes
extern "C" {
#include "mz.h"
#include "mz_strm.h"
#include "mz_strm_os.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"
}

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace DarkThumbs {
namespace Engine {

// Static data
const unsigned char ArchiveDecoder::ZIP_SIGNATURE[4] = { 0x50, 0x4B, 0x03, 0x04 }; // "PK\x03\x04"

const wchar_t* ArchiveDecoder::m_extensions[] = {
    L".zip",
    L".cbz"  // Comic book archive (ZIP-based)
};

const uint32_t ArchiveDecoder::m_extensionCount = sizeof(m_extensions) / sizeof(m_extensions[0]);

// WIC factory singleton (shared with ImageDecoder)
static IWICImagingFactory* g_pWICFactory = nullptr;

IWICImagingFactory* ArchiveDecoder::GetWICFactory() {
    if (!g_pWICFactory) {
        CoInitialize(nullptr);
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&g_pWICFactory)
        );
        if (FAILED(hr)) {
            return nullptr;
        }
    }
    return g_pWICFactory;
}

ArchiveDecoder::ArchiveDecoder() {
}

ArchiveDecoder::~ArchiveDecoder() {
}

DecoderInfo ArchiveDecoder::GetInfo() const {
    DecoderInfo info;
    info.name = GetName();
    info.version = L"1.0.0";
    info.extensionCount = m_extensionCount;
    info.supportedExtensions = m_extensions;
    info.supportsGPU = false;
    info.isArchiveDecoder = true;
    return info;
}

bool ArchiveDecoder::IsArchiveFormat(const void* pData, size_t dataSize) {
    if (!pData || dataSize < 4) {
        return false;
    }

    // Check for ZIP signature: PK\x03\x04
    const unsigned char* bytes = static_cast<const unsigned char*>(pData);
    return (bytes[0] == ZIP_SIGNATURE[0] &&
            bytes[1] == ZIP_SIGNATURE[1] &&
            bytes[2] == ZIP_SIGNATURE[2] &&
            bytes[3] == ZIP_SIGNATURE[3]);
}

bool ArchiveDecoder::IsImageFile(const std::wstring& filename) {
    // Convert to lowercase for comparison
    std::wstring lower = filename;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);

    // Check common image extensions
    static const wchar_t* imageExts[] = {
        L".jpg", L".jpeg", L".png", L".bmp", L".gif", L".tif", L".tiff",
        L".webp", L".avif", L".heic", L".heif", L".jxl"
    };

    for (const auto& ext : imageExts) {
        if (lower.size() >= wcslen(ext)) {
            if (lower.compare(lower.size() - wcslen(ext), wcslen(ext), ext) == 0) {
                return true;
            }
        }
    }

    return false;
}

bool ArchiveDecoder::CanDecode(const wchar_t* filePath) {
    // Check extension
    const wchar_t* ext = PathFindExtensionW(filePath);
    if (!ext || *ext == L'\0') {
        return false;
    }

    // Check against supported extensions
    for (uint32_t i = 0; i < m_extensionCount; i++) {
        if (_wcsicmp(ext, m_extensions[i]) == 0) {
            return true;
        }
    }

    return false;
}

HRESULT ArchiveDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
    PROFILE_SCOPE(ProfileComponent::DECODE_ARCHIVE);
    
    // Extract first image from archive
    std::vector<unsigned char> imageData;
    std::wstring imageName;
    
    HRESULT hr = ExtractFirstImage(request.filePath, imageData, imageName);
    if (FAILED(hr)) {
        return hr;
    }

    // Decode the extracted image
    hr = DecodeImageData(imageData, imageName, request.width, request.height, &result.hBitmap);
    if (SUCCEEDED(hr)) {
        // Set result dimensions
        BITMAP bm;
        if (GetObject(result.hBitmap, sizeof(BITMAP), &bm)) {
            result.width = bm.bmWidth;
            result.height = bm.bmHeight;
        }
        result.status = S_OK;
    }
    
    return hr;
}

HRESULT ArchiveDecoder::ExtractFirstImage(const wchar_t* archivePath,
                                          std::vector<unsigned char>& imageData,
                                          std::wstring& imageName) {
    // Convert wstring to UTF-8
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, archivePath, -1, nullptr, 0, nullptr, nullptr);
    if (utf8Size <= 0) {
        return E_INVALIDARG;
    }

    std::unique_ptr<char[]> utf8Path(new char[utf8Size]);
    WideCharToMultiByte(CP_UTF8, 0, archivePath, -1, utf8Path.get(), utf8Size, nullptr, nullptr);

    // Create ZIP reader
    void* reader = mz_zip_reader_create();
    if (!reader) {
        return E_OUTOFMEMORY;
    }

    // Open ZIP file
    int32_t err = mz_zip_reader_open_file(reader, utf8Path.get());
    if (err != MZ_OK) {
        mz_zip_reader_delete(&reader);
        return E_FAIL;
    }

    // Find first image file
    err = mz_zip_reader_goto_first_entry(reader);
    std::wstring firstImageName;
    int64_t firstImageSize = 0;
    
    while (err == MZ_OK) {
        mz_zip_file* file_info = nullptr;
        if (mz_zip_reader_entry_get_info(reader, &file_info) == MZ_OK && file_info) {
            // Skip directories
            if (!mz_zip_reader_entry_is_dir(reader)) {
                // Convert filename to wstring
                int wideSize = MultiByteToWideChar(CP_UTF8, 0, file_info->filename, -1, nullptr, 0);
                if (wideSize > 0) {
                    std::unique_ptr<wchar_t[]> wideName(new wchar_t[wideSize]);
                    MultiByteToWideChar(CP_UTF8, 0, file_info->filename, -1, wideName.get(), wideSize);
                    std::wstring filename(wideName.get());

                    // Check if it's an image
                    if (IsImageFile(filename)) {
                        if (firstImageName.empty() || _wcsicmp(filename.c_str(), firstImageName.c_str()) < 0) {
                            firstImageName = filename;
                            firstImageSize = file_info->uncompressed_size;
                        }
                    }
                }
            }
        }

        err = mz_zip_reader_goto_next_entry(reader);
    }

    if (firstImageName.empty()) {
        mz_zip_reader_close(reader);
        mz_zip_reader_delete(&reader);
        return E_FAIL;
    }

    imageName = firstImageName;

    // Limit to 32MB
    const int64_t MAX_IMAGE_SIZE = 32 * 1024 * 1024;
    if (firstImageSize > MAX_IMAGE_SIZE) {
        mz_zip_reader_close(reader);
        mz_zip_reader_delete(&reader);
        return E_OUTOFMEMORY;
    }

    // Convert image name to UTF-8
    utf8Size = WideCharToMultiByte(CP_UTF8, 0, imageName.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (utf8Size <= 0) {
        mz_zip_reader_close(reader);
        mz_zip_reader_delete(&reader);
        return E_INVALIDARG;
    }

    std::unique_ptr<char[]> utf8Name(new char[utf8Size]);
    WideCharToMultiByte(CP_UTF8, 0, imageName.c_str(), -1, utf8Name.get(), utf8Size, nullptr, nullptr);

    // Locate the entry
    err = mz_zip_reader_locate_entry(reader, utf8Name.get(), 0);
    if (err != MZ_OK) {
        mz_zip_reader_close(reader);
        mz_zip_reader_delete(&reader);
        return E_FAIL;
    }

    // Open entry for reading
    err = mz_zip_reader_entry_open(reader);
    if (err != MZ_OK) {
        mz_zip_reader_close(reader);
        mz_zip_reader_delete(&reader);
        return E_FAIL;
    }

    // Get entry info
    mz_zip_file* file_info = nullptr;
    err = mz_zip_reader_entry_get_info(reader, &file_info);
    if (err != MZ_OK || !file_info) {
        mz_zip_reader_entry_close(reader);
        mz_zip_reader_close(reader);
        mz_zip_reader_delete(&reader);
        return E_FAIL;
    }

    // Allocate buffer
    size_t fileSize = static_cast<size_t>(file_info->uncompressed_size);
    imageData.resize(fileSize);

    // Read entry data
    int32_t bytesRead = mz_zip_reader_entry_read(reader, imageData.data(), static_cast<int32_t>(fileSize));
    
    mz_zip_reader_entry_close(reader);
    mz_zip_reader_close(reader);
    mz_zip_reader_delete(&reader);

    if (bytesRead != static_cast<int32_t>(fileSize)) {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT ArchiveDecoder::DecodeImageData(const std::vector<unsigned char>& imageData,
                                        const std::wstring& imageName,
                                        UINT targetWidth, UINT targetHeight,
                                        HBITMAP* phBitmap) {
    (void)imageName; // Unused for now

    IWICImagingFactory* pFactory = GetWICFactory();
    if (!pFactory) {
        return E_FAIL;
    }

    // Create stream from memory
    IWICStream* pStream = nullptr;
    HRESULT hr = pFactory->CreateStream(&pStream);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pStream->InitializeFromMemory(
        const_cast<BYTE*>(imageData.data()),
        static_cast<DWORD>(imageData.size())
    );
    if (FAILED(hr)) {
        pStream->Release();
        return hr;
    }

    // Create decoder
    IWICBitmapDecoder* pDecoder = nullptr;
    hr = pFactory->CreateDecoderFromStream(
        pStream,
        nullptr,
        WICDecodeMetadataCacheOnDemand,
        &pDecoder
    );
    pStream->Release();

    if (FAILED(hr)) {
        return hr;
    }

    // Get first frame
    IWICBitmapFrameDecode* pFrame = nullptr;
    hr = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hr)) {
        pDecoder->Release();
        return hr;
    }

    // Convert to 32bpp BGRA
    IWICFormatConverter* pConverter = nullptr;
    hr = pFactory->CreateFormatConverter(&pConverter);
    if (FAILED(hr)) {
        pFrame->Release();
        pDecoder->Release();
        return hr;
    }

    hr = pConverter->Initialize(
        pFrame,
        GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );
    pFrame->Release();

    if (FAILED(hr)) {
        pConverter->Release();
        pDecoder->Release();
        return hr;
    }

    // Scale if needed
    IWICBitmapSource* pSource = pConverter;
    IWICBitmapScaler* pScaler = nullptr;

    UINT width, height;
    pConverter->GetSize(&width, &height);

    if ((targetWidth > 0 && targetHeight > 0) &&
        (width != targetWidth || height != targetHeight)) {
        hr = pFactory->CreateBitmapScaler(&pScaler);
        if (SUCCEEDED(hr)) {
            hr = pScaler->Initialize(
                pConverter,
                targetWidth,
                targetHeight,
                WICBitmapInterpolationModeFant
            );
            if (SUCCEEDED(hr)) {
                pSource = pScaler;
                width = targetWidth;
                height = targetHeight;
            }
        }
    }

    // Create HBITMAP
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -(LONG)height; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HDC hdc = GetDC(nullptr);
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    ReleaseDC(nullptr, hdc);

    if (!hBitmap) {
        if (pScaler) pScaler->Release();
        pConverter->Release();
        pDecoder->Release();
        return E_FAIL;
    }

    // Copy pixels
    UINT stride = width * 4;
    hr = pSource->CopyPixels(nullptr, stride, stride * height, static_cast<BYTE*>(pBits));

    if (pScaler) pScaler->Release();
    pConverter->Release();
    pDecoder->Release();

    if (FAILED(hr)) {
        DeleteObject(hBitmap);
        return hr;
    }

    *phBitmap = hBitmap;
    return S_OK;
}

} // namespace Engine
} // namespace DarkThumbs
