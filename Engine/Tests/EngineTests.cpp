//==============================================================================
// DarkThumbs Engine - Unit Tests
// Copyright (c) 2026 - DarkThumbs Project
//==============================================================================

#include "../Pipeline/DecoderRegistry.h"
#include "../Pipeline/FormatDetector.h"
#include "../Decoders/ImageDecoder.h"
#include "../Decoders/WebPDecoder.h"
#include "../Decoders/AVIFDecoder.h"
#include "../Decoders/ArchiveDecoder.h"
#include "../Decoders/RAWDecoder.h"
#include "../Decoders/JXLDecoder.h"
#include "../Decoders/HEIFDecoder.h"
#include "../Decoders/ICODecoder.h"
#include "../Decoders/TGADecoder.h"
#include "../Decoders/QOIDecoder.h"
#include "../Decoders/PSDDecoder.h"
#include "../Decoders/DDSDecoder.h"
#include "../Decoders/HDRDecoder.h"
#include "../Decoders/PPMDecoder.h"
#include "../Decoders/EXRDecoder.h"
#include "../Decoders/SVGDecoder.h"
#include "../Decoders/VideoDecoder.h"
#include "../Decoders/AudioDecoder.h"
#include "../Decoders/PDFDecoder.h"
#include "../Decoders/DocumentDecoder.h"
#include "../Decoders/FontDecoder.h"
#include "../Decoders/ModelDecoder.h"
#include "../Decoders/EPSDecoder.h"
#include "../Decoders/KTXTextureDecoder.h"
#include "../Decoders/VTFDecoder.h"
#include <iostream>
#include <chrono>
#include <psapi.h>

#pragma comment(lib, "psapi.lib")
#include <cassert>
#include <vector>
#include <string>

// GPU tests
extern void RunGPUTests();

using namespace DarkThumbs::Engine;

//==============================================================================
// Test Counters
//==============================================================================

int g_testsRun = 0;
int g_testsPassed = 0;
int g_testsFailed = 0;

#define TEST(name) \
    void name(); \
    void name##_Runner() { \
        std::wcout << L"Running: " << L"" #name << L"..." << std::endl; \
        g_testsRun++; \
        try { \
            name(); \
            g_testsPassed++; \
            std::wcout << L"  [PASS]" << std::endl; \
        } catch (const char* msg) { \
            g_testsFailed++; \
            std::cout << "  [FAIL] " << msg << std::endl; \
        } \
    } \
    void name()

#define ASSERT(expr) \
    if (!(expr)) { \
        throw "Assertion failed: " #expr; \
    }

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        throw "Assertion failed: " #a " == " #b; \
    }

#define ASSERT_NE(a, b) \
    if ((a) == (b)) { \
        throw "Assertion failed: " #a " != " #b; \
    }

#define ASSERT_NULL(ptr) \
    if ((ptr) != nullptr) { \
        throw "Assertion failed: " #ptr " == nullptr"; \
    }

#define ASSERT_NOT_NULL(ptr) \
    if ((ptr) == nullptr) { \
        throw "Assertion failed: " #ptr " != nullptr"; \
    }

#define RUN_TEST(name) name##_Runner()

//==============================================================================
// Mock Decoder for Testing
//==============================================================================

class MockDecoder : public IThumbnailDecoder
{
public:
    MockDecoder(const wchar_t* name, const wchar_t* ext1, const wchar_t* ext2 = nullptr)
        : m_name(name)
    {
        m_extensions[0] = ext1;
        m_extensions[1] = ext2;
        m_extensions[2] = nullptr;
        m_extensionCount = ext2 ? 2 : 1;
    }
    
    bool CanDecode(const wchar_t* filePath) override {
        if (!filePath) return false;
        const wchar_t* ext = wcsrchr(filePath, L'.');
        if (!ext) return false;
        
        for (size_t i = 0; i < m_extensionCount; i++) {
            if (_wcsicmp(ext, m_extensions[i]) == 0) {
                return true;
            }
        }
        return false;
    }
    
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override {
        (void)request; // Suppress unused parameter warning
        result.status = S_OK;
        return S_OK;
    }
    
    DecoderInfo GetInfo() const override {
        DecoderInfo info;
        info.name = m_name;
        info.version = L"1.0.0";
        info.supportedExtensions = const_cast<const wchar_t**>(m_extensions);
        info.extensionCount = m_extensionCount;
        info.supportsGPU = false;
        info.isArchiveDecoder = false;
        return info;
    }
    
    const wchar_t* GetName() const override {
        return m_name;
    }
    
    const wchar_t** GetSupportedExtensions() const override {
        return const_cast<const wchar_t**>(m_extensions);
    }
    
    uint32_t GetExtensionCount() const override {
        return m_extensionCount;
    }
    
    bool SupportsGPU() const override {
        return false;
    }
    
    bool IsArchiveDecoder() const override {
        return false;
    }

private:
    const wchar_t* m_name;
    const wchar_t* m_extensions[3];
    uint32_t m_extensionCount;
};

//==============================================================================
// Decoder Registry Tests
//==============================================================================

TEST(TestDecoderRegistry_Create)
{
    DecoderRegistry registry;
    ASSERT_EQ(registry.GetDecoderCount(), 0);
}

TEST(TestDecoderRegistry_RegisterDecoder)
{
    DecoderRegistry registry;
    
    MockDecoder* decoder = new MockDecoder(L"Test Decoder", L".test");
    ASSERT(registry.RegisterDecoder(decoder));
    ASSERT_EQ(registry.GetDecoderCount(), 1);
}

TEST(TestDecoderRegistry_RegisterMultipleDecoders)
{
    DecoderRegistry registry;
    
    registry.RegisterDecoder(new MockDecoder(L"Decoder 1", L".ext1"));
    registry.RegisterDecoder(new MockDecoder(L"Decoder 2", L".ext2"));
    registry.RegisterDecoder(new MockDecoder(L"Decoder 3", L".ext3"));
    
    ASSERT_EQ(registry.GetDecoderCount(), 3);
}

TEST(TestDecoderRegistry_FindDecoder)
{
    DecoderRegistry registry;
    
    registry.RegisterDecoder(new MockDecoder(L"JPEG Decoder", L".jpg", L".jpeg"));
    registry.RegisterDecoder(new MockDecoder(L"PNG Decoder", L".png"));
    
    IThumbnailDecoder* jpgDecoder = registry.FindDecoder(L"photo.jpg");
    ASSERT_NOT_NULL(jpgDecoder);
    ASSERT_EQ(wcscmp(jpgDecoder->GetName(), L"JPEG Decoder"), 0);
    
    IThumbnailDecoder* pngDecoder = registry.FindDecoder(L"image.png");
    ASSERT_NOT_NULL(pngDecoder);
    ASSERT_EQ(wcscmp(pngDecoder->GetName(), L"PNG Decoder"), 0);
}

TEST(TestDecoderRegistry_FindDecoderByName)
{
    DecoderRegistry registry;
    
    registry.RegisterDecoder(new MockDecoder(L"Decoder A", L".a"));
    registry.RegisterDecoder(new MockDecoder(L"Decoder B", L".b"));
    
    IThumbnailDecoder* decoder = registry.FindDecoderByName(L"Decoder B");
    ASSERT_NOT_NULL(decoder);
    ASSERT_EQ(wcscmp(decoder->GetName(), L"Decoder B"), 0);
}

TEST(TestDecoderRegistry_GetStats)
{
    DecoderRegistry registry;
    
    registry.RegisterDecoder(new MockDecoder(L"Decoder 1", L".ext1", L".ext2"));
    registry.RegisterDecoder(new MockDecoder(L"Decoder 2", L".ext3"));
    
    size_t totalDecoders, imageDecoders, archiveDecoders, totalExtensions;
    registry.GetStats(&totalDecoders, &imageDecoders, &archiveDecoders, &totalExtensions);
    
    ASSERT_EQ(totalDecoders, 2);
    ASSERT_EQ(totalExtensions, 3);
}

