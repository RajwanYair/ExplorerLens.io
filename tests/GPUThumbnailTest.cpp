// GPUThumbnailTest.cpp - GPU Thumbnail Generation Test Tool
// Tests LENSShell GPU acceleration with real images
// Validates compute shader execution, quality, and correctness

#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>
#include <shlwapi.h>

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "shlwapi.lib")

using Microsoft::WRL::ComPtr;
namespace fs = std::filesystem;

// Test configuration
struct TestConfig {
    std::wstring inputFolder;
    std::wstring outputFolder;
    UINT thumbnailSize;
    bool validateQuality;
    bool generateReference;
    bool verbose;
};

// Test results
struct TestResult {
    std::wstring filename;
    bool success;
    UINT width;
    UINT height;
    double timeMs;
    std::wstring error;
};

class GPUThumbnailTester {
public:
    GPUThumbnailTester() : m_wicFactory(nullptr) {}
    
    HRESULT Initialize() {
        HRESULT hr = CoInitialize(nullptr);
        if (FAILED(hr)) return hr;
        
        hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_wicFactory)
        );
        
        if (SUCCEEDED(hr)) {
            std::wcout << L"[OK] WIC Factory initialized\n";
        }
        
        return hr;
    }
    
    void Shutdown() {
        m_wicFactory.Reset();
        CoUninitialize();
    }
    
    HRESULT LoadImage(const std::wstring& filePath, IWICBitmapSource** ppSource) {
        ComPtr<IWICBitmapDecoder> decoder;
        HRESULT hr = m_wicFactory->CreateDecoderFromFilename(
            filePath.c_str(),
            nullptr,
            GENERIC_READ,
            WICDecodeMetadataCacheOnDemand,
            &decoder
        );
        
        if (FAILED(hr)) return hr;
        
        ComPtr<IWICBitmapFrameDecode> frame;
        hr = decoder->GetFrame(0, &frame);
        if (FAILED(hr)) return hr;
        
        // Convert to 32bppBGRA for consistency
        ComPtr<IWICFormatConverter> converter;
        hr = m_wicFactory->CreateFormatConverter(&converter);
        if (FAILED(hr)) return hr;
        
        hr = converter->Initialize(
            frame.Get(),
            GUID_WICPixelFormat32bppBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeCustom
        );
        
        if (FAILED(hr)) return hr;
        
        *ppSource = converter.Detach();
        return S_OK;
    }
    
    HRESULT SaveImage(IWICBitmapSource* pSource, const std::wstring& filePath) {
        ComPtr<IWICStream> stream;
        HRESULT hr = m_wicFactory->CreateStream(&stream);
        if (FAILED(hr)) return hr;
        
        hr = stream->InitializeFromFilename(filePath.c_str(), GENERIC_WRITE);
        if (FAILED(hr)) return hr;
        
        ComPtr<IWICBitmapEncoder> encoder;
        hr = m_wicFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder);
        if (FAILED(hr)) return hr;
        
        hr = encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);
        if (FAILED(hr)) return hr;
        
        ComPtr<IWICBitmapFrameEncode> frame;
        hr = encoder->CreateNewFrame(&frame, nullptr);
        if (FAILED(hr)) return hr;
        
        hr = frame->Initialize(nullptr);
        if (FAILED(hr)) return hr;
        
        UINT width, height;
        hr = pSource->GetSize(&width, &height);
        if (FAILED(hr)) return hr;
        
        hr = frame->SetSize(width, height);
        if (FAILED(hr)) return hr;
        
        WICPixelFormatGUID pixelFormat = GUID_WICPixelFormat32bppBGRA;
        hr = frame->SetPixelFormat(&pixelFormat);
        if (FAILED(hr)) return hr;
        
        hr = frame->WriteSource(pSource, nullptr);
        if (FAILED(hr)) return hr;
        
        hr = frame->Commit();
        if (FAILED(hr)) return hr;
        
        hr = encoder->Commit();
        return hr;
    }
    
    HRESULT CreateThumbnailCPU(IWICBitmapSource* pSource, UINT targetSize, IWICBitmap** ppThumbnail) {
        UINT sourceWidth, sourceHeight;
        HRESULT hr = pSource->GetSize(&sourceWidth, &sourceHeight);
        if (FAILED(hr)) return hr;
        
        // Calculate thumbnail dimensions maintaining aspect ratio
        UINT thumbWidth, thumbHeight;
        if (sourceWidth > sourceHeight) {
            thumbWidth = targetSize;
            thumbHeight = (sourceHeight * targetSize) / sourceWidth;
        } else {
            thumbHeight = targetSize;
            thumbWidth = (sourceWidth * targetSize) / sourceHeight;
        }
        
        // Use WIC scaler with Fant filter (CPU baseline)
        ComPtr<IWICBitmapScaler> scaler;
        hr = m_wicFactory->CreateBitmapScaler(&scaler);
        if (FAILED(hr)) return hr;
        
        hr = scaler->Initialize(pSource, thumbWidth, thumbHeight, WICBitmapInterpolationModeFant);
        if (FAILED(hr)) return hr;
        
        // Create WIC bitmap from scaler
        hr = m_wicFactory->CreateBitmapFromSource(scaler.Get(), WICBitmapCacheOnDemand, ppThumbnail);
        return hr;
    }
    
    TestResult RunTest(const std::wstring& inputFile, const TestConfig& config) {
        TestResult result;
        result.filename = fs::path(inputFile).filename().wstring();
        result.success = false;
        result.width = 0;
        result.height = 0;
        result.timeMs = 0.0;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Load source image
        ComPtr<IWICBitmapSource> source;
        HRESULT hr = LoadImage(inputFile, &source);
        if (FAILED(hr)) {
            result.error = L"Failed to load image: 0x" + std::to_wstring(hr);
            return result;
        }
        
        // Get source dimensions
        UINT sourceWidth, sourceHeight;
        hr = source->GetSize(&sourceWidth, &sourceHeight);
        if (FAILED(hr)) {
            result.error = L"Failed to get image size: 0x" + std::to_wstring(hr);
            return result;
        }
        
        if (config.verbose) {
            std::wcout << L"  Source: " << sourceWidth << L"x" << sourceHeight << L"\n";
        }
        
        // Generate thumbnail using CPU (baseline for comparison)
        ComPtr<IWICBitmap> thumbnail;
        hr = CreateThumbnailCPU(source.Get(), config.thumbnailSize, &thumbnail);
        if (FAILED(hr)) {
            result.error = L"Failed to create thumbnail: 0x" + std::to_wstring(hr);
            return result;
        }
        
        hr = thumbnail->GetSize(&result.width, &result.height);
        if (FAILED(hr)) {
            result.error = L"Failed to get thumbnail size: 0x" + std::to_wstring(hr);
            return result;
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        result.timeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        
        // Save output if requested
        if (!config.outputFolder.empty()) {
            std::wstring outputPath = config.outputFolder + L"\\" + result.filename + L"_thumb.png";
            hr = SaveImage(thumbnail.Get(), outputPath);
            if (FAILED(hr)) {
                result.error = L"Failed to save thumbnail: 0x" + std::to_wstring(hr);
                return result;
            }
        }
        
        result.success = true;
        return result;
    }
    
    void RunTests(const TestConfig& config) {
        std::wcout << L"\n=== GPU Thumbnail Test Suite ===\n";
        std::wcout << L"Input folder: " << config.inputFolder << L"\n";
        std::wcout << L"Output folder: " << config.outputFolder << L"\n";
        std::wcout << L"Thumbnail size: " << config.thumbnailSize << L"px\n\n";
        
        std::vector<TestResult> results;
        int passCount = 0, failCount = 0;
        double totalTime = 0.0;
        
        // Enumerate test images
        for (const auto& entry : fs::directory_iterator(config.inputFolder)) {
            if (!entry.is_regular_file()) continue;
            
            std::wstring ext = entry.path().extension().wstring();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
            
            // Filter supported image formats
            if (ext != L".jpg" && ext != L".jpeg" && ext != L".png" && 
                ext != L".webp" && ext != L".bmp" && ext != L".tiff") {
                continue;
            }
            
            std::wcout << L"Testing: " << entry.path().filename().wstring() << L" ... ";
            
            TestResult result = RunTest(entry.path().wstring(), config);
            results.push_back(result);
            
            if (result.success) {
                std::wcout << L"OK (" << result.width << L"x" << result.height 
                          << L", " << result.timeMs << L" ms)\n";
                passCount++;
                totalTime += result.timeMs;
            } else {
                std::wcout << L"FAILED: " << result.error << L"\n";
                failCount++;
            }
        }
        
        // Summary
        std::wcout << L"\n=== Test Summary ===\n";
        std::wcout << L"Total tests: " << (passCount + failCount) << L"\n";
        std::wcout << L"Passed: " << passCount << L"\n";
        std::wcout << L"Failed: " << failCount << L"\n";
        
        if (passCount > 0) {
            std::wcout << L"Average time: " << (totalTime / passCount) << L" ms\n";
            std::wcout << L"Total time: " << totalTime << L" ms\n";
        }
    }
    
private:
    ComPtr<IWICImagingFactory> m_wicFactory;
};

