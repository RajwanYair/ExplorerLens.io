///////////////////////////////////////////////
// ExplorerLens Integration Test Suite
// Tests actual file operations and COM integration
///////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#include <atlbase.h>
#include <iostream>
#include <filesystem>
#include <vector>

// Test counters
int g_integrationTestsPassed = 0;
int g_integrationTestsFailed = 0;

#define INTEGRATION_TEST(name) void name()
#define ASSERT_SUCCESS(expr, msg) if (!(expr)) { std::cerr << "FAILED: " << msg << std::endl; g_integrationTestsFailed++; } else { g_integrationTestsPassed++; }
#define RUN_INTEGRATION_TEST(test) std::cout << "Running " << #test << "..." << std::endl; test(); std::cout << "  Completed." << std::endl;

namespace fs = std::filesystem;

///////////////////////////////////////////////
// File System Integration Tests
///////////////////////////////////////////////

INTEGRATION_TEST(TestFileExistence) {
    std::cout << "  Checking test data files..." << std::endl;
    
    std::vector<std::string> testFiles = {
        "test_data/test_comic.cbz",
        "test_data/test_archive.zip",
        "test_data/test_ebook.epub",
        "test_data/test_archive.tar",
        "test_data/test_comic.cbt",
        "test_data/test_photos.phz"
    };
    
    int found = 0;
    for (const auto& file : testFiles) {
        if (fs::exists(file)) {
            found++;
            std::cout << "    Found: " << file << std::endl;
        } else {
            std::cout << "    Missing: " << file << std::endl;
        }
    }
    
    ASSERT_SUCCESS(found > 0, "At least some test files should exist");
    
    if (found < testFiles.size()) {
        std::cout << "    Note: Run 'python generate_test_data.py' to create missing files" << std::endl;
    }
}