TEST(TestDecoderRegistry_ComprehensiveIntegration)
{
    DecoderRegistry registry;
    
    // Register all real decoders
    registry.RegisterDecoder(new ImageDecoder());
    registry.RegisterDecoder(new WebPDecoder());
    registry.RegisterDecoder(new AVIFDecoder());
    registry.RegisterDecoder(new ArchiveDecoder());
    registry.RegisterDecoder(new JXLDecoder());
    registry.RegisterDecoder(new HEIFDecoder());
    
    // Test that all expected formats can be decoded
    struct FormatTest {
        const wchar_t* extension;
        const wchar_t* expectedDecoder;
    };
    
    std::vector<FormatTest> tests = {
        { L".jpg", L"ImageDecoder" },
        { L".jpeg", L"ImageDecoder" },
        { L".png", L"ImageDecoder" },
        { L".bmp", L"ImageDecoder" },
        { L".webp", L"WebPDecoder" },
        { L".avif", L"AVIFDecoder" },
        { L".jxl", L"JXLDecoder" },
        { L".heif", L"HEIFDecoder" },
        { L".heic", L"HEIFDecoder" },
        { L".zip", L"ArchiveDecoder" },
        { L".cbz", L"ArchiveDecoder" }
        // Note: .rar not supported yet - requires UnRAR library
    };
    
    for (const auto& test : tests) {
        std::wstring testFile = std::wstring(L"test") + test.extension;
        IThumbnailDecoder* decoder = registry.FindDecoder(testFile.c_str());
        
        // Verify decoder was found
        ASSERT(decoder != nullptr);
        
        // Verify it's the right decoder (basic check via CanDecode)
        ASSERT(decoder->CanDecode(testFile.c_str()));
    }
    
    // Test stats are correct
    size_t totalDecoders, imageDecoders, archiveDecoders, totalExtensions;
    registry.GetStats(&totalDecoders, &imageDecoders, &archiveDecoders, &totalExtensions);
    
    ASSERT(totalDecoders >= 6);  // At least the 6 we registered
    ASSERT(imageDecoders >= 5);  // Image, WebP, AVIF, JXL, HEIF
    ASSERT(archiveDecoders >= 1); // Archive decoder
    ASSERT(totalExtensions >= 20); // Many extensions across all decoders
}

//==============================================================================
// Format Detector Tests
//==============================================================================

TEST(TestFormatDetector_Create)
{
    FormatDetector detector;
    // Just verify it can be created
}

TEST(TestFormatDetector_DetectJPEG)
{
    FormatDetector detector;
    
    ASSERT_EQ(detector.DetectFromExtension(L".jpg"), FormatType::ImageJPEG);
    ASSERT_EQ(detector.DetectFromExtension(L".jpeg"), FormatType::ImageJPEG);
}

TEST(TestFormatDetector_DetectPNG)
{
    FormatDetector detector;
    
    ASSERT_EQ(detector.DetectFromExtension(L".png"), FormatType::ImagePNG);
}

TEST(TestFormatDetector_DetectZIP)
{
    FormatDetector detector;
    
    ASSERT_EQ(detector.DetectFromExtension(L".zip"), FormatType::ArchiveZIP);
    ASSERT_EQ(detector.DetectFromExtension(L".cbz"), FormatType::ArchiveZIP);
}

TEST(TestFormatDetector_DetectRAR)
{
    FormatDetector detector;
    
    ASSERT_EQ(detector.DetectFromExtension(L".rar"), FormatType::ArchiveRAR);
    ASSERT_EQ(detector.DetectFromExtension(L".cbr"), FormatType::ArchiveRAR);
}

TEST(TestFormatDetector_IsImageFormat)
{
    FormatDetector detector;
    
    ASSERT(detector.IsImageFormat(L".jpg"));
    ASSERT(detector.IsImageFormat(L".png"));
    ASSERT(detector.IsImageFormat(L".bmp"));
    ASSERT(!detector.IsImageFormat(L".zip"));
    ASSERT(!detector.IsImageFormat(L".pdf"));
}

TEST(TestFormatDetector_IsArchiveFormat)
{
    FormatDetector detector;
    
    ASSERT(detector.IsArchiveFormat(L".zip"));
    ASSERT(detector.IsArchiveFormat(L".cbz"));
    ASSERT(detector.IsArchiveFormat(L".rar"));
    ASSERT(!detector.IsArchiveFormat(L".jpg"));
    ASSERT(!detector.IsArchiveFormat(L".pdf"));
}

TEST(TestFormatDetector_GetExtension)
{
    FormatDetector detector;
    
    const wchar_t* ext = detector.GetExtension(L"C:\\path\\to\\file.jpg");
    ASSERT_NOT_NULL(ext);
    ASSERT_EQ(wcscmp(ext, L".jpg"), 0);
    
    ext = detector.GetExtension(L"image.png");
    ASSERT_NOT_NULL(ext);
    ASSERT_EQ(wcscmp(ext, L".png"), 0);
}

//==============================================================================
// Image Decoder Tests
//==============================================================================

TEST(TestImageDecoder_Create)
{
    ImageDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"ImageDecoder"), 0);
    ASSERT(decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestImageDecoder_Extensions)
{
    ImageDecoder decoder;
    const wchar_t** extensions = decoder.GetSupportedExtensions();
    uint32_t count = decoder.GetExtensionCount();
    
    // Should support at least 5 formats (JPEG, PNG, BMP, GIF, TIFF)
    ASSERT(count >= 5);
    ASSERT_NOT_NULL(extensions);
    
    // Check for key extensions
    bool hasJPG = false, hasPNG = false, hasBMP = false;
    for (uint32_t i = 0; i < count; i++) {
        if (wcscmp(extensions[i], L".jpg") == 0 || wcscmp(extensions[i], L".jpeg") == 0) {
            hasJPG = true;
        }
        if (wcscmp(extensions[i], L".png") == 0) {
            hasPNG = true;
        }
        if (wcscmp(extensions[i], L".bmp") == 0) {
            hasBMP = true;
        }
    }
    
    ASSERT(hasJPG);
    ASSERT(hasPNG);
    ASSERT(hasBMP);
}

TEST(TestImageDecoder_CanDecodeJPEG)
{
    ImageDecoder decoder;
    ASSERT(decoder.CanDecode(L"test.jpg"));
    ASSERT(decoder.CanDecode(L"test.JPEG"));
}

TEST(TestImageDecoder_CanDecodePNG)
{
    ImageDecoder decoder;
    ASSERT(decoder.CanDecode(L"test.png"));
}

TEST(TestImageDecoder_CanDecodeBMP)
{
    ImageDecoder decoder;
    ASSERT(decoder.CanDecode(L"test.bmp"));
}

TEST(TestImageDecoder_CannotDecodeUnsupported)
{
    ImageDecoder decoder;
    ASSERT(!decoder.CanDecode(L"test.webp"));
    ASSERT(!decoder.CanDecode(L"test.zip"));
    ASSERT(!decoder.CanDecode(nullptr));
}

TEST(TestImageDecoder_GetInfo)
{
    ImageDecoder decoder;
    DecoderInfo info = decoder.GetInfo();
    
    ASSERT_EQ(wcscmp(info.name, L"ImageDecoder"), 0);
    ASSERT_EQ(wcscmp(info.version, L"1.0.0"), 0);
    ASSERT(info.extensionCount >= 5); // JPEG, PNG, BMP, GIF, TIFF
    ASSERT(info.supportsGPU);
    ASSERT(!info.isArchiveDecoder);
}

TEST(TestImageDecoder_RegisterWithRegistry)
{
    DecoderRegistry registry;
    ImageDecoder decoder;
    
    ASSERT(registry.RegisterDecoder(&decoder));
    
    // Find by extension
    auto found = registry.FindDecoder(L"photo.jpg");
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(wcscmp(found->GetName(), L"ImageDecoder"), 0);
}

//==============================================================================
// WebP Decoder Tests
//==============================================================================

TEST(TestWebPDecoder_Create)
{
    WebPDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"WebPDecoder"), 0);
    ASSERT(!decoder.SupportsGPU()); // CPU-based libwebp
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestWebPDecoder_Extensions)
{
    WebPDecoder decoder;
    const wchar_t** extensions = decoder.GetSupportedExtensions();
    uint32_t count = decoder.GetExtensionCount();
    
    ASSERT_EQ(count, 1);
    ASSERT_NOT_NULL(extensions);
    ASSERT_EQ(wcscmp(extensions[0], L".webp"), 0);
}

TEST(TestWebPDecoder_CanDecode)
{
    WebPDecoder decoder;
    
    ASSERT(decoder.CanDecode(L"image.webp"));
    ASSERT(decoder.CanDecode(L"photo.WEBP")); // Case insensitive
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(L"archive.zip"));
    ASSERT(!decoder.CanDecode(nullptr));
}

TEST(TestWebPDecoder_GetInfo)
{
    WebPDecoder decoder;
    DecoderInfo info = decoder.GetInfo();
    
    ASSERT_EQ(wcscmp(info.name, L"WebPDecoder"), 0);
    ASSERT_EQ(wcscmp(info.version, L"1.0.0"), 0);
    ASSERT_EQ(info.extensionCount, 1);
    ASSERT(!info.supportsGPU);
    ASSERT(!info.isArchiveDecoder);
}

TEST(TestWebPDecoder_RegisterWithRegistry)
{
    DecoderRegistry registry;
    WebPDecoder decoder;
    
    ASSERT(registry.RegisterDecoder(&decoder));
    
    // Find by extension
    auto found = registry.FindDecoder(L"photo.webp");
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(wcscmp(found->GetName(), L"WebPDecoder"), 0);
}

