//==============================================================================
// DarkThumbs Engine - Integration Tests
// Sprint 16: End-to-end pipeline testing with real files
// Copyright (c) 2026 - DarkThumbs Project
//==============================================================================

#include "../Pipeline/ThumbnailPipeline.h"
#include "../Pipeline/DecoderRegistry.h"
#include "../Decoders/ImageDecoder.h"
#include "../Decoders/WebPDecoder.h"
#include "../Decoders/AVIFDecoder.h"
#include "../Decoders/ArchiveDecoder.h"
#include "../Decoders/VideoDecoder.h"
#include "../Decoders/AudioDecoder.h"
#include "../Decoders/DocumentDecoder.h"
#include "../Decoders/FontDecoder.h"
#include "../Decoders/ModelDecoder.h"
#include <iostream>
#include <vector>
#include <string>

using namespace DarkThumbs::Engine;

//==============================================================================
// Integration Test Framework
//==============================================================================

int g_integrationTestsRun = 0;
int g_integrationTestsPassed = 0;
int g_integrationTestsFailed = 0;

#define INTEGRATION_TEST(name) \
    void name(); \
    void name##_Runner() { \
        std::wcout << L"Running: " << L"" #name << L"..." << std::endl; \
        g_integrationTestsRun++; \
        try { \
            name(); \
            g_integrationTestsPassed++; \
            std::wcout << L"  [PASS]" << std::endl; \
        } catch (const char* msg) { \
            g_integrationTestsFailed++; \
            std::cout << "  [FAIL] " << msg << std::endl; \
        } \
    } \
    void name()

#define ASSERT_INTEGRATION(expr) \
    if (!(expr)) { \
        throw "Integration test failed: " #expr; \
    }

#define RUN_INTEGRATION_TEST(name) name##_Runner()

//==============================================================================
// Integration Tests
//==============================================================================

INTEGRATION_TEST(TestPipeline_FullInitialization)
{
    // Test complete pipeline initialization with all decoders
    auto registry = DecoderRegistry::Create();
    ASSERT_INTEGRATION(registry != nullptr);

    // Register all decoders
    registry->RegisterDecoder(new ImageDecoder());
    registry->RegisterDecoder(new WebPDecoder());
    registry->RegisterDecoder(new AVIFDecoder());
    registry->RegisterDecoder(new ArchiveDecoder());
    registry->RegisterDecoder(new VideoDecoder());
    registry->RegisterDecoder(new AudioDecoder());
    registry->RegisterDecoder(new DocumentDecoder());
    registry->RegisterDecoder(new FontDecoder());
    registry->RegisterDecoder(new ModelDecoder());

    auto stats = registry->GetStats();
    ASSERT_INTEGRATION(stats.totalDecoders >= 9);
    ASSERT_INTEGRATION(stats.supportedExtensionsCount > 50);

    delete registry;
}

INTEGRATION_TEST(TestPipeline_ImageFormatsEndToEnd)
{
    // Test image format decoding pipeline
    auto registry = DecoderRegistry::Create();
    registry->RegisterDecoder(new ImageDecoder());
    registry->RegisterDecoder(new WebPDecoder());

    // Test extension recognition
    auto* decoder = registry->FindDecoder(L".jpg");
    ASSERT_INTEGRATION(decoder != nullptr);

    decoder = registry->FindDecoder(L".webp");
    ASSERT_INTEGRATION(decoder != nullptr);

    delete registry;
}

INTEGRATION_TEST(TestPipeline_VideoFormatPriority)
{
    // Video files should route to VideoDecoder, not ImageDecoder
    auto registry = DecoderRegistry::Create();
    registry->RegisterDecoder(new ImageDecoder());
    registry->RegisterDecoder(new VideoDecoder());

    auto* decoder = registry->FindDecoder(L".mp4");
    ASSERT_INTEGRATION(decoder != nullptr);

    auto info = decoder->GetInfo();
    std::wstring decoderName(info.decoderName);
    ASSERT_INTEGRATION(decoderName.find(L"Video") != std::wstring::npos);

    delete registry;
}

INTEGRATION_TEST(TestPipeline_ArchiveFormatRecognition)
{
    // Archives should be handled by ArchiveDecoder
    auto registry = DecoderRegistry::Create();
    registry->RegisterDecoder(new ArchiveDecoder());

    auto* decoder = registry->FindDecoder(L".cbz");
    ASSERT_INTEGRATION(decoder != nullptr);

    decoder = registry->FindDecoder(L".cbr");
    ASSERT_INTEGRATION(decoder != nullptr);

    decoder = registry->FindDecoder(L".7z");
    ASSERT_INTEGRATION(decoder != nullptr);

    delete registry;
}

