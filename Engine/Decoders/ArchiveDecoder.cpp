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

namespace ExplorerLens {
namespace Engine {

// Static data
const unsigned char ArchiveDecoder::ZIP_SIGNATURE[4] = { 0x50, 0x4B, 0x03, 0x04 }; // "PK\x03\x04"
const unsigned char ArchiveDecoder::RAR_SIGNATURE[4] = { 0x52, 0x61, 0x72, 0x21 }; // "Rar!"
const unsigned char ArchiveDecoder::SEVENZ_SIGNATURE[6] = { 0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C }; // "7z\xBC\xAF\x27\x1C"
const unsigned char ArchiveDecoder::TAR_SIGNATURE[5] = { 0x75, 0x73, 0x74, 0x61, 0x72 }; // "ustar" at offset 257

const wchar_t* ArchiveDecoder::m_extensions[] = {
 L".zip",
 L".cbz", // Comic book archive (ZIP)
 L".cb7", // Comic book archive (7z)
 L".cbr", // Comic book archive (RAR)
 L".cbt", // Comic book archive (TAR)
 L".7z", // 7-Zip archive
 L".rar", // RAR archive
 L".tar", // TAR archive
 L".tar.gz", // Compressed TAR
 L".tgz", // Compressed TAR (short)
 L".tar.bz2", // Bzip2 compressed TAR
 L".tbz", // Bzip2 compressed TAR (short)
 L".tar.xz", // XZ compressed TAR
 L".txz" // XZ compressed TAR (short)
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
 if (!filePath || !*filePath) {
 return false;
 }

 // Get simple extension first
 const wchar_t* ext = PathFindExtensionW(filePath);
 if (!ext || *ext == L'\0') {
 return false;
 }

 // Check simple extension match
 for (uint32_t i = 0; i < m_extensionCount; i++) {
 if (_wcsicmp(ext, m_extensions[i]) == 0) {
 return true;
 }
 }

 // Check compound extensions (.tar.gz, .tar.bz2, .tar.xz)
 // Find the second-to-last dot for compound extension matching
 std::wstring path(filePath);
 size_t lastDot = path.rfind(L'.');
 if (lastDot != std::wstring::npos && lastDot > 0) {
 size_t prevDot = path.rfind(L'.', lastDot - 1);
 if (prevDot != std::wstring::npos) {
 std::wstring compoundExt = path.substr(prevDot);
 for (uint32_t i = 0; i < m_extensionCount; i++) {
 if (_wcsicmp(compoundExt.c_str(), m_extensions[i]) == 0) {
 return true;
 }
 }
 }
 }

 return false;
}

HRESULT ArchiveDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
 PROFILE_SCOPE(ProfileComponent::DECODE_ARCHIVE);
 
 // Extract best cover image from archive (prefers cover.*, folder.*, 001.*)
 std::vector<unsigned char> imageData;
 std::wstring imageName;
 
