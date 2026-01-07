# DarkThumbs v5.1.0 - Testing Checklist

**Version:** v5.1.0  
**Date:** November 24, 2025  
**Status:** Production Ready - Testing Phase

---

## Pre-Installation Testing

### Build Verification ✅ COMPLETE
- [x] Clean build with 0 errors, 0 warnings
- [x] Binary size: 1.41 MB (CBXShell.dll)
- [x] Static linking verified (no external DLL dependencies)
- [x] All preprocessor definitions active:
  - [x] ENABLE_WEBP_SUPPORT
  - [x] ENABLE_HEIF_SUPPORT
  - [x] ENABLE_AVIF_SUPPORT
  - [x] ENABLE_PDF_SUPPORT
  - [x] ENABLE_VIDEO_SUPPORT
  - [x] DISABLE_RAR_SUPPORT

---

## Installation Testing

### Installation Process
- [ ] Run `install-x64-fixed.cmd` as Administrator
- [ ] Verify no errors during registration
- [ ] Check registry entries created:
  - [ ] `HKCR\CLSID\{...}\InprocServer32` points to CBXShell.dll
  - [ ] `HKCR\.cbz\shellex\{...}` handler registered
  - [ ] All 31+ file extensions registered
- [ ] Verify Explorer restart successful
- [ ] Check CBXManager.exe launches without errors

### Uninstallation Process
- [ ] Run `uninstall-x64-fixed.cmd` as Administrator
- [ ] Verify all registry entries removed
- [ ] Verify DLL unregistered
- [ ] Check no orphaned registry keys
- [ ] Verify Explorer restart successful

---

## Format Support Testing

### Archive Formats (4 formats)
- [ ] **ZIP (.zip)** - Test with DEFLATE, BZIP2, ZSTD, LZMA compression
- [ ] **RAR (.rar)** - Should be DISABLED (DISABLE_RAR_SUPPORT)
- [ ] **7-Zip (.7z)** - Test with LZMA compression
- [ ] **TAR (.tar)** - Test uncompressed and compressed variants

### Comic Book Formats (4 formats)
- [ ] **CBZ (.cbz)** - ZIP with images
- [ ] **CBR (.cbr)** - Should be DISABLED (RAR disabled)
- [ ] **CB7 (.cb7)** - 7-Zip with images
- [ ] **CBT (.cbt)** - TAR with images

### Ebook Formats (5 formats)
- [ ] **EPUB (.epub)** - Standard ebook
- [ ] **MOBI (.mobi)** - Kindle format
- [ ] **AZW (.azw)** - Kindle DRM-free
- [ ] **AZW3 (.azw3)** - Kindle Format 8
- [ ] **FB2 (.fb2)** - FictionBook 2.0

### Modern Image Formats (8 formats)
- [ ] **WebP (.webp)** - Test lossy and lossless variants
  - Library: libwebp 1.5.0 (static)
  - Expected: Should work via libwebp
- [ ] **AVIF (.avif)** - Test standard AVIF files
  - Implementation: Windows WIC
  - Expected: Should work on Windows 10+ with AVIF codec installed
- [ ] **HEIF (.heif)** - Test HEIF files
  - Implementation: Windows WIC
  - Expected: Should work on Windows 10+ with HEIF codec installed
- [ ] **HEIC (.heic)** - Test iPhone photos
  - Implementation: Windows WIC
  - Expected: Should work on Windows 10+ with HEIF codec installed
- [ ] **TIFF (.tif, .tiff)** - Test single and multi-page TIFF
  - Implementation: Windows WIC
  - Expected: Should work (native Windows support)
- [ ] **SVG (.svg)** - DEFERRED (not enabled in v5.1.0)
- [ ] **DNG (.dng)** - DEFERRED (not enabled in v5.1.0)

### RAW Camera Formats (5 formats) - DEFERRED
- [ ] **Canon RAW (.cr2, .cr3)** - Not enabled in v5.1.0
- [ ] **Nikon RAW (.nef)** - Not enabled in v5.1.0
- [ ] **Sony RAW (.arw)** - Not enabled in v5.1.0
- [ ] **Olympus RAW (.orf)** - Not enabled in v5.1.0

### Video Formats (10 formats)
Test frame extraction at 10% position:
- [ ] **MP4 (.mp4, .m4v)** - MPEG-4 video
- [ ] **AVI (.avi)** - Audio Video Interleave
- [ ] **MKV (.mkv)** - Matroska container
- [ ] **MOV (.mov)** - QuickTime movie
- [ ] **WMV (.wmv)** - Windows Media Video
- [ ] **FLV (.flv)** - Flash Video
- [ ] **WebM (.webm)** - WebM video
- [ ] **MPEG (.mpg, .mpeg)** - MPEG video

### Document Formats (1 format)
- [ ] **PDF (.pdf)** - Test single and multi-page PDFs
  - Implementation: Windows.Data.Pdf API
  - Expected: Should work on Windows 10 1803+
  - Test: First page thumbnail extraction
  - Test: Password-protected PDFs (should handle gracefully)

---

## Feature Testing

### Thumbnail Caching System
- [ ] **First Access** - Verify thumbnails generate (100-500ms per file)
- [ ] **Second Access** - Verify instant load from cache (<1ms)
- [ ] **Cache Location** - Verify PNG files in `%LOCALAPPDATA%\DarkThumbs\cache`
- [ ] **Cache Keys** - Verify MD5 hash + format type + size naming
- [ ] **Cache Cleanup** - Test automatic cleanup at 500MB limit
- [ ] **LRU Policy** - Verify least recently used files removed first

