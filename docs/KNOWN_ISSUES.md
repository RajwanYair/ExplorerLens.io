# ExplorerLens Known Issues & Troubleshooting
**Version:** 14.0.0 "Apex"  
**Last Updated:** July 2025  
**Audit Status:** ✅ All entries verified against current codebase (Sprint 348)

## Current Known Issues

### Critical (P0)

**None currently.** All P0 issues have been resolved as of v7.0.0.

---

### High Priority (P1)

#### 1. JPEG XL (.jxl) Build Configuration
**Status:** ✅ **Working** (libjxl 0.11.1 linked in current build)  
**Impact:** `.jxl` support active when built with `-DHAS_LIBJXL=ON` (default ON)  

**Details:**
- JXL decoder fully operational in current Engine build
- libjxl 0.11.1 + brotli + highway libraries linked
- Build-LibJXL.ps1 available for rebuilding from source
- Latest build: 0 errors, 0 warnings with JXL support enabled

#### 2. HEIF/HEIC Support
**Status:** ✅ **Integrated** (libheif 1.19.5 + libde265 1.0.15 built and linked)  
**Impact:** Native HEIF thumbnails work without WIC dependency  

**Details:**
- Native HEIFDecoder.cpp fully operational in Engine with `HAS_LIBHEIF=ON`
- libheif 1.19.5 + libde265 1.0.15 built and linked in Release configuration
- No HEVC codec dependency required for basic HEIF decode
- Build scripts (Build-LibHEIF.ps1) validated with local and proxy-based source refresh

---

### Medium Priority (P2)

#### 3. Large Archive Performance (>500MB)
**Status:** ✅ **Significantly Improved** (Sprint 14: Memory-Mapped I/O)  
**Impact:** First-thumbnail latency reduced by 68% (2.5s → 0.8s for 500MB archives)  
**Remaining:** Very large archives (>1GB) may still take 2-5 seconds for first thumbnail

**Affected Formats:** `.zip`, `.rar`, `.7z`, `.cbz`, `.cbr` over 500MB

**Mitigation:**
- ExplorerLens only extracts the first image file (not entire archive)
- Performance depends on archive structure (first file position)
- Central directory reading adds overhead for very large archives

#### 4. RAW Photo Color Accuracy
**Status:** LibRaw color management limitations  
**Impact:** Some RAW thumbnails may appear slightly different from Lightroom/Capture One  
**Workaround:** Adjust color management in LibRaw settings (advanced users)

**Details:**
- LibRaw uses embedded JPEG thumbnail when available (fast but may be pre-processed by camera)
- Full RAW decode uses camera matrix but may differ from proprietary software
- Affects certain Canon CR3, Nikon NEF, Sony ARW files

#### 5. Video Thumbnails Missing for Some Codecs
**Status:** ✅ **RESOLVED** (K-Lite Codec Pack 19.4.5 installed)  
**Impact:** Previously: thumbnails missing for AV1, VP9, HEVC (in MKV), ProRes  
**Resolution:** K-Lite Codec Pack provides DirectShow and Media Foundation filters for all major video codecs.

**Details:**
ExplorerLens uses Media Foundation (primary) and Shell IThumbnailProvider (fallback) for video thumbnails. K-Lite Codec Pack 19.4.5 Basic installs LAV Filters which provide:
- ✅ H.264/H.265/HEVC - All containers (MP4, MKV, MOV)
- ✅ AV1 - WebM and MP4 containers
- ✅ VP8/VP9 - WebM containers
- ✅ ProRes - MOV containers
- ✅ MPEG-2, MPEG-4, DivX, Xvid - Legacy formats
- ✅ WMV, FLV, RMVB - Streaming media formats

**K-Lite Integration Notes:**
- K-Lite registers system-wide Media Foundation transforms (MFTs) and DirectShow filters
- ExplorerLens automatically picks up these codecs via `MFCreateSourceReaderFromURL()`
- The Shell fallback path (`ExtractFrameShell()`) also benefits from K-Lite's IThumbnailProvider
- No code changes needed - K-Lite codec detection is automatic
- DXVA2 hardware acceleration works with K-Lite for H.264/H.265/AV1

If K-Lite is not installed on user machines:
```powershell
# Option 1: K-Lite Codec Pack (recommended)
# Download from https://codecguide.com/download_kl.htm

# Option 2: LAV Filters
# Download from https://github.com/Nevcairiel/LAVFilters/releases
```