 HRESULT hr = ExtractBestCoverImage(request.filePath, imageData, imageName);
 if (FAILED(hr)) {
 // Fallback to first image if no cover found
 hr = ExtractFirstImage(request.filePath, imageData, imageName);
 }
 
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

// ============================================================================
// Cover Image Detection
// ============================================================================

bool ArchiveDecoder::IsCoverImage(const std::wstring& filename) {
 std::wstring lower = filename;
 std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
 
 // Check for common cover image names
 return (lower.find(L"cover") != std::wstring::npos ||
 lower.find(L"folder") != std::wstring::npos ||
 lower.find(L"front") != std::wstring::npos ||
 lower.find(L"thumb") != std::wstring::npos ||
 lower.find(L"poster") != std::wstring::npos ||
 lower.find(L"001") != std::wstring::npos ||
 lower.find(L"00") != std::wstring::npos);
}

int ArchiveDecoder::GetImagePriority(const std::wstring& filename) {
 std::wstring lower = filename;
 std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
 
 // Higher priority = better cover image candidate
 if (lower.find(L"cover") != std::wstring::npos) return 100;
 if (lower.find(L"folder") != std::wstring::npos) return 90;
 if (lower.find(L"front") != std::wstring::npos) return 80;
 if (lower.find(L"poster") != std::wstring::npos) return 70;
 if (lower.find(L"thumb") != std::wstring::npos) return 60;
 if (lower.find(L"001.") != std::wstring::npos) return 50;
 if (lower.find(L"01.") != std::wstring::npos) return 40;
 if (lower.find(L"00.") != std::wstring::npos) return 45;
 
 // Prefer files in root directory (fewer path separators)
 int slashCount = 0;
 for (wchar_t c : filename) {
 if (c == L'/' || c == L'\\') slashCount++;
 }
 return 30 - slashCount; // Penalize deep nesting
}

// ============================================================================
// Format Detection
// ============================================================================

bool ArchiveDecoder::IsZipFormat(const wchar_t* filePath) {
 const wchar_t* ext = PathFindExtensionW(filePath);
 return ext && (_wcsicmp(ext, L".zip") == 0 || _wcsicmp(ext, L".cbz") == 0);
}

bool ArchiveDecoder::IsRarFormat(const wchar_t* filePath) {
 const wchar_t* ext = PathFindExtensionW(filePath);
 return ext && (_wcsicmp(ext, L".rar") == 0 || _wcsicmp(ext, L".cbr") == 0);
}

bool ArchiveDecoder::Is7zFormat(const wchar_t* filePath) {
 const wchar_t* ext = PathFindExtensionW(filePath);
 return ext && (_wcsicmp(ext, L".7z") == 0 || _wcsicmp(ext, L".cb7") == 0);
}

bool ArchiveDecoder::IsTarFormat(const wchar_t* filePath) {
 const wchar_t* ext = PathFindExtensionW(filePath);
 if (!ext) return false;
 std::wstring extStr(ext);
 return (extStr.find(L".tar") != std::wstring::npos || _wcsicmp(ext, L".cbt") == 0);
}

// ============================================================================
// Best Cover Image Extraction
// ============================================================================

HRESULT ArchiveDecoder::ExtractBestCoverImage(const wchar_t* archivePath,
 std::vector<unsigned char>& imageData,
 std::wstring& imageName) {
 // Only implemented for ZIP format currently
 if (!IsZipFormat(archivePath)) {
 return E_NOTIMPL;
 }
 
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

 // Find best cover image
 err = mz_zip_reader_goto_first_entry(reader);
 std::wstring bestImageName;
 int bestPriority = -1;
 int64_t bestImageSize = 0;
 
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
 int priority = GetImagePriority(filename);
 if (priority > bestPriority) {
 bestPriority = priority;
 bestImageName = filename;
 bestImageSize = file_info->uncompressed_size;
 }
 }
 }
 }
 }

 err = mz_zip_reader_goto_next_entry(reader);
 }

 if (bestImageName.empty()) {
 mz_zip_reader_close(reader);
 mz_zip_reader_delete(&reader);
 return E_FAIL;
 }

 imageName = bestImageName;

 // Limit to 32MB
 const int64_t MAX_IMAGE_SIZE = 32 * 1024 * 1024;
 if (bestImageSize > MAX_IMAGE_SIZE) {
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

// ============================================================================
// First Image Extraction (Fallback)
// ============================================================================

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

// ============================================================================
// Archive Metadata Extraction
// ============================================================================

bool ArchiveDecoder::GetArchiveMetadata(const wchar_t* filePath, ArchiveMetadata& metadata) {
 if (!filePath) return false;
 
 // Only implemented for ZIP format currently
 if (!IsZipFormat(filePath)) {
 return false;
 }
 
 metadata.format = L"ZIP";
 
 // Convert wstring to UTF-8
 int utf8Size = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, nullptr, 0, nullptr, nullptr);
 if (utf8Size <= 0) {
 return false;
 }

 std::unique_ptr<char[]> utf8Path(new char[utf8Size]);
 WideCharToMultiByte(CP_UTF8, 0, filePath, -1, utf8Path.get(), utf8Size, nullptr, nullptr);

 // Create ZIP reader
 void* reader = mz_zip_reader_create();
 if (!reader) {
 return false;
 }

 // Open ZIP file
 int32_t err = mz_zip_reader_open_file(reader, utf8Path.get());
 if (err != MZ_OK) {
 mz_zip_reader_delete(&reader);
 return false;
 }

 // Iterate through entries
 err = mz_zip_reader_goto_first_entry(reader);
 while (err == MZ_OK) {
 mz_zip_file* file_info = nullptr;
 if (mz_zip_reader_entry_get_info(reader, &file_info) == MZ_OK && file_info) {
 if (!mz_zip_reader_entry_is_dir(reader)) {
 metadata.totalFiles++;
 metadata.uncompressedSize += file_info->uncompressed_size;
 
 // Check if encrypted
 if ((file_info->flag & MZ_ZIP_FLAG_ENCRYPTED) != 0) {
 metadata.isEncrypted = true;
 }
 
 // Count image files
 int wideSize = MultiByteToWideChar(CP_UTF8, 0, file_info->filename, -1, nullptr, 0);
 if (wideSize > 0) {
 std::unique_ptr<wchar_t[]> wideName(new wchar_t[wideSize]);
 MultiByteToWideChar(CP_UTF8, 0, file_info->filename, -1, wideName.get(), wideSize);
 std::wstring filename(wideName.get());
 
 if (IsImageFile(filename)) {
 metadata.totalImages++;
 }
 }
 }
 }
 err = mz_zip_reader_goto_next_entry(reader);
 }

 mz_zip_reader_close(reader);
 mz_zip_reader_delete(&reader);
 
 return true;
}

} // namespace Engine
} // namespace ExplorerLens

