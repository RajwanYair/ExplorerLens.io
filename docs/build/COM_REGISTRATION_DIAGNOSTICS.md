# COM Registration Diagnostics

## Issue

The installation script hangs at "Registering COM DLL" when running `regsvr32.exe /s LENSShell.dll`.

## Diagnostic Results

### 1. DLL Dependencies ✅

All dependencies are satisfied:

- **System DLLs**: KERNEL32, USER32, GDI32, SHELL32, ole32, OLEAUT32, SHLWAPI
- **Direct3D**: d3d11.dll, D3DCOMPILER_47.dll, d2d1.dll, DWrite.dll
- **GDI+**: gdiplus.dll, MSIMG32.dll
- **Crypto**: bcrypt.dll
- **Visual C++ Runtime**: MSVCP140.dll, VCRUNTIME140.dll, VCRUNTIME140_1.dll
- **C Runtime**: api-ms-win-crt-* (multiple)

### 2. COM Exports ✅

All required COM functions are properly exported:

- `DllCanUnloadNow` - Ordinal 2
- `DllGetClassObject` - Ordinal 3
- `DllRegisterServer` - Ordinal 4
- `DllUnregisterServer` - Ordinal 5

### 3. Dry-Run Mode ✅

The installation script now supports `-DryRun` parameter:

```powershell
.\scripts\install.ps1 -DryRun -Configuration Release
```

This previews all operations without requiring administrator privileges or making system changes.

## Timeout Protection

The installation script now includes 30-second timeout for COM registration:

- If registration exceeds 30 seconds, the process is killed
- Fallback to verbose registration (without `/s` flag) for diagnostic output
- Suggests manual registration command for troubleshooting

## Manual Registration Testing

To test COM registration manually (as Administrator):

### Silent Registration (production)

```powershell
# Replace <install-path> with your actual installation directory
regsvr32.exe /s "<install-path>\x64\Release\LENSShell.dll"
```

### Verbose Registration (diagnostics)

```powershell
regsvr32.exe "<install-path>\x64\Release\LENSShell.dll"
```

This shows a message box with success/failure details.

### Check Event Viewer

```powershell
# Open Event Viewer
eventvwr.msc

# Navigate to: Windows Logs > Application
# Look for errors from source "Microsoft-Windows-COMRuntime" or "regsvr32"
```

### Process Monitor

Use Sysinternals Process Monitor to capture detailed registration activity:

```powershell
# Download from https://learn.microsoft.com/sysinternals/downloads/procmon
procmon.exe

# Filter: Process Name is regsvr32.exe
# Capture during registration to see file/registry operations
```

## Common Causes of Registration Hang

1. **Antivirus/Security Software**

- Windows Defender or third-party AV may block COM registration
- Temporarily disable real-time protection for testing

1. **Previous Installation Artifacts**

- Old registry entries may conflict
- Check: `HKEY_CLASSES_ROOT\CLSID\{your-CLSID}`
- Delete old entries before reinstalling

1. **DllRegisterServer Implementation**

- Blocking I/O operations during registration
- Message boxes or dialogs waiting for user input
- Deadlocks in COM apartment threading

1. **File System Permissions**

- DLL in OneDrive folder may have special restrictions
- OneDrive sync can interfere with registration
- Try copying DLL to local disk (C:\Temp) for testing

1. **Windows Explorer Locks**

- If DLL is already loaded by Explorer, registration may hang
- Restart Explorer: `taskkill /f /im explorer.exe && start explorer.exe`

## Next Steps

1. **Try Manual Registration**

 ```powershell
 # Open PowerShell as Administrator, navigate to project root
 cd "<install-path>"
 regsvr32.exe "x64\Release\LENSShell.dll"
 ```

1. **Check Event Logs**

 ```powershell
 Get-EventLog -LogName Application -Newest 20 -Source "*COM*","*regsvr32*" -ErrorAction SilentlyContinue
 ```

1. **Test from Local Disk**

 ```powershell
 # Copy DLL to local disk
 Copy-Item "x64\Release\LENSShell.dll" "C:\Temp\LENSShell.dll"
 regsvr32.exe "C:\Temp\LENSShell.dll"
 ```

1. **Verify Visual C++ Redistributable**

 ```powershell
 # Check if VC++ 2022 Redistributable (x64) is installed
 Get-ItemProperty HKLM:\Software\Microsoft\VisualStudio\14.0\VC\Runtimes\x64 -ErrorAction SilentlyContinue
 ```

1. **Process Monitor Capture**

- Start Process Monitor
- Filter for regsvr32.exe
- Capture during registration
- Look for "ACCESS DENIED" or "NAME NOT FOUND" events

## Installation Script Features

### Timeout Protection

```powershell
$proc = Start-Process -FilePath regsvr32.exe -ArgumentList "/s", $dllPath -PassThru -NoNewWindow
$waitResult = $proc.WaitForExit(30000) # 30 seconds
if (-not $waitResult) {
 $proc.Kill()
 Write-Host "Registration timed out" -ForegroundColor Red
}
```

### Dry-Run Mode

```powershell
.\scripts\install.ps1 -DryRun
# Shows:
# [DRY RUN] Would create: C:\Program Files\ExplorerLens.io
# [DRY RUN] Would copy: LENSShell.dll
# [DRY RUN] Would register: C:\Program Files\ExplorerLens.io\LENSShell.dll
# [DRY RUN] Command: regsvr32.exe /s "C:\Program Files\ExplorerLens.io\LENSShell.dll"
```

### Conditional Admin Check

- Admin rights only required for actual installation
- Dry-run mode works without elevation
- Runtime check instead of `#Requires -RunAsAdministrator`

## See Also

- [INSTALLATION_READY.md](INSTALLATION_READY.md) - Installation instructions
- [BUILD_STATUS.md](../BUILD_STATUS.md) - Build status and metrics