#### 6. Explorer Thumbnail Cache Corruption
**Status:** Windows bug (external to ExplorerLens)  
**Impact:** Thumbnails disappear or show wrong images  
**Workaround:** Clear Windows thumbnail cache

```powershell
# Method 1: Disk Cleanup
cleanmgr /sageset:65535 /sagerun:65535

# Method 2: Manual delete
Remove-Item "$env:LOCALAPPDATA\Microsoft\Windows\Explorer\*.db" -Force

# Method 3: LENSManager
.\LENSManager.exe /ClearCache
```

---

### Low Priority (P3)

#### 7. Dark Mode Support in LENSManager
**Status:** Partial implementation  
**Impact:** LENSManager UI doesn't fully respect Windows dark mode  
**Workaround:** None. Cosmetic issue only.

**Note:** DarkModeHelper.h implements dark mode for dialogs. Dark mode was re-enabled in Sprint 8 with conditional OnCtlColor handlers. WinUI 3 manager (Sprint 18-19) provides a fully modern alternative.

#### 8. Network Drive Performance
**Status:** By design  
**Impact:** Thumbnails on network drives are slower (latency)  
**Workaround:** Enable aggressive caching

```powershell
# Increase cache size for network drives
Set-ItemProperty -Path "HKLM:\Software\ExplorerLens" -Name "CacheSize" -Value 4096
Set-ItemProperty -Path "HKLM:\Software\ExplorerLens" -Name "NetworkCacheTTL" -Value 3600
```

#### 9. Multi-Monitor DPI Scaling
**Status:** Known Windows Explorer limitation  
**Impact:** Thumbnails may appear blurry on mixed-DPI setups  
**Workaround:** Set all monitors to same scaling factor

**Details:**
- Explorer requests thumbnail size based on primary monitor DPI
- Shell extensions receive fixed pixel size (e.g., 256x256)
- If secondary monitor has different DPI, scaling artifacts occur
- Limitation affects all shell extensions, not just ExplorerLens

---

## Resolved Issues (Fixed in v6.2.0)

### ✅ MSVC CRT Runtime Mismatch (Sprint 1)
**Was:** Linker warnings about LIBCMT conflicts  
**Fixed:** All external libraries rebuilt with /MD flag  
**Details:** See `build-scripts/Rebuild-All-With-MD.ps1`

### ✅ Explorer Crashes with Malformed Archives (Sprint 22)
**Was:** Access violations caused Explorer crashes  
**Fixed:** SEH exception wrapper in `LENSShellClass::GetThumbnail`  
**Details:** See `LENSShell/LENSShellClass.cpp` lines 172-188

### ✅ Infinite Retry Loop on Corrupted Decoders (Sprint 22)
**Was:** Bad decoder keeps retrying, freezing Explorer  
**Fixed:** Circuit breaker pattern isolates failing decoders  
**Details:** See `Engine/Utils/DecoderCircuitBreaker.h`

### ✅ Memory Leaks in COM Objects (Sprint 14)
**Was:** IStream objects not released, COM leak on shutdown  
**Fixed:** RAII wrappers (`ScopedCOMPtr`, `ScopedHandle`)  
**Details:** See `Engine/Utils/MemoryLeakDetection.h`

---

## Reporting New Issues

### Before Reporting

1. **Search existing issues** on GitHub
2. **Verify** issue reproduces in latest version
3. **Collect diagnostics**:

```powershell
# Enable debug logging
Set-ItemProperty -Path "HKLM:\Software\ExplorerLens" -Name "DebugLog" -Value 1

# Reproduce issue
# Navigate to problem file in Explorer

# Collect logs
Copy-Item "$env:TEMP\ExplorerLens-Debug.log" "ExplorerLens-Debug-Issue.log"

# Check Event Viewer
Get-EventLog -LogName Application -Source ExplorerLens -Newest 10 | Format-List
```

### Issue Template

When opening a GitHub issue, include:

1. **Environment:**
   - Windows version (e.g., Windows 11 23H2)
   - ExplorerLens version (e.g., 6.2.0)
   - GPU model (e.g., NVIDIA RTX 4090)

2. **Problem Description:**
   - What you expected to happen
   - What actually happened
   - Steps to reproduce

3. **Sample File:**
   - Upload problem file (if small)
   - Or provide file characteristics (format, size, tool used to create)

4. **Logs:**
   - Attach `ExplorerLens-Debug.log`
   - Event Viewer errors
   - Screenshot of problem (if visual)

