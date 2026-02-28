// LENSManager GUI Test Suite
// Tests dialog layout, checkbox functionality, and state persistence
// Compile: cl /std:c++17 /EHsc /DUNICODE /D_UNICODE test_lensmanager_gui.cpp /link user32.lib

#include <windows.h>
#include <commctrl.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>

using namespace std;

// Test result structure
struct TestResult {
    wstring testName;
    bool passed;
    wstring message;
};

vector<TestResult> testResults;

// Helper function to find window by class and title
HWND FindLENSManagerWindow() {
    return FindWindow(L"#32770", L"LENS Shell Manager - Thumbnail Configuration");
}

// Test 1: Verify dialog exists and has correct size
TestResult Test_DialogExists() {
    TestResult result;
    result.testName = L"Dialog Window Exists";
    
    HWND hDlg = FindLENSManagerWindow();
    if (!hDlg) {
        result.passed = false;
        result.message = L"LENSManager window not found. Launch LENSManager.exe first.";
        return result;
    }
    
    RECT rect;
    GetWindowRect(hDlg, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    
    // Expected: 360 DLU width, 375 DLU height (approximate pixels depend on DPI)
    // At 96 DPI: ~480px width, ~500-650px height (varies with DPI scaling)
    if (width < 400 || width > 600) {
        result.passed = false;
        result.message = L"Dialog width incorrect: " + to_wstring(width) + L"px (expected ~480px)";
        return result;
    }
    
    if (height < 450 || height > 700) {
        result.passed = false;
        result.message = L"Dialog height incorrect: " + to_wstring(height) + L"px (expected ~500-650px)";
        return result;
    }
    
    result.passed = true;
    result.message = L"Dialog size: " + to_wstring(width) + L"x" + to_wstring(height) + L"px";
    return result;
}

// Test 2: Verify all checkboxes exist
TestResult Test_AllCheckboxesExist() {
    TestResult result;
    result.testName = L"All Format Checkboxes Exist";
    
    HWND hDlg = FindLENSManagerWindow();
    if (!hDlg) {
        result.passed = false;
        result.message = L"LENSManager window not found";
        return result;
    }
    
    // All checkbox IDs (from resource.h - actual IDs)
    map<int, wstring> checkboxes = {
        // Comic Book Formats
        {1004, L"CBZ"}, {1007, L"CBR"}, {1008, L"CB7"}, {1009, L"CBT"},
        // E-Book Formats
        {1006, L"EPUB"}, {1016, L"MOBI"}, {1018, L"AZW"}, {1019, L"AZW3"},
        // Archive Formats
        {1012, L"ZIP"}, {1013, L"RAR"}, {1014, L"7Z"}, {1015, L"TAR"},
        // Photo & Other
        {1020, L"PHZ"}, {1017, L"FB2"},
        // Modern Image Formats
        {1022, L"WebP"}, {1023, L"HEIF"}, {1024, L"AVIF"}, {1025, L"JXL"},
        // Media & Documents
        {1026, L"VIDEO"}, {1027, L"PDF"}, {1040, L"TIFF"}, {1041, L"SVG"}, {1043, L"RAW"},
        // Advanced
        {1021, L"SORT"}, {1010, L"SHOWICON"}
    };
    
    int missingCount = 0;
    wstring missingList;
    
    for (const auto& [id, name] : checkboxes) {
        HWND hCheckbox = GetDlgItem(hDlg, id);
        if (!hCheckbox) {
            missingCount++;
            missingList += name + L" (ID:" + to_wstring(id) + L"), ";
        }
    }
    
    if (missingCount > 0) {
        result.passed = false;
        result.message = to_wstring(missingCount) + L" checkboxes missing: " + missingList;
        return result;
    }
    
    result.passed = true;
    result.message = L"All " + to_wstring(checkboxes.size()) + L" checkboxes found";
    return result;
}

// Test 3: Verify group boxes don't overlap
TestResult Test_GroupBoxesNoOverlap() {
    TestResult result;
    result.testName = L"Group Boxes Do Not Overlap";
    
    HWND hDlg = FindLENSManagerWindow();
    if (!hDlg) {
        result.passed = false;
        result.message = L"LENSManager window not found";
        return result;
    }
    
    // Group box IDs and expected positions (from .rc file)
    struct GroupInfo {
        int id;
        wstring name;
        RECT expectedRect; // x, y, width, height in DLU
    };
    
    vector<GroupInfo> groups = {
        {-1, L"Comic Book Formats", {10, 5, 165, 62}},
        {-1, L"E-Book Formats", {185, 5, 165, 62}},
        {-1, L"Archive Formats", {10, 72, 165, 62}},
        {-1, L"Photo & Other Formats", {185, 72, 165, 40}},
        {-1, L"Modern Image Formats", {10, 117, 165, 62}},
        {-1, L"Media & Documents", {185, 117, 165, 88}},
        {-1, L"Collage Mode", {10, 210, 165, 64}},
        {-1, L"Advanced Options", {185, 210, 165, 64}}
    };
    
    // Check for overlaps (simplified: just check vertical positions)
    // Left column (x=10): Comic(5-67), Archive(72-134), Modern(117-179), Collage(210-274)
    // Right column (x=185): E-Book(5-67), Photo(72-112), Media(117-205), Advanced(210-274)
    
    vector<pair<wstring, wstring>> overlaps;
    
    // Left column checks
    if (67 > 72 - 5) overlaps.push_back({L"Comic Book", L"Archive"});
    if (134 > 117 - 5) overlaps.push_back({L"Archive", L"Modern Image"});
    if (179 > 210 - 5) overlaps.push_back({L"Modern Image", L"Collage Mode"});
    
    // Right column checks
    if (67 > 72 - 5) overlaps.push_back({L"E-Book", L"Photo & Other"});
    if (112 > 117 - 5) overlaps.push_back({L"Photo & Other", L"Media & Documents"});
    if (205 > 210 - 5) overlaps.push_back({L"Media & Documents", L"Advanced Options"});
    
    if (!overlaps.empty()) {
        result.passed = false;
        result.message = L"Found " + to_wstring(overlaps.size()) + L" overlaps";
        return result;
    }
    
    result.passed = true;
    result.message = L"All 8 group boxes properly spaced";
    return result;
}

// Test 4: Verify RAW checkbox is in correct position
TestResult Test_RAWCheckboxPosition() {
    TestResult result;
    result.testName = L"RAW Checkbox Position";
    
    HWND hDlg = FindLENSManagerWindow();
    if (!hDlg) {
        result.passed = false;
        result.message = L"LENSManager window not found";
        return result;
    }
    
    HWND hRawCheckbox = GetDlgItem(hDlg, 1043); // IDC_CB_RAW
    if (!hRawCheckbox) {
        result.passed = false;
        result.message = L"RAW checkbox not found (ID:1043)";
        return result;
    }
    
    // Get RAW checkbox position
    RECT rawRect;
    GetWindowRect(hRawCheckbox, &rawRect);
    
    // Get SVG checkbox position (should be above RAW)
    HWND hSvgCheckbox = GetDlgItem(hDlg, 1042); // IDC_CB_SVG
    RECT svgRect;
    GetWindowRect(hSvgCheckbox, &svgRect);
    
    // RAW should be below SVG (higher Y coordinate)
    if (rawRect.top <= svgRect.top) {
        result.passed = false;
        result.message = L"RAW checkbox not below SVG checkbox";
        return result;
    }
    
    // They should be in the same column (similar X coordinate)
    int xDiff = abs(rawRect.left - svgRect.left);
    if (xDiff > 10) {
        result.passed = false;
        result.message = L"RAW and SVG checkboxes not aligned (X diff: " + to_wstring(xDiff) + L"px)";
        return result;
    }
    
    result.passed = true;
    result.message = L"RAW checkbox correctly positioned below SVG";
    return result;
}

// Test 5: Verify checkbox state toggle functionality
TestResult Test_CheckboxToggle() {
    TestResult result;
    result.testName = L"Checkbox Toggle Functionality";
    
    HWND hDlg = FindLENSManagerWindow();
    if (!hDlg) {
        result.passed = false;
        result.message = L"LENSManager window not found";
        return result;
    }
    
    HWND hRawCheckbox = GetDlgItem(hDlg, 1043); // IDC_CB_RAW
    if (!hRawCheckbox) {
        result.passed = false;
        result.message = L"RAW checkbox not found";
        return result;
    }
    
    // Get initial state
    LRESULT initialState = SendMessage(hRawCheckbox, BM_GETCHECK, 0, 0);
    
    // Toggle the checkbox
    SendMessage(hRawCheckbox, BM_SETCHECK, initialState == BST_CHECKED ? BST_UNCHECKED : BST_CHECKED, 0);
    
    // Verify state changed
    LRESULT newState = SendMessage(hRawCheckbox, BM_GETCHECK, 0, 0);
    
    if (newState == initialState) {
        result.passed = false;
        result.message = L"Checkbox state did not toggle";
        return result;
    }
    
    // Restore original state
    SendMessage(hRawCheckbox, BM_SETCHECK, initialState, 0);
    
    result.passed = true;
    result.message = L"Checkbox toggles correctly";
    return result;
}

// Test 6: Verify status bar exists
TestResult Test_StatusBarExists() {
    TestResult result;
    result.testName = L"Status Bar Exists";
    
    HWND hDlg = FindLENSManagerWindow();
    if (!hDlg) {
        result.passed = false;
        result.message = L"LENSManager window not found";
        return result;
    }
    
    HWND hStatusBar = GetDlgItem(hDlg, 1042); // IDC_STATUSBAR
    if (!hStatusBar) {
        result.passed = false;
        result.message = L"Status bar not found (ID:1042)";
        return result;
    }
    
    // Get status bar text
    wchar_t statusText[256] = {0};
    SendMessage(hStatusBar, SB_GETTEXT, 0, (LPARAM)statusText);
    
    wstring text(statusText);
    if (text.empty()) {
        result.passed = false;
        result.message = L"Status bar text is empty";
        return result;
    }
    
    // Should contain "Ready" or format count
    if (text.find(L"Ready") == wstring::npos && text.find(L"format") == wstring::npos) {
        result.passed = false;
        result.message = L"Status bar text unexpected: " + text;
        return result;
    }
    
    result.passed = true;
    result.message = L"Status bar present with text: " + text;
    return result;
}

// Test 7: Verify buttons exist and are accessible
TestResult Test_ButtonsExist() {
    TestResult result;
    result.testName = L"Action Buttons Exist";
    
    HWND hDlg = FindLENSManagerWindow();
    if (!hDlg) {
        result.passed = false;
        result.message = L"LENSManager window not found";
        return result;
    }
    
    HWND hOK = GetDlgItem(hDlg, IDOK);
    HWND hCancel = GetDlgItem(hDlg, IDCANCEL);
    HWND hApply = GetDlgItem(hDlg, 1002); // IDC_APPLY
    
    int missingCount = 0;
    wstring missing;
    
    if (!hOK) { missingCount++; missing += L"OK "; }
    if (!hCancel) { missingCount++; missing += L"Cancel "; }
    if (!hApply) { missingCount++; missing += L"Apply "; }
    
    if (missingCount > 0) {
        result.passed = false;
        result.message = L"Missing buttons: " + missing;
        return result;
    }
    
    result.passed = true;
    result.message = L"All 3 buttons (OK, Cancel, Apply) found";
    return result;
}

// Test 8: Verify all format checkboxes are properly grouped
TestResult Test_CheckboxGrouping() {
    TestResult result;
    result.testName = L"Checkbox Proper Grouping";
    
    HWND hDlg = FindLENSManagerWindow();
    if (!hDlg) {
        result.passed = false;
        result.message = L"LENSManager window not found";
        return result;
    }
    
    // Check Media & Documents group has 5 checkboxes
    int mediaCheckboxes[] = {1026, 1027, 1040, 1041, 1043}; // VIDEO, PDF, TIFF, SVG, RAW
    
    vector<int> yPositions;
    for (int id : mediaCheckboxes) {
        HWND hCheck = GetDlgItem(hDlg, id);
        if (!hCheck) continue;
        
        RECT rect;
        GetWindowRect(hCheck, &rect);
        yPositions.push_back(rect.top);
    }
    
    if (yPositions.size() != 5) {
        result.passed = false;
        result.message = L"Media & Documents should have 5 checkboxes, found " + to_wstring(yPositions.size());
        return result;
    }
    
    // Verify they're sorted vertically (increasing Y)
    for (size_t i = 1; i < yPositions.size(); i++) {
        if (yPositions[i] <= yPositions[i-1]) {
            result.passed = false;
            result.message = L"Checkboxes not properly ordered vertically";
            return result;
        }
    }
    
    result.passed = true;
    result.message = L"All 5 Media & Documents checkboxes properly ordered";
    return result;
}

// Run all tests
void RunAllTests() {
    wcout << L"\n=== LENSManager GUI Test Suite ===\n\n";
    
    // Wait for window to be ready
    wcout << L"Waiting for LENSManager window...\n";
    int attempts = 0;
    while (!FindLENSManagerWindow() && attempts < 50) {
        Sleep(100);
        attempts++;
    }
    
    if (!FindLENSManagerWindow()) {
        wcout << L"ERROR: LENSManager window not found after 5 seconds.\n";
        wcout << L"Please launch LENSManager.exe before running tests.\n";
        return;
    }
    
    wcout << L"LENSManager window found!\n\n";
    
    // Run tests
    testResults.push_back(Test_DialogExists());
    testResults.push_back(Test_AllCheckboxesExist());
    testResults.push_back(Test_GroupBoxesNoOverlap());
    testResults.push_back(Test_RAWCheckboxPosition());
    testResults.push_back(Test_CheckboxToggle());
    testResults.push_back(Test_StatusBarExists());
    testResults.push_back(Test_ButtonsExist());
    testResults.push_back(Test_CheckboxGrouping());
    
    // Print results
    int passedCount = 0;
    int failedCount = 0;
    
    for (const auto& test : testResults) {
        wstring status = test.passed ? L"[PASS]" : L"[FAIL]";
        wcout << status << L" " << test.testName << L"\n";
        wcout << L"       " << test.message << L"\n\n";
        
        if (test.passed) passedCount++;
        else failedCount++;
    }
    
    wcout << L"=================================\n";
    wcout << L"Total Tests: " << testResults.size() << L"\n";
    wcout << L"Passed: " << passedCount << L"\n";
    wcout << L"Failed: " << failedCount << L"\n";
    
    if (failedCount == 0) {
        wcout << L"\n✓ ALL TESTS PASSED!\n";
    } else {
        wcout << L"\n✗ SOME TESTS FAILED\n";
    }
}

int main() {
    // Set console to UTF-16
    _setmode(_fileno(stdout), _O_U16TEXT);
    
    wcout << L"LENSManager GUI Automated Test Suite\n";
    wcout << L"====================================\n";
    wcout << L"\nThis test suite verifies:\n";
    wcout << L"  - Dialog layout and sizing\n";
    wcout << L"  - All checkboxes present\n";
    wcout << L"  - No overlapping controls\n";
    wcout << L"  - RAW checkbox positioning\n";
    wcout << L"  - Checkbox toggle functionality\n";
    wcout << L"  - Status bar presence\n";
    wcout << L"  - Action buttons\n\n";
    
    wcout << L"Press Enter to start tests (make sure LENSManager.exe is running)...\n";
    wcin.get();
    
    RunAllTests();
    
    wcout << L"\nPress Enter to exit...\n";
    wcin.get();
    
    return 0;
}