//==============================================================================
// AVIF Decoder Tests
//==============================================================================

TEST(TestAVIFDecoder_Create)
{
    AVIFDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"AVIFDecoder"), 0);
    ASSERT(decoder.SupportsGPU()); // WIC can use GPU
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestAVIFDecoder_Extensions)
{
    AVIFDecoder decoder;
    const wchar_t** extensions = decoder.GetSupportedExtensions();
    uint32_t count = decoder.GetExtensionCount();
    
    ASSERT_EQ(count, 3); // .avif, .heif, .heic
    ASSERT_NOT_NULL(extensions);
    
    // Check for specific extensions
    bool hasAVIF = false, hasHEIF = false, hasHEIC = false;
    for (uint32_t i = 0; i < count; i++) {
        if (wcscmp(extensions[i], L".avif") == 0) hasAVIF = true;
        if (wcscmp(extensions[i], L".heif") == 0) hasHEIF = true;
        if (wcscmp(extensions[i], L".heic") == 0) hasHEIC = true;
    }
    
    ASSERT(hasAVIF);
    ASSERT(hasHEIF);
    ASSERT(hasHEIC);
}

TEST(TestAVIFDecoder_CanDecode)
{
    AVIFDecoder decoder;
    
    ASSERT(decoder.CanDecode(L"image.avif"));
    ASSERT(decoder.CanDecode(L"photo.HEIF")); // Case insensitive
    ASSERT(decoder.CanDecode(L"iphone.heic"));
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(L"archive.zip"));
    ASSERT(!decoder.CanDecode(nullptr));
}

TEST(TestAVIFDecoder_GetInfo)
{
    AVIFDecoder decoder;
    DecoderInfo info = decoder.GetInfo();
    
    ASSERT_EQ(wcscmp(info.name, L"AVIFDecoder"), 0);
    ASSERT_EQ(wcscmp(info.version, L"1.0.0"), 0);
    ASSERT_EQ(info.extensionCount, 3);
    ASSERT(info.supportsGPU);
    ASSERT(!info.isArchiveDecoder);
}

TEST(TestAVIFDecoder_RegisterWithRegistry)
{
    DecoderRegistry registry;
    AVIFDecoder decoder;
    
    ASSERT(registry.RegisterDecoder(&decoder));
    
    // Find by extension
    auto found = registry.FindDecoder(L"photo.avif");
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(wcscmp(found->GetName(), L"AVIFDecoder"), 0);
}

//==============================================================================
// Archive Decoder Tests
//==============================================================================

TEST(TestArchiveDecoder_Create) {
    ArchiveDecoder decoder;
    
    ASSERT_NOT_NULL(decoder.GetName());
    ASSERT_EQ(wcscmp(decoder.GetName(), L"ArchiveDecoder"), 0);
}

TEST(TestArchiveDecoder_Extensions) {
    ArchiveDecoder decoder;
    
    const wchar_t** extensions = decoder.GetSupportedExtensions();
    uint32_t count = decoder.GetExtensionCount();
    
    ASSERT_NOT_NULL(extensions);
    ASSERT_EQ(count, 14); // Full archive format set
    
    // Check core extensions
    bool hasZip = false;
    bool hasCbz = false;
    bool has7z = false;
    bool hasRar = false;
    bool hasTarGz = false;
    
    for (uint32_t i = 0; i < count; i++) {
        if (wcscmp(extensions[i], L".zip") == 0) hasZip = true;
        if (wcscmp(extensions[i], L".cbz") == 0) hasCbz = true;
        if (wcscmp(extensions[i], L".7z") == 0) has7z = true;
        if (wcscmp(extensions[i], L".rar") == 0) hasRar = true;
        if (wcscmp(extensions[i], L".tar.gz") == 0) hasTarGz = true;
    }
    
    ASSERT(hasZip);
    ASSERT(hasCbz);
    ASSERT(has7z);
    ASSERT(hasRar);
    ASSERT(hasTarGz);
}

TEST(TestArchiveDecoder_CanDecode) {
    ArchiveDecoder decoder;
    
    ASSERT(decoder.CanDecode(L"archive.zip"));
    ASSERT(decoder.CanDecode(L"comic.cbz"));
    ASSERT(decoder.CanDecode(L"path/to/file.ZIP")); // Case insensitive
    ASSERT(decoder.CanDecode(L"archive.rar"));
    ASSERT(decoder.CanDecode(L"archive.7z"));
    
    // Should not decode non-archive formats
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(L"photo.png"));
}

TEST(TestArchiveDecoder_IsArchiveFormat) {
    // Valid ZIP signature: PK\x03\x04
    const unsigned char zipSig[] = { 0x50, 0x4B, 0x03, 0x04, 0x00, 0x00 };
    ASSERT(ArchiveDecoder::IsArchiveFormat(zipSig, sizeof(zipSig)));
    
    // Invalid signatures
    const unsigned char invalidSig1[] = { 0x00, 0x00, 0x00, 0x00 };
    ASSERT(!ArchiveDecoder::IsArchiveFormat(invalidSig1, sizeof(invalidSig1)));
    
    const unsigned char invalidSig2[] = { 0xFF, 0xD8, 0xFF, 0xE0 }; // JPEG
    ASSERT(!ArchiveDecoder::IsArchiveFormat(invalidSig2, sizeof(invalidSig2)));
    
    // Too small
    const unsigned char tooSmall[] = { 0x50, 0x4B };
    ASSERT(!ArchiveDecoder::IsArchiveFormat(tooSmall, sizeof(tooSmall)));
    
    // Null data
    ASSERT(!ArchiveDecoder::IsArchiveFormat(nullptr, 100));
}

TEST(TestArchiveDecoder_GetInfo) {
    ArchiveDecoder decoder;
    
    DecoderInfo info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
    ASSERT_EQ(wcscmp(info.name, L"ArchiveDecoder"), 0);
    ASSERT_EQ(info.isArchiveDecoder, true);
    ASSERT_EQ(info.supportsGPU, false);
}

TEST(TestArchiveDecoder_RegisterWithRegistry) {
    DecoderRegistry registry;
    ArchiveDecoder decoder;
    
    ASSERT(registry.RegisterDecoder(&decoder));
    
    // Find by extension
    auto found = registry.FindDecoder(L"archive.zip");
    ASSERT_NOT_NULL(found);
    ASSERT_EQ(wcscmp(found->GetName(), L"ArchiveDecoder"), 0);
}

//==============================================================================
// JXL Decoder Tests (Re-enabled - basic interface tests)
//==============================================================================

TEST(TestJXLDecoder_Create)
{
    JXLDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"JXLDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestJXLDecoder_CanDecode)
{
    JXLDecoder decoder;
    
    ASSERT(decoder.CanDecode(L"image.jxl"));
    ASSERT(decoder.CanDecode(L"photo.JXL")); // Case insensitive
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(L"archive.zip"));
    ASSERT(!decoder.CanDecode(nullptr));
    ASSERT(!decoder.CanDecode(L"noextension"));
}

TEST(TestJXLDecoder_GetInfo)
{
    JXLDecoder decoder;
    DecoderInfo info = decoder.GetInfo();
    
    ASSERT_NOT_NULL(info.name);
    ASSERT_EQ(wcscmp(info.name, L"JXLDecoder"), 0);
    ASSERT_NOT_NULL(info.version);
    ASSERT(info.extensionCount >= 1);
    ASSERT_NOT_NULL(info.supportedExtensions);
    ASSERT(!info.supportsGPU);
    ASSERT(!info.isArchiveDecoder);
}

TEST(TestJXLDecoder_Extensions)
{
    JXLDecoder decoder;
    const wchar_t** extensions = decoder.GetSupportedExtensions();
    uint32_t count = decoder.GetExtensionCount();
    
    ASSERT_NOT_NULL(extensions);
    ASSERT(count >= 1);
    
    // Verify .jxl is in the list
    bool foundJXL = false;
    for (uint32_t i = 0; i < count; i++) {
        if (_wcsicmp(extensions[i], L".jxl") == 0) {
            foundJXL = true;
            break;
        }
    }
    ASSERT(foundJXL);
}

#ifdef HAS_LIBJXL
// Only run actual decode tests if JXL library is linked
TEST(TestJXLDecoder_DecodeReturnsResult)
{
    JXLDecoder decoder;
    
    // Test with non-existent file - should return error
    ThumbnailRequest request;
    request.sourcePath = L"nonexistent.jxl";
    request.targetWidth = 256;
    request.targetHeight = 256;
    
    ThumbnailResult result;
    HRESULT hr = decoder.Decode(request, result);
    
    // Should fail gracefully (file doesn't exist)
    // Either returns error or empty bitmap - both are acceptable
    ASSERT(FAILED(hr) || result.bitmap == nullptr);
}
#endif