---

## Workarounds for Common Problems

### Problem: Thumbnails Appearing but Blurry

**Cause:** Using "Medium Icons" or "Small Icons" view  
**Solution:**

```powershell
# Use larger thumbnail sizes
# In Explorer: View → Extra Large Icons or Large Icons
```

### Problem: Thumbnails Slow on First View

**Cause:** Expected behavior (cold cache)  
**Explanation:**

- First view: Decode from disk (slow)
- Subsequent views: Load from cache (fast)
- 100-500ms for complex formats is normal

**Optimization:**

```powershell
# Pre-warm cache (walk directory tree)
Get-ChildItem -Path "D:\Comics" -Recurse | Out-Null
```

### Problem: High CPU Usage When Browsing Folders

**Cause:** Explorer requests many thumbnails simultaneously  
**Solution:**

```powershell
# Reduce thumbnail pre-fetching
Set-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced" `
  -Name "IconsOnly" -Value 0

# Or use "Details" view for large folders, switch to thumbnails only when needed
```

### Problem: GPU Not Used (Task Manager shows 0% GPU)

**Cause:** Small images use CPU-only fast path  
**Verification:**

```powershell
# Check GPU usage with large images (>2MB)
# GPU acceleration kicks in for:
# - Images >1920x1080
# - Video thumbnails
# - RAW photos
```

If still no GPU usage:

```powershell
# Force GPU acceleration
Set-ItemProperty -Path "HKLM:\Software\ExplorerLens" -Name "ForceGPU" -Value 1
```

---

## Performance Expectations

### Thumbnail Generation Times (Typical)

| Format       | File Size | First View | Cached View |
|--------------|-----------|------------|-------------|
| JPG/PNG      | 5 MB      | 20-50 ms   | 1-5 ms      |
| WebP         | 2 MB      | 30-60 ms   | 1-5 ms      |
| RAW (CR2)    | 25 MB     | 100-300 ms | 1-5 ms      |
| CBZ (ZIP)    | 50 MB     | 200-500 ms | 1-5 ms      |
| CBR (RAR)    | 50 MB     | 300-800 ms | 1-5 ms      |
| Video (MP4)  | 500 MB    | 500-2000 ms| 1-5 ms      |
| JXL          | 1 MB      | 40-80ms    | 1-5 ms      |

**Hardware:** Intel i7-12700K, 32GB RAM, NVIDIA RTX 3080, NVMe SSD

### Memory Usage

- **Idle:** 50-100 MB
- **Active (10 thumbnails):** 150-300 MB
- **Heavy Load (100 thumbnails):** 500 MB - 1 GB

High memory usage is temporary and released after thumbnail generation completes.

---

## Compatibility Matrix

### Tested Configurations

| Windows Version | Status | Notes |
|-----------------|--------|-------|
| Windows 11 24H2 | ✅ Fully supported | Recommended |
| Windows 11 23H2 | ✅ Fully supported | |
| Windows 11 22H2 | ✅ Fully supported | |
| Windows 10 22H2 | ✅ Supported | Some WIC codecs may be missing |
| Windows 10 21H2 | ⚠️ Limited | HEIF support requires codec pack |
| Windows 10 <1809| ❌ Not supported | Missing D3D11 features |

### GPU Compatibility

| GPU Type | Status | Performance |
|----------|--------|-------------|
| NVIDIA RTX 40xx | ✅ Excellent | Full acceleration |
| NVIDIA RTX 30xx | ✅ Excellent | Full acceleration |
| NVIDIA GTX 16xx | ✅ Good | Full acceleration |
| AMD RX 7000 | ✅ Excellent | Full acceleration |
| AMD RX 6000 | ✅ Good | Full acceleration |
| Intel Arc | ✅ Good | Full acceleration |
| Intel Iris Xe | ✅ Acceptable | Limited performance |
| Intel UHD 630 | ⚠️ Minimal | CPU fallback recommended |

---

## Getting Help

- **GitHub Issues:** https://github.com/yourusername/ExplorerLens/issues
- **Discussions:** https://github.com/yourusername/ExplorerLens/discussions
- **User Guide:** [USER_GUIDE.md](USER_GUIDE.md)
- **Developer Guide:** [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)

**Note:** This is open-source software provided as-is. Community support only.

---

**Document Version:** 1.1  
**Last Updated:** February 17, 2026