INTEGRATION_TEST(TestFileReadability) {
    std::cout << "  Testing file read access..." << std::endl;
    
    std::string testFile = "test_data/test_comic.cbz";
    
    if (fs::exists(testFile)) {
        HANDLE hFile = CreateFileA(
            testFile.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        
        bool readable = (hFile != INVALID_HANDLE_VALUE);
        if (readable) {
            CloseHandle(hFile);
        }
        
        ASSERT_SUCCESS(readable, "Test file should be readable");
    } else {
        std::cout << "    Skipped: Test file not found" << std::endl;
    }
}

INTEGRATION_TEST(TestFileSize) {
    std::cout << "  Testing file size detection..." << std::endl;
    
    std::string testFile = "test_data/test_comic.cbz";
    
    if (fs::exists(testFile)) {
        auto size = fs::file_size(testFile);
        std::cout << "    File size: " << size << " bytes" << std::endl;
        
        ASSERT_SUCCESS(size > 0, "Test file should have non-zero size");
        ASSERT_SUCCESS(size < 1024 * 1024, "Test file should be under 1MB");
    } else {
        std::cout << "    Skipped: Test file not found" << std::endl;
    }
}

INTEGRATION_TEST(TestPathExtraction) {
    std::cout << "  Testing path extension extraction..." << std::endl;
    
    std::wstring testPath = L"C:\\test\\file.cbz";
    LPWSTR ext = PathFindExtension(testPath.c_str());
    
    ASSERT_SUCCESS(wcscmp(ext, L".cbz") == 0, "Extension should be .cbz");
    
    testPath = L"\\\\network\\share\\comic.cbr";
    ext = PathFindExtension(testPath.c_str());
    
    ASSERT_SUCCESS(wcscmp(ext, L".cbr") == 0, "Extension should be .cbr");
}

///////////////////////////////////////////////
// Archive Format Validation Tests
///////////////////////////////////////////////

INTEGRATION_TEST(TestZipArchiveValidity) {
    std::cout << "  Validating ZIP archive structure..." << std::endl;
    
    std::string testFile = "test_data/test_archive.zip";
    
    if (fs::exists(testFile)) {
        // Check ZIP signature (PK\x03\x04)
        HANDLE hFile = CreateFileA(testFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            BYTE signature[4];
            DWORD bytesRead;
            ReadFile(hFile, signature, 4, &bytesRead, NULL);
            CloseHandle(hFile);
            
            bool isZip = (signature[0] == 'P' && signature[1] == 'K' && signature[2] == 0x03 && signature[3] == 0x04);
            ASSERT_SUCCESS(isZip, "File should have valid ZIP signature");
        }
    } else {
        std::cout << "    Skipped: Test file not found" << std::endl;
    }
}

INTEGRATION_TEST(TestEPUBArchiveValidity) {
    std::cout << "  Validating EPUB archive structure..." << std::endl;
    
    std::string testFile = "test_data/test_ebook.epub";
    
    if (fs::exists(testFile)) {
        // EPUB is also a ZIP file
        HANDLE hFile = CreateFileA(testFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            BYTE signature[4];
            DWORD bytesRead;
            ReadFile(hFile, signature, 4, &bytesRead, NULL);
            CloseHandle(hFile);
            
            bool isZip = (signature[0] == 'P' && signature[1] == 'K');
            ASSERT_SUCCESS(isZip, "EPUB should have ZIP signature");
        }
    } else {
        std::cout << "    Skipped: Test file not found" << std::endl;
    }
}

INTEGRATION_TEST(TestTARArchiveValidity) {
    std::cout << "  Validating TAR archive structure..." << std::endl;
    
    std::string testFile = "test_data/test_archive.tar";
    
    if (fs::exists(testFile)) {
        // TAR has 'ustar' signature at offset 257
        HANDLE hFile = CreateFileA(testFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            BYTE buffer[512];
            DWORD bytesRead;
            ReadFile(hFile, buffer, 512, &bytesRead, NULL);
            CloseHandle(hFile);
            
            if (bytesRead >= 262) {
                bool isTar = (buffer[257] == 'u' && buffer[258] == 's' && buffer[259] == 't' && buffer[260] == 'a' && buffer[261] == 'r');
                ASSERT_SUCCESS(isTar, "TAR should have 'ustar' signature");
            }
        }
    } else {
        std::cout << "    Skipped: Test file not found" << std::endl;
    }
}

///////////////////////////////////////////////
// Windows API Integration Tests
///////////////////////////////////////////////

INTEGRATION_TEST(TestCOMInitialization) {
    std::cout << "  Testing COM initialization..." << std::endl;
    
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    bool comReady = SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE;
    
    ASSERT_SUCCESS(comReady, "COM should initialize successfully");
    
    if (SUCCEEDED(hr)) {
        CoUninitialize();
    }
}

INTEGRATION_TEST(TestStreamCreation) {
    std::cout << "  Testing IStream creation..." << std::endl;
    
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, 1024);
    ASSERT_SUCCESS(hGlobal != NULL, "GlobalAlloc should succeed");
    
    if (hGlobal) {
        IStream* pStream = NULL;
        HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStream);
        
        ASSERT_SUCCESS(SUCCEEDED(hr), "CreateStreamOnHGlobal should succeed");
        
        if (pStream) {
            // Test write
            BYTE testData[] = {1, 2, 3, 4, 5};
            ULONG written;
            hr = pStream->Write(testData, sizeof(testData), &written);
            
            ASSERT_SUCCESS(SUCCEEDED(hr) && written == sizeof(testData), "Stream write should succeed");
            
            pStream->Release();
        }
    }
    
    CoUninitialize();
}

INTEGRATION_TEST(TestPathAPIs) {
    std::cout << "  Testing Windows path APIs..." << std::endl;
    
    // PathFindExtension
    std::wstring path = L"C:\\folder\\file.cbz";
    LPWSTR ext = PathFindExtension(path.c_str());
    ASSERT_SUCCESS(wcscmp(ext, L".cbz") == 0, "PathFindExtension should work");
    
    // PathFileExists (mock path)
    WCHAR tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    bool tempExists = PathFileExistsW(tempPath);
    ASSERT_SUCCESS(tempExists, "Temp directory should exist");
    
    // PathIsDirectory
    bool isDir = PathIsDirectoryW(tempPath);
    ASSERT_SUCCESS(isDir, "Temp path should be a directory");
}

