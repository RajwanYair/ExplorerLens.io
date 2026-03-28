# ExplorerLens Troubleshooting Guide
**Documentation Completion** 
**Date:** March 28, 2026

---

## Table of Contents
1. [Installation Issues](#installation-issues)
2. [Thumbnails Not Showing](#thumbnails-not-showing)
3. [Performance Problems](#performance-problems) 
4. [COM Registration Failures](#com-registration-failures)
5. [File Format Issues](#file-format-issues)
6. [Windows Explorer Integration](#windows-explorer-integration)
7. [Build & Development Issues](#build--development-issues)
8. [Advanced Diagnostics](#advanced-diagnostics)

---

## Installation Issues

### ❌ **Problem: Installer fails with "Access Denied"**

**Cause:** Insufficient permissions

**Solution:**
1. Right-click installer → **Run as Administrator**
2. Or: Open elevated PowerShell:
 ```powershell
 Start-Process -FilePath "ExplorerLens-Setup.msi" -Verb RunAs
 ```

---

### ❌ **Problem: "Windows protected your PC" warning**

**Cause:** Unsigned binaries (SmartScreen filter)

**Solution:**
1. Click "More info"
2. Click "Run anyway"
3. **Long-term fix:** Use code-signed binaries (see [CODE_SIGNING.md](CODE_SIGNING.md))

---

### ❌ **Problem: Previous version won't uninstall**

**Cause:** Corrupted registry or incomplete uninstall

**Solution:**
1. **Clean uninstall:**
 ```powershell
 # Force remove via product code
 msiexec /x {9E6ECB90-5A61-42BD-B851-D3297D9C7F39} /qn
 ```

2. **Manual cleanup:**
 ```powershell
 # Run LENSManager as admin
 cd "C:\Program Files\ExplorerLens"
 .\LENSManager.exe
 # Click "Uninstall All" button
 ```

3. **Registry cleanup (advanced):**
 - Open `regedit.exe` as Administrator
 - Delete: `HKEY_CLASSES_ROOT\CLSID\{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}`
 - Delete: `HKEY_LOCAL_MACHINE\SOFTWARE\ExplorerLens`

---

## Thumbnails Not Showing

### ❌ **Problem: No thumbnails appear for supported formats**

**Cause #1:** Explorer thumbnail cache corruption

**Solution:** Clear thumbnail cache:
```powershell
# Stop Explorer
Stop-Process -Name explorer -Force

# Clear thumbnail database
Remove-Item "$env:LOCALAPPDATA\Microsoft\Windows\Explorer\thumbcache_*.db" -Force

# Restart Explorer
Start-Process explorer.exe
```

---

**Cause #2:** Shell extension not registered

**Solution:** Re-register LENSShell.dll:
```powershell
cd "C:\Program Files\ExplorerLens"
regsvr32 /u LENSShell.dll # Unregister
regsvr32 LENSShell.dll # Re-register
```

**Verify registration:**
```powershell
# Check CLSID exists
Test-Path "HKCR:\CLSID\{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}"
# Should return: True
```

---

**Cause #3:** Conflicting thumbnail provider

**Solution:** Check for conflicts:
1. Open **LENSManager.exe**
2. Click "Scan for Conflicts" button
3. Review "Third-Party Handlers" tab
4. Backup and remove conflicting handlers if needed

**Example conflict:** Windows Photo Viewer or Adobe Photoshop handlers

---

**Cause #4:** File extension not associated

**Solution:** Verify extension registration:
```powershell
# Check .webp extension
Get-ItemProperty "HKCR:\.webp\shellex\{e357fccd-a995-4576-b01f-234630154e96}"

# Should show: (default) = "{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}"
```

**Re-register via LENSManager:**
1. Open LENSManager.exe as Administrator
2. Select desired formats
3. Click "Install" button

---

### ❌ **Problem: Thumbnails work but are low quality/pixelated**

**Cause:** Windows rendering at low resolution

**Solution:**
1. **Increase thumbnail size:**
 - Right-click Explorer folder view
 - Choose "Extra Large Icons" or "Large Icons"

2. **Adjust DPI scaling (4K displays):**
 - Right-click desktop → **Display settings**
 - Set "Scale" to 150% or 200%
 - Restart Explorer

---

### ❌ **Problem: Thumbnails show for some files but not others**

**Cause:** File-specific issues (corruption, encryption, permissions)

**Solution:**
1. **Check file permissions:**
 ```powershell
 Get-Acl "problem_file.jpg" | Format-List
 # Ensure current user has Read permissions
 ```

2. **Test file validity:**
 - Try opening file in native app (Photoshop, VLC, etc.)
 - Corrupted files won't generate thumbnails

3. **Check file size limits:**
 - Default max: 50 MP for images, 4K for videos
 - Edit: `HKLM\SOFTWARE\ExplorerLens\MaxImageSize` (pixels)

---

## Performance Problems

### ❌ **Problem: Explorer freezes when browsing folders with images**

**Cause:** Synchronous thumbnail generation blocking UI

**Solution:**
1. **Reduce thumbnail size:**
 - File Explorer → View → Smaller icons
 - Reduces processing load per thumbnail

2. **Disable preview pane:**
 - View → Preview pane (toggle off)
 - Pane forces immediate thumbnail generation

3. **Enable caching:**
 - Verify cache enabled:
 ```powershell
 Get-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name EnableCache
 # Should be: 1
 ```

4. **Increase cache size:**
 ```powershell
 # Set cache to 1 GB (default: 500 MB)
 Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name CacheSizeMB -Value 1024
 ```

---

### ❌ **Problem: High CPU usage when browsing folders**

**Cause:** Decoder inefficiency or large file processing

**Solution:**
1. **Check GPU acceleration:**
 ```powershell
 # Verify GPU enabled
 Get-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name UseGPU
 # Should be: 1
 ```

2. **Limit concurrent decodes:**
 ```powershell
 # Reduce thread count (default: CPU core count)
 Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxThreads -Value 4
 ```

3. **Skip large files:**
 ```powershell
 # Set max file size (default: unlimited)
 Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxFileSizeMB -Value 50
 ```

---

### ❌ **Problem: Thumbnails take a long time to appear**

**Cause:** First-time generation without cache

**Solution:**
- **Expected behavior:** First view is slow, subsequent views fast (cached)
- **Pre-generate cache:**
 ```powershell
 # Use benchmark tool to pre-populate cache
 cd "C:\Program Files\ExplorerLens"
 .\EngineBenchmark.exe --cache-warmup "C:\Photos"
 ```

---

## COM Registration Failures

### ❌ **Problem: regsvr32 fails with "0x80004005"**

**Cause:** Missing dependencies or DLL load failure

**Solution:**
1. **Check dependencies:**
 ```powershell
 # Use Dependency Walker or dumpbin
 dumpbin /dependents "LENSShell.dll"
 ```

2. **Install Visual C++ Redistributable:**
 ```powershell
 winget install Microsoft.VCRedist.2022.x64
 ```

3. **Verify DLL bitness:**
 ```powershell
 # LENSShell.dll MUST be 64-bit for modern Windows
 dumpbin /headers LENSShell.dll | Select-String "machine"
 # Should show: 8664 machine (x64)
 ```

---

### ❌ **Problem: "LoadLibrary failed with error 126"**

**Cause:** Missing external DLLs (libwebp, libavif, etc.)

**Solution:**
1. **Verify DLLs present:**
 ```powershell
 Test-Path "C:\Program Files\ExplorerLens\libwebp.dll"
 Test-Path "C:\Program Files\ExplorerLens\libavif.dll"
 ```

2. **Reinstall from MSI:** Ensures all dependencies copied

3. **Manual DLL placement:**
 - Copy DLLs from `SDK/bin/` to `C:\Program Files\ExplorerLens\`

---

## File Format Issues

### ❌ **Problem: WEBP files don't show thumbnails**

**Cause:** libwebp not loaded or outdated

**Solution:**
```powershell
# Check libwebp version
cd "C:\Program Files\ExplorerLens"
.\EngineTests.exe --gtest_filter="*WebP*"

# Expected: All WebP tests pass
```

**Update libwebp:**
- Rebuild with latest: [Build-LibWebP-NMake.ps1](../build-scripts/external-libs/Build-LibWebP-NMake.ps1)

---

### ❌ **Problem: AVIF files show black thumbnails**

**Cause:** AVIF decoder initialization failure

**Solution:**
1. **Check libavif + dav1d present:**
 ```powershell
 Test-Path "C:\Program Files\ExplorerLens\avif.dll"
 Test-Path "C:\Program Files\ExplorerLens\dav1d.dll"
 ```

2. **Verify decoder registration:**
 ```powershell
 .\IntegrationTests.exe
 # Should show: AVIFDecoder registered (9 total decoders)
 ```

---

### ❌ **Problem: JPEG XL (.jxl) files not recognized**

**Cause:** JXL support not compiled in

**Solution:**
1. **Check CMake flag:**
 ```powershell
 cd build
 cat CMakeCache.txt | Select-String "HAS_LIBJXL"
 # Should show: HAS_LIBJXL:BOOL=ON
 ```

2. **Rebuild with JXL:**
 ```powershell
 cmake -S . -B build -DHAS_LIBJXL=ON
 cmake --build build --config Release
 ```

---

### ❌ **Problem: RAW files (.CR3, .ARW) show generic icons**

**Cause:** LibRaw not installed or metadata corruption

**Solution:**
1. **Verify LibRaw linkage:**
 ```powershell
 dumpbin /imports LENSShell.dll | Select-String "libraw"
 ```

2. **Test specific RAW format:**
 ```powershell
 .\EngineTests.exe --gtest_filter="*CR3*"
 ```

3. **Metadata stripping:** Some RAW files have no embedded thumbnails
 - ExplorerLens will decode full image (slower)

---

## Windows Explorer Integration

### ❌ **Problem: Context menu entries missing**

**Cause:** Shell extension not approved

**Solution:**
```powershell
# Add to approved shell extensions
$clsid = "{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}"
New-Item "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" -Force
Set-ItemProperty "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" `
 -Name $clsid -Value "ExplorerLens Shell Extension"
```

---

### ❌ **Problem: Explorer crashes when viewing certain folders**

**Cause:** Decoder exception not handled

**Solution:**
1. **Enable crash dumps:**
 ```powershell
 Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name EnableCrashDumps -Value 1
 ```

2. **Reproduce crash:**
 - Navigate to problematic folder
 - Crash dump saved to: `C:\ProgramData\ExplorerLens\Crashes\`

3. **Report crash:**
 - Open dump in WinDbg or Visual Studio
 - File GitHub issue with stack trace

---

### ❌ **Problem: Thumbnails appear after restart but disappear later**

**Cause:** Another program overwriting registry

**Solution:**
1. **Monitor registry changes:**
 ```powershell
 # Use Process Monitor (procmon.exe) to track writes to:
 # HKCR\.webp\shellex\{e357fccd-...}
 ```

2. **Lock registry key (advanced):**
 - Set permissions to prevent write access
 - regedit → Right-click key → Permissions → Advanced

---

## Build & Development Issues

### ❌ **Problem: CMake configuration fails**

**Cause:** Missing dependencies or incorrect paths

**Solution:**
```powershell
# Clear CMake cache
Remove-Item build -Recurse -Force

# Reconfigure with verbose output
cmake -S . -B build --debug-output
```

**Common issues:**
- JPEG XL not found: Set `-DLIBJXL_ROOT=C:\Path\To\libjxl`
- Visual Studio not detected: Install **C++ Desktop Development** workload

---

### ❌ **Problem: Build fails with LNK2001 (unresolved external)**

**Cause:** Missing library linkage

**Solution:**
1. **Check CMakeLists.txt:**
 - Ensure `target_link_libraries()` includes all deps

2. **Verify library exists:**
 ```powershell
 Test-Path "SDK/lib/libwebp.lib"
 ```

3. **Check library bitness:**
 ```powershell
 dumpbin /headers SDK/lib/libwebp.lib | Select-String "machine"
 # Should match: x64
 ```

---

### ❌ **Problem: Tests fail with "DLL not found"**

**Cause:** Test executable can't find runtime DLLs

**Solution:**
```powershell
# Copy DLLs to test directory
Copy-Item SDK/bin/*.dll build/bin/Release/
```

**Or add to PATH:**
```powershell
$env:PATH += ";$PWD\SDK\bin"
.\build\bin\Release\EngineTests.exe
```

---

## Advanced Diagnostics

### 🔍 **Enable Debug Logging**

```powershell
# Enable verbose logging
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name LogLevel -Value 4

# Levels: 0=Off, 1=Error, 2=Warn, 3=Info, 4=Debug

# View logs
Get-Content "C:\ProgramData\ExplorerLens\Logs\Engine.log" -Tail 50 -Wait
```

---

### 🔍 **Test Individual Decoders**

```powershell
# Test specific decoder
.\EngineTests.exe --gtest_filter="TestImageDecoder_*"

# All decoders:
.\EngineTests.exe --gtest_list_tests | Select-String "Decoder"
```

---

### 🔍 **Performance Profiling**

```powershell
# Run benchmark suite
.\EngineBenchmark.exe

# Specific test:
.\EngineBenchmark.exe --benchmark_filter="DecodeWebP"

# Output JSON results
.\EngineBenchmark.exe --benchmark_format=json --benchmark_out=results.json
```

---

### 🔍 **Registry Dump for Support**

```powershell
# Export ExplorerLens registry for debugging
reg export "HKLM\SOFTWARE\ExplorerLens" ExplorerLens_config.reg
reg export "HKCR\CLSID\{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}" ExplorerLens_com.reg

# Attach .reg files to GitHub issue
```

---

### 🔍 **Test File Decoding**

```powershell
# Manual decode test
$testFile = "C:\Photos\sample.webp"

# Using EngineTests
.\EngineTests.exe --test-file="$testFile"

# Check decoder selection
.\IntegrationTests.exe
# Output shows: ".webp -> WebPDecoder"
```

---

## Common Error Codes

| Error Code | Meaning | Solution |
|------------|---------|----------|
| **0x80004005** | E_FAIL (generic) | Check event logs: `Get-WinEvent -LogName Application -MaxEvents 50` |
| **0x80070002** | File not found | Verify file path and permissions |
| **0x8007000E** | Out of memory | Close other apps, increase page file |
| **0x80070005** | Access denied | Run as Administrator or check ACLs |
| **0x800401F3** | Invalid class string | Re-register COM server: `regsvr32 LENSShell.dll` |
| **0x80040154** | Class not registered | Reinstall ExplorerLens or run LENSManager |

---

## Getting Help

### **Before Reporting Issues:**
1. ✅ Check this troubleshooting guide
2. ✅ Verify latest version installed
3. ✅ Clear thumbnail cache and test again
4. ✅ Run diagnostic tests: `.\EngineTests.exe`

### **GitHub Issues:**
- Repository: https://github.com/RajwanYair/ExplorerLens.io/issues
- Include:
  - Windows version (`winver`)
  - ExplorerLens version (`Get-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name Version`)
  - Log files (`C:\ProgramData\ExplorerLens\Logs\`)
  - Screenshot of issue
  - Steps to reproduce

### **Community Support:**
- Discussions: https://github.com/RajwanYair/ExplorerLens.io/discussions

---

## Current Known Issues

**Version:** 24.1.0 "Altair-R" — all P0 and P1 issues resolved

| Priority | Issue | Status | Workaround |
|----------|-------|--------|------------|
| P2 | Large archives >500MB first-thumbnail latency | Improved −68% (0.8s) | None for most; use `MaxFileSizeMB` registry key to skip very large files |
| P2 | RAW color accuracy (CR3/NEF/ARW) | LibRaw limitation | Slight variation from Lightroom expected; use embedded JPEG for speed |
| P2 | Explorer thumbnail cache corruption | Windows bug | `LENSManager.exe /ClearCache` — see [Thumbnails Not Showing](#thumbnails-not-showing) |
| P3 | Network drive performance | By design — network latency | `Set-ItemProperty HKLM:\Software\ExplorerLens -Name NetworkCacheTTL -Value 3600` |
| P3 | Multi-monitor mixed DPI scaling | Windows Explorer limitation | Set all monitors to same scaling factor |

### Resolved in v15.0.0

| Issue | Fix |
|-------|-----|
| JPEG XL build config | libjxl 0.11.1 integrated, `HAS_LIBJXL=ON` default |
| HEIF/HEIC requires external codec | Native HEIFDecoder with libheif 1.19.5 + libde265 1.0.15 |
| Video thumbnails missing (AV1/VP9/HEVC) | K-Lite Codec Pack + LAV Filters integration |
| LENSManager dark mode | `SetWindowTheme` + DarkModeHelper.h |
| RAR/CBR/JXR routing | LENSTYPE_RAR/CBR/JXR constants added |

### Resolved in v6.2.0

| Issue | Fix |
|-------|-----|
| MSVC CRT runtime mismatch | All external libs rebuilt with `/MD` |
| Explorer crash on malformed archives | SEH wrapper in `LENSShellClass::GetThumbnail` |

---

**Last Updated:** March 28, 2026 
**Version:** 24.1.0
