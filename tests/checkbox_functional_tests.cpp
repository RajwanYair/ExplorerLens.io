// CBXManager Checkbox Functional Tests
// Tests that each checkbox correctly enables/disables its registry handler
// Compile: msbuild checkbox_tests.vcxproj /p:Configuration=Release /p:Platform=x64

#include <windows.h>
#include <shlobj.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>

using namespace std;

// Test result
struct TestResult {
    wstring testName;
    bool passed;
    wstring message;
};

vector<TestResult> g_testResults;

// Registry check helper
bool CheckRegistryKey(const wstring& keyPath) {
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

// Delete registry key helper
void DeleteRegistryKey(const wstring& keyPath) {
    RegDeleteKey(HKEY_CURRENT_USER, keyPath.c_str());
}

// Find CBXManager window
HWND FindCBXManager() {
    return FindWindow(L"#32770", L"CBX Shell Manager - Thumbnail Configuration");
}

// Click checkbox helper
void ClickCheckbox(HWND hDlg, int checkboxID) {
    HWND hCheck = GetDlgItem(hDlg, checkboxID);
    if (!hCheck) return;
    
    // Simulate click
    SendMessage(hCheck, BM_SETCHECK, BST_CHECKED, 0);
    SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(checkboxID, BN_CLICKED), (LPARAM)hCheck);
}

// Unclick checkbox helper
void UnclickCheckbox(HWND hDlg, int checkboxID) {
    HWND hCheck = GetDlgItem(hDlg, checkboxID);
    if (!hCheck) return;
    
    SendMessage(hCheck, BM_SETCHECK, BST_UNCHECKED, 0);
    SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(checkboxID, BN_CLICKED), (LPARAM)hCheck);
}

// Click Apply button
void ClickApply(HWND hDlg) {
    HWND hApply = GetDlgItem(hDlg, 1002); // IDC_APPLY
    if (hApply) {
        SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(1002, BN_CLICKED), (LPARAM)hApply);
        Sleep(500); // Wait for registry operations
    }
}

// Test structure for checkbox tests
struct CheckboxTest {
    int checkboxID;
    wstring name;
    wstring extension;
    wstring thKeyPath;  // Thumbnail handler key
    wstring ihKeyPath;  // InfoTip handler key
};

