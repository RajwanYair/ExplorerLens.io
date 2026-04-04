// test_gpu.cpp - Unit tests for GPU renderer
#include <cstring>
#include <iostream>
#include <vector>
#include "../Core/Types.h"
#include "../GPU/D3D11Renderer.h"

using namespace ExplorerLens;
using namespace ExplorerLens::Engine;

// Test helpers
void PrintTestHeader(const char* testName)
{
    std::wcout << L"Running: " << testName << L"..." << std::endl;
}

void PrintTestResult(bool passed)
{
    if (passed) {
        std::wcout << L" [PASS]" << std::endl;
    } else {
        std::wcout << L" [FAIL]" << std::endl;
    }
}

// Create simple test image (red/blue gradient)
std::vector<uint8_t> CreateTestImage(uint32_t width, uint32_t height)
{
    std::vector<uint8_t> pixels(width * height * 4);
    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            uint32_t offset = (y * width + x) * 4;
            pixels[offset + 0] = static_cast<uint8_t>((x * 255) / width);   // R
            pixels[offset + 1] = 0;                                         // G
            pixels[offset + 2] = static_cast<uint8_t>((y * 255) / height);  // B
            pixels[offset + 3] = 255;                                       // A
        }
    }
    return pixels;
}

// Test 1: Create renderer
bool TestGPURenderer_Create()
{
    PrintTestHeader("TestGPURenderer_Create");

    IGPURenderer* pRenderer = CreateD3D11Renderer();
    bool passed = (pRenderer != nullptr);

    if (pRenderer) {
        delete pRenderer;
    }

    PrintTestResult(passed);
    return passed;
}

// Test 2: Initialize renderer (may fail in headless/CI environments — treat as soft pass)
bool TestGPURenderer_Initialize()
{
    PrintTestHeader("TestGPURenderer_Initialize");

    IGPURenderer* pRenderer = CreateD3D11Renderer();
    if (!pRenderer) {
        std::wcout << L" [SKIP] No renderer available (headless environment)" << std::endl;
        PrintTestResult(true);  // Soft pass - no GPU is acceptable
        return true;
    }

    HRESULT hr = pRenderer->Initialize();
    bool passed = true;  // Default pass — GPU unavailability is not a failure

    if (SUCCEEDED(hr)) {
        bool available = pRenderer->IsAvailable();
        passed = available;
    } else {
        std::wcout << L" [SKIP] GPU init returned 0x" << std::hex << hr << std::dec << L" (no GPU)" << std::endl;
    }

    delete pRenderer;
    PrintTestResult(passed);
    return passed;
}

// Test 3: Get GPU info
bool TestGPURenderer_GetGPUInfo()
{
    PrintTestHeader("TestGPURenderer_GetGPUInfo");

    IGPURenderer* pRenderer = CreateD3D11Renderer();
    if (!pRenderer) {
        std::wcout << L" [SKIP] No renderer" << std::endl;
        PrintTestResult(true);
        return true;
    }

    HRESULT hr = pRenderer->Initialize();
    if (FAILED(hr)) {
        std::wcout << L" [SKIP] GPU init failed (headless)" << std::endl;
        delete pRenderer;
        PrintTestResult(true);
        return true;
    }

    wchar_t deviceName[256] = {};
    uint32_t memoryMB = 0;
    hr = pRenderer->GetGPUInfo(deviceName, 256, &memoryMB);

    bool passed = SUCCEEDED(hr) && wcslen(deviceName) > 0;

    if (passed) {
        std::wcout << L" GPU: " << deviceName << L" (" << memoryMB << L" MB)" << std::endl;
    }

    delete pRenderer;
    PrintTestResult(passed);
    return passed;
}

// Test 4: Get renderer type
bool TestGPURenderer_GetRendererType()
{
    PrintTestHeader("TestGPURenderer_GetRendererType");

    IGPURenderer* pRenderer = CreateD3D11Renderer();
    if (!pRenderer) {
        std::wcout << L" [SKIP] No renderer" << std::endl;
        PrintTestResult(true);
        return true;
    }

    HRESULT hr = pRenderer->Initialize();
    if (FAILED(hr)) {
        std::wcout << L" [SKIP] GPU init failed (headless)" << std::endl;
        delete pRenderer;
        PrintTestResult(true);
        return true;
    }

    const wchar_t* type = pRenderer->GetRendererType();
    bool passed = (type != nullptr && wcslen(type) > 0);

    if (passed) {
        std::wcout << L" Renderer: " << type << std::endl;
    }

    delete pRenderer;
    PrintTestResult(passed);
    return passed;
}