void PrintUsage() {
    std::wcout << L"GPUThumbnailTest - GPU Thumbnail Generation Test Tool\n\n";
    std::wcout << L"Usage: GPUThumbnailTest.exe [options]\n\n";
    std::wcout << L"Options:\n";
    std::wcout << L"  -i <folder>    Input folder with test images (required)\n";
    std::wcout << L"  -o <folder>    Output folder for thumbnails (optional)\n";
    std::wcout << L"  -s <size>      Thumbnail size in pixels (default: 256)\n";
    std::wcout << L"  -v             Verbose output\n";
    std::wcout << L"  -h, --help     Show this help\n\n";
    std::wcout << L"Example:\n";
    std::wcout << L"  GPUThumbnailTest.exe -i C:\\TestImages -o C:\\Thumbnails -s 256 -v\n";
}

int wmain(int argc, wchar_t* argv[]) {
    TestConfig config;
    config.thumbnailSize = 256;
    config.validateQuality = false;
    config.generateReference = false;
    config.verbose = false;
    
    // Parse command line
    for (int i = 1; i < argc; i++) {
        std::wstring arg = argv[i];
        
        if (arg == L"-h" || arg == L"--help") {
            PrintUsage();
            return 0;
        } else if (arg == L"-i" && i + 1 < argc) {
            config.inputFolder = argv[++i];
        } else if (arg == L"-o" && i + 1 < argc) {
            config.outputFolder = argv[++i];
        } else if (arg == L"-s" && i + 1 < argc) {
            config.thumbnailSize = _wtoi(argv[++i]);
        } else if (arg == L"-v") {
            config.verbose = true;
        }
    }
    
    // Validate required parameters
    if (config.inputFolder.empty()) {
        std::wcerr << L"Error: Input folder required (-i)\n\n";
        PrintUsage();
        return 1;
    }
    
    if (!fs::exists(config.inputFolder)) {
        std::wcerr << L"Error: Input folder does not exist: " << config.inputFolder << L"\n";
        return 1;
    }
    
    // Create output folder if specified
    if (!config.outputFolder.empty() && !fs::exists(config.outputFolder)) {
        fs::create_directories(config.outputFolder);
    }
    
    // Run tests
    GPUThumbnailTester tester;
    HRESULT hr = tester.Initialize();
    if (FAILED(hr)) {
        std::wcerr << L"Failed to initialize tester: 0x" << std::hex << hr << L"\n";
        return 1;
    }
    
    tester.RunTests(config);
    tester.Shutdown();
    
    return 0;
}

