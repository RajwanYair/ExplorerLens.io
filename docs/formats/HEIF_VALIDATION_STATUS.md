# HEIF/HEIC Format Support - Validation Status
**Sprint 15 - Format Support Expansion**  
**Status**: Ready for Testing ⚠️  
**Last Updated**: February 9, 2026

## Overview
DarkThumbs has **two HEIF/HEIC decoder implementations** for different components:

### 1. CBXShell WIC Decoder (Production-Ready)
**Location**: `CBXShell/heif_decoder_native.cpp` (194 lines)  
**Implementation**: Windows Imaging Component (WIC)  
**Status**: ✅ Complete, needs validation

**Features**:
- Uses Windows 11 built-in HEIF codec (no external dependencies)
- Format detection via ISOBMFF ftyp box analysis
- Supports HEIC/HEIX brands (iPhone photos)
- Dark mode background blending
- 32bpp BGRA output

**Supported Formats**:
- `.heic` - HEIF Image (iPhone standard)
- `.heix` - HEIF Extended Range
- `.hevc` / `.hevx` - HEVC-based HEIF
- `.heif` / `.hif` - Generic HEIF
- `.mif1` - Multi-Image HEIF

### 2. Engine LibHEIF Decoder (Advanced Features)
**Location**: `Engine/Decoders/HEIFDecoder.cpp` (475 lines)  
**Implementation**: libheif library  
**Status**: ✅ Complete, integrated in Engine

**Features**:
- Cross-platform compatibility via libheif
- HDR support (with tone mapping option)
- Embedded thumbnail extraction
- Multiple image sequences
- Greater format flexibility

## Validation Requirements

### Prerequisites
✅ **Windows 11** - Built-in HEIF codec included  
⚠️ **Windows 10** - Requires "HEIF Image Extensions" from Microsoft Store  
  - Download: https://www.microsoft.com/store/productId/9PMMSR1CGPWG

### Test Cases

#### Test Case 1: iPhone HEIC Photo
**File**: Sample iPhone 13/14/15 HEIC image  
**Expected**: 
- ✅ Thumbnail renders within 50ms
- ✅ Correct orientation (EXIF)
- ✅ Colors accurate (P3 color space → sRGB conversion)

#### Test Case 2: HEIF Sequence/Burst
**File**: Multi-image HEIF file  
**Expected**:
- ✅ First image extracted
- ✅ No crash on sequence navigation

#### Test Case 3: HEIF with Transparency
**File**: HEIF with alpha channel  
**Expected**:
- ✅ Alpha preserved or blended with background
- ✅ Dark mode background applied correctly

#### Test Case 4: 4K HEIF Image
**File**: High-resolution HEIF (3840x2160+)  
**Expected**:
- ✅ Thumbnail generated within 200ms
- ✅ No memory issues
- ✅ Proper downsampling

### Performance Targets
- **WIC Decode**: < 50ms (Windows 11)
- **LibHEIF Decode**: < 100ms (embedded thumbnail priority)
- **Memory**: < 100MB for 4K images

## Integration Status

### CBXShell Integration
- [x] Decoder implementation complete
- [x] Format detection (IsHEIFFormat)
- [x] WIC pipeline integration
- [x] Dark mode support
- [ ] **TODO**: Validate with real HEIC files
- [ ] **TODO**: Performance profiling

### Engine Integration
- [x] HEIFDecoder.cpp complete
- [x] Registered in DecoderRegistry
- [x] libheif linked in CMake
- [x] Extension list configured
- [ ] **TODO**: End-to-end test with sample files
- [ ] **TODO**: Compare WIC vs libheif performance

## Known Issues
### Windows 10 Support
⚠️ **Issue**: WIC decoder requires HEIF codec extension  
**Solution**: Check for codec availability, provide download link if missing  
**Code**: Add `CheckHEIFCodecAvailable()` function

