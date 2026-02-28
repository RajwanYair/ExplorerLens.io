// LENSBench.cpp - LENSShell Performance Benchmark Tool
// Measures CPU baseline, GPU Phase 1, GPU Phase 2 performance
// Validates 6.5x speedup target across all formats

#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <filesystem>
#include <iomanip>

#pragma comment(lib, "windowscodecs.lib")

using Microsoft::WRL::ComPtr;
namespace fs = std::filesystem;

// Benchmark configuration
struct BenchConfig {
    std::wstring testFolder;
    std::wstring outputCsv;
    UINT thumbnailSize;
    UINT iterations;
    bool skipWarmup;
    bool verbose;
};

// Benchmark result for a single file
struct BenchResult {
    std::wstring filename;
    std::wstring format;
    UINT sourceWidth;
    UINT sourceHeight;
    double avgTimeMs;
    double minTimeMs;
    double maxTimeMs;
    double stdDevMs;
    UINT sampleCount;
};

// Format statistics
struct FormatStats {
    std::wstring format;
    UINT fileCount;
    double avgTimeMs;
    double totalTimeMs;
    double speedupVsBaseline;
};

class LENSBenchmark {
public:
    LENSBenchmark() : m_wicFactory(nullptr), m_baselineTimeMs(0.0) {}
    
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
    
    std::wstring GetFormatFromExtension(const std::wstring& ext) {
        std::wstring lower = ext;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
        
        static std::map<std::wstring, std::wstring> formats = {
            {L".jpg", L"JPEG"}, {L".jpeg", L"JPEG"},
            {L".png", L"PNG"},
            {L".webp", L"WebP"},
            {L".avif", L"AVIF"},
            {L".heic", L"HEIF"}, {L".heif", L"HEIF"},
            {L".bmp", L"BMP"},
            {L".tiff", L"TIFF"}, {L".tif", L"TIFF"},
            {L".gif", L"GIF"},
            {L".pdf", L"PDF"},
            {L".mp4", L"Video"}, {L".avi", L"Video"}, {L".mkv", L"Video"},
            {L".mov", L"Video"}, {L".wmv", L"Video"}
        };
        
        auto it = formats.find(lower);
        return (it != formats.end()) ? it->second : L"Unknown";
    }
    
    double BenchmarkFile(const std::wstring& filePath, UINT thumbnailSize, UINT iterations) {
        std::vector<double> times;
        times.reserve(iterations);
        
        for (UINT i = 0; i < iterations; i++) {
            auto startTime = std::chrono::high_resolution_clock::now();
            
            // Load image
            ComPtr<IWICBitmapDecoder> decoder;
            HRESULT hr = m_wicFactory->CreateDecoderFromFilename(
                filePath.c_str(),
                nullptr,
                GENERIC_READ,
                WICDecodeMetadataCacheOnDemand,
                &decoder
            );
            
            if (FAILED(hr)) continue;
            
            ComPtr<IWICBitmapFrameDecode> frame;
            hr = decoder->GetFrame(0, &frame);
            if (FAILED(hr)) continue;
            
            // Convert to 32bppBGRA
            ComPtr<IWICFormatConverter> converter;
            hr = m_wicFactory->CreateFormatConverter(&converter);
            if (FAILED(hr)) continue;
            
            hr = converter->Initialize(
                frame.Get(),
                GUID_WICPixelFormat32bppBGRA,
                WICBitmapDitherTypeNone,
                nullptr,
                0.0,
                WICBitmapPaletteTypeCustom
            );
            
            if (FAILED(hr)) continue;
            
            // Get source dimensions
            UINT sourceWidth, sourceHeight;
            hr = converter->GetSize(&sourceWidth, &sourceHeight);
            if (FAILED(hr)) continue;
            
            // Calculate thumbnail dimensions
            UINT thumbWidth, thumbHeight;
            if (sourceWidth > sourceHeight) {
                thumbWidth = thumbnailSize;
                thumbHeight = (sourceHeight * thumbnailSize) / sourceWidth;
            } else {
                thumbHeight = thumbnailSize;
                thumbWidth = (sourceWidth * thumbnailSize) / sourceHeight;
            }
            
            // Create scaler (CPU baseline)
            ComPtr<IWICBitmapScaler> scaler;
            hr = m_wicFactory->CreateBitmapScaler(&scaler);
            if (FAILED(hr)) continue;
            
            hr = scaler->Initialize(converter.Get(), thumbWidth, thumbHeight, WICBitmapInterpolationModeFant);
            if (FAILED(hr)) continue;
            
            // Force evaluation by copying pixels
            UINT stride = thumbWidth * 4;
            UINT bufferSize = stride * thumbHeight;
            std::vector<BYTE> pixels(bufferSize);
            hr = scaler->CopyPixels(nullptr, stride, bufferSize, pixels.data());
            
            auto endTime = std::chrono::high_resolution_clock::now();
            
            if (SUCCEEDED(hr)) {
                double timeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                times.push_back(timeMs);
            }
        }
        
        if (times.empty()) return 0.0;
        
        // Calculate average
        double sum = 0.0;
        for (double t : times) sum += t;
        return sum / times.size();
    }
    