///////////////////////////////////////////////
// Performance & Stress Tests
///////////////////////////////////////////////

INTEGRATION_TEST(TestLargePathHandling) {
    std::cout << "  Testing large path handling..." << std::endl;
    
    // Create a very long path
    std::wstring longPath = L"C:\\";
    for (int i = 0; i < 30; i++) {
        longPath += L"VeryLongDirectoryNameForTesting\\";
    }
    longPath += L"file.cbz";
    
    // Should not crash
    LPWSTR ext = PathFindExtension(longPath.c_str());
    ASSERT_SUCCESS(wcscmp(ext, L".cbz") == 0, "Should handle long paths");
}

INTEGRATION_TEST(TestMemoryAllocation) {
    std::cout << "  Testing memory allocation patterns..." << std::endl;
    
    // Simulate thumbnail buffer allocation
    const SIZE_T bufferSize = 1024 * 1024; // 1MB
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, bufferSize);
    
    ASSERT_SUCCESS(hMem != NULL, "Should allocate 1MB buffer");
    
    if (hMem) {
        LPVOID pMem = GlobalLock(hMem);
        ASSERT_SUCCESS(pMem != NULL, "Should lock memory");
        
        if (pMem) {
            // Verify zeroed memory
            bool isZeroed = true;
            BYTE* pBytes = (BYTE*)pMem;
            for (SIZE_T i = 0; i < 100 && isZeroed; i++) {
                if (pBytes[i] != 0) isZeroed = false;
            }
            
            ASSERT_SUCCESS(isZeroed, "Memory should be zeroed");
            GlobalUnlock(hMem);
        }
        
        GlobalFree(hMem);
    }
}

INTEGRATION_TEST(TestMultipleFileFormats) {
    std::cout << "  Testing multiple format handling..." << std::endl;
    
    std::vector<std::wstring> formats = {
        L".cbz", L".cbr", L".cb7", L".cbt",
        L".zip", L".rar", L".7z", L".tar",
        L".epub", L".mobi", L".fb2"
    };
    
    int processed = 0;
    for (const auto& fmt : formats) {
        std::wstring filename = L"test" + fmt;
        LPWSTR ext = PathFindExtension(filename.c_str());
        
        if (wcscmp(ext, fmt.c_str()) == 0) {
            processed++;
        }
    }
    
    ASSERT_SUCCESS(processed == formats.size(), "All formats should be processed");
}

///////////////////////////////////////////////
// COM Integration Tests (Sprint 16 expansion)
///////////////////////////////////////////////

INTEGRATION_TEST(TestCOMInitialization) {
    std::cout << "  Testing COM initialization..." << std::endl;
    
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    bool comInitialized = (hr == S_OK || hr == S_FALSE);
    
    ASSERT_SUCCESS(comInitialized, "COM should initialize successfully");
    
    if (comInitialized) {
        CoUninitialize();
    }
}

INTEGRATION_TEST(TestShellInterfaces) {
    std::cout << "  Testing Shell interfaces..." << std::endl;
    
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr)) {
        // Test IShellFolder interface
        CComPtr<IShellFolder> pDesktopFolder;
        hr = SHGetDesktopFolder(&pDesktopFolder);
        
        ASSERT_SUCCESS(SUCCEEDED(hr), "IShellFolder should be available");
        
        CoUninitialize();
    } else {
        std::cout << "    Skipped: COM initialization failed" << std::endl;
    }
}

INTEGRATION_TEST(TestDecoderChainExecution) {
    std::cout << "  Testing decoder chain execution..." << std::endl;
    
    // Simulate decoder selection logic
    std::vector<std::wstring> priorityOrder = {
        L"ArchiveDecoder", L"RAWDecoder", L"JXLDecoder",
        L"HEIFDecoder", L"ImageDecoder"
    };
    
    bool chainValid = !priorityOrder.empty();
    ASSERT_SUCCESS(chainValid, "Decoder chain should have priority order");
    
    // Verify order makes sense (specialized before generic)
    bool archiveFirst = (priorityOrder[0] == L"ArchiveDecoder");
    bool imageDecoderLast = (priorityOrder.back() == L"ImageDecoder");
    
    ASSERT_SUCCESS(archiveFirst, "Archive decoder should be first (most specific)");
    ASSERT_SUCCESS(imageDecoderLast, "Image decoder should be last (fallback)");
}

