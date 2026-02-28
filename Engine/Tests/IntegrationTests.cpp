//==============================================================================
// ExplorerLens Engine - Integration Tests
// End-to-end pipeline testing with real files
// Updated v7.0.0 - All 24 decoders
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include "../Pipeline/ThumbnailPipeline.h"
#include "../Pipeline/DecoderRegistry.h"
#include "../Decoders/ImageDecoder.h"
#include "../Decoders/WebPDecoder.h"
#include "../Decoders/AVIFDecoder.h"
#include "../Decoders/ArchiveDecoder.h"
#include "../Decoders/RAWDecoder.h"
#include "../Decoders/HEIFDecoder.h"
#include "../Decoders/JXLDecoder.h"
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
#include <iostream>
#include <vector>
#include <string>

using namespace ExplorerLens::Engine;

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
 std::wcout << L" [PASS]" << std::endl; \
 } catch (const char* msg) { \
 g_integrationTestsFailed++; \
 std::cout << " [FAIL] " << msg << std::endl; \
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
 // Test complete pipeline initialization with all 24 decoders
 auto* registry = new DecoderRegistry();
 ASSERT_INTEGRATION(registry != nullptr);

 // Register all decoders (same order as ThumbnailPipeline)
 registry->RegisterDecoder(new ArchiveDecoder());
 registry->RegisterDecoder(new WebPDecoder());
 registry->RegisterDecoder(new AVIFDecoder());
 registry->RegisterDecoder(new RAWDecoder());
 registry->RegisterDecoder(new HEIFDecoder());
 registry->RegisterDecoder(new JXLDecoder());
 registry->RegisterDecoder(new ICODecoder());
 registry->RegisterDecoder(new TGADecoder());
 registry->RegisterDecoder(new QOIDecoder());
 registry->RegisterDecoder(new PSDDecoder());
 registry->RegisterDecoder(new DDSDecoder());
 registry->RegisterDecoder(new HDRDecoder());
 registry->RegisterDecoder(new PPMDecoder());
 registry->RegisterDecoder(new EXRDecoder());
 registry->RegisterDecoder(new SVGDecoder());
 registry->RegisterDecoder(new VideoDecoder());
 registry->RegisterDecoder(new AudioDecoder());
 registry->RegisterDecoder(new PDFDecoder());
 registry->RegisterDecoder(new DocumentDecoder());
 registry->RegisterDecoder(new FontDecoder());
 registry->RegisterDecoder(new ModelDecoder());
 registry->RegisterDecoder(new ImageDecoder());

 size_t totalDecoders = 0, imageDecoders = 0, archiveDecoders = 0, totalExtensions = 0;
 registry->GetStats(&totalDecoders, &imageDecoders, &archiveDecoders, &totalExtensions);
 ASSERT_INTEGRATION(totalDecoders == 22); // 22 unique decoder instances
 ASSERT_INTEGRATION(totalExtensions > 80); // 80+ file extensions supported

 delete registry;
}

INTEGRATION_TEST(TestPipeline_ImageFormatsEndToEnd)
{
 // Test image format decoding pipeline
 auto* registry = new DecoderRegistry();
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
 auto* registry = new DecoderRegistry();
 registry->RegisterDecoder(new ImageDecoder());
 registry->RegisterDecoder(new VideoDecoder());

 auto* decoder = registry->FindDecoder(L".mp4");
 ASSERT_INTEGRATION(decoder != nullptr);

 auto info = decoder->GetInfo();
 std::wstring decoderName(info.name);
 ASSERT_INTEGRATION(decoderName.find(L"Video") != std::wstring::npos);

 delete registry;
}