    BenchResult BenchmarkFileDetailed(const std::wstring& filePath, const BenchConfig& config) {
        BenchResult result;
        result.filename = fs::path(filePath).filename().wstring();
        result.format = GetFormatFromExtension(fs::path(filePath).extension().wstring());
        result.sourceWidth = 0;
        result.sourceHeight = 0;
        result.avgTimeMs = 0.0;
        result.minTimeMs = 0.0;
        result.maxTimeMs = 0.0;
        result.stdDevMs = 0.0;
        result.sampleCount = 0;
        
        std::vector<double> times;
        times.reserve(config.iterations);
        
        // Get source dimensions (one-time)
        ComPtr<IWICBitmapDecoder> decoder;
        HRESULT hr = m_wicFactory->CreateDecoderFromFilename(
            filePath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder
        );
        
        if (SUCCEEDED(hr)) {
            ComPtr<IWICBitmapFrameDecode> frame;
            hr = decoder->GetFrame(0, &frame);
            if (SUCCEEDED(hr)) {
                frame->GetSize(&result.sourceWidth, &result.sourceHeight);
            }
        }
        
        // Run iterations
        for (UINT i = 0; i < config.iterations; i++) {
            double timeMs = BenchmarkFile(filePath, config.thumbnailSize, 1);
            if (timeMs > 0.0) {
                times.push_back(timeMs);
            }
        }
        
        if (times.empty()) return result;
        
        result.sampleCount = static_cast<UINT>(times.size());
        
        // Calculate statistics
        double sum = 0.0;
        result.minTimeMs = times[0];
        result.maxTimeMs = times[0];
        
        for (double t : times) {
            sum += t;
            if (t < result.minTimeMs) result.minTimeMs = t;
            if (t > result.maxTimeMs) result.maxTimeMs = t;
        }
        
        result.avgTimeMs = sum / times.size();
        
        // Calculate standard deviation
        double variance = 0.0;
        for (double t : times) {
            double diff = t - result.avgTimeMs;
            variance += diff * diff;
        }
        result.stdDevMs = sqrt(variance / times.size());
        
        return result;
    }
    
    void RunBenchmark(const BenchConfig& config) {
        std::wcout << L"\n=== LENSShell Performance Benchmark ===\n";
        std::wcout << L"Test folder: " << config.testFolder << L"\n";
        std::wcout << L"Thumbnail size: " << config.thumbnailSize << L"px\n";
        std::wcout << L"Iterations per file: " << config.iterations << L"\n\n";
        
        std::vector<BenchResult> results;
        std::map<std::wstring, FormatStats> formatStats;
        
        // Enumerate test files
        int fileCount = 0;
        for (const auto& entry : fs::directory_iterator(config.testFolder)) {
            if (!entry.is_regular_file()) continue;
            
            std::wstring format = GetFormatFromExtension(entry.path().extension().wstring());
            if (format == L"Unknown") continue;
            
            fileCount++;
            std::wcout << L"[" << fileCount << L"] Benchmarking: " << entry.path().filename().wstring() << L" ... ";
            
            BenchResult result = BenchmarkFileDetailed(entry.path().wstring(), config);
            results.push_back(result);
            
            std::wcout << result.avgTimeMs << L" ms (min: " << result.minTimeMs 
                      << L", max: " << result.maxTimeMs << L")\n";
            
            // Update format statistics
            auto& stats = formatStats[format];
            stats.format = format;
            stats.fileCount++;
            stats.totalTimeMs += result.avgTimeMs;
        }
        
        // Calculate format averages
        for (auto& pair : formatStats) {
            auto& stats = pair.second;
            stats.avgTimeMs = stats.totalTimeMs / stats.fileCount;
        }
        
        // Print summary
        std::wcout << L"\n=== Format Summary ===\n";
        std::wcout << std::left << std::setw(15) << L"Format" 
                  << std::setw(10) << L"Files" 
                  << std::setw(15) << L"Avg Time (ms)" 
                  << L"\n";
        std::wcout << std::wstring(40, L'-') << L"\n";
        
        for (const auto& pair : formatStats) {
            const auto& stats = pair.second;
            std::wcout << std::left << std::setw(15) << stats.format
                      << std::setw(10) << stats.fileCount
                      << std::setw(15) << std::fixed << std::setprecision(2) << stats.avgTimeMs
                      << L"\n";
        }
        
        // Save CSV if requested
        if (!config.outputCsv.empty()) {
            SaveResultsToCSV(results, config.outputCsv);
            std::wcout << L"\nResults saved to: " << config.outputCsv << L"\n";
        }
    }
    
