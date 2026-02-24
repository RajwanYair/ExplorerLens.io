// Quick diagnostic tool to dump control positions
#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <iomanip>

int main() {
    HWND hDlg = FindWindow(L"#32770", L"LENS Shell Manager - Thumbnail Configuration");
    if (!hDlg) {
        std::wcout << L"LENSManager window not found!\n";
        return 1;
    }
    
    std::wcout << L"=== Control Positions ===\n\n";
    
    struct ControlInfo {
        int id;
        const wchar_t* name;
    };
    
    ControlInfo controls[] = {
        {1040, L"TIFF"}, {1041, L"SVG"}, {1043, L"RAW"},
        {1026, L"VIDEO"}, {1027, L"PDF"}, {1042, L"STATUSBAR"}
    };
    
    for (const auto& ctrl : controls) {
        HWND hCtrl = GetDlgItem(hDlg, ctrl.id);
        if (hCtrl) {
            RECT rect;
            GetWindowRect(hCtrl, &rect);
            ScreenToClient(hDlg, (POINT*)&rect.left);
            ScreenToClient(hDlg, (POINT*)&rect.right);
            
            std::wcout << std::left << std::setw(12) << ctrl.name 
                      << L" (ID:" << std::setw(4) << ctrl.id << L") "
                      << L"Pos: (" << std::setw(3) << rect.left << L", " 
                      << std::setw(3) << rect.top << L") "
                      << L"Size: " << std::setw(3) << (rect.right - rect.left) 
                      << L"x" << (rect.bottom - rect.top) << L"\n";
        } else {
            std::wcout << std::left << std::setw(12) << ctrl.name 
                      << L" (ID:" << std::setw(4) << ctrl.id << L") "
                      << L"NOT FOUND\n";
        }
    }
    
    return 0;
}