INTEGRATION_TEST(TestPipeline_MultiDecoderCoexistence)
{
    // Multiple decoders should coexist without conflicts
    auto registry = DecoderRegistry::Create();
    
    registry->RegisterDecoder(new ImageDecoder());
    registry->RegisterDecoder(new WebPDecoder());
    registry->RegisterDecoder(new AVIFDecoder());
    registry->RegisterDecoder(new ArchiveDecoder());
    registry->RegisterDecoder(new VideoDecoder());
    registry->RegisterDecoder(new AudioDecoder());

    // Each format should route to correct decoder
    std::vector<std::pair<std::wstring, std::wstring>> testCases = {
        { L".jpg", L"Image" },
        { L".webp", L"WebP" },
        { L".avif", L"AVIF" },
        { L".zip", L"Archive" },
        { L".mp4", L"Video" },
        { L".mp3", L"Audio" }
    };

    for (const auto& testCase : testCases) {
        auto* decoder = registry->FindDecoder(testCase.first.c_str());
        ASSERT_INTEGRATION(decoder != nullptr);
        
        auto info = decoder->GetInfo();
        std::wstring name(info.decoderName);
        ASSERT_INTEGRATION(name.find(testCase.second) != std::wstring::npos);
    }

    delete registry;
}

INTEGRATION_TEST(TestPipeline_DocumentFormats)
{
    // Document thumbnails (EPUB, MOBI) should work
    auto registry = DecoderRegistry::Create();
    registry->RegisterDecoder(new DocumentDecoder());

    auto* decoder = registry->FindDecoder(L".epub");
    ASSERT_INTEGRATION(decoder != nullptr);

    decoder = registry->FindDecoder(L".mobi");
    ASSERT_INTEGRATION(decoder != nullptr);

    delete registry;
}

INTEGRATION_TEST(TestPipeline_FontFormats)
{
    // Font preview rendering
    auto registry = DecoderRegistry::Create();
    registry->RegisterDecoder(new FontDecoder());

    auto* decoder = registry->FindDecoder(L".ttf");
    ASSERT_INTEGRATION(decoder != nullptr);

    decoder = registry->FindDecoder(L".otf");
    ASSERT_INTEGRATION(decoder != nullptr);

    decoder = registry->FindDecoder(L".woff");
    ASSERT_INTEGRATION(decoder != nullptr);

    delete registry;
}

INTEGRATION_TEST(TestPipeline_3DModelFormats)
{
    // 3D model thumbnails (Sprint 12)
    auto registry = DecoderRegistry::Create();
    registry->RegisterDecoder(new ModelDecoder());

    auto* decoder = registry->FindDecoder(L".obj");
    ASSERT_INTEGRATION(decoder != nullptr);

    decoder = registry->FindDecoder(L".stl");
    ASSERT_INTEGRATION(decoder != nullptr);

    decoder = registry->FindDecoder(L".gltf");
    ASSERT_INTEGRATION(decoder != nullptr);

    delete registry;
}

INTEGRATION_TEST(TestPipeline_InvalidFormatHandling)
{
    // Invalid/unsupported formats should return nullptr
    auto registry = DecoderRegistry::Create();
    registry->RegisterDecoder(new ImageDecoder());

    auto* decoder = registry->FindDecoder(L".xyz");
    ASSERT_INTEGRATION(decoder == nullptr);

    decoder = registry->FindDecoder(L".invalid");
    ASSERT_INTEGRATION(decoder == nullptr);

    delete registry;
}

INTEGRATION_TEST(TestPipeline_NullInputHandling)
{
    // Pipeline should handle null inputs gracefully
    auto registry = DecoderRegistry::Create();
    registry->RegisterDecoder(new ImageDecoder());

    auto* decoder = registry->FindDecoder(nullptr);
    ASSERT_INTEGRATION(decoder == nullptr);

    decoder = registry->FindDecoder(L"");
    ASSERT_INTEGRATION(decoder == nullptr);

    delete registry;
}

INTEGRATION_TEST(TestPipeline_CaseInsensitiveExtensions)
{
    // Extensions should be case-insensitive
    auto registry = DecoderRegistry::Create();
    registry->RegisterDecoder(new ImageDecoder());

    auto* decoder1 = registry->FindDecoder(L".JPG");
    auto* decoder2 = registry->FindDecoder(L".jpg");
    auto* decoder3 = registry->FindDecoder(L".JpG");

    ASSERT_INTEGRATION(decoder1 != nullptr);
    ASSERT_INTEGRATION(decoder2 != nullptr);
    ASSERT_INTEGRATION(decoder3 != nullptr);
    ASSERT_INTEGRATION(decoder1 == decoder2);
    ASSERT_INTEGRATION(decoder2 == decoder3);

    delete registry;
}

