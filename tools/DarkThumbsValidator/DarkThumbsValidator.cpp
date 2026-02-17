// DarkThumbs.Validator.exe
// Sprint 13: Real-File Test Fixtures & Compatibility Kit
// Validates decoder functionality across the test corpus
// Date: February 17, 2026

#include <windows.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

// Forward declare thumbnail engine functions
extern "C" {
    typedef HRESULT(__stdcall* PFN_CreateThumbnail)(const wchar_t* filePath, UINT width, UINT height, HBITMAP* phBitmap);
}

struct ValidationResult {
    std::wstring filePath;
    std::wstring format;
    bool success = false;
    double timeMs = 0.0;
    size_t fileSize = 0;
    size_t peakMemoryBytes = 0;
    std::wstring errorMessage;
};

struct ValidationStats {
    int totalFiles = 0;
    int successCount = 0;
    int failureCount = 0;
    int crashCount = 0;
    double totalTimeMs = 0.0;
    size_t totalMemoryBytes = 0;
};

class DarkThumbsValidator {
private:
    HMODULE m_engineDll = nullptr;
    PFN_CreateThumbnail m_createThumbnail = nullptr;
    std::vector<ValidationResult> m_results;
    ValidationStats m_stats;
    bool m_verbose = false;
    
public:
    DarkThumbsValidator(bool verbose = false) : m_verbose(verbose) {}
    
    ~DarkThumbsValidator() {
        if (m_engineDll) {
            FreeLibrary(m_engineDll);
        }
    }
    
    bool Initialize() {
        // Try to load the engine DLL
        std::wstring enginePath = L"DarkThumbsEngine.dll";
        
        // Check in standard locations
        std::vector<std::wstring> searchPaths = {
            L"..\\x64\\Release\\DarkThumbsEngine.dll",
            L"..\\..\\x64\\Release\\DarkThumbsEngine.dll",
            L"..\\Engine\\Release\\Release\\DarkThumbsEngine.dll",
            L"DarkThumbsEngine.dll"
        };
        
        for (const auto& path : searchPaths) {
            if (fs::exists(path)) {
                enginePath = path;
                break;
            }
        }
        
        m_engineDll = LoadLibraryW(enginePath.c_str());
        if (!m_engineDll) {
            std::wcerr << L"Failed to load engine DLL: " << enginePath << L" (Error: " << GetLastError() << L")" << std::endl;
            return false;
        }
        
        m_createThumbnail = reinterpret_cast<PFN_CreateThumbnail>(
            GetProcAddress(m_engineDll, "CreateThumbnail"));
        
        if (!m_createThumbnail) {
            std::wcerr << L"Failed to find CreateThumbnail export" << std::endl;
            return false;
        }
        
        if (m_verbose) {
            std::wcout << L"✓ Engine loaded: " << enginePath << std::endl;
        }
        
        return true;
    }
    