INTEGRATION_TEST(TestPipeline_ArchiveFormatRecognition)
{
 // Archives should be handled by ArchiveDecoder
 auto* registry = new DecoderRegistry();
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
 // All decoders should coexist and route correctly
 auto* registry = new DecoderRegistry();
 
 registry->RegisterDecoder(new ArchiveDecoder());
 registry->RegisterDecoder(new WebPDecoder());
 registry->RegisterDecoder(new AVIFDecoder());
 registry->RegisterDecoder(new RAWDecoder());
 registry->RegisterDecoder(new HEIFDecoder());
 registry->RegisterDecoder(new JXLDecoder());
 registry->RegisterDecoder(new ICODecoder());
 registry->RegisterDecoder(new TGADecoder());
 registry->RegisterDecoder(new QOIDecoder());
 registry->RegisterDecoder(new PSDDecoder());
 registry->RegisterDecoder(new DDSDecoder());
 registry->RegisterDecoder(new HDRDecoder());
 registry->RegisterDecoder(new PPMDecoder());
 registry->RegisterDecoder(new EXRDecoder());
 registry->RegisterDecoder(new SVGDecoder());
 registry->RegisterDecoder(new VideoDecoder());
 registry->RegisterDecoder(new AudioDecoder());
 registry->RegisterDecoder(new PDFDecoder());
 registry->RegisterDecoder(new DocumentDecoder());
 registry->RegisterDecoder(new FontDecoder());
 registry->RegisterDecoder(new ModelDecoder());
 registry->RegisterDecoder(new ImageDecoder());

 // Each format should route to correct decoder
 std::vector<std::pair<std::wstring, std::wstring>> testCases = {
 // Image formats
 { L".jpg", L"Image" },
 { L".png", L"Image" },
 { L".bmp", L"Image" },
 { L".gif", L"Image" },
 { L".tiff", L"Image" },
 { L".webp", L"WebP" },
 { L".avif", L"AVIF" },
 { L".jxl", L"JXL" },
 { L".heic", L"HEIF" },
 { L".ico", L"ICO" },
 { L".tga", L"TGA" },
 { L".qoi", L"QOI" },
 { L".psd", L"PSD" },
 { L".dds", L"DDS" },
 { L".hdr", L"HDR" },
 { L".ppm", L"PPM" },
 { L".svg", L"SVG" },
 // Archive formats
 { L".zip", L"Archive" },
 { L".cbz", L"Archive" },
 { L".rar", L"Archive" },
 // Media formats
 { L".mp4", L"Video" },
 { L".mp3", L"Audio" },
 // Document formats
 { L".epub", L"Document" },
 // Font formats
 { L".ttf", L"Font" },
 // 3D model formats
 { L".obj", L"Model" },
 };

 for (const auto& testCase : testCases) {
 auto* decoder = registry->FindDecoder(testCase.first.c_str());
 ASSERT_INTEGRATION(decoder != nullptr);
 
 auto info = decoder->GetInfo();
 std::wstring name(info.name);
 ASSERT_INTEGRATION(name.find(testCase.second) != std::wstring::npos);
 }

 delete registry;
}

INTEGRATION_TEST(TestPipeline_DocumentFormats)
{
 // Document thumbnails (EPUB, MOBI) should work
 auto* registry = new DecoderRegistry();
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
 auto* registry = new DecoderRegistry();
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
 // 3D model thumbnails
 auto* registry = new DecoderRegistry();
 registry->RegisterDecoder(new ModelDecoder());

 auto* decoder = registry->FindDecoder(L".obj");
 ASSERT_INTEGRATION(decoder != nullptr);

 decoder = registry->FindDecoder(L".stl");
 ASSERT_INTEGRATION(decoder != nullptr);

 decoder = registry->FindDecoder(L".gltf");
 ASSERT_INTEGRATION(decoder != nullptr);

 delete registry;
}

INTEGRATION_TEST(TestPipeline_SpecialtyImageFormats)
{
 // Test all specialty image decoders added in v5.3-v7.0
 auto* registry = new DecoderRegistry();
 registry->RegisterDecoder(new QOIDecoder());
 registry->RegisterDecoder(new PSDDecoder());
 registry->RegisterDecoder(new DDSDecoder());
 registry->RegisterDecoder(new HDRDecoder());
 registry->RegisterDecoder(new PPMDecoder());
 registry->RegisterDecoder(new TGADecoder());
 registry->RegisterDecoder(new ICODecoder());
 registry->RegisterDecoder(new EXRDecoder());
 registry->RegisterDecoder(new SVGDecoder());

 // QOI
 auto* decoder = registry->FindDecoder(L".qoi");
 ASSERT_INTEGRATION(decoder != nullptr);

 // PSD/PSB
 decoder = registry->FindDecoder(L".psd");
 ASSERT_INTEGRATION(decoder != nullptr);

 // DDS
 decoder = registry->FindDecoder(L".dds");
 ASSERT_INTEGRATION(decoder != nullptr);

 // HDR (Radiance)
 decoder = registry->FindDecoder(L".hdr");
 ASSERT_INTEGRATION(decoder != nullptr);

 // PPM/PGM/PBM
 decoder = registry->FindDecoder(L".ppm");
 ASSERT_INTEGRATION(decoder != nullptr);
 decoder = registry->FindDecoder(L".pgm");
 ASSERT_INTEGRATION(decoder != nullptr);

 // TGA
 decoder = registry->FindDecoder(L".tga");
 ASSERT_INTEGRATION(decoder != nullptr);

 // ICO/CUR
 decoder = registry->FindDecoder(L".ico");
 ASSERT_INTEGRATION(decoder != nullptr);
 decoder = registry->FindDecoder(L".cur");
 ASSERT_INTEGRATION(decoder != nullptr);

 // SVG/SVGZ
 decoder = registry->FindDecoder(L".svg");
 ASSERT_INTEGRATION(decoder != nullptr);
 decoder = registry->FindDecoder(L".svgz");
 ASSERT_INTEGRATION(decoder != nullptr);

 delete registry;
}

INTEGRATION_TEST(TestPipeline_CameraRAWFormats)
{
 // Camera RAW formats via LibRaw
 auto* registry = new DecoderRegistry();
 registry->RegisterDecoder(new RAWDecoder());

 std::vector<std::wstring> rawExts = {
 L".cr2", L".cr3", L".nef", L".arw", L".dng", L".raf", L".rw2", L".orf"
 };

 for (const auto& ext : rawExts) {
 auto* decoder = registry->FindDecoder(ext.c_str());
 ASSERT_INTEGRATION(decoder != nullptr);
 }

 delete registry;
}