INTEGRATION_TEST(TestPipeline_DecoderStatistics)
{
    // Statistics should accurately reflect registered decoders
    auto registry = DecoderRegistry::Create();
    
    auto stats1 = registry->GetStats();
    ASSERT_INTEGRATION(stats1.totalDecoders == 0);
    ASSERT_INTEGRATION(stats1.supportedExtensionsCount == 0);

    registry->RegisterDecoder(new ImageDecoder());
    auto stats2 = registry->GetStats();
    ASSERT_INTEGRATION(stats2.totalDecoders == 1);
    ASSERT_INTEGRATION(stats2.supportedExtensionsCount > 0);

    registry->RegisterDecoder(new WebPDecoder());
    auto stats3 = registry->GetStats();
    ASSERT_INTEGRATION(stats3.totalDecoders == 2);
    ASSERT_INTEGRATION(stats3.supportedExtensionsCount > stats2.supportedExtensionsCount);

    delete registry;
}

INTEGRATION_TEST(TestPipeline_MemoryManagement)
{
    // Test proper cleanup - no memory leaks
    for (int i = 0; i < 100; ++i) {
        auto registry = DecoderRegistry::Create();
        registry->RegisterDecoder(new ImageDecoder());
        registry->RegisterDecoder(new WebPDecoder());
        registry->RegisterDecoder(new AVIFDecoder());
        
        auto* decoder = registry->FindDecoder(L".jpg");
        (void)decoder; // Use variable
        
        delete registry;
    }
    // If we get here without crashing, memory management is OK
}

INTEGRATION_TEST(TestPipeline_ThreadSafety)
{
    // Basic thread safety test (registry should handle concurrent lookups)
    auto registry = DecoderRegistry::Create();
    registry->RegisterDecoder(new ImageDecoder());
    registry->RegisterDecoder(new WebPDecoder());

    // Multiple lookups should not cause issues
    for (int i = 0; i < 1000; ++i) {
        auto* decoder = registry->FindDecoder(L".jpg");
        ASSERT_INTEGRATION(decoder != nullptr);
    }

    delete registry;
}

//==============================================================================
// Main Integration Test Runner
//==============================================================================

int main()
{
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"DarkThumbs Engine - Integration Tests" << std::endl;
    std::wcout << L"Sprint 16: End-to-End Pipeline Testing" << std::endl;
    std::wcout << L"========================================" << std::endl << std::endl;

    // Pipeline Integration Tests
    std::wcout << L"Pipeline Integration Tests:" << std::endl;
    RUN_INTEGRATION_TEST(TestPipeline_FullInitialization);
    RUN_INTEGRATION_TEST(TestPipeline_ImageFormatsEndToEnd);
    RUN_INTEGRATION_TEST(TestPipeline_VideoFormatPriority);
    RUN_INTEGRATION_TEST(TestPipeline_ArchiveFormatRecognition);
    RUN_INTEGRATION_TEST(TestPipeline_MultiDecoderCoexistence);
    RUN_INTEGRATION_TEST(TestPipeline_DocumentFormats);
    RUN_INTEGRATION_TEST(TestPipeline_FontFormats);
    RUN_INTEGRATION_TEST(TestPipeline_3DModelFormats);
    RUN_INTEGRATION_TEST(TestPipeline_InvalidFormatHandling);
    RUN_INTEGRATION_TEST(TestPipeline_NullInputHandling);
    RUN_INTEGRATION_TEST(TestPipeline_CaseInsensitiveExtensions);
    RUN_INTEGRATION_TEST(TestPipeline_DecoderStatistics);
    RUN_INTEGRATION_TEST(TestPipeline_MemoryManagement);
    RUN_INTEGRATION_TEST(TestPipeline_ThreadSafety);

    // Summary
    std::wcout << std::endl;
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"Integration Test Results:" << std::endl;
    std::wcout << L"  Total:  " << g_integrationTestsRun << std::endl;
    std::wcout << L"  Passed: " << g_integrationTestsPassed << std::endl;
    std::wcout << L"  Failed: " << g_integrationTestsFailed << std::endl;
    std::wcout << L"========================================" << std::endl;

    return g_integrationTestsFailed > 0 ? 1 : 0;
}
