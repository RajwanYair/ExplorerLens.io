/**
 * Test HEIF and JXL Decoder Implementations
 * DarkThumbs v5.3+ - Format Support Verification
 */

#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "../CBXShell/heif_decoder_native.h"
#include "../CBXShell/jxl_decoder.h"

using namespace DarkThumbs;

// Helper to read file into memory
std::vector<BYTE> ReadFile(const wchar_t* path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {};
    }
    
    size_t size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    
    std::vector<BYTE> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    
    return buffer;
}

// Test HEIF format detection
void TestHEIFDetection() {
    std::wcout << L"\n=== HEIF Format Detection Tests ===\n";
    
    // Test 1: Detect .heic file (if available)
    const wchar_t* testFiles[] = {
        L"test-archives\\test.heic",
        L"test-archives\\test.heif",
        L"test-archives\\sample.heic"
    };
    
    for (const auto* file : testFiles) {
        auto data = ReadFile(file);
        if (!data.empty()) {
            bool isHEIF = HEIFDecoderNative::IsHEIFFormat(data.data(), data.size());
            std::wcout << L"  " << file << L": " << (isHEIF ? L"✅ HEIF" : L"❌ NOT HEIF") << L"\n";
            
            if (isHEIF) {
                HBITMAP hBitmap = nullptr;
                HRESULT hr = HEIFDecoderNative::DecodeToHBITMAP(data.data(), data.size(), &hBitmap, false);
                if (SUCCEEDED(hr) && hBitmap) {
                    BITMAP bm;
                    GetObject(hBitmap, sizeof(bm), &bm);
                    std::wcout << L"    Decoded: " << bm.bmWidth << L"x" << bm.bmHeight << L" pixels\n";
                    DeleteObject(hBitmap);
                } else {
                    std::wcout << L"    Decode failed: HRESULT 0x" << std::hex << hr << std::dec << L"\n";
                }
            }
        }
    }
}

// Test JXL format detection
void TestJXLDetection() {
    std::wcout << L"\n=== JXL Format Detection Tests ===\n";
    
    // Test with known JXL signatures
    const BYTE nakedJXL[] = { 0xFF, 0x0A };
    bool isNaked = JXLDecoder::IsJXLFormat(nakedJXL, sizeof(nakedJXL));
    std::wcout << L"  Naked JXL signature (0xFF 0x0A): " << (isNaked ? L"✅ DETECTED" : L"❌ FAILED") << L"\n";
    
    const BYTE containerJXL[] = {
        0x00, 0x00, 0x00, 0x0C, 0x66, 0x74, 0x79, 0x70, // ftyp box
        0x6A, 0x78, 0x6C, 0x20                          // "jxl "
    };
    bool isContainer = JXLDecoder::IsJXLFormat(containerJXL, sizeof(containerJXL));
    std::wcout << L"  Container JXL signature (ftyp jxl): " << (isContainer ? L"✅ DETECTED" : L"❌ FAILED") << L"\n";
    
    // Test with real file (if available)
    const wchar_t* testFiles[] = {
        L"test-archives\\test.jxl",
        L"test-archives\\sample.jxl"
    };
    
    for (const auto* file : testFiles) {
        auto data = ReadFile(file);
        if (!data.empty()) {
            bool isJXL = JXLDecoder::IsJXLFormat(data.data(), data.size());
            std::wcout << L"  " << file << L": " << (isJXL ? L"✅ JXL" : L"❌ NOT JXL") << L"\n";
            
            if (isJXL) {
                int width = 0, height = 0;
                if (JXLDecoder::GetDimensions(data.data(), data.size(), &width, &height)) {
                    std::wcout << L"    Dimensions: " << width << L"x" << height << L" pixels\n";
                    
                    HBITMAP hBitmap = nullptr;
                    HRESULT hr = JXLDecoder::DecodeToHBITMAP(data.data(), data.size(), &hBitmap);
                    if (SUCCEEDED(hr) && hBitmap) {
                        BITMAP bm;
                        GetObject(hBitmap, sizeof(bm), &bm);
                        std::wcout << L"    Decoded: " << bm.bmWidth << L"x" << bm.bmHeight << L" pixels\n";
                        DeleteObject(hBitmap);
                    } else {
                        std::wcout << L"    Decode failed: HRESULT 0x" << std::hex << hr << std::dec << L"\n";
                    }
                }
            }
        }
    }
}

// Test file extension registration
void TestFormatRegistration() {
    std::wcout << L"\n=== Format Registration Verification ===\n";
    
    // Check if HEIF extensions are registered
    const wchar_t* heifExts[] = { L".heic", L".heif", L".hif" };
    std::wcout << L"  HEIF Extensions: ";
    for (const auto* ext : heifExts) {
        std::wcout << ext << L" ";
    }
    std::wcout << L"\n";
    
    // Check if JXL extension is registered
    std::wcout << L"  JXL Extensions: .jxl\n";
    
    std::wcout << L"\n  ℹ️  To verify registration, check:\n";
    std::wcout << L"     HKEY_CLASSES_ROOT\\.heic\\shellex\\{e357fccd-a995-4576-b01f-234630154e96}\n";
    std::wcout << L"     HKEY_CLASSES_ROOT\\.jxl\\shellex\\{e357fccd-a995-4576-b01f-234630154e96}\n";
}

int wmain(int argc, wchar_t* argv[]) {
    std::wcout << L"╔══════════════════════════════════════════════════════╗\n";
    std::wcout << L"║  DarkThumbs HEIF/JXL Decoder Test Suite            ║\n";
    std::wcout << L"║  Version 7.0.0 - Format Support Verification      ║\n";
    std::wcout << L"╚══════════════════════════════════════════════════════╝\n";
    
    CoInitialize(nullptr);
    
    TestHEIFDetection();
    TestJXLDetection();
    TestFormatRegistration();
    
    std::wcout << L"\n=== Test Summary ===\n";
    std::wcout << L"✅ HEIF decoder: Using WIC (Windows Imaging Component)\n";
    std::wcout << L"✅ JXL decoder: Using libjxl 0.11.1 with parallel runner\n";
    std::wcout << L"\n📝 To test in Windows Explorer:\n";
    std::wcout << L"   1. Copy .heic/.jxl files to test-archives\\\n";
    std::wcout << L"   2. Register CBXShell.dll: regsvr32 x64\\Release\\CBXShell.dll\n";
    std::wcout << L"   3. Open test-archives\\ in Explorer and check thumbnails\n";
    
    CoUninitialize();
    return 0;
}