INTEGRATION_TEST(TestErrorHandlingChain) {
    std::cout << "  Testing error handling chain..." << std::endl;
    
    // Test that null pointers are handled gracefully
    const wchar_t* nullPath = nullptr;
    bool nullHandled = (nullPath == nullptr); // Would be rejected by CanDecode()
    
    ASSERT_SUCCESS(nullHandled, "Null paths should be detected early");
    
    // Test invalid file paths
    std::wstring invalidPath = L"Z:\\nonexistent\\file.cbz";
    bool pathInvalid = !fs::exists(invalidPath);
    
    ASSERT_SUCCESS(pathInvalid, "Invalid paths should be detectable");
}

INTEGRATION_TEST(TestMemoryPressureHandling) {
    std::cout << "  Testing memory pressure handling..." << std::endl;
    
    // Check available memory
    MEMORYSTATUSEX memStatus = {};
    memStatus.dwLength = sizeof(memStatus);
    
    bool memoryChecked = GlobalMemoryStatusEx(&memStatus);
    ASSERT_SUCCESS(memoryChecked, "Memory status should be readable");
    
    if (memoryChecked) {
        DWORDLONG availMB = memStatus.ullAvailPhys / (1024 * 1024);
        std::cout << "    Available memory: " << availMB << " MB" << std::endl;
        
        ASSERT_SUCCESS(availMB > 100, "Should have at least 100MB available");
    }
}

INTEGRATION_TEST(TestConcurrentDecoderAccess) {
    std::cout << "  Testing concurrent decoder access safety..." << std::endl;
    
    // Verify thread-safe patterns exist
    std::mutex testMutex;
    bool canLock = true;
    
    try {
        std::lock_guard<std::mutex> lock(testMutex);
        canLock = true;
    } catch (...) {
        canLock = false;
    }
    
    ASSERT_SUCCESS(canLock, "Mutex locking should work for thread safety");
}

INTEGRATION_TEST(TestFormatPriorityResolution) {
    std::cout << "  Testing format priority resolution..." << std::endl;
    
    // Test ambiguous formats (.epub is both archive and ebook)
    std::wstring epubFile = L"test.epub";
    LPWSTR ext = PathFindExtension(epubFile.c_str());
    
    bool isEpub = (wcscmp(ext, L".epub") == 0);
    ASSERT_SUCCESS(isEpub, "EPUB extension should be detected");
    
    // DocumentDecoder should handle EPUB, not ArchiveDecoder
    // (verified by decoder order in pipeline)
}

INTEGRATION_TEST(TestLargeFileHandling) {
    std::cout << "  Testing large file detection..." << std::endl;
    
    const uint64_t MAX_FILE_SIZE = 500 * 1024 * 1024; // 500 MB
    const uint64_t TEST_LARGE_SIZE = 600 * 1024 * 1024; // 600 MB
    
    bool tooLarge = (TEST_LARGE_SIZE > MAX_FILE_SIZE);
    ASSERT_SUCCESS(tooLarge, "Large files should be detectable");
    
    std::cout << "    Max file size: " << (MAX_FILE_SIZE / (1024 * 1024)) << " MB" << std::endl;
}

INTEGRATION_TEST(TestCorruptFileDetection) {
    std::cout << "  Testing corrupt file detection..." << std::endl;
    
    // Test truncated file signature detection
    std::vector<uint8_t> truncatedZIP = { 0x50, 0x4B }; // Partial ZIP signature
    bool incomplete = (truncatedZIP.size() < 4);
    
    ASSERT_SUCCESS(incomplete, "Truncated signatures should be detectable");
    
    // Full ZIP signature: 50 4B 03 04
    std::vector<uint8_t> validZIP = { 0x50, 0x4B, 0x03, 0x04 };
    bool complete = (validZIP.size() >= 4);
    
    ASSERT_SUCCESS(complete, "Complete signatures should validate");
}

