; ============================================================
; ExplorerLens v15.0.0 - Inno Setup Installer Script
; Download Inno Setup from: https://jrsoftware.org/isinfo.php
; Build command: iscc ExplorerLens-Installer.iss
; ============================================================

#define MyAppName "ExplorerLens"
#define MyAppVersion "15.0.0"
#define MyAppPublisher "ExplorerLens Project"
#define MyAppURL "https://github.com/yourusername/explorerlens"
#define MyAppExeName "LENSManager.exe"

[Setup]
; App information
AppId={{A7B3C4D5-E6F7-8901-2345-6789ABCDEF01}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
AppCopyright=Copyright (C) 2026 {#MyAppPublisher}

; Installation paths
DefaultDirName={autopf64}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=..\LICENSE
InfoBeforeFile=..\README.md
OutputDir=..\release-packages
OutputBaseFilename=ExplorerLens-v{#MyAppVersion}-x64-Setup
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern

; System requirements
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0.19041
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=dialog

; Visual settings
SetupIconFile=..\LENSManager\LENSManager.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
WizardImageFile=compiler:WizModernImage-IS.bmp
WizardSmallImageFile=compiler:WizModernSmallImage-IS.bmp

; Version information
VersionInfoVersion={#MyAppVersion}.0
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription={#MyAppName} Setup
VersionInfoCopyright=Copyright (C) 2025
VersionInfoProductName={#MyAppName}
VersionInfoProductVersion={#MyAppVersion}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Main application files (to Program Files)
Source: "..\x64\Release\LENSManager.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\external\compression-libs\unrar\x64\Release\UnRAR64.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\tests\tests\GPUValidator.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion isreadme

; COM DLL (to System32 - will be registered)
Source: "..\x64\Release\LENSShell.dll"; DestDir: "{sys}"; Flags: ignoreversion restartreplace uninsrestartdelete

; Documentation (optional)
Source: "..\docs\*.md"; DestDir: "{app}\docs"; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: "*archive*,*old*"

[Icons]
Name: "{group}\Configuration Manager"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\GPU Validator"; Filename: "{app}\GPUValidator.exe"
Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}"; Filename: "{#MyAppURL}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\ExplorerLens Config"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
; Add installation path to registry
Root: HKLM; Subkey: "Software\{#MyAppName}"; Flags: uninsdeletekeyifempty
Root: HKLM; Subkey: "Software\{#MyAppName}"; ValueType: string; ValueName: "InstallPath"; ValueData: "{app}"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "Software\{#MyAppName}"; ValueType: string; ValueName: "Version"; ValueData: "{#MyAppVersion}"; Flags: uninsdeletevalue

[Run]
; Register COM DLL from System32 location
Filename: "regsvr32.exe"; Parameters: "/s ""{sys}\LENSShell.dll"""; StatusMsg: "Registering COM component..."; Flags: runhidden waituntilterminated

; Optional: Launch configuration manager
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallRun]
; Unregister COM DLL before deletion
Filename: "regsvr32.exe"; Parameters: "/s /u ""{sys}\LENSShell.dll"""; StatusMsg: "Unregistering COM component..."; Flags: runhidden waituntilterminated

[Code]
// Custom code for installation

function InitializeSetup(): Boolean;
var
  ResultCode: Integer;
begin
  Result := True;
  
  // Check if running on 64-bit Windows
  if not Is64BitInstallMode then
  begin
    MsgBox('This application requires 64-bit Windows 10 or later.', mbCritical, MB_OK);
    Result := False;
    Exit;
  end;
  
  // Check Windows version
  if not (GetWindowsVersion >= $0A000000) then
  begin
    MsgBox('This application requires Windows 10 build 19041 or later.' + #13#10 + 
           'Your version is not supported.', mbCritical, MB_OK);
    Result := False;
    Exit;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then
  begin
    // Stop Windows Explorer before installation
    Exec('taskkill.exe', '/f /im explorer.exe', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    Sleep(1000);
  end;
  
  if CurStep = ssPostInstall then
  begin
    // Restart Windows Explorer after installation
    Exec(ExpandConstant('{win}\explorer.exe'), '', '', SW_SHOW, ewNoWait, ResultCode);
    Sleep(500);
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  ResultCode: Integer;
begin
  if CurUninstallStep = usUninstall then
  begin
    // Stop Windows Explorer before uninstallation
    Exec('taskkill.exe', '/f /im explorer.exe', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    Sleep(1000);
  end;
  
  if CurUninstallStep = usPostUninstall then
  begin
    // Restart Windows Explorer after uninstallation
    Exec(ExpandConstant('{win}\explorer.exe'), '', '', SW_SHOW, ewNoWait, ResultCode);
  end;
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
var
  ResultCode: Integer;
begin
  Result := '';
  
  // Unregister old version if exists
  if FileExists(ExpandConstant('{sys}\LENSShell.dll')) then
  begin
    Exec('regsvr32.exe', '/s /u "' + ExpandConstant('{sys}\LENSShell.dll') + '"', '', 
         SW_HIDE, ewWaitUntilTerminated, ResultCode);
  end;
end;

