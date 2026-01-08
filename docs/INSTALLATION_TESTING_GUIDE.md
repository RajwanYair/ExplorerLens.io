# DarkThumbs Installation & Testing Guide

## Prerequisites

### System Requirements
- Windows 10/11 x64
- Administrator privileges
- Visual C++ Redistributable 2022 (should be installed with Windows)

### Build Status
✅ All libraries built and ready:
- LZMA SDK 24.08 (2 MB) - 7z/xz support
- UnRAR 7.2.2 (330 KB) - RAR support
- zlib, lz4, zstd, minizip-ng, libwebp (4+ MB total)

✅ Main components built:
- CBXShell.dll (1,354 KB) - Shell extension
- CBXManager.exe (293 KB) - Configuration tool
- UnRAR64.dll (330 KB) - RAR decompression

## Installation Steps

### Step 1: Run Installation Script

Open PowerShell **as Administrator**:

```powershell
cd "C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\DarkThumbs"
.\scripts\install.ps1 -Configuration Release
```

**What happens**:
1. ✅ Unregisters any existing DLL
2. ✅ Stops Windows Explorer to release file locks
3. ✅ Copies files to `C:\Program Files\DarkThumbs\`
4. ✅ Registers CBXShell.dll as COM object
5. ✅ Restarts Windows Explorer automatically
6. ✅ Shows installation summary

**Expected output**:
```
========================================
DarkThumbs Installation Script
========================================

Unregistering existing COM DLL...
  ✓ Existing DLL unregistered
Stopping Explorer to release file locks...
  ✓ Explorer stopped

Installing DarkThumbs Release build...
  Source: ...\x64\Release
  Target: C:\Program Files\DarkThumbs

Verifying source files...
  ✓ CBXShell.dll (1354 KB)
  ✓ CBXManager.exe (293 KB)
  ✓ UnRAR64.dll (330 KB)

Creating installation directory...
  ✓ Directory created

Copying files...
  ✓ Copied CBXShell.dll
  ✓ Copied CBXManager.exe
  ✓ Copied UnRAR64.dll

Registering COM DLL...
  Running: regsvr32.exe /s "C:\Program Files\DarkThumbs\CBXShell.dll"
  (This may take 10-30 seconds...)
  ✓ COM DLL registered successfully

========================================
INSTALLATION COMPLETE
========================================

  ✓ Restarting Windows Explorer...
  ✓ Explorer restarted

Installation directory:
  C:\Program Files\DarkThumbs

Installed files:
  CBXManager.exe (293 KB)
  CBXShell.dll (1354 KB)
  UnRAR64.dll (330 KB)

Next steps:
  1. Test thumbnail generation on .cbz/.cbr files
  2. Run CBXManager.exe to configure settings

To uninstall:
  .\scripts\install.ps1 -Unregister
