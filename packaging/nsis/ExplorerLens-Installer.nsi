; ============================================================
; ExplorerLens v15.0.0 - NSIS Installer Script
; Nullsoft Scriptable Install System
; Download NSIS from: https://nsis.sourceforge.io/
; ============================================================

!define PRODUCT_NAME "ExplorerLens"
!define PRODUCT_VERSION "15.0.0"
!define PRODUCT_PUBLISHER "ExplorerLens Project"
!define PRODUCT_WEB_SITE "https://github.com/yourusername/explorerlens"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; Modern UI
!include "MUI2.nsh"
!include "x64.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\win.bmp"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis.bmp"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\LENSManager.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch ExplorerLens Configuration Manager"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Language
!insertmacro MUI_LANGUAGE "English"

; Installer attributes
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "ExplorerLens-v${PRODUCT_VERSION}-x64-Setup.exe"
InstallDir "$PROGRAMFILES64\${PRODUCT_NAME}"
InstallDirRegKey HKLM "${PRODUCT_UNINST_KEY}" "InstallLocation"
ShowInstDetails show
ShowUnInstDetails show
RequestExecutionLevel admin

; Version information
VIProductVersion "${PRODUCT_VERSION}.0.0"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "ProductVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "CompanyName" "${PRODUCT_PUBLISHER}"
VIAddVersionKey "FileVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "FileDescription" "${PRODUCT_NAME} Setup"
VIAddVersionKey "LegalCopyright" "Copyright (C) 2025 ${PRODUCT_PUBLISHER}"

; ============================================================
; Installer Section
; ============================================================

Section "MainSection" SEC01
    ; Check if 64-bit Windows
    ${If} ${RunningX64}
        ; Continue with installation
    ${Else}
        MessageBox MB_OK|MB_ICONSTOP "This application requires 64-bit Windows 10 or later."
        Abort
    ${EndIf}

    ; Stop Windows Explorer
    DetailPrint "Stopping Windows Explorer..."
    nsExec::ExecToLog 'taskkill /f /im explorer.exe'
    Sleep 1000

    ; Unregister old version if exists
    IfFileExists "$SYSDIR\LENSShell.dll" 0 +3
        DetailPrint "Unregistering previous version..."
        nsExec::ExecToLog 'regsvr32 /s /u "$SYSDIR\LENSShell.dll"'

    ; Set output path to installation directory
    SetOutPath "$INSTDIR"
    SetOverwrite on

    ; Copy main files
    DetailPrint "Installing main files..."
    File "bin\LENSManager.exe"
    File "bin\UnRAR64.dll"
    File "tools\GPUValidator.exe"
    File "LICENSE"
    File "README.md"

    ; Copy DLL to System32
    DetailPrint "Installing COM component..."
    SetOutPath "$SYSDIR"
    File "bin\LENSShell.dll"

    ; Register COM component from System32
    DetailPrint "Registering COM component..."
    nsExec::ExecToLog 'regsvr32 /s "$SYSDIR\LENSShell.dll"'
    Pop $0
    ${If} $0 != 0
        MessageBox MB_OK|MB_ICONEXCLAMATION "Warning: COM registration may have failed. Error code: $0"
    ${EndIf}

    ; Create Start Menu shortcuts
    CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
    CreateShortcut "$SMPROGRAMS\${PRODUCT_NAME}\Configuration Manager.lnk" "$INSTDIR\LENSManager.exe"
    CreateShortcut "$SMPROGRAMS\${PRODUCT_NAME}\GPU Validator.lnk" "$INSTDIR\GPUValidator.exe"
    CreateShortcut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe"

    ; Create Desktop shortcut (optional)
    CreateShortcut "$DESKTOP\ExplorerLens Config.lnk" "$INSTDIR\LENSManager.exe"

    ; Restart Windows Explorer
    DetailPrint "Restarting Windows Explorer..."
    Exec '"$WINDIR\explorer.exe"'
    Sleep 1000

SectionEnd

; ============================================================
; Post-installation Section
; ============================================================

Section -Post
    ; Write uninstaller
    WriteUninstaller "$INSTDIR\uninstall.exe"

    ; Write registry keys for uninstaller
    WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME} ${PRODUCT_VERSION}"
    WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\LENSManager.exe"
    WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
    WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
    WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
    WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoModify" 1
    WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoRepair" 1

    ; Calculate installed size
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "EstimatedSize" "$0"

SectionEnd

; ============================================================
; Uninstaller Section
; ============================================================

Section Uninstall
    ; Stop Windows Explorer
    DetailPrint "Stopping Windows Explorer..."
    nsExec::ExecToLog 'taskkill /f /im explorer.exe'
    Sleep 1000

    ; Unregister COM component
    DetailPrint "Unregistering COM component..."
    nsExec::ExecToLog 'regsvr32 /s /u "$SYSDIR\LENSShell.dll"'

    ; Delete System32 DLL
    Delete "$SYSDIR\LENSShell.dll"

    ; Delete installation files
    Delete "$INSTDIR\LENSManager.exe"
    Delete "$INSTDIR\UnRAR64.dll"
    Delete "$INSTDIR\GPUValidator.exe"
    Delete "$INSTDIR\LICENSE"
    Delete "$INSTDIR\README.md"
    Delete "$INSTDIR\uninstall.exe"

    ; Remove Start Menu shortcuts
    Delete "$SMPROGRAMS\${PRODUCT_NAME}\Configuration Manager.lnk"
    Delete "$SMPROGRAMS\${PRODUCT_NAME}\GPU Validator.lnk"
    Delete "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk"
    RMDir "$SMPROGRAMS\${PRODUCT_NAME}"

    ; Remove Desktop shortcut
    Delete "$DESKTOP\ExplorerLens Config.lnk"

    ; Remove installation directory
    RMDir "$INSTDIR"

    ; Remove registry keys
    DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"

    ; Restart Windows Explorer
    DetailPrint "Restarting Windows Explorer..."
    Exec '"$WINDIR\explorer.exe"'

    SetAutoClose true
SectionEnd

; ============================================================
; Helper Functions
; ============================================================

Function .onInit
    ; Check admin rights
    UserInfo::GetAccountType
    Pop $0
    ${If} $0 != "admin"
        MessageBox MB_OK|MB_ICONSTOP "Administrator privileges are required to install ${PRODUCT_NAME}."
        SetErrorLevel 740
        Quit
    ${EndIf}

    ; Check Windows version (Windows 10 minimum)
    ${IfNot} ${AtLeastWin10}
        MessageBox MB_OK|MB_ICONSTOP "${PRODUCT_NAME} requires Windows 10 or later (64-bit)."
        Abort
    ${EndIf}
FunctionEnd

Function un.onInit
    ; Check admin rights
    UserInfo::GetAccountType
    Pop $0
    ${If} $0 != "admin"
        MessageBox MB_OK|MB_ICONSTOP "Administrator privileges are required to uninstall ${PRODUCT_NAME}."
        SetErrorLevel 740
        Quit
    ${EndIf}

    MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove ${PRODUCT_NAME}?" IDYES +2
    Abort
FunctionEnd