// All checkboxes to test
vector<CheckboxTest> GetAllCheckboxTests() {
    return {
        // Comic Book Formats
        {1004, L"CBZ", L".CBZ", L"SOFTWARE\\Classes\\.CBZ\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.CBZ\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1007, L"CBR", L".CBR", L"SOFTWARE\\Classes\\.CBR\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.CBR\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1008, L"CB7", L".CB7", L"SOFTWARE\\Classes\\.CB7\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.CB7\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1009, L"CBT", L".CBT", L"SOFTWARE\\Classes\\.CBT\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.CBT\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        
        // E-Book Formats
        {1006, L"EPUB", L".EPUB", L"SOFTWARE\\Classes\\.EPUB\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.EPUB\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1016, L"MOBI", L".MOBI", L"SOFTWARE\\Classes\\.MOBI\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.MOBI\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1018, L"AZW", L".AZW", L"SOFTWARE\\Classes\\.AZW\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.AZW\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1019, L"AZW3", L".AZW3", L"SOFTWARE\\Classes\\.AZW3\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.AZW3\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        
        // Archive Formats
        {1012, L"ZIP", L".ZIP", L"SOFTWARE\\Classes\\.ZIP\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.ZIP\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1013, L"RAR", L".RAR", L"SOFTWARE\\Classes\\.RAR\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.RAR\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1014, L"7Z", L".7Z", L"SOFTWARE\\Classes\\.7Z\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.7Z\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1015, L"TAR", L".TAR", L"SOFTWARE\\Classes\\.TAR\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.TAR\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        
        // Photo & Other
        {1020, L"PHZ", L".PHZ", L"SOFTWARE\\Classes\\.PHZ\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.PHZ\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1017, L"FB2", L".FB2", L"SOFTWARE\\Classes\\.FB2\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.FB2\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        
        // Modern Image Formats
        {1022, L"WebP", L".WEBP", L"SOFTWARE\\Classes\\.WEBP\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.WEBP\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1023, L"HEIF", L".HEIF", L"SOFTWARE\\Classes\\.HEIF\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.HEIF\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1024, L"AVIF", L".AVIF", L"SOFTWARE\\Classes\\.AVIF\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.AVIF\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1025, L"JXL", L".JXL", L"SOFTWARE\\Classes\\.JXL\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.JXL\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        
        // Media & Documents  
        {1026, L"VIDEO (MP4)", L".MP4", L"SOFTWARE\\Classes\\.MP4\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.MP4\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1027, L"PDF", L".PDF", L"SOFTWARE\\Classes\\.PDF\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.PDF\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1040, L"TIFF", L".TIF", L"SOFTWARE\\Classes\\.TIF\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.TIF\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1041, L"SVG", L".SVG", L"SOFTWARE\\Classes\\.SVG\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.SVG\\shellex\\{00021500-0000-0000-C000-000000000046}"},
        {1043, L"RAW (DNG)", L".DNG", L"SOFTWARE\\Classes\\.DNG\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}", L"SOFTWARE\\Classes\\.DNG\\shellex\\{00021500-0000-0000-C000-000000000046}"}
    };
}

// Test: Enable checkbox and verify registry keys created
TestResult TestCheckboxEnable(HWND hDlg, const CheckboxTest& test) {
    TestResult result;
    result.testName = L"Enable " + test.name;
    
    // Clean up any existing keys
    DeleteRegistryKey(test.thKeyPath);
    DeleteRegistryKey(test.ihKeyPath);
    
    // Uncheck first
    UnclickCheckbox(hDlg, test.checkboxID);
    ClickApply(hDlg);
    
    // Now enable
    ClickCheckbox(hDlg, test.checkboxID);
    ClickApply(hDlg);
    
    // Verify registry keys exist
    bool thExists = CheckRegistryKey(test.thKeyPath);
    bool ihExists = CheckRegistryKey(test.ihKeyPath);
    
    if (thExists && ihExists) {
        result.passed = true;
        result.message = L"Both TH and IH keys created successfully";
    } else if (thExists && !ihExists) {
        result.passed = false;
        result.message = L"TH key created but IH key missing";
    } else if (!thExists && ihExists) {
        result.passed = false;
        result.message = L"IH key created but TH key missing";
    } else {
        result.passed = false;
        result.message = L"Neither registry key was created";
    }
    
    return result;
}

// Test: Disable checkbox and verify registry keys removed
TestResult TestCheckboxDisable(HWND hDlg, const CheckboxTest& test) {
    TestResult result;
    result.testName = L"Disable " + test.name;
    
    // First enable
    ClickCheckbox(hDlg, test.checkboxID);
    ClickApply(hDlg);
    
    // Now disable
    UnclickCheckbox(hDlg, test.checkboxID);
    ClickApply(hDlg);
    
    // Verify registry keys removed
    bool thExists = CheckRegistryKey(test.thKeyPath);
    bool ihExists = CheckRegistryKey(test.ihKeyPath);
    
    if (!thExists && !ihExists) {
        result.passed = true;
        result.message = L"Both keys removed successfully";
    } else if (thExists || ihExists) {
        result.passed = false;
        result.message = L"Registry keys not removed properly";
    } else {
        result.passed = true;
        result.message = L"Keys removed";
    }
    
    return result;
}

// Run all checkbox tests
void RunCheckboxTests(HWND hDlg) {
    wcout << L"\n=== Checkbox Functional Tests ===\n\n";
    wcout << L"Testing that each checkbox correctly manages registry handlers...\n\n";
    
    auto tests = GetAllCheckboxTests();
    int totalTests = tests.size() * 2; // Enable + Disable for each
    int passedTests = 0;
    
    for (const auto& test : tests) {
        // Test enable
        auto enableResult = TestCheckboxEnable(hDlg, test);
        g_testResults.push_back(enableResult);
        if (enableResult.passed) passedTests++;
        
        wcout << (enableResult.passed ? L"[PASS] " : L"[FAIL] ") 
              << enableResult.testName << L"\n";
        if (!enableResult.passed) {
            wcout << L"       " << enableResult.message << L"\n";
        }
        
        // Test disable
        auto disableResult = TestCheckboxDisable(hDlg, test);
        g_testResults.push_back(disableResult);
        if (disableResult.passed) passedTests++;
        
        wcout << (disableResult.passed ? L"[PASS] " : L"[FAIL] ") 
              << disableResult.testName << L"\n";
        if (!disableResult.passed) {
            wcout << L"       " << disableResult.message << L"\n";
        }
        
        wcout << L"\n";
    }
    
    wcout << L"===================================\n";
    wcout << L"Total Tests: " << totalTests << L"\n";
    wcout << L"Passed: " << passedTests << L"\n";
    wcout << L"Failed: " << (totalTests - passedTests) << L"\n";
    
    if (passedTests == totalTests) {
        wcout << L"\n✓ ALL CHECKBOX TESTS PASSED!\n";
    } else {
        wcout << L"\n✗ SOME TESTS FAILED\n";
    }
}

int main() {
    wcout << L"CBXManager Checkbox Functional Test Suite\n";
    wcout << L"==========================================\n\n";
    
    // Find CBXManager window
    wcout << L"Waiting for CBXManager window...\n";
    HWND hDlg = NULL;
    for (int i = 0; i < 100; i++) {
        hDlg = FindCBXManager();
        if (hDlg) break;
        Sleep(100);
    }
    
    if (!hDlg) {
        wcout << L"ERROR: CBXManager window not found!\n";
        wcout << L"Please launch CBXManager.exe first.\n";
        return 1;
    }
    
    wcout << L"CBXManager found!\n\n";
    wcout << L"WARNING: This test will modify registry settings.\n";
    wcout << L"It will enable/disable handlers to verify functionality.\n";
    wcout << L"Press Enter to continue or Ctrl+C to cancel...\n";
    wcin.get();
    
    RunCheckboxTests(hDlg);
    
    wcout << L"\nPress Enter to exit...\n";
    wcin.get();
    
    return 0;
}