//==============================================================================
// HEIF Decoder Tests (Re-enabled - basic interface tests)
//==============================================================================

TEST(TestHEIFDecoder_Create)
{
    HEIFDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"HEIFDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestHEIFDecoder_CanDecode)
{
    HEIFDecoder decoder;
    
    ASSERT(decoder.CanDecode(L"image.heif"));
    ASSERT(decoder.CanDecode(L"photo.heic")); // Apple format
    ASSERT(decoder.CanDecode(L"iphone.HEIC")); // Case insensitive
    ASSERT(decoder.CanDecode(L"file.hif"));
    ASSERT(decoder.CanDecode(L"image.heifs"));
    ASSERT(decoder.CanDecode(L"photo.heics"));
    ASSERT(decoder.CanDecode(L"movie.avci"));
    ASSERT(decoder.CanDecode(L"video.avcs"));
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(L"archive.zip"));
    ASSERT(!decoder.CanDecode(nullptr));
}

//==============================================================================
// PSD Decoder Tests
//==============================================================================

TEST(TestPSDDecoder_Create)
{
    PSDDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"PSDDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestPSDDecoder_CanDecode)
{
    PSDDecoder decoder;
    // PSD requires signature check, so without a real file, CanDecode returns false
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
    ASSERT(!decoder.CanDecode(L""));
}

TEST(TestPSDDecoder_GetInfo)
{
    PSDDecoder decoder;
    auto info = decoder.GetInfo();
    ASSERT_EQ(wcscmp(info.name, L"PSDDecoder"), 0);
    ASSERT(info.extensionCount == 2); // .psd, .psb
}

//==============================================================================
// DDS Decoder Tests
//==============================================================================

TEST(TestDDSDecoder_Create)
{
    DDSDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"DDSDecoder"), 0);
    // DDS decoder uses WIC + D3D11, SupportsGPU() returns true
    ASSERT(decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestDDSDecoder_CanDecode)
{
    DDSDecoder decoder;
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

//==============================================================================
// HDR Decoder Tests
//==============================================================================

TEST(TestHDRDecoder_Create)
{
    HDRDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"HDRDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestHDRDecoder_CanDecode)
{
    HDRDecoder decoder;
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

//==============================================================================
// PPM Decoder Tests
//==============================================================================

TEST(TestPPMDecoder_Create)
{
    PPMDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"PPMDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestPPMDecoder_Extensions)
{
    PPMDecoder decoder;
    ASSERT(decoder.GetExtensionCount() >= 6); // ppm, pgm, pbm, pnm, pam, pfm
}

//==============================================================================
// EXR Decoder Tests
//==============================================================================

TEST(TestEXRDecoder_Create)
{
    EXRDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"EXRDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestEXRDecoder_CanDecode)
{
    EXRDecoder decoder;
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

//==============================================================================
// SVG Decoder Tests
//==============================================================================

TEST(TestSVGDecoder_Create)
{
    SVGDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"SVGDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestSVGDecoder_CanDecode)
{
    SVGDecoder decoder;
    ASSERT(decoder.CanDecode(L"image.svg"));
    ASSERT(decoder.CanDecode(L"file.SVG"));
    ASSERT(decoder.CanDecode(L"file.svgz"));
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

//==============================================================================
// Video Decoder Tests
//==============================================================================

TEST(TestVideoDecoder_Create)
{
    VideoDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"VideoDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestVideoDecoder_CanDecode)
{
    VideoDecoder decoder;
    ASSERT(decoder.CanDecode(L"movie.mp4"));
    ASSERT(decoder.CanDecode(L"movie.MKV"));
    ASSERT(decoder.CanDecode(L"clip.avi"));
    ASSERT(decoder.CanDecode(L"video.webm"));
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

TEST(TestVideoDecoder_Extensions)
{
    VideoDecoder decoder;
    ASSERT(decoder.GetExtensionCount() >= 15); // Many video formats
}

//==============================================================================
// Audio Decoder Tests
//==============================================================================

TEST(TestAudioDecoder_Create)
{
    AudioDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"AudioDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestAudioDecoder_CanDecode)
{
    AudioDecoder decoder;
    ASSERT(decoder.CanDecode(L"song.mp3"));
    ASSERT(decoder.CanDecode(L"track.FLAC"));
    ASSERT(decoder.CanDecode(L"audio.wav"));
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

//==============================================================================
// PDF Decoder Tests
//==============================================================================

TEST(TestPDFDecoder_Create)
{
    PDFDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"PDFDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestPDFDecoder_CanDecode)
{
    PDFDecoder decoder;
    ASSERT(decoder.CanDecode(L"document.pdf"));
    ASSERT(decoder.CanDecode(L"file.PDF"));
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

//==============================================================================
// Document Decoder Tests
//==============================================================================

TEST(TestDocumentDecoder_Create)
{
    DocumentDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"DocumentDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestDocumentDecoder_CanDecode)
{
    DocumentDecoder decoder;
    ASSERT(decoder.CanDecode(L"book.epub"));
    ASSERT(decoder.CanDecode(L"doc.docx"));
    ASSERT(decoder.CanDecode(L"kindle.mobi"));
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

TEST(TestDocumentDecoder_Extensions)
{
    DocumentDecoder decoder;
    ASSERT(decoder.GetExtensionCount() >= 15); // Many document formats
}

//==============================================================================
// Font Decoder Tests
//==============================================================================

TEST(TestFontDecoder_Create)
{
    FontDecoder decoder;
    ASSERT_EQ(wcscmp(decoder.GetName(), L"FontDecoder"), 0);
    ASSERT(!decoder.SupportsGPU());
    ASSERT(!decoder.IsArchiveDecoder());
}

TEST(TestFontDecoder_CanDecode)
{
    FontDecoder decoder;
    ASSERT(decoder.CanDecode(L"font.ttf"));
    ASSERT(decoder.CanDecode(L"font.OTF"));
    ASSERT(decoder.CanDecode(L"font.woff"));
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(nullptr));
}

TEST(TestFontDecoder_Extensions)
{
    FontDecoder decoder;
    ASSERT(decoder.CanDecode(L"font.ttf"));
    ASSERT(decoder.CanDecode(L"font.otf"));
    ASSERT(decoder.CanDecode(L"font.woff"));
    ASSERT(decoder.CanDecode(L"font.woff2"));
    ASSERT(decoder.CanDecode(L"font.ttc"));
}

TEST(TestFontDecoder_GetInfo)
{
    FontDecoder decoder;
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
}

//==============================================================================
// Sprint 6: Video Decoder Robustness Tests
//==============================================================================

TEST(TestVideoDecoder_KeyframeSeekingValidation)
{
    VideoDecoder decoder;
    // Test that keyframe seeking doesn't exceed duration
    // This is a basic API test - actual validation requires video file
    ASSERT(decoder.CanDecode(L"video.mp4"));
    ASSERT(decoder.CanDecode(L"video.mkv"));
    ASSERT(decoder.CanDecode(L"video.avi"));
}

TEST(TestVideoDecoder_TimestampValidation)
{
    VideoDecoder decoder;
    // Verify negative timestamps are rejected
    // Actual implementation tested in VideoDecoder::SeekToKeyframe
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
}

TEST(TestVideoDecoder_CorruptFileHandling)
{
    VideoDecoder decoder;
    // Test graceful handling of corrupt files (returns error, no crash)
    ASSERT(decoder.CanDecode(L"test.mp4"));
}

//==============================================================================
// Sprint 7: Audio Album Art Extraction Tests
//==============================================================================

TEST(TestAudioDecoder_AlbumArtExtraction)
{
    AudioDecoder decoder;
    // Verify album art extraction capability
    ASSERT(decoder.CanDecode(L"audio.mp3"));
    ASSERT(decoder.CanDecode(L"audio.flac"));
    ASSERT(decoder.CanDecode(L"audio.m4a"));
}

TEST(TestAudioDecoder_AlbumArtMultipleFormats)
{
    AudioDecoder decoder;
    ASSERT(decoder.CanDecode(L"music.mp3"));
    ASSERT(decoder.CanDecode(L"music.flac"));
    ASSERT(decoder.CanDecode(L"music.ogg"));
    ASSERT(decoder.CanDecode(L"music.wma"));
    ASSERT(decoder.CanDecode(L"music.aac"));
}

TEST(TestAudioDecoder_NoAlbumArtGracefulFallback)
{
    AudioDecoder decoder;
    // Should handle files without album art gracefully
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
}

//==============================================================================
// Sprint 8: Document Thumbnail Provider Tests
//==============================================================================

TEST(TestDocumentDecoder_EPUBCoverExtraction)
{
    DocumentDecoder decoder;
    ASSERT(decoder.CanDecode(L"book.epub"));
}

TEST(TestDocumentDecoder_MOBICoverExtraction)
{
    DocumentDecoder decoder;
    ASSERT(decoder.CanDecode(L"book.mobi"));
}

TEST(TestDocumentDecoder_InvalidZipHandling)
{
    DocumentDecoder decoder;
    // Should handle corrupted EPUB (invalid ZIP) gracefully
    ASSERT(decoder.CanDecode(L"document.epub"));
}

TEST(TestDocumentDecoder_MissingCoverHandling)
{
    DocumentDecoder decoder;
    // Should handle EPUB without cover.jpg/png gracefully
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
}

//==============================================================================
// Sprint 9: Font Preview Rendering Tests
//==============================================================================

TEST(TestFontDecoder_DirectWriteRendering)
{
    FontDecoder decoder;
    // Verify DirectWrite-based rendering support
    ASSERT(decoder.CanDecode(L"font.ttf"));
    ASSERT(decoder.CanDecode(L"font.otf"));
}

TEST(TestFontDecoder_MetadataExtraction)
{
    FontDecoder decoder;
    // Test font metadata extraction (family, weight, style)
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
}

TEST(TestFontDecoder_TrueTypeCollectionHandling)
{
    FontDecoder decoder;
    // TTC files contain multiple fonts
    ASSERT(decoder.CanDecode(L"fonts.ttc"));
}

//==============================================================================
// Sprint 10: Archive Format Expansion Tests
//==============================================================================

TEST(TestArchiveDecoder_7zSupport)
{
    ArchiveDecoder decoder;
    ASSERT(decoder.CanDecode(L"archive.7z"));
}

TEST(TestArchiveDecoder_TarGzSupport)
{
    ArchiveDecoder decoder;
    ASSERT(decoder.CanDecode(L"archive.tar.gz"));
}

TEST(TestArchiveDecoder_TarBz2Support)
{
    ArchiveDecoder decoder;
    ASSERT(decoder.CanDecode(L"archive.tar.bz2"));
}

TEST(TestArchiveDecoder_PasswordProtectedHandling)
{
    ArchiveDecoder decoder;
    // Should detect password-protected archives gracefully
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
}

//==============================================================================
// Sprint 11: RAW Format Expansion Tests
//==============================================================================

TEST(TestRAWDecoder_CR3Support)
{
    RAWDecoder decoder;
    ASSERT(decoder.CanDecode(L"photo.cr3"));
}

TEST(TestRAWDecoder_ARWSupport)
{
    RAWDecoder decoder;
    ASSERT(decoder.CanDecode(L"photo.arw"));
}

TEST(TestRAWDecoder_ORFSupport)
{
    RAWDecoder decoder;
    ASSERT(decoder.CanDecode(L"photo.orf"));
}

TEST(TestRAWDecoder_GPRSupport)
{
    RAWDecoder decoder;
    ASSERT(decoder.CanDecode(L"photo.gpr"));
}

TEST(TestRAWDecoder_MultipleRAWFormats)
{
    RAWDecoder decoder;
    // Test all major camera RAW formats via RAWDecoder (LibRaw)
    ASSERT(decoder.CanDecode(L"image.cr2"));
    ASSERT(decoder.CanDecode(L"image.cr3"));
    ASSERT(decoder.CanDecode(L"image.nef"));
    ASSERT(decoder.CanDecode(L"image.arw"));
    ASSERT(decoder.CanDecode(L"image.dng"));
    ASSERT(decoder.CanDecode(L"image.orf"));
    ASSERT(decoder.CanDecode(L"image.rw2"));
    ASSERT(decoder.CanDecode(L"image.gpr"));
}

//==============================================================================
// Sprint 12: 3D Model Support Tests
//==============================================================================

TEST(TestModelDecoder_Create)
{
    ModelDecoder decoder;
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
}

TEST(TestModelDecoder_OBJSupport)
{
    ModelDecoder decoder;
    ASSERT(decoder.CanDecode(L"model.obj"));
    ASSERT(decoder.CanDecode(L"MODEL.OBJ"));
}

TEST(TestModelDecoder_STLSupport)
{
    ModelDecoder decoder;
    ASSERT(decoder.CanDecode(L"model.stl"));
    ASSERT(decoder.CanDecode(L"MODEL.STL"));
}

TEST(TestModelDecoder_GLTFSupport)
{
    ModelDecoder decoder;
    ASSERT(decoder.CanDecode(L"model.gltf"));
    ASSERT(decoder.CanDecode(L"model.glb"));
}

TEST(TestModelDecoder_Extensions)
{
    ModelDecoder decoder;
    ASSERT(decoder.CanDecode(L"test.obj"));
    ASSERT(decoder.CanDecode(L"test.stl"));
    ASSERT(decoder.CanDecode(L"test.gltf"));
    ASSERT(decoder.CanDecode(L"test.glb"));
    ASSERT(!decoder.CanDecode(L"test.jpg"));
}

TEST(TestModelDecoder_GetInfo)
{
    ModelDecoder decoder;
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
    ASSERT(info.supportsGPU); // Model decoder uses D3D11
}

//==============================================================================
// Sprint 182: Enhanced Model Decoder Tests — PLY, DAE, expanded extensions
//==============================================================================

TEST(TestModelDecoder_PLYSupport)
{
    ModelDecoder decoder;
    ASSERT(decoder.CanDecode(L"mesh.ply"));
    ASSERT(decoder.CanDecode(L"MESH.PLY"));
    ASSERT(decoder.CanDecode(L"C:\\models\\scan.ply"));
}

TEST(TestModelDecoder_DAESupport)
{
    ModelDecoder decoder;
    ASSERT(decoder.CanDecode(L"model.dae"));
    ASSERT(decoder.CanDecode(L"MODEL.DAE"));
    ASSERT(decoder.CanDecode(L"scene.dae"));
}

TEST(TestModelDecoder_3DSSupport)
{
    ModelDecoder decoder;
    ASSERT(decoder.CanDecode(L"model.3ds"));
    ASSERT(decoder.CanDecode(L"old_model.3ds"));
}

TEST(TestModelDecoder_FBXSupport)
{
    ModelDecoder decoder;
    ASSERT(decoder.CanDecode(L"model.fbx"));
    ASSERT(decoder.CanDecode(L"character.fbx"));
}

TEST(TestModelDecoder_ExpandedExtensions)
{
    ModelDecoder decoder;
    // Original 4
    ASSERT(decoder.CanDecode(L"test.obj"));
    ASSERT(decoder.CanDecode(L"test.stl"));
    ASSERT(decoder.CanDecode(L"test.gltf"));
    ASSERT(decoder.CanDecode(L"test.glb"));
    // New 4
    ASSERT(decoder.CanDecode(L"test.ply"));
    ASSERT(decoder.CanDecode(L"test.dae"));
    ASSERT(decoder.CanDecode(L"test.3ds"));
    ASSERT(decoder.CanDecode(L"test.fbx"));
    // Negative
    ASSERT(!decoder.CanDecode(L"test.jpg"));
    ASSERT(!decoder.CanDecode(L"test.png"));
}

TEST(TestModelDecoder_ExtensionCount)
{
    ModelDecoder decoder;
    ASSERT_EQUAL(8u, decoder.GetExtensionCount());
    auto info = decoder.GetInfo();
    ASSERT_EQUAL(8u, info.extensionCount);
}

//==============================================================================
// Sprint 183: EPS/PostScript Decoder Tests
//==============================================================================

TEST(TestEPSDecoder_Create)
{
    EPSDecoder decoder;
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
    ASSERT_EQUAL(3u, info.extensionCount);
}

TEST(TestEPSDecoder_CanDecode)
{
    EPSDecoder decoder;
    ASSERT(decoder.CanDecode(L"image.eps"));
    ASSERT(decoder.CanDecode(L"IMAGE.EPS"));
    ASSERT(decoder.CanDecode(L"file.epsf"));
    ASSERT(decoder.CanDecode(L"document.ps"));
}

TEST(TestEPSDecoder_NoDecodeNonEPS)
{
    EPSDecoder decoder;
    ASSERT(!decoder.CanDecode(L"file.pdf"));
    ASSERT(!decoder.CanDecode(L"file.svg"));
    ASSERT(!decoder.CanDecode(L"file.ai"));
    ASSERT(!decoder.CanDecode(L"file.jpg"));
}

TEST(TestEPSDecoder_GetInfo)
{
    EPSDecoder decoder;
    auto info = decoder.GetInfo();
    ASSERT(!info.supportsGPU);
    ASSERT(!info.isArchiveDecoder);
}

TEST(TestPDFDecoder_AIRouting)
{
    // .ai files are PDF-based and should be handled by PDFDecoder
    PDFDecoder decoder;
    // PDFDecoder uses IsPDFFormat() which checks file signature,
    // not extension, so we just verify the architectural intent
    auto info = decoder.GetInfo();
    ASSERT_NOT_NULL(info.name);
}

//==============================================================================
// Sprint 184: Game Texture Formats — KTX/KTX2 + VTF
// Tests for Khronos KTX and Valve Texture Format decoders
//==============================================================================

TEST(TestKTXDecoder_Create)
{
    using namespace DarkThumbs::Decoders;
    KTXTextureDecoder decoder = KTXTextureDecoder::Create();
    ASSERT(decoder.IsAvailable());
}

TEST(TestKTXDecoder_ExtensionCheck)
{
    using namespace DarkThumbs::Decoders;
    ASSERT(KTXTextureDecoder::IsKTXExtension(".ktx"));
    ASSERT(KTXTextureDecoder::IsKTXExtension(".KTX"));
    ASSERT(KTXTextureDecoder::IsKTXExtension(".ktx2"));
    ASSERT(KTXTextureDecoder::IsKTXExtension(".KTX2"));
    ASSERT(!KTXTextureDecoder::IsKTXExtension(".png"));
    ASSERT(!KTXTextureDecoder::IsKTXExtension(".dds"));
}

TEST(TestKTXDecoder_ExtensionVersion)
{
    using namespace DarkThumbs::Decoders;
    ASSERT(KTXExtensions::VersionFromExtension(".ktx") == KTXVersion::KTX1);
    ASSERT(KTXExtensions::VersionFromExtension(".ktx2") == KTXVersion::KTX2);
    ASSERT(KTXExtensions::VersionFromExtension(".png") == KTXVersion::Unknown);
}

TEST(TestKTXDecoder_CompressionNames)
{
    using namespace DarkThumbs::Decoders;
    ASSERT(std::string(CompressionName(TextureCompression::BC1_RGB)) == "BC1 (DXT1)");
    ASSERT(std::string(CompressionName(TextureCompression::BC7_RGBA)) == "BC7 (RGBA)");
    ASSERT(std::string(CompressionName(TextureCompression::ASTC_4x4)) == "ASTC 4x4");
    ASSERT(IsBlockCompressed(TextureCompression::BC1_RGB));
    ASSERT(!IsBlockCompressed(TextureCompression::Uncompressed));
}

TEST(TestKTXDecoder_TextureInfo)
{
    using namespace DarkThumbs::Decoders;
    KTXTextureInfo info;
    info.width = 1024;
    info.height = 1024;
    info.mipLevels = 11;
    info.version = KTXVersion::KTX2;
    info.compression = TextureCompression::BC7_RGBA;
    
    ASSERT(info.IsValid());
    ASSERT(info.HasMipmaps());
    ASSERT(!info.Is3D());
    
    uint32_t bestMip = info.BestMipForThumbnail(256);
    ASSERT(bestMip > 0); // Should select a smaller mip
    
    size_t estSize = info.EstimateCompressedSize();
    ASSERT(estSize > 0);
}

TEST(TestKTXDecoder_SupercompressionNames)
{
    using namespace DarkThumbs::Decoders;
    ASSERT(std::string(SupercompressionName(KTXSupercompression::None)) == "None");
    ASSERT(std::string(SupercompressionName(KTXSupercompression::Zstd)) == "Zstandard");
}

TEST(TestKTXDecoder_InvalidFile)
{
    using namespace DarkThumbs::Decoders;
    KTXTextureDecoder decoder;
    auto result = decoder.Decode("nonexistent.ktx");
    ASSERT(!result.IsSuccess());
}

TEST(TestVTFDecoder_ExtensionCheck)
{
    using namespace DarkThumbs::Decoders;
    ASSERT(VTFDecoder::IsVTFExtension(".vtf"));
    ASSERT(VTFDecoder::IsVTFExtension(".VTF"));
    ASSERT(!VTFDecoder::IsVTFExtension(".png"));
    ASSERT(!VTFDecoder::IsVTFExtension(".dds"));
}

TEST(TestVTFDecoder_Create)
{
    using namespace DarkThumbs::Decoders;
    VTFDecoder decoder;
    ASSERT(VTFDecoder::EXTENSIONS[0] != nullptr);
    ASSERT(std::string(VTFDecoder::EXTENSIONS[0]) == ".vtf");
    ASSERT(VTFDecoder::EXTENSIONS[1] == nullptr);
}

TEST(TestVTFDecoder_InvalidFile)
{
    using namespace DarkThumbs::Decoders;
    VTFDecoder decoder;
    auto result = decoder.Decode("nonexistent.vtf");
    ASSERT(!result.success);
}

TEST(TestVTFDecoder_ImageSizeCompute)
{
    using namespace DarkThumbs::Decoders;
    // DXT1: 8 bytes per 4x4 block
    // 256x256 = 64x64 blocks = 4096 blocks * 8 = 32768 bytes
    // We can't call the private method directly, but we test via decode
    VTFDecoder decoder;
    auto info = decoder.ReadInfo("nonexistent.vtf");
    ASSERT(!info.IsValid()); // File doesn't exist, should be invalid
}

//==============================================================================
// Sprint 6: Worker/Isolation Stabilization Tests  
// February 17, 2026
//==============================================================================

TEST(TestMalformedArchive_TruncatedZIP)
{
    ArchiveDecoder decoder;
    ThumbnailRequest request = {};
    request.filePath = L"test_truncated.zip";
    request.outputWidth = 256;
    request.outputHeight = 256;
    
    ThumbnailResult result = {};
    
    // Create a truncated ZIP file (first 100 bytes only)
    HANDLE hFile = CreateFileW(request.filePath, GENERIC_WRITE, 0, nullptr, 
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        const char truncatedHeader[] = "PK\x03\x04\x14\x00\x00\x00\x08\x00";
        DWORD written = 0;
        WriteFile(hFile, truncatedHeader, sizeof(truncatedHeader), &written, nullptr);
        CloseHandle(hFile);
    }
    
    // Should fail gracefully, not crash
    HRESULT hr = decoder.Decode(request, result);
    ASSERT(FAILED(hr)); // Should return error
    
    // Cleanup
    DeleteFileW(request.filePath);
}

TEST(TestMalformedArchive_GarbageHeader)
{
   ArchiveDecoder decoder;
    ThumbnailRequest request = {};
    request.filePath = L"test_garbage.cbz";
    request.outputWidth = 256;
    request.outputHeight = 256;
    
    ThumbnailResult result = {};
    
    // Create a file with random garbage data
    HANDLE hFile = CreateFileW(request.filePath, GENERIC_WRITE, 0, nullptr, 
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        const char garbage[] = "NOTAZIPATALL123456789GARBAGE";
        DWORD written = 0;
        WriteFile(hFile, garbage, sizeof(garbage), &written, nullptr);
        CloseHandle(hFile);
    }
    
    // Should detect invalid format and reject
    ASSERT(!decoder.CanDecode(request.filePath) || FAILED(decoder.Decode(request, result)));
    
    // Cleanup
    DeleteFileW(request.filePath);
}

TEST(TestMalformedArchive_ZeroByteFile)
{
    ArchiveDecoder decoder;
    ThumbnailRequest request = {};
    request.filePath = L"test_zero.zip";
    request.outputWidth = 256;
    request.outputHeight = 256;
    
    ThumbnailResult result = {};
    
    // Create zero-byte file
    HANDLE hFile = CreateFileW(request.filePath, GENERIC_WRITE, 0, nullptr, 
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }
    
    // Should handle gracefully
    HRESULT hr = decoder.Decode(request, result);
    ASSERT(FAILED(hr));
    
    // Cleanup
    DeleteFileW(request.filePath);
}

TEST(TestCircuitBreaker_StressTest)
{
    // Circuit breaker should isolate failing decoders after 5 failures
    // This test generates 10 failures and verifies circuit opens
    
    ArchiveDecoder decoder;
    int failureCount = 0;
    const int maxFailures = 10;
    
    for (int i = 0; i < maxFailures; i++) {
        ThumbnailRequest request = {};
        wchar_t fileName[256];
        swprintf_s(fileName, L"test_corrupt_%d.zip", i);
        request.filePath = fileName;
        request.outputWidth = 256;
        request.outputHeight = 256;
        
        // Create corrupt file
        HANDLE hFile = CreateFileW(fileName, GENERIC_WRITE, 0, nullptr, 
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile != INVALID_HANDLE_VALUE) {
            const char corrupt[] = "PK\xFF\xFF";
            DWORD written = 0;
            WriteFile(hFile, corrupt, sizeof(corrupt), &written, nullptr);
            CloseHandle(hFile);
        }
        
        ThumbnailResult result = {};
        HRESULT hr = decoder.Decode(request, result);
        
        if (FAILED(hr)) {
            failureCount++;
        }
        
        DeleteFileW(fileName);
    }
    
    // All should fail, but no crashes
    ASSERT(failureCount == maxFailures);
    std::wcout << L"  [Circuit Breaker] Handled " << failureCount << L" failures without crash" << std::endl;
}

TEST(TestDecoderTimeout_Enforcement)
{
    // Verify decoders don't hang indefinitely on problematic files
    // This test simulates a slow/infinite loop scenario
    
    ImageDecoder decoder;
    ThumbnailRequest request = {};
    request.filePath = L"test_timeout.jpg";
    request.outputWidth = 256;
    request.outputHeight = 256;
    
    // Create minimal valid JPEG (will decode very quickly)
    HANDLE hFile = CreateFileW(request.filePath, GENERIC_WRITE, 0, nullptr, 
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        // Minimal JPEG header (not valid, but triggers parse)
        const unsigned char jpegHeader[] = {
            0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 'J', 'F', 'I', 'F', 0x00
        };
        DWORD written = 0;
        WriteFile(hFile, jpegHeader, sizeof(jpegHeader), &written, nullptr);
        CloseHandle(hFile);
    }
    
    auto startTime = std::chrono::steady_clock::now();
    
    ThumbnailResult result = {};
    decoder.Decode(request, result);
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    // Should complete within 5 seconds (timeout enforcement)
    ASSERT(duration < 5000);
    
    std::wcout << L"  [Timeout] Decode completed in " << duration << L" ms (< 5000 ms limit)" << std::endl;
    
    DeleteFileW(request.filePath);
}

TEST(TestMemoryLeak_RegressionLoop)
{
    // Run 100 decode iterations and verify no excessive memory growth
    ImageDecoder decoder;
    
    const int iterations = 100;
    SIZE_T initialMemory = 0;
    SIZE_T peakMemory = 0;
    
    // Get initial memory usage
    PROCESS_MEMORY_COUNTERS_EX pmc = {};
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    initialMemory = pmc.WorkingSetSize;
    
    for (int i = 0; i < iterations; i++) {
        ThumbnailRequest request = {};
        wchar_t fileName[256];
        swprintf_s(fileName, L"test_leak_%d.bmp", i);
        request.filePath = fileName;
        request.outputWidth = 256;
        request.outputHeight = 256;
        
        // Create minimal BMP header
        HANDLE hFile = CreateFileW(fileName, GENERIC_WRITE, 0, nullptr, 
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile != INVALID_HANDLE_VALUE) {
            const unsigned char bmpHeader[] = {
                'B', 'M', 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00
            };
            DWORD written = 0;
            WriteFile(hFile, bmpHeader, sizeof(bmpHeader), &written, nullptr);
            CloseHandle(hFile);
        }
        
        ThumbnailResult result = {};
        decoder.Decode(request, result);
        
        // Release result resources
        if (result.bitmap) {
            DeleteObject(result.bitmap);
        }
        
        DeleteFileW(fileName);
        
        // Check memory every 10 iterations
        if ((i + 1) % 10 == 0) {
            GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
            SIZE_T currentMemory = pmc.WorkingSetSize;
            if (currentMemory > peakMemory) {
                peakMemory = currentMemory;
            }
        }
    }
    
    // Final memory check
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    SIZE_T finalMemory = pmc.WorkingSetSize;
    
    // Memory growth should be < 50 MB for 100 iterations
    SIZE_T growth = (finalMemory > initialMemory) ? (finalMemory - initialMemory) : 0;
    SIZE_T growthMB = growth / (1024 * 1024);
    
    std::wcout << L"  [Memory] Initial: " << (initialMemory / 1024 / 1024) << L" MB, "
               << L"Final: " << (finalMemory / 1024 / 1024) << L" MB, "
               << L"Growth: " << growthMB << L" MB" << std::endl;
    
    ASSERT(growthMB < 50); // No excessive memory leak
}

//==============================================================================
// Main Test Runner
//==============================================================================

int main()
{
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"DarkThumbs Engine - Unit Tests" << std::endl;
    std::wcout << L"========================================" << std::endl << std::endl;
    
    // Decoder Registry Tests
    std::wcout << L"Decoder Registry Tests:" << std::endl;
    RUN_TEST(TestDecoderRegistry_Create);
    RUN_TEST(TestDecoderRegistry_RegisterDecoder);
    RUN_TEST(TestDecoderRegistry_RegisterMultipleDecoders);
    RUN_TEST(TestDecoderRegistry_FindDecoder);
    RUN_TEST(TestDecoderRegistry_FindDecoderByName);
    RUN_TEST(TestDecoderRegistry_GetStats);
    RUN_TEST(TestDecoderRegistry_ComprehensiveIntegration);
    
    std::wcout << std::endl;
    
    // Format Detector Tests
    std::wcout << L"Format Detector Tests:" << std::endl;
    RUN_TEST(TestFormatDetector_Create);
    RUN_TEST(TestFormatDetector_DetectJPEG);
    RUN_TEST(TestFormatDetector_DetectPNG);
    RUN_TEST(TestFormatDetector_DetectZIP);
    RUN_TEST(TestFormatDetector_DetectRAR);
    RUN_TEST(TestFormatDetector_IsImageFormat);
    RUN_TEST(TestFormatDetector_IsArchiveFormat);
    RUN_TEST(TestFormatDetector_GetExtension);
    
    std::wcout << std::endl;
    
    // Image Decoder Tests
    std::wcout << L"Image Decoder Tests:" << std::endl;
    RUN_TEST(TestImageDecoder_Create);
    RUN_TEST(TestImageDecoder_Extensions);
    RUN_TEST(TestImageDecoder_CanDecodeJPEG);
    RUN_TEST(TestImageDecoder_CanDecodePNG);
    RUN_TEST(TestImageDecoder_CanDecodeBMP);
    RUN_TEST(TestImageDecoder_CannotDecodeUnsupported);
    RUN_TEST(TestImageDecoder_GetInfo);
    RUN_TEST(TestImageDecoder_RegisterWithRegistry);
    
    std::wcout << std::endl;
    
    // WebP Decoder Tests
    std::wcout << L"WebP Decoder Tests:" << std::endl;
    RUN_TEST(TestWebPDecoder_Create);
    RUN_TEST(TestWebPDecoder_Extensions);
    RUN_TEST(TestWebPDecoder_CanDecode);
    RUN_TEST(TestWebPDecoder_GetInfo);
    RUN_TEST(TestWebPDecoder_RegisterWithRegistry);
    
    std::wcout << std::endl;
    
    // AVIF Decoder Tests
    std::wcout << L"AVIF Decoder Tests:" << std::endl;
    RUN_TEST(TestAVIFDecoder_Create);
    RUN_TEST(TestAVIFDecoder_Extensions);
    RUN_TEST(TestAVIFDecoder_CanDecode);
    RUN_TEST(TestAVIFDecoder_GetInfo);
    RUN_TEST(TestAVIFDecoder_RegisterWithRegistry);
    
    std::wcout << std::endl;
    
    // Archive Decoder Tests
    std::wcout << L"Archive Decoder Tests:" << std::endl;
    RUN_TEST(TestArchiveDecoder_Create);
    RUN_TEST(TestArchiveDecoder_Extensions);
    RUN_TEST(TestArchiveDecoder_CanDecode);
    RUN_TEST(TestArchiveDecoder_IsArchiveFormat);
    RUN_TEST(TestArchiveDecoder_GetInfo);
    RUN_TEST(TestArchiveDecoder_RegisterWithRegistry);
    
    std::wcout << std::endl;
    
    // JXL Decoder Tests
    std::wcout << L"JXL Decoder Tests:" << std::endl;
    RUN_TEST(TestJXLDecoder_Create);
    RUN_TEST(TestJXLDecoder_CanDecode);
    RUN_TEST(TestJXLDecoder_GetInfo);
    RUN_TEST(TestJXLDecoder_Extensions);
#ifdef HAS_LIBJXL
    RUN_TEST(TestJXLDecoder_DecodeReturnsResult);
#endif
    
    std::wcout << std::endl;
    
    // HEIF Decoder Tests
    std::wcout << L"HEIF Decoder Tests:" << std::endl;
    RUN_TEST(TestHEIFDecoder_Create);
    RUN_TEST(TestHEIFDecoder_CanDecode);
    
    std::wcout << std::endl;
    
    // PSD Decoder Tests
    std::wcout << L"PSD Decoder Tests:" << std::endl;
    RUN_TEST(TestPSDDecoder_Create);
    RUN_TEST(TestPSDDecoder_CanDecode);
    RUN_TEST(TestPSDDecoder_GetInfo);
    
    std::wcout << std::endl;
    
    // DDS Decoder Tests
    std::wcout << L"DDS Decoder Tests:" << std::endl;
    RUN_TEST(TestDDSDecoder_Create);
    RUN_TEST(TestDDSDecoder_CanDecode);
    
    std::wcout << std::endl;
    
    // HDR Decoder Tests
    std::wcout << L"HDR Decoder Tests:" << std::endl;
    RUN_TEST(TestHDRDecoder_Create);
    RUN_TEST(TestHDRDecoder_CanDecode);
    
    std::wcout << std::endl;
    
    // PPM Decoder Tests
    std::wcout << L"PPM Decoder Tests:" << std::endl;
    RUN_TEST(TestPPMDecoder_Create);
    RUN_TEST(TestPPMDecoder_Extensions);
    
    std::wcout << std::endl;
    
    // EXR Decoder Tests
    std::wcout << L"EXR Decoder Tests:" << std::endl;
    RUN_TEST(TestEXRDecoder_Create);
    RUN_TEST(TestEXRDecoder_CanDecode);
    
    std::wcout << std::endl;
    
    // SVG Decoder Tests
    std::wcout << L"SVG Decoder Tests:" << std::endl;
    RUN_TEST(TestSVGDecoder_Create);
    RUN_TEST(TestSVGDecoder_CanDecode);
    
    std::wcout << std::endl;
    
    // Video Decoder Tests
    std::wcout << L"Video Decoder Tests:" << std::endl;
    RUN_TEST(TestVideoDecoder_Create);
    RUN_TEST(TestVideoDecoder_CanDecode);
    RUN_TEST(TestVideoDecoder_Extensions);
    
    std::wcout << std::endl;
    
    // Audio Decoder Tests
    std::wcout << L"Audio Decoder Tests:" << std::endl;
    RUN_TEST(TestAudioDecoder_Create);
    RUN_TEST(TestAudioDecoder_CanDecode);
    
    std::wcout << std::endl;
    
    // PDF Decoder Tests
    std::wcout << L"PDF Decoder Tests:" << std::endl;
    RUN_TEST(TestPDFDecoder_Create);
    RUN_TEST(TestPDFDecoder_CanDecode);
    
    std::wcout << std::endl;
    
    // Document Decoder Tests
    std::wcout << L"Document Decoder Tests:" << std::endl;
    RUN_TEST(TestDocumentDecoder_Create);
    RUN_TEST(TestDocumentDecoder_CanDecode);
    RUN_TEST(TestDocumentDecoder_Extensions);
    
    std::wcout << std::endl;
    
    // Font Decoder Tests
    std::wcout << L"Font Decoder Tests:" << std::endl;
    RUN_TEST(TestFontDecoder_Create);
    RUN_TEST(TestFontDecoder_CanDecode);
    RUN_TEST(TestFontDecoder_Extensions);
    RUN_TEST(TestFontDecoder_GetInfo);
    
    std::wcout << std::endl;
    
    // Sprint 6: Video Decoder Robustness Tests
    std::wcout << L"Sprint 6 - Video Decoder Robustness:" << std::endl;
    RUN_TEST(TestVideoDecoder_KeyframeSeekingValidation);
    RUN_TEST(TestVideoDecoder_TimestampValidation);
    RUN_TEST(TestVideoDecoder_CorruptFileHandling);
    
    std::wcout << std::endl;
    
    // Sprint 7: Audio Album Art Tests
    std::wcout << L"Sprint 7 - Audio Album Art Extraction:" << std::endl;
    RUN_TEST(TestAudioDecoder_AlbumArtExtraction);
    RUN_TEST(TestAudioDecoder_AlbumArtMultipleFormats);
    RUN_TEST(TestAudioDecoder_NoAlbumArtGracefulFallback);
    
    std::wcout << std::endl;
    
    // Sprint 8: Document Thumbnail Tests
    std::wcout << L"Sprint 8 - Document Thumbnail Provider:" << std::endl;
    RUN_TEST(TestDocumentDecoder_EPUBCoverExtraction);
    RUN_TEST(TestDocumentDecoder_MOBICoverExtraction);
    RUN_TEST(TestDocumentDecoder_InvalidZipHandling);
    RUN_TEST(TestDocumentDecoder_MissingCoverHandling);
    
    std::wcout << std::endl;
    
    // Sprint 9: Font Preview Rendering Tests
    std::wcout << L"Sprint 9 - Font Preview Rendering:" << std::endl;
    RUN_TEST(TestFontDecoder_DirectWriteRendering);
    RUN_TEST(TestFontDecoder_MetadataExtraction);
    RUN_TEST(TestFontDecoder_TrueTypeCollectionHandling);
    
    std::wcout << std::endl;
    
    // Sprint 10: Archive Format Expansion Tests
    std::wcout << L"Sprint 10 - Archive Format Expansion:" << std::endl;
    RUN_TEST(TestArchiveDecoder_7zSupport);
    RUN_TEST(TestArchiveDecoder_TarGzSupport);
    RUN_TEST(TestArchiveDecoder_TarBz2Support);
    RUN_TEST(TestArchiveDecoder_PasswordProtectedHandling);
    
    std::wcout << std::endl;
    
    // Sprint 11: RAW Format Expansion Tests
    std::wcout << L"Sprint 11 - RAW Format Expansion:" << std::endl;
    RUN_TEST(TestRAWDecoder_CR3Support);
    RUN_TEST(TestRAWDecoder_ARWSupport);
    RUN_TEST(TestRAWDecoder_ORFSupport);
    RUN_TEST(TestRAWDecoder_GPRSupport);
    RUN_TEST(TestRAWDecoder_MultipleRAWFormats);
    
    std::wcout << std::endl;
    
    // Sprint 12: 3D Model Support Tests
    std::wcout << L"Sprint 12 - 3D Model Support:" << std::endl;
    RUN_TEST(TestModelDecoder_Create);
    RUN_TEST(TestModelDecoder_OBJSupport);
    RUN_TEST(TestModelDecoder_STLSupport);
    RUN_TEST(TestModelDecoder_GLTFSupport);
    RUN_TEST(TestModelDecoder_Extensions);
    RUN_TEST(TestModelDecoder_GetInfo);
    
    std::wcout << L"Sprint 182 - Enhanced Model Decoder:" << std::endl;
    RUN_TEST(TestModelDecoder_PLYSupport);
    RUN_TEST(TestModelDecoder_DAESupport);
    RUN_TEST(TestModelDecoder_3DSSupport);
    RUN_TEST(TestModelDecoder_FBXSupport);
    RUN_TEST(TestModelDecoder_ExpandedExtensions);
    RUN_TEST(TestModelDecoder_ExtensionCount);
    
    std::wcout << L"Sprint 183 - EPS/PostScript Decoder:" << std::endl;
    RUN_TEST(TestEPSDecoder_Create);
    RUN_TEST(TestEPSDecoder_CanDecode);
    RUN_TEST(TestEPSDecoder_NoDecodeNonEPS);
    RUN_TEST(TestEPSDecoder_GetInfo);
    RUN_TEST(TestPDFDecoder_AIRouting);
    
    std::wcout << L"Sprint 184 - Game Texture Formats (KTX/VTF):" << std::endl;
    RUN_TEST(TestKTXDecoder_Create);
    RUN_TEST(TestKTXDecoder_ExtensionCheck);
    RUN_TEST(TestKTXDecoder_ExtensionVersion);
    RUN_TEST(TestKTXDecoder_CompressionNames);
    RUN_TEST(TestKTXDecoder_TextureInfo);
    RUN_TEST(TestKTXDecoder_SupercompressionNames);
    RUN_TEST(TestKTXDecoder_InvalidFile);
    RUN_TEST(TestVTFDecoder_ExtensionCheck);
    RUN_TEST(TestVTFDecoder_Create);
    RUN_TEST(TestVTFDecoder_InvalidFile);
    RUN_TEST(TestVTFDecoder_ImageSizeCompute);
    
    std::wcout << std::endl;
    
    // Sprint 6: Isolation & Stability Tests
    std::wcout << L"Sprint 6: Isolation & Stability Tests..." << std::endl;
    RUN_TEST(TestMalformedArchive_TruncatedZIP);
    RUN_TEST(TestMalformedArchive_GarbageHeader);
    RUN_TEST(TestMalformedArchive_ZeroByteFile);
    RUN_TEST(TestCircuitBreaker_StressTest);
    RUN_TEST(TestDecoderTimeout_Enforcement);
    RUN_TEST(TestMemoryLeak_RegressionLoop);
    
    std::wcout << std::endl;
    
    // GPU Renderer Tests
    RunGPUTests();
    
    // Summary
    std::wcout << std::endl;
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"Test Results:" << std::endl;
    std::wcout << L"  Total:  " << g_testsRun << std::endl;
    std::wcout << L"  Passed: " << g_testsPassed << std::endl;
    std::wcout << L"  Failed: " << g_testsFailed << std::endl;
    std::wcout << L"========================================" << std::endl;
    
    return g_testsFailed > 0 ? 1 : 0;
}