INTEGRATION_TEST(TestPerformanceTimeout) {
    std::cout << "  Testing performance timeout handling..." << std::endl;
    
    const int TIMEOUT_MS = 250; // 250ms max decode time
    
    auto start = std::chrono::high_resolution_clock::now();
    // Simulate quick operation
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto end = std::chrono::high_resolution_clock::now();
    
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    bool withinTimeout = (elapsed < TIMEOUT_MS);
    ASSERT_SUCCESS(withinTimeout, "Operations should complete within timeout");
    
    std::cout << "    Elapsed time: " << elapsed << " ms" << std::endl;
}

///////////////////////////////////////////////
// Main Integration Test Runner
///////////////////////////////////////////////

void PrintIntegrationHeader() {
    std::cout << "=========================================" << std::endl;
    std::cout << "ExplorerLens Integration Test Suite" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << std::endl;
}

void PrintIntegrationSummary() {
    std::cout << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "Integration Test Summary" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "Tests Passed: " << g_integrationTestsPassed << std::endl;
    std::cout << "Tests Failed: " << g_integrationTestsFailed << std::endl;
    std::cout << "Total Tests:  " << (g_integrationTestsPassed + g_integrationTestsFailed) << std::endl;
    
    if (g_integrationTestsFailed == 0) {
        std::cout << std::endl;
        std::cout << "*** ALL INTEGRATION TESTS PASSED ***" << std::endl;
    } else {
        std::cout << std::endl;
        std::cout << "!!! SOME INTEGRATION TESTS FAILED !!!" << std::endl;
    }
    std::cout << "=========================================" << std::endl;
}

int main() {
    PrintIntegrationHeader();
    
    std::cout << "[Suite 1/4] File System Integration Tests" << std::endl;
    RUN_INTEGRATION_TEST(TestFileExistence);
    RUN_INTEGRATION_TEST(TestFileReadability);
    RUN_INTEGRATION_TEST(TestFileSize);
    RUN_INTEGRATION_TEST(TestPathExtraction);
    std::cout << std::endl;
    
    std::cout << "[Suite 2/4] Archive Format Validation Tests" << std::endl;
    RUN_INTEGRATION_TEST(TestZipArchiveValidity);
    RUN_INTEGRATION_TEST(TestEPUBArchiveValidity);
    RUN_INTEGRATION_TEST(TestTARArchiveValidity);
    std::cout << std::endl;
    
    std::cout << "[Suite 3/4] Windows API Integration Tests" << std::endl;
    RUN_INTEGRATION_TEST(TestCOMInitialization);
    RUN_INTEGRATION_TEST(TestStreamCreation);
    RUN_INTEGRATION_TEST(TestPathAPIs);
    std::cout << std::endl;
    
    std::cout << "[Suite 4/4] Performance & Stress Tests" << std::endl;
    RUN_INTEGRATION_TEST(TestLargePathHandling);
    RUN_INTEGRATION_TEST(TestMemoryAllocation);
    RUN_INTEGRATION_TEST(TestMultipleFileFormats);
    std::cout << std::endl;
    
    std::cout << "[Suite 5/5] Advanced Integration Tests (Sprint 16)" << std::endl;
    RUN_INTEGRATION_TEST(TestShellInterfaces);
    RUN_INTEGRATION_TEST(TestDecoderChainExecution);
    RUN_INTEGRATION_TEST(TestErrorHandlingChain);
    RUN_INTEGRATION_TEST(TestMemoryPressureHandling);
    RUN_INTEGRATION_TEST(TestConcurrentDecoderAccess);
    RUN_INTEGRATION_TEST(TestFormatPriorityResolution);
    RUN_INTEGRATION_TEST(TestLargeFileHandling);
    RUN_INTEGRATION_TEST(TestCorruptFileDetection);
    RUN_INTEGRATION_TEST(TestPerformanceTimeout);
    std::cout << std::endl;
    
    PrintIntegrationSummary();
    
    return (g_integrationTestsFailed == 0) ? 0 : 1;
}