```

### Step 2: Verify Installation

Check installed files:
```powershell
Get-ChildItem "C:\Program Files\DarkThumbs"
```

Expected files:
- CBXShell.dll (1,354 KB)
- CBXManager.exe (293 KB)
- UnRAR64.dll (330 KB)

## Testing Archive Formats

### Test Files Location
```
test-archives\
├── test-comic.cbz (ZIP-based comic) - ✅ Ready
├── test-archive.zip (ZIP archive) - ✅ Ready
└── test-image.png (Test image) - ✅ Ready
```

### Create Additional Test Files

**For .cb7 (7-Zip comic) testing**:
```powershell
# Create a test .cb7 file using 7-Zip
7z a test-archives\test-comic.cb7 test-archives\test-image.png
```

**For .cbr (RAR comic) testing**:
If you have WinRAR installed:
```powershell
# Create a test .cbr file
rar a test-archives\test-comic.cbr test-archives\test-image.png
```

Or use online tools to create a small .cbr file.

### Testing Procedure

1. **Open Windows Explorer**:
   ```powershell
   explorer test-archives
   ```

2. **Change view to Large Icons or Extra Large Icons**:
   - View → Large icons
   - Or press: Ctrl + Mouse wheel up

3. **Verify thumbnail generation for each format**:

   **ZIP/CBZ Files** (✅ Should work - minizip-ng):
   - `test-archive.zip` - Should show thumbnail
   - `test-comic.cbz` - Should show thumbnail

   **RAR/CBR Files** (✅ Should work - UnRAR64.dll):
   - `test-comic.cbr` - Should show thumbnail (if created)

   **7z/CB7 Files** (✅ Should work - LZMA SDK):
   - `test-comic.cb7` - Should show thumbnail (if created)

4. **Check for errors**:
   - Right-click file → Properties
   - Should show file details without errors
   - Thumbnails should appear within 1-2 seconds

### Expected Behavior

✅ **Success indicators**:
- Thumbnail appears in Explorer
- Thumbnail shows first image from archive
- No error messages in Explorer
- Performance is responsive (<2 seconds per thumbnail)

❌ **Failure indicators**:
- Generic file icon instead of thumbnail
- "Thumbnail unavailable" message
- Windows Event Viewer shows errors
- Explorer becomes unresponsive

## Troubleshooting

### If thumbnails don't appear

1. **Check Event Viewer**:
   ```powershell
   eventvwr.msc
   # Navigate to: Windows Logs > Application
   # Look for errors from "DarkThumbs" or "CBXShell"
   ```

2. **Verify COM registration**:
   ```powershell
   # Check if DLL is registered
   reg query "HKCR\CLSID" /s /f "DarkThumbs"
   ```

3. **Restart Explorer manually**:
   ```powershell
   Stop-Process -Name explorer -Force
   Start-Process explorer.exe
   ```

4. **Check DLL dependencies**:
   ```powershell
   # Verify UnRAR64.dll is present
   Test-Path "C:\Program Files\DarkThumbs\UnRAR64.dll"
   ```

5. **Try verbose registration**:
   ```powershell
   # As Administrator
   regsvr32 "C:\Program Files\DarkThumbs\CBXShell.dll"
   # Should show success dialog
   ```

### If installation fails

See [COM_REGISTRATION_DIAGNOSTICS.md](COM_REGISTRATION_DIAGNOSTICS.md) for detailed troubleshooting.

Common fixes:
- Close all Explorer windows
- Run installation again
- Check antivirus isn't blocking registration
- Verify you have administrator privileges

## Performance Testing

### Metrics to Check

1. **Thumbnail Generation Time**:
   - Should be <2 seconds per file
   - Batch generation should be efficient

2. **Memory Usage**:
   - Explorer.exe memory should remain stable
   - No memory leaks during repeated thumbnail views

3. **CPU Usage**:
   - Brief spike during thumbnail generation
   - Should return to normal quickly

### Monitoring Tools

**Task Manager** (Ctrl+Shift+Esc):
- Monitor Explorer.exe CPU and memory
- Watch for sustained high usage

**Performance Monitor** (perfmon):
- Track detailed metrics
- Create baseline measurements

## Configuration

### Using CBXManager.exe

Run the configuration utility:
```powershell
& "C:\Program Files\DarkThumbs\CBXManager.exe"
```

**Available settings**:
- Thumbnail cache options
- Format priorities
- Performance tuning
- Debug logging

### Registry Settings

**Enable performance metrics**:
```powershell
reg add "HKCU\Software\DarkThumbs\Settings" /v EnableProfiling /t REG_DWORD /d 1 /f
```

**View metrics**:
```powershell
& "C:\Program Files\DarkThumbs\DarkThumbsMetrics.exe" show
```

## Uninstallation

To remove DarkThumbs:

```powershell
# As Administrator
.\scripts\install.ps1 -Unregister
```

This will:
1. Unregister COM DLL
2. Remove files from Program Files
3. Clean up registry entries

## Test Results Template

Document your testing:

```markdown
## Test Results - [Date]

### Installation
- [ ] Installation completed successfully
- [ ] No errors in output
- [ ] Files copied to Program Files
- [ ] Explorer restarted automatically

### Archive Formats
- [ ] .zip files show thumbnails
- [ ] .cbz files show thumbnails
- [ ] .cbr files show thumbnails (NEW - UnRAR)
- [ ] .cb7 files show thumbnails (NEW - LZMA)

### Performance
- Thumbnail generation time: _____ seconds
- Explorer memory usage: _____ MB
- CPU usage spike: _____% (acceptable if brief)

### Issues Found
- Issue 1: _____
- Issue 2: _____

### Overall Status
✅ All tests passed
⚠️ Minor issues (list above)
❌ Critical failures (list above)
```

## Support Resources

- [COM_REGISTRATION_DIAGNOSTICS.md](COM_REGISTRATION_DIAGNOSTICS.md) - COM troubleshooting
- [INSTALLATION_FIX_2026-01-08.md](INSTALLATION_FIX_2026-01-08.md) - Recent fixes
- [BUILD_ITERATION_2_2026-01-08.md](BUILD_ITERATION_2_2026-01-08.md) - Build details
- [PERFORMANCE_METRICS.md](PERFORMANCE_METRICS.md) - Metrics system

## Next Steps After Testing

1. **If all tests pass**:
   - Document successful formats
   - Benchmark performance
   - Create more complex test files

2. **If issues found**:
   - Check Event Viewer for errors
   - Run with debug logging enabled
   - Review COM registration
   - Check DLL dependencies

3. **Advanced testing**:
   - Test with large archives (100+ MB)
   - Test with many files (1000+ images)
   - Test concurrent thumbnail generation
   - Test different image formats within archives

## Build Information

**Last Build**: 2026-01-08
**Version**: 5.3.0
**Compiler**: MSVC 19.50.35720.0 (VS 2026)
**Configuration**: Release x64
**Libraries**: 9 total (4.3 MB)
**Status**: Production ready for testing