### Color Space Handling
⚠️ **Note**: iPhone uses Display P3 color space  
**Consideration**: WIC automatically converts to sRGB, but colors may appear slightly different  
**Impact**: Low - acceptable for thumbnails

## Testing Script (PowerShell)

```powershell
# Test-HEIF-Support.ps1
# Quick validation of HEIF/HEIC support

Write-Host "Testing HEIF/HEIC Support..." -ForegroundColor Cyan

# Check Windows version
$winVer = [System.Environment]::OSVersion.Version
Write-Host "Windows Version: $($winVer.Major).$($winVer.Minor) Build $($winVer.Build)"

if ($winVer.Major -lt 10) {
    Write-Host "ERROR: HEIF requires Windows 10 or later" -ForegroundColor Red
    exit 1
}

# Check for HEIF codec (Windows 10)
if ($winVer.Build -lt 22000) { # Windows 11 = 22000+
    Write-Host "Windows 10 detected - checking for HEIF codec..." -ForegroundColor Yellow
    
    # Check if codec is installed via registry
    $codecKey = "HKLM:\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Appx\\PackageState\\*HEIF*"
    $heifInstalled = Test-Path $codecKey
    
    if (!$heifInstalled) {
        Write-Host "WARNING: HEIF codec not found. Install from:" -ForegroundColor Yellow
        Write-Host "  https://www.microsoft.com/store/productId/9PMMSR1CGPWG" -ForegroundColor Cyan
    } else {
        Write-Host "HEIF codec installed ✓" -ForegroundColor Green
    }
}

# Test file locations
$testDirs = @(
    "C:\\Users\\$env:USERNAME\\Pictures",
    "C:\\Users\\$env:USERNAME\\OneDrive\\Pictures",
    "C:\\Users\\$env:USERNAME\\iCloudPhotos"
)

$heifFiles = @()
foreach ($dir in $testDirs) {
    if (Test-Path $dir) {
        $found = Get-ChildItem $dir -Recurse -Include *.heic,*.heif -ErrorAction SilentlyContinue | Select-Object -First 5
        $heifFiles += $found
    }
}

if ($heifFiles.Count -gt 0) {
    Write-Host "Found $($heifFiles.Count) HEIF/HEIC files for testing:" -ForegroundColor Green
    $heifFiles | ForEach-Object { Write-Host "  - $($_.FullName)" }
} else {
    Write-Host "No HEIF/HEIC test files found. Consider:" -ForegroundColor Yellow
    Write-Host "  1. Transfer iPhone photos via USB" -ForegroundColor Gray
    Write-Host "  2. Download sample HEIC from test-images.org" -ForegroundColor Gray
    Write-Host "  3. Use online HEIC converter to create test files" -ForegroundColor Gray
}

Write-Host "`nReady for manual testing!" -ForegroundColor Green
```

## Next Steps
1. **Acquire Test Files** (Priority: High)
   - Transfer iPhone HEIC photos
   - Download sample HEIF files
   - Create test set with various scenarios

2. **Run Validation Tests** (Priority: High)
   - Test CBXShell thumbnail rendering
   - Test Engine decoding pipeline
   - Performance profiling

3. **Document Results** (Priority: Medium)
   - Update this document with test results
   - Create benchmark comparison (WIC vs libheif)
   - Note any issues discovered

4. **Production Readiness** (Priority: Low)
   - Add Windows 10 codec check
   - Graceful fallback if codec missing
   - User notification system

## References
- **libheif**: https://github.com/strukturag/libheif
- **Windows HEIF Codec**: https://learn.microsoft.com/windows/uwp/audio-video-camera/heif-image
- **HEIF Specification**: ISO/IEC 23008-12
- **WIC Documentation**: https://learn.microsoft.com/windows/win32/wic/-wic-lh

---
**Status Legend**:  
✅ Complete | ⚠️ Needs Testing | ❌ Not Started | 🚧 In Progress