    ValidationResult ValidateFile(const std::wstring& filePath) {
        ValidationResult result;
        result.filePath = filePath;
        result.fileSize = fs::file_size(filePath);
        result.format = fs::path(filePath).extension().wstring();
        
        // Measure memory before
        PROCESS_MEMORY_COUNTERS_EX pmc = {};
        pmc.cb = sizeof(pmc);
        GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        size_t memBefore = pmc.PrivateUsage;
        
        // Measure time
        auto start = std::chrono::high_resolution_clock::now();
        
        __try {
            HBITMAP hBitmap = nullptr;
            HRESULT hr = m_createThumbnail(filePath.c_str(), 256, 256, &hBitmap);
            
            auto end = std::chrono::high_resolution_clock::now();
            result.timeMs = std::chrono::duration<double, std::milli>(end - start).count();
            
            if (SUCCEEDED(hr) && hBitmap) {
                result.success = true;
                DeleteObject(hBitmap);
            } else {
                result.success = false;
                result.errorMessage = L"CreateThumbnail returned failure HRESULT: 0x" + 
                    std::to_wstring(hr);
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            result.success = false;
            result.errorMessage = L"CRASH: Exception code 0x" + 
                std::to_wstring(GetExceptionCode());
            m_stats.crashCount++;
        }
        
        // Measure memory after
        GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        size_t memAfter = pmc.PrivateUsage;
        result.peakMemoryBytes = (memAfter > memBefore) ? (memAfter - memBefore) : 0;
        
        return result;
    }
    
    void ValidateDirectory(const std::wstring& dirPath, bool recursive = true) {
        if (!fs::exists(dirPath)) {
            std::wcerr << L"Directory not found: " << dirPath << std::endl;
            return;
        }
        
        std::wcout << L"\n=== Validating: " << dirPath << L" ===" << std::endl;
        
        auto iterator = recursive ? 
            fs::recursive_directory_iterator(dirPath) : 
            fs::directory_iterator(dirPath);
        
        for (const auto& entry : iterator) {
            if (!entry.is_regular_file()) continue;
            
            auto ext = entry.path().extension().wstring();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            
            // Skip non-image files
            if (ext.empty() || ext == L".md" || ext == L".txt") continue;
            
            m_stats.totalFiles++;
            auto result = ValidateFile(entry.path().wstring());
            m_results.push_back(result);
            
            if (result.success) {
                m_stats.successCount++;
                m_stats.totalTimeMs += result.timeMs;
                m_stats.totalMemoryBytes += result.peakMemoryBytes;
                
                if (m_verbose) {
                    std::wcout << L"✓ " << entry.path().filename().wstring() 
                              << L" (" << result.timeMs << L" ms)" << std::endl;
                }
            } else {
                m_stats.failureCount++;
                std::wcerr << L"✗ " << entry.path().filename().wstring() 
                          << L": " << result.errorMessage << std::endl;
            }
        }
    }
    
    void PrintSummary() {
        std::wcout << L"\n==================================" << std::endl;
        std::wcout << L"        VALIDATION SUMMARY        " << std::endl;
        std::wcout << L"==================================" << std::endl;
        std::wcout << L"Total Files:     " << m_stats.totalFiles << std::endl;
        std::wcout << L"Success:         " << m_stats.successCount 
                  << L" (" << (m_stats.totalFiles > 0 ? 
                     (100.0 * m_stats.successCount / m_stats.totalFiles) : 0.0) 
                  << L"%)" << std::endl;
        std::wcout << L"Failures:        " << m_stats.failureCount << std::endl;
        std::wcout << L"Crashes:         " << m_stats.crashCount << std::endl;
        
        if (m_stats.successCount > 0) {
            std::wcout << L"Avg Time:        " << (m_stats.totalTimeMs / m_stats.successCount) 
                      << L" ms" << std::endl;
            std::wcout << L"Avg Memory:      " << (m_stats.totalMemoryBytes / m_stats.successCount / 1024) 
                      << L" KB" << std::endl;
        }
        
        std::wcout << L"==================================" << std::endl;
    }
    
    bool ExportResults(const std::wstring& outputPath) {
        std::wofstream outFile(outputPath);
        if (!outFile.is_open()) {
            std::wcerr << L"Failed to open output file: " << outputPath << std::endl;
            return false;
        }
        
        // Write header
        outFile << L"FilePath,Format,Success,TimeMs,FileSizeBytes,MemoryBytes,ErrorMessage\n";
        
        // Write results
        for (const auto& result : m_results) {
            outFile << result.filePath << L","
                   << result.format << L","
                   << (result.success ? L"TRUE" : L"FALSE") << L","
                   << result.timeMs << L","
                   << result.fileSize << L","
                   << result.peakMemoryBytes << L","
                   << result.errorMessage << L"\n";
        }
        
        outFile.close();
        std::wcout << L"Results exported to: " << outputPath << std::endl;
        return true;
    }
    
    int GetExitCode() const {
        return (m_stats.failureCount > 0 || m_stats.crashCount > 0) ? 1 : 0;
    }
};

void PrintUsage() {
    std::wcout << L"DarkThumbs Validator v1.0\n"
              << L"Usage: DarkThumbsValidator.exe [options] <path>\n\n"
              << L"Options:\n"
              << L"  -v, --verbose        Verbose output (show all files)\n"
              << L"  -r, --recursive      Scan directories recursively (default)\n"
              << L"  -o, --output <file>  Export results to CSV file\n"
              << L"  -h, --help          Show this help message\n\n"
              << L"Examples:\n"
              << L"  DarkThumbsValidator.exe tests\\data\\corpus\n"
              << L"  DarkThumbsValidator.exe -v -o results.csv tests\\data\\corpus\n"
              << std::endl;
}

int wmain(int argc, wchar_t* argv[]) {
    if (argc < 2) {
        PrintUsage();
        return 1;
    }
    
    bool verbose = false;
    bool recursive = true;
    std::wstring outputPath;
    std::wstring targetPath;
    
    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::wstring arg = argv[i];
        
        if (arg == L"-v" || arg == L"--verbose") {
            verbose = true;
        }
        else if (arg == L"-r" || arg == L"--recursive") {
            recursive = true;
        }
        else if (arg == L"-o" || arg == L"--output") {
            if (i + 1 < argc) {
                outputPath = argv[++i];
            }
        }
        else if (arg == L"-h" || arg == L"--help") {
            PrintUsage();
            return 0;
        }
        else {
            targetPath = arg;
        }
    }
    
    if (targetPath.empty()) {
        std::wcerr << L"Error: No target path specified\n" << std::endl;
        PrintUsage();
        return 1;
    }
    
    // Initialize validator
    DarkThumbsValidator validator(verbose);
    
    if (!validator.Initialize()) {
        std::wcerr << L"Failed to initialize validator" << std::endl;
        return 1;
    }
    
    // Run validation
    validator.ValidateDirectory(targetPath, recursive);
    
    // Print summary
    validator.PrintSummary();
    
    // Export results if requested
    if (!outputPath.empty()) {
        validator.ExportResults(outputPath);
    }
    
    return validator.GetExitCode();
}
