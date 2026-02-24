#include "../Engine.h"
#include "../Pipeline/FormatDetector.h"
#include "../Pipeline/ThumbnailPipeline.h"
#include "../Utils/HardwareCapabilities.h"
#include "../Utils/PerformanceProfiler.h"
#include "../Utils/SIMDScaler.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>

using namespace ExplorerLens;
using namespace ExplorerLens::Engine;

void PrintHeader(const std::wstring &title) {
  std::wcout << L"\n" << std::wstring(80, L'=') << L"\n";
  std::wcout << L"  " << title << L"\n";
  std::wcout << std::wstring(80, L'=') << L"\n";
}

void PrintResult(const std::wstring &test, bool success, uint32_t timeMs = 0) {
  std::wcout << std::left << std::setw(50) << test;
  if (success) {
    std::wcout << L"[OK]";
    if (timeMs > 0) {
      std::wcout << L"  " << timeMs << L" ms";
    }
  } else {
    std::wcout << L"[FAIL]";
  }
  std::wcout << std::endl;
}

// Find test images in workspace
std::vector<std::wstring> FindTestImages() {
  std::vector<std::wstring> images;

  // Try to find actual test images
  std::vector<std::wstring> testPaths = {
      L"C:\\Users\\ryair\\OneDrive - Intel "
      L"Corporation\\Documents\\MyScripts\\ExplorerLens\\test-archives\\test-"
      L"image-1.png",
      L"C:\\Users\\ryair\\OneDrive - Intel "
      L"Corporation\\Documents\\MyScripts\\ExplorerLens\\test-archives\\test-"
      L"image-2.jpg",
      L"C:\\Users\\ryair\\OneDrive - Intel "
      L"Corporation\\Documents\\MyScripts\\ExplorerLens\\test-archives\\sample."
      L"png",
      L"C:\\Windows\\Web\\Wallpaper\\Windows\\img0.jpg"};

  for (const auto &path : testPaths) {
    DWORD attr = GetFileAttributesW(path.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
      images.push_back(path);
    }
  }

  return images;
}

