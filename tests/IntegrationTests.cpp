///////////////////////////////////////////////
// DarkThumbs Integration Test Suite
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
// Main Integration Test Runner
///////////////////////////////////////////////

void PrintIntegrationHeader() {
    std::cout << "=========================================" << std::endl;
    std::cout << "DarkThumbs Integration Test Suite" << std::endl;
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
    
    PrintIntegrationSummary();
    
    return (g_integrationTestsFailed == 0) ? 0 : 1;
}
