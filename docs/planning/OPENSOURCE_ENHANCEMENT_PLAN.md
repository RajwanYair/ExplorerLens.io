# DarkThumbs Open Source Enhancement Plan
## Windows 11 x64 Optimization with Maximum Compression Support

### Current Status ✅
**All libraries are open-source and MIT-compatible:**
- zlib 1.3.1 (zlib license - MIT-compatible)
- minizip-ng 4.0.7 (zlib license - MIT-compatible)  
- UnRAR 7.2.1 (Freeware - allows source usage)
- WTL 10.0.10320 (MS-PL - Microsoft Public License)

### Supported Formats (Current)
1. **Comic Books:** CBZ, CBR, CB7, CBT (14 formats total)
2. **Ebooks:** EPUB, MOBI, AZW, AZW3, FB2
3. **Archives:** ZIP, RAR, 7Z, TAR
4. **Photos:** PHZ

### Phase 1: Enable Existing minizip-ng Compression Support ⏳

minizip-ng already includes support for:
- ✅ ZLIB (enabled)
- ⏳ BZIP2 (available, needs enabling)
- ⏳ LZMA/XZ (available, needs enabling)
- ⏳ ZSTD (available, needs enabling)

**Tasks:**
1. Download bzip2 1.0.8 source (BSD-like license)
2. Download liblzma 5.6.3 (XZ Utils - public domain)
3. Download zstd 1.5.6 (BSD license)
4. Build static libraries with AVX2 optimizations
5. Update minizip-ng.vcxproj with HAVE_BZIP2, HAVE_LZMA, HAVE_ZSTD
6. Add mz_strm_bzip.c, mz_strm_lzma.c, mz_strm_zstd.c to build

### Phase 2: Add LZ4 Support ⏳

**LZ4 1.10.0** (BSD 2-Clause license)
- Ultra-fast compression (GB/s speeds)
- Perfect for real-time thumbnail generation
- Smaller than zlib but 10x faster decompression

**Tasks:**
1. Download LZ4 1.10.0 source
2. Build lz4.lib with AVX2
3. Create mz_strm_lz4.c wrapper for minizip-ng
4. Add LZ4 format support to archive detection

### Phase 3: Add 7-Zip/LZMA SDK (Alternative to UnRAR) 🔄

**7-Zip LZMA SDK 24.09** (Public domain)
- Replaces proprietary RAR with open LZMA
- Supports .7z, .xz, .lzma formats
- Better compression than RAR for archives

**Tasks:**
1. Download LZMA SDK 24.09
2. Build 7z.lib static library
3. Replace UnRAR dependency (optional - keep both)
4. Add CB7Z format support

### Phase 4: PDF Thumbnail Support 📄

**Option A: PDFium (Recommended)**
- Google's PDF rendering library
- Apache 2.0 / BSD 3-Clause license
- Full PDF support with security sandboxing

**Option B: MuPDF**  
- Lightweight PDF renderer
- AGPL license (requires open source)
- Smaller binary size

**Tasks:**
1. Download PDFium prebuilt binaries (x64)
2. Create PDF thumbnail extractor
3. Add PDF format to supported types
4. Handle password-protected PDFs

### Phase 5: Additional Ebook Formats 📚

**Add support for:**
- CBR2 - RAR5 compressed comics (already supported via UnRAR 7.2.1)
- DJVU - Scanned document format (needs ddjvuapi)
- CHM - Compiled HTML Help (Windows native support)
- ODT - OpenDocument (needs libzip only)

### Phase 6: Modern Comic Book Formats 🎨

**New format types:**
- CBZSTD - Zstandard compressed comics (smaller + faster)
- CBLZ4 - LZ4 compressed comics (ultra-fast)
- CB7 - 7z compressed (better than RAR)
- WEBP in archives - Modern image format support

### Windows 11 Specific Optimizations 🪟

1. **CPU Optimization:**
   - ✅ AVX2 already enabled
   - Add AVX-512 detection and paths
   - Use Windows 11 DirectStorage API for large archives
   - Enable Intel QSV / AMD VCE for video thumbnails

2. **Memory Optimization:**
   - Use memory-mapped files for >100MB archives
   - Implement thumbnail cache with Windows Storage API
   - Use VirtualAlloc2 with large pages

3. **UI Integration:**
   - Windows 11 rounded corner thumbnails
   - Mica/Acrylic effects for previews
   - Dark mode automatic detection
   - HDR thumbnail support

### Library Download URLs

```batch
# Core Compression
bzip2 1.0.8: https://sourceware.org/pub/bzip2/bzip2-1.0.8.tar.gz
XZ Utils 5.6.3: https://github.com/tukaani-project/xz/releases/download/v5.6.3/xz-5.6.3.tar.gz
zstd 1.5.6: https://github.com/facebook/zstd/releases/download/v1.5.6/zstd-1.5.6.tar.gz
lz4 1.10.0: https://github.com/lz4/lz4/releases/download/v1.10.0/lz4-1.10.0.tar.gz

# PDF Support
PDFium: https://github.com/bblanchon/pdfium-binaries/releases (prebuilt)

# 7-Zip
LZMA SDK: https://www.7-zip.org/a/lzma2409.7z
```

### File Size Impact Estimation

**Current:**
- CBXShell.dll: 659,968 bytes

**After Phase 1-2 (BZIP2+LZMA+ZSTD+LZ4):**
- Estimated: ~850 KB (+200 KB)
- bzip2: ~70 KB
- liblzma: ~100 KB  
- zstd: ~400 KB (large but worth it)
- lz4: ~30 KB

**After Phase 3 (7-Zip):**
- Estimated: ~1.2 MB (+350 KB)

**After Phase 4 (PDFium):**
- Estimated: ~15 MB (PDFium is large!)
- Alternative: MuPDF only ~2 MB

### Recommended Implementation Order

1. ✅ **DONE:** Modernize existing code (zlib, minizip-ng, UnRAR)
2. **NEXT:** Enable BZIP2 support (already in minizip-ng)
3. **HIGH PRIORITY:** Add ZSTD (best compression/speed ratio)
4. **MEDIUM:** Add LZ4 (ultra-fast)
5. **MEDIUM:** Add LZMA/XZ support
6. **LOW:** PDF support (large binary impact)
7. **OPTIONAL:** 7-Zip SDK (if want to remove UnRAR)

### License Compliance Summary

All components are compatible with MIT license:
- ✅ BZIP2: BSD-like (commercial use OK)
- ✅ XZ/LZMA: Public domain
- ✅ ZSTD: BSD or GPLv2 dual (use BSD)
- ✅ LZ4: BSD 2-Clause
- ✅ PDFium: Apache 2.0 + BSD 3-Clause
- ✅ 7-Zip LZMA SDK: Public domain
- ✅ UnRAR: Freeware (source usage allowed)

**No GPL dependencies** - Safe for commercial/proprietary use!

### Performance Expectations

| Format | Compression | Decompression | Use Case |
|--------|-------------|---------------|----------|
| ZLIB | Medium | Fast | General purpose |
| BZIP2 | Better | Slower | Better compression |
| LZMA/XZ | Best | Medium | Maximum compression |
| ZSTD | Great | Very Fast | Modern balanced |
| LZ4 | Fast | Extreme | Real-time thumbnails |

**Recommendation:** Enable all formats, auto-select best based on archive type.