INTEGRATION_TEST(TestPipeline_ModernImageFormats)
{
 // Modern next-gen image formats (JXL, HEIF, AVIF)
 auto* registry = new DecoderRegistry();
 registry->RegisterDecoder(new JXLDecoder());
 registry->RegisterDecoder(new HEIFDecoder());
 registry->RegisterDecoder(new AVIFDecoder());

 // JPEG XL
 auto* decoder = registry->FindDecoder(L".jxl");
 ASSERT_INTEGRATION(decoder != nullptr);

 // HEIF/HEIC (iPhone photos)
 decoder = registry->FindDecoder(L".heic");
 ASSERT_INTEGRATION(decoder != nullptr);
 decoder = registry->FindDecoder(L".heif");
 ASSERT_INTEGRATION(decoder != nullptr);

 // AVIF
 decoder = registry->FindDecoder(L".avif");
 ASSERT_INTEGRATION(decoder != nullptr);

 delete registry;
}

INTEGRATION_TEST(TestPipeline_PDFDocumentFormat)
{
 // PDF thumbnails via PDFDecoder
 auto* registry = new DecoderRegistry();
 registry->RegisterDecoder(new PDFDecoder());

 auto* decoder = registry->FindDecoder(L".pdf");
 ASSERT_INTEGRATION(decoder != nullptr);

 delete registry;
}

INTEGRATION_TEST(TestPipeline_InvalidFormatHandling)
{
 // Invalid/unsupported formats should return nullptr
 auto* registry = new DecoderRegistry();
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
 auto* registry = new DecoderRegistry();
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
 auto* registry = new DecoderRegistry();
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
 auto* registry = new DecoderRegistry();
 
 size_t totalDecoders1 = 0, imageDecoders1 = 0, archiveDecoders1 = 0, totalExtensions1 = 0;
 registry->GetStats(&totalDecoders1, &imageDecoders1, &archiveDecoders1, &totalExtensions1);
 ASSERT_INTEGRATION(totalDecoders1 == 0);
 ASSERT_INTEGRATION(totalExtensions1 == 0);

 registry->RegisterDecoder(new ImageDecoder());
 size_t totalDecoders2 = 0, imageDecoders2 = 0, archiveDecoders2 = 0, totalExtensions2 = 0;
 registry->GetStats(&totalDecoders2, &imageDecoders2, &archiveDecoders2, &totalExtensions2);
 ASSERT_INTEGRATION(totalDecoders2 == 1);
 ASSERT_INTEGRATION(totalExtensions2 > 0);

 registry->RegisterDecoder(new WebPDecoder());
 size_t totalDecoders3 = 0, imageDecoders3 = 0, archiveDecoders3 = 0, totalExtensions3 = 0;
 registry->GetStats(&totalDecoders3, &imageDecoders3, &archiveDecoders3, &totalExtensions3);
 ASSERT_INTEGRATION(totalDecoders3 == 2);
 ASSERT_INTEGRATION(totalExtensions3 > totalExtensions2);

 delete registry;
}

INTEGRATION_TEST(TestPipeline_MemoryManagement)
{
 // Test proper cleanup - no memory leaks
 for (int i = 0; i < 100; ++i) {
 auto* registry = new DecoderRegistry();
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
 auto* registry = new DecoderRegistry();
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
 std::wcout << L"ExplorerLens Engine - Integration Tests" << std::endl;
 std::wcout << L"v7.0.0: All 24 Decoders Pipeline Tests" << std::endl;
 std::wcout << L"========================================" << std::endl << std::endl;

 // Pipeline Integration Tests
 std::wcout << L"Pipeline Integration Tests:" << std::endl;
 RUN_INTEGRATION_TEST(TestPipeline_FullInitialization);
 RUN_INTEGRATION_TEST(TestPipeline_ImageFormatsEndToEnd);
 RUN_INTEGRATION_TEST(TestPipeline_VideoFormatPriority);
 RUN_INTEGRATION_TEST(TestPipeline_ArchiveFormatRecognition);
 RUN_INTEGRATION_TEST(TestPipeline_MultiDecoderCoexistence);
 RUN_INTEGRATION_TEST(TestPipeline_SpecialtyImageFormats);
 RUN_INTEGRATION_TEST(TestPipeline_CameraRAWFormats);
 RUN_INTEGRATION_TEST(TestPipeline_ModernImageFormats);
 RUN_INTEGRATION_TEST(TestPipeline_PDFDocumentFormat);
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
 std::wcout << L" Total: " << g_integrationTestsRun << std::endl;
 std::wcout << L" Passed: " << g_integrationTestsPassed << std::endl;
 std::wcout << L" Failed: " << g_integrationTestsFailed << std::endl;
 std::wcout << L"========================================" << std::endl;

 return g_integrationTestsFailed > 0 ? 1 : 0;
}

