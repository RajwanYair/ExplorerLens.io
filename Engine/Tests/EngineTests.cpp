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
#include <iostream>
#include <cassert>

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
        std::wcout << L"Running: " << L#name << L"..." << std::endl; \
        g_testsRun++; \
        try { \
            name(); \
            g_testsPassed++; \
            std::wcout << L"  [PASS]" << std::endl; \
        } catch (const char* msg) { \
            g_testsFailed++; \
            std::wcout << L"  [FAIL] " << msg << std::endl; \
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
    ASSERT_EQ(count, 2); // .zip, .cbz
    
    // Check specific extensions
    bool hasZip = false;
    bool hasCbz = false;
    
    for (uint32_t i = 0; i < count; i++) {
        if (wcscmp(extensions[i], L".zip") == 0) hasZip = true;
        if (wcscmp(extensions[i], L".cbz") == 0) hasCbz = true;
    }
    
    ASSERT(hasZip);
    ASSERT(hasCbz);
}

TEST(TestArchiveDecoder_CanDecode) {
    ArchiveDecoder decoder;
    
    ASSERT(decoder.CanDecode(L"archive.zip"));
    ASSERT(decoder.CanDecode(L"comic.cbz"));
    ASSERT(decoder.CanDecode(L"path/to/file.ZIP")); // Case insensitive
    
    // Should not decode non-archive formats
    ASSERT(!decoder.CanDecode(L"image.jpg"));
    ASSERT(!decoder.CanDecode(L"photo.png"));
    ASSERT(!decoder.CanDecode(L"archive.rar")); // Not supported yet
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