int main() {
  // Initialize COM for WIC
  HRESULT hrCOM = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (FAILED(hrCOM)) {
    std::wcerr << L"ERROR: Failed to initialize COM (HRESULT=0x" << std::hex
               << hrCOM << L")\n";
    return 1;
  }

  std::wcout << L"ExplorerLens Engine Performance Benchmark\n";
  std::wcout << L"Version: 7.0.0\n";
  std::wcout << L"========================================\n\n";

  // Find test images
  auto testImages = FindTestImages();
  if (testImages.empty()) {
    std::wcerr << L"No test images found!\n";
    return 1;
  }

  std::wcout << L"Found " << testImages.size() << L" test images:\n";
  for (const auto &img : testImages) {
    std::wcout << L"  - " << img << L"\n";
  }
  std::wcout << L"\n";

  // Enable performance profiling
  PerformanceProfiler::GetInstance().SetEnabled(true);

  // Create and initialize pipeline
  ThumbnailPipeline pipeline;
  PipelineConfig config;
  config.enableGPU = true;
  config.enableCache = true;

  if (!pipeline.Initialize(config)) {
    std::wcerr << L"Failed to initialize ThumbnailPipeline\n";
    return 1;
  }

  std::wcout << L"Pipeline initialized successfully\n";
  std::wcout << L"GPU: " << (config.enableGPU ? L"Enabled" : L"Disabled")
             << L"\n";
  std::wcout << L"Cache: " << (config.enableCache ? L"Enabled" : L"Disabled")
             << L"\n\n";

  // Benchmark 1: Single thumbnail generation
  PrintHeader(L"Benchmark 1: Single Thumbnail Generation (256x256)");
  for (const auto &imagePath : testImages) {
    ThumbnailRequest request;
    request.filePath = imagePath.c_str();
    request.width = 256;
    request.height = 256;
    request.flags = ThumbnailFlags::PreserveAspect | ThumbnailFlags::UseCache;
    request.archiveEntry = nullptr;

    ThumbnailResult result = pipeline.GenerateThumbnail(request);

    std::wstring filename =
        imagePath.substr(imagePath.find_last_of(L"\\/ ") + 1);
    PrintResult(filename, SUCCEEDED(result.status), result.generationTimeMs);

    if (result.hBitmap) {
      DeleteObject(result.hBitmap);
    }
  }

  // Benchmark 2: Cache hit performance
  PrintHeader(L"Benchmark 2: Cache Hit Performance");
  if (!testImages.empty()) {
    const std::wstring &testImage = testImages[0];

    // First generation (cache miss)
    ThumbnailRequest request;
    request.filePath = testImage.c_str();
    request.width = 256;
    request.height = 256;
    request.flags = ThumbnailFlags::PreserveAspect | ThumbnailFlags::UseCache;

    ThumbnailResult result1 = pipeline.GenerateThumbnail(request);
    PrintResult(L"First generation (cache miss)", SUCCEEDED(result1.status),
                result1.generationTimeMs);
    if (result1.hBitmap)
      DeleteObject(result1.hBitmap);

    // Subsequent generations (cache hits)
    for (int i = 0; i < 5; i++) {
      ThumbnailResult result = pipeline.GenerateThumbnail(request);
      std::wstring testName = L"Cache hit #" + std::to_wstring(i + 1);
      PrintResult(testName, SUCCEEDED(result.status), result.generationTimeMs);
      if (result.hBitmap)
        DeleteObject(result.hBitmap);
    }
  }

  // Benchmark 3: Batch generation
  PrintHeader(L"Benchmark 3: Batch Generation (20 thumbnails)");
  auto startTime = std::chrono::high_resolution_clock::now();

  int successCount = 0;
  uint64_t totalTime = 0;
  const int batchSize = 20;

  for (int i = 0; i < batchSize; i++) {
    const std::wstring &imagePath = testImages[i % testImages.size()];

    ThumbnailRequest request;
    request.filePath = imagePath.c_str();
    request.width = 256;
    request.height = 256;
    request.flags = ThumbnailFlags::PreserveAspect | ThumbnailFlags::UseCache;

    ThumbnailResult result = pipeline.GenerateThumbnail(request);

    if (SUCCEEDED(result.status)) {
      successCount++;
      totalTime += result.generationTimeMs;
    }

    if (result.hBitmap) {
      DeleteObject(result.hBitmap);
    }
  }

  auto endTime = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      endTime - startTime);

  std::wcout << L"Batch completed: " << successCount << L"/" << batchSize
             << L" successful\n";
  std::wcout << L"Total time: " << duration.count() << L" ms\n";
  std::wcout << L"Average per image: "
             << (successCount > 0 ? totalTime / successCount : 0) << L" ms\n";
  std::wcout << L"Throughput: " << std::fixed << std::setprecision(1)
             << (duration.count() > 0 ? successCount * 1000.0 / duration.count()
                                      : 0.0)
             << L" images/sec\n";

  // Benchmark 4: Different sizes
  PrintHeader(L"Benchmark 4: Different Thumbnail Sizes");
  if (!testImages.empty()) {
    const std::wstring &testImage = testImages[0];
    const std::vector<std::pair<int, int>> sizes = {
        {96, 96}, {128, 128}, {256, 256}, {512, 512}};

    for (const auto &[width, height] : sizes) {
      ThumbnailRequest request;
      request.filePath = testImage.c_str();
      request.width = width;
      request.height = height;
      request.flags = ThumbnailFlags::PreserveAspect;

      ThumbnailResult result = pipeline.GenerateThumbnail(request);

      std::wstring sizeName =
          std::to_wstring(width) + L"x" + std::to_wstring(height);
      PrintResult(sizeName, SUCCEEDED(result.status), result.generationTimeMs);

      if (result.hBitmap) {
        DeleteObject(result.hBitmap);
      }
    }
  }

  // Benchmark 4: Format Detection Performance
  PrintHeader(L"Benchmark 4: Format Detection Performance");
  std::wcout
      << L"Testing format detection speed (10000 iterations per format)\n\n";

  // Test various format extensions
  struct FormatTest {
    const wchar_t *name;
    const wchar_t *extension;
  };

  std::vector<FormatTest> formats = {
      {L"JPEG", L".jpg"},      {L"PNG", L".png"},   {L"WebP", L".webp"},
      {L"JPEG XL", L".jxl"},   {L"AVIF", L".avif"}, {L"HEIF", L".heif"},
      {L"RAW (CR2)", L".cr2"}, {L"ZIP", L".zip"},   {L"RAR", L".rar"},
      {L"CBZ", L".cbz"}};

  FormatDetector detector;
  const int iterations = 10000;

  for (const auto &format : formats) {
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    // Run detection multiple times
    DetectedFormat detectedType = DetectedFormat::Unknown;
    for (int i = 0; i < iterations; i++) {
      detectedType = detector.DetectFromExtension(format.extension);
    }

    QueryPerformanceCounter(&end);
    double totalMs = (end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
    double avgUs =
        (totalMs * 1000.0) / iterations; // microseconds per detection

    std::wcout << std::left << std::setw(15) << format.name << std::right
               << std::setw(10) << std::fixed << std::setprecision(3) << avgUs
               << L" μs/detection" << std::setw(15) << std::fixed
               << std::setprecision(0) << (1000000.0 / avgUs) << L" det/sec"
               << L"  ["
               << (detectedType != DetectedFormat::Unknown ? L"OK" : L"FAIL")
               << L"]\n";
  }

  std::wcout << L"\nTarget: < 1 μs per extension-based detection\n";
  std::wcout << L"Extension detection is O(1) hash lookup - very fast!\n";

  // Benchmark 5: SIMD Scaling Performance
  PrintHeader(L"Benchmark 5: SIMD Scaling Performance");
  std::wcout << L"Testing image scaling with SIMD optimizations\n\n";

  // Test different image sizes and methods
  struct ScaleTest {
    const wchar_t *size;
    uint32_t width;
    uint32_t height;
  };

  std::vector<ScaleTest> sizes = {{L"HD (1920x1080)", 1920, 1080},
                                  {L"4K (3840x2160)", 3840, 2160},
                                  {L"8K (7680x4320)", 7680, 4320}};

  const uint32_t thumbWidth = 256;
  const uint32_t thumbHeight = 256;

  // Check CPU features using HardwareCapabilities
  auto &hwCaps = HardwareCapabilities::Get();
  const auto &cpu = hwCaps.GetCPU();
  std::wcout << L"CPU Features:\n";
  std::wcout << L"  AVX2:  " << (cpu.hasAVX2 ? L"Yes" : L"No") << L"\n";
  std::wcout << L"  SSE4.1: " << (cpu.hasSSE41 ? L"Yes" : L"No") << L"\n";
  std::wcout << L"  FMA:   " << (cpu.hasFMA ? L"Yes" : L"No") << L"\n\n";

  for (const auto &test : sizes) {
    // Create source buffer (simulated BGRA image)
    uint32_t srcStride = test.width * 4;
    uint32_t dstStride = thumbWidth * 4;
    std::vector<uint8_t> srcData(srcStride * test.height, 128); // Gray image
    std::vector<uint8_t> dstData(dstStride * thumbHeight);

    // Benchmark scaling
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    const int scaleIterations = 10; // Fewer iterations for large images
    double avgMs = 0.0;
    double mpixPerSec = 0.0;

    // Test AVX2 if available
    if (cpu.hasAVX2) {
      QueryPerformanceCounter(&start);
      for (int i = 0; i < scaleIterations; i++) {
        SIMD::SIMDScaler::ScaleBGRA_Bilinear_AVX2(
            srcData.data(), test.width, test.height, srcStride, dstData.data(),
            thumbWidth, thumbHeight, dstStride);
      }
      QueryPerformanceCounter(&end);
      avgMs = ((end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart) /
              scaleIterations;
      mpixPerSec = (test.width * test.height / 1000000.0) / (avgMs / 1000.0);

      std::wcout << std::left << std::setw(25) << test.size << L"AVX2:   "
                 << std::right << std::setw(8) << std::fixed
                 << std::setprecision(2) << avgMs << L" ms  " << std::setw(6)
                 << std::fixed << std::setprecision(1) << mpixPerSec
                 << L" Mpix/s\n";
    }

    // Test scalar fallback
    QueryPerformanceCounter(&start);
    for (int i = 0; i < scaleIterations; i++) {
      SIMD::SIMDScaler::ScaleBGRA_Bilinear_Scalar(
          srcData.data(), test.width, test.height, srcStride, dstData.data(),
          thumbWidth, thumbHeight, dstStride);
    }
    QueryPerformanceCounter(&end);
    avgMs = ((end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart) /
            scaleIterations;
    mpixPerSec = (test.width * test.height / 1000000.0) / (avgMs / 1000.0);

    std::wcout << std::left << std::setw(25) << L"" << L"Scalar: " << std::right
               << std::setw(8) << std::fixed << std::setprecision(2) << avgMs
               << L" ms  " << std::setw(6) << std::fixed << std::setprecision(1)
               << mpixPerSec << L" Mpix/s\n\n";
  }

  std::wcout << L"Target: > 100 Mpix/s for real-time thumbnail generation\n";

  // Print pipeline statistics
  PrintHeader(L"Pipeline Statistics");
  uint64_t totalRequests, cacheHits, cacheMisses;
  double avgTime;
  pipeline.GetStatistics(totalRequests, cacheHits, cacheMisses, avgTime);

  std::wcout << L"Total Requests: " << totalRequests << L"\n";
  std::wcout << L"Cache Hits: " << cacheHits << L" (" << std::fixed
             << std::setprecision(1)
             << (totalRequests > 0 ? cacheHits * 100.0 / totalRequests : 0.0)
             << L"%)\n";
  std::wcout << L"Cache Misses: " << cacheMisses << L"\n";
  std::wcout << L"Average Time: " << std::fixed << std::setprecision(2)
             << avgTime << L" ms\n";

  // Print profiling results
  PrintHeader(L"Performance Profiling Results");
  std::wcout << PerformanceProfiler::GetInstance().GenerateReport() << L"\n";

  // Export detailed report
  std::wstring reportPath = L"performance_report.txt";
  if (PerformanceProfiler::GetInstance().ExportToFile(reportPath)) {
    std::wcout << L"\nDetailed report exported to: " << reportPath << L"\n";
  }

  pipeline.Shutdown();

  std::wcout << L"\nBenchmark completed successfully!\n";

  // Uninitialize COM
  CoUninitialize();

  return 0;
}