// Test 5: Render thumbnail
bool TestGPURenderer_RenderThumbnail()
{
    PrintTestHeader("TestGPURenderer_RenderThumbnail");

    IGPURenderer* pRenderer = CreateD3D11Renderer();
    if (!pRenderer) {
        std::wcout << L" [SKIP] No renderer" << std::endl;
        PrintTestResult(true);
        return true;
    }

    HRESULT hr = pRenderer->Initialize();
    if (FAILED(hr)) {
        std::wcout << L" [SKIP] GPU init failed (headless)" << std::endl;
        delete pRenderer;
        PrintTestResult(true);
        return true;
    }

    // Create test image (64x64)
    const uint32_t srcWidth = 64;
    const uint32_t srcHeight = 64;
    auto pixels = CreateTestImage(srcWidth, srcHeight);

    // Render to 32x32 thumbnail
    const uint32_t targetWidth = 32;
    const uint32_t targetHeight = 32;
    HBITMAP hBitmap = nullptr;

    hr = pRenderer->RenderThumbnail(pixels.data(), srcWidth, srcHeight, targetWidth, targetHeight, &hBitmap);

    bool passed = SUCCEEDED(hr) && (hBitmap != nullptr);

    if (hBitmap) {
        // Verify bitmap dimensions
        BITMAP bm;
        if (GetObject(hBitmap, sizeof(BITMAP), &bm)) {
            passed = (bm.bmWidth == 32 && bm.bmHeight == 32);
        }
        DeleteObject(hBitmap);
    }

    delete pRenderer;
    PrintTestResult(passed);
    return passed;
}

// Test 6: Multiple renders (stress test)
bool TestGPURenderer_MultipleRenders()
{
    PrintTestHeader("TestGPURenderer_MultipleRenders");

    IGPURenderer* pRenderer = CreateD3D11Renderer();
    if (!pRenderer) {
        std::wcout << L" [SKIP] No renderer" << std::endl;
        PrintTestResult(true);
        return true;
    }

    HRESULT hr = pRenderer->Initialize();
    if (FAILED(hr)) {
        std::wcout << L" [SKIP] GPU init failed (headless)" << std::endl;
        delete pRenderer;
        PrintTestResult(true);
        return true;
    }

    // Create test image
    const uint32_t srcWidth = 128;
    const uint32_t srcHeight = 128;
    auto pixels = CreateTestImage(srcWidth, srcHeight);

    bool passed = true;
    const int numRenders = 10;

    for (int i = 0; i < numRenders && passed; i++) {
        const uint32_t targetWidth = 32;
        const uint32_t targetHeight = 32;
        HBITMAP hBitmap = nullptr;

        hr = pRenderer->RenderThumbnail(pixels.data(), srcWidth, srcHeight, targetWidth, targetHeight, &hBitmap);
        passed = SUCCEEDED(hr) && (hBitmap != nullptr);

        if (hBitmap) {
            DeleteObject(hBitmap);
        }
    }

    if (passed) {
        std::wcout << L" Successfully rendered " << numRenders << L" thumbnails" << std::endl;
    }

    delete pRenderer;
    PrintTestResult(passed);
    return passed;
}

// Test 7: Different scale factors
bool TestGPURenderer_ScaleFactors()
{
    PrintTestHeader("TestGPURenderer_ScaleFactors");

    IGPURenderer* pRenderer = CreateD3D11Renderer();
    if (!pRenderer) {
        std::wcout << L" [SKIP] No renderer" << std::endl;
        PrintTestResult(true);
        return true;
    }

    HRESULT hr = pRenderer->Initialize();
    if (FAILED(hr)) {
        std::wcout << L" [SKIP] GPU init failed (headless)" << std::endl;
        delete pRenderer;
        PrintTestResult(true);
        return true;
    }

    // Create test image (256x256)
    const uint32_t srcWidth = 256;
    const uint32_t srcHeight = 256;
    auto pixels = CreateTestImage(srcWidth, srcHeight);

    // Test various scale factors
    uint32_t targetSizes[] = {16, 32, 64, 128, 192};
    bool passed = true;

    for (uint32_t size : targetSizes) {
        const uint32_t targetWidth = size;
        const uint32_t targetHeight = size;
        HBITMAP hBitmap = nullptr;

        hr = pRenderer->RenderThumbnail(pixels.data(), srcWidth, srcHeight, targetWidth, targetHeight, &hBitmap);

        if (FAILED(hr) || !hBitmap) {
            passed = false;
            break;
        }

        // Verify dimensions
        BITMAP bm;
        if (!GetObject(hBitmap, sizeof(BITMAP), &bm) || bm.bmWidth != static_cast<LONG>(size)
            || bm.bmHeight != static_cast<LONG>(size)) {
            passed = false;
            DeleteObject(hBitmap);
            break;
        }

        DeleteObject(hBitmap);
    }

    delete pRenderer;
    PrintTestResult(passed);
    return passed;
}

// Test runner
void RunGPUTests()
{
    std::wcout << L"\nGPU Renderer Tests:" << std::endl;

    int passed = 0;
    int total = 0;

#define RUN_TEST(test) \
    {                  \
        total++;       \
        if (test())    \
            passed++;  \
    }

    RUN_TEST(TestGPURenderer_Create);
    RUN_TEST(TestGPURenderer_Initialize);
    RUN_TEST(TestGPURenderer_GetGPUInfo);
    RUN_TEST(TestGPURenderer_GetRendererType);
    RUN_TEST(TestGPURenderer_RenderThumbnail);
    RUN_TEST(TestGPURenderer_MultipleRenders);
    RUN_TEST(TestGPURenderer_ScaleFactors);

#undef RUN_TEST
}