### Performance Benchmarks
Test with 100 files in a folder:
- [ ] **Cold Cache** - First view timing: _____ seconds
- [ ] **Warm Cache** - Second view timing: _____ seconds
- [ ] **Performance Ratio** - Calculate speedup: _____x faster

### Configuration Management (CBXManager.exe)
- [ ] **GUI Launch** - Verify CBXManager.exe opens
- [ ] **Format Selection** - Enable/disable individual formats
- [ ] **REG Export** - Export configuration to .reg file
- [ ] **REG Import** - Import configuration from .reg file
- [ ] **JSON Export** - Export configuration to .json file
- [ ] **JSON Import** - Import configuration from .json file
- [ ] **Change Summary** - Verify before/after dialog shows changes
- [ ] **Rollback** - Test one-click restore to previous settings

---

## Integration Testing

### Windows Explorer Integration
- [ ] **Thumbnail View** - Verify thumbnails appear in Explorer
- [ ] **Large Icons** - Test with Large Icons view (256x256)
- [ ] **Medium Icons** - Test with Medium Icons view (128x128)
- [ ] **Small Icons** - Test with Small Icons view (48x48)
- [ ] **Extra Large Icons** - Test with Extra Large Icons view (256x256+)
- [ ] **Details View** - Verify thumbnails in Details view
- [ ] **Dark Mode** - Test with Windows 11 dark theme
- [ ] **Light Mode** - Test with Windows 11 light theme
- [ ] **High DPI** - Test on 4K monitor (if available)

### Shell Context Menu
- [ ] **Right-click** - Verify no crashes on right-click
- [ ] **Properties** - Verify Properties dialog works
- [ ] **File Operations** - Copy, move, delete, rename work correctly

---

## Stress Testing

### Large File Collections
- [ ] **100 files** - Test folder with 100 archives
- [ ] **500 files** - Test folder with 500 archives
- [ ] **1000+ files** - Test folder with 1000+ archives
- [ ] **Memory Usage** - Monitor memory consumption
- [ ] **CPU Usage** - Monitor CPU usage during thumbnail generation

### Large Archive Files
- [ ] **Small archives** - <10 MB
- [ ] **Medium archives** - 10-100 MB
- [ ] **Large archives** - 100-500 MB
- [ ] **Very large archives** - 500+ MB
- [ ] **Timeout Handling** - Verify graceful timeout on very large files

---

## Error Handling Testing

### Invalid Files
- [ ] **Corrupted archives** - Verify graceful failure
- [ ] **Truncated files** - Verify no crash
- [ ] **Wrong extension** - .cbz file that's actually .txt
- [ ] **Empty files** - 0-byte files
- [ ] **Password-protected** - Archives with passwords

### Edge Cases
- [ ] **No images in archive** - Archive with only text files
- [ ] **Single image** - Archive with one image
- [ ] **Many images** - Archive with 100+ images
- [ ] **Mixed formats** - Archive with JPEG, PNG, WebP, AVIF
- [ ] **Nested archives** - Archive within archive

---

## Compatibility Testing

### Windows Versions
- [ ] **Windows 11** - Primary target
- [ ] **Windows 10** - Secondary target
- [ ] **Windows 10 1803+** - Required for PDF support

### System Configurations
- [ ] **Clean Windows install** - No codecs
- [ ] **With HEIF codec** - Windows Store HEIF extension installed
- [ ] **With AVIF codec** - Windows Store AV1 extension installed
- [ ] **With video codecs** - K-Lite or similar codec pack

---

## Regression Testing

### Previously Working Features
- [ ] **Basic JPEG/PNG** - Verify still works
- [ ] **ZIP archives** - Verify still works
- [ ] **EPUB ebooks** - Verify still works
- [ ] **Dark mode** - Verify adaptive backgrounds
- [ ] **DPI awareness** - Verify high-DPI scaling

---

## Performance Comparison

### Before v5.1.0 (v5.0.x)
- Cold cache: _____ seconds for 100 files
- No caching support

### After v5.1.0
- Cold cache: _____ seconds for 100 files
- Warm cache: _____ seconds for 100 files
- Improvement: _____x faster

---

## Test Results Summary

**Date Tested:** _____________  
**Tester:** _____________  
**Windows Version:** _____________  
**Build Version:** v5.1.0 (CBXShell.dll 1.41 MB, Nov 24 2025 09:52 AM)

### Overall Status
- [ ] ✅ PASS - All tests passed
- [ ] ⚠️ PARTIAL - Some tests failed (list below)
- [ ] ❌ FAIL - Critical failures

### Critical Issues Found
1. _____________
2. _____________
3. _____________

### Minor Issues Found
1. _____________
2. _____________
3. _____________

### Performance Results
- Cache performance: _____x faster on warm cache
- Large file handling: _____ (acceptable/slow/crashes)
- Memory usage: _____ MB average
- CPU usage: _____ % average

### Recommendations
- [ ] Ready for production release
- [ ] Needs fixes before release
- [ ] Needs additional testing

---

## Next Steps After Testing

### If All Tests Pass ✅
1. Update version to v5.1.0 RELEASE
2. Create installer package
3. Update WHATS_NEW.md with test results
4. Tag release in git (if using version control)
5. Prepare release notes

### If Issues Found ⚠️
1. Document all issues in GitHub/issue tracker
2. Prioritize critical vs. minor issues
3. Fix critical issues
4. Retest affected areas
5. Update build and retest

### Distribution Checklist
- [ ] Installer package created
- [ ] Release notes written
- [ ] README.md updated
- [ ] WHATS_NEW.md updated
- [ ] All documentation reviewed
- [ ] Backup of working build created