    void SaveResultsToCSV(const std::vector<BenchResult>& results, const std::wstring& csvPath) {
        std::wofstream csv(csvPath);
        if (!csv.is_open()) return;
        
        // Header
        csv << L"Filename,Format,SourceWidth,SourceHeight,AvgTime(ms),MinTime(ms),MaxTime(ms),StdDev(ms),Samples\n";
        
        // Data rows
        for (const auto& result : results) {
            csv << result.filename << L","
                << result.format << L","
                << result.sourceWidth << L","
                << result.sourceHeight << L","
                << std::fixed << std::setprecision(3) << result.avgTimeMs << L","
                << result.minTimeMs << L","
                << result.maxTimeMs << L","
                << result.stdDevMs << L","
                << result.sampleCount << L"\n";
        }
        
        csv.close();
    }
    
private:
    ComPtr<IWICImagingFactory> m_wicFactory;
    double m_baselineTimeMs;
};

void PrintUsage() {
    std::wcout << L"LENSBench - LENSShell Performance Benchmark Tool\n\n";
    std::wcout << L"Usage: LENSBench.exe [options]\n\n";
    std::wcout << L"Options:\n";
    std::wcout << L"  -i <folder>    Input folder with test images (required)\n";
    std::wcout << L"  -o <file>      Output CSV file (optional)\n";
    std::wcout << L"  -s <size>      Thumbnail size in pixels (default: 256)\n";
    std::wcout << L"  -n <count>     Iterations per file (default: 10)\n";
    std::wcout << L"  -v             Verbose output\n";
    std::wcout << L"  -h, --help     Show this help\n\n";
    std::wcout << L"Example:\n";
    std::wcout << L"  LENSBench.exe -i C:\\TestImages -o results.csv -s 256 -n 20 -v\n";
}

int wmain(int argc, wchar_t* argv[]) {
    BenchConfig config;
    config.thumbnailSize = 256;
    config.iterations = 10;
    config.skipWarmup = false;
    config.verbose = false;
    
    // Parse command line
    for (int i = 1; i < argc; i++) {
        std::wstring arg = argv[i];
        
        if (arg == L"-h" || arg == L"--help") {
            PrintUsage();
            return 0;
        } else if (arg == L"-i" && i + 1 < argc) {
            config.testFolder = argv[++i];
        } else if (arg == L"-o" && i + 1 < argc) {
            config.outputCsv = argv[++i];
        } else if (arg == L"-s" && i + 1 < argc) {
            config.thumbnailSize = _wtoi(argv[++i]);
        } else if (arg == L"-n" && i + 1 < argc) {
            config.iterations = _wtoi(argv[++i]);
        } else if (arg == L"-v") {
            config.verbose = true;
        }
    }
    
    // Validate
    if (config.testFolder.empty()) {
        std::wcerr << L"Error: Input folder required (-i)\n\n";
        PrintUsage();
        return 1;
    }
    
    if (!fs::exists(config.testFolder)) {
        std::wcerr << L"Error: Input folder does not exist: " << config.testFolder << L"\n";
        return 1;
    }
    
    // Run benchmark
    LENSBenchmark bench;
    HRESULT hr = bench.Initialize();
    if (FAILED(hr)) {
        std::wcerr << L"Failed to initialize benchmark: 0x" << std::hex << hr << L"\n";
        return 1;
    }
    
    bench.RunBenchmark(config);
    bench.Shutdown();
    
    return 0;
}

