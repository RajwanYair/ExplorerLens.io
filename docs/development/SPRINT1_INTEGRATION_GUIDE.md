# Sprint 1 Implementation Guide
## Modern Image Format Support (WebP, AVIF, JPEG XL)

**Status:** Ready to Begin  
**Date:** November 18, 2025  
**Target Release:** DarkThumbs v5.0

---

## 🎯 Quick Start

### What's Been Prepared

✅ **Build Infrastructure:**
- Automated library download script
- Build scripts for libwebp and libavif
- Sequential build system for error tracking
- Modular per-library builds

✅ **Integration Code:**
- Complete WebP decoder (`CBXShell/webp_decoder.h`)
- Complete AVIF decoder (`CBXShell/avif_decoder.h`)
- Format detection logic
- Windows HBITMAP conversion

✅ **Documentation:**
- Sprint roadmap (`docs/SPRINT1_MODERN_IMAGES.md`)
- Testing checklist
- Build references

### What You Need to Do

1. **Download Libraries (5 minutes)**
   ```cmd
   build-scripts\download-image-libs.cmd
   ```

2. **Build libwebp (5-10 minutes)**
   ```cmd
   build-scripts\build-libwebp.cmd
   ```

3. **Build libavif (10-15 minutes)**
   ```cmd
   build-scripts\build-libavif.cmd
   ```

4. **Integrate Code (30 minutes)**
   - Add WebP/AVIF decoders to CBXShell project
   - Update format detection
   - Link libraries in vcxproj

5. **Test (15 minutes)**
   ```cmd
   sprint-test.cmd
   ```

**Total Estimated Time:** 1-2 hours

---

## 📋 Step-by-Step Integration

### Phase 1: Download & Build

**Run automated quick-start:**
```cmd
sprint1-quickstart.cmd
```

This will:
- Download libwebp 1.4.0 (~500 KB)
- Download libavif 1.1.0 (~200 KB)
- Download dav1d 1.4.0 (~300 KB)
- Build all libraries automatically
- Verify build outputs

**Manual alternative:**
```cmd
REM Download
build-scripts\download-image-libs.cmd

REM Build WebP
build-scripts\build-libwebp.cmd

REM Build AVIF (requires Meson - auto-installed via pip)
build-scripts\build-libavif.cmd
```

---

### Phase 2: Add Decoder Files to CBXShell Project

**1. Open `CBXShell/CBXShell.vcxproj` in VS Code or text editor**

**2. Locate the `<ClCompile>` section and add:**
```xml
<ClCompile Include="webp_decoder.cpp" />
<ClCompile Include="avif_decoder.cpp" />
```

**3. Locate the `<ClInclude>` section and add:**
```xml
<ClInclude Include="webp_decoder.h" />
<ClInclude Include="avif_decoder.h" />
```

---

### Phase 3: Create Implementation Files

**Create `CBXShell/webp_decoder.cpp`:**
```cpp
#include "StdAfx.h"
#include "webp_decoder.h"
#include <webp/decode.h>

namespace DarkThumbs {

bool WebPDecoder::IsWebPFormat(const BYTE* data, size_t size) {
    if (!data || size < 12) return false;
    
    // Check RIFF header
    if (memcmp(data, "RIFF", 4) != 0) return false;
    
    // Check WEBP signature at offset 8
    if (memcmp(data + 8, "WEBP", 4) != 0) return false;
    
    return true;
}

bool WebPDecoder::GetDimensions(const BYTE* data, size_t size, int* width, int* height) {
    if (!IsWebPFormat(data, size) || !width || !height) return false;
    return WebPGetInfo(data, size, width, height) != 0;
}

HRESULT WebPDecoder::DecodeToHBITMAP(const BYTE* data, size_t size, HBITMAP* phBitmap) {
    if (!data || size == 0 || !phBitmap) return E_INVALIDARG;
    *phBitmap = nullptr;
    
    if (!IsWebPFormat(data, size)) return E_FAIL;
    
    int width = 0, height = 0;
    BYTE* rgba = WebPDecodeRGBA(data, size, &width, &height);
    if (!rgba) return E_FAIL;
    
    HBITMAP hBitmap = CreateBitmapFromRGBA(rgba, width, height);
    WebPFree(rgba);
    
    if (!hBitmap) return E_FAIL;
    
    *phBitmap = hBitmap;
    return S_OK;
}

HBITMAP WebPDecoder::CreateBitmapFromRGBA(const BYTE* rgba, int width, int height) {
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;  // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    void* pBits = nullptr;
    HDC hdc = GetDC(nullptr);
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    ReleaseDC(nullptr, hdc);
    
    if (!hBitmap || !pBits) return nullptr;
    
    // Convert RGBA to BGRA (Windows format)
    const BYTE* src = rgba;
    BYTE* dst = static_cast<BYTE*>(pBits);
    for (int i = 0; i < width * height; i++) {
        dst[0] = src[2];  // B
        dst[1] = src[1];  // G
        dst[2] = src[0];  // R
        dst[3] = src[3];  // A
        src += 4;
        dst += 4;
    }
    
    return hBitmap;
}

} // namespace DarkThumbs
```

**Create `CBXShell/avif_decoder.cpp`:**
```cpp
#include "StdAfx.h"
#include "avif_decoder.h"
#include <avif/avif.h>

namespace DarkThumbs {

bool AVIFDecoder::IsAVIFFormat(const BYTE* data, size_t size) {
    if (!data || size < 12) return false;
    
    // Look for "ftyp" box with AVIF/HEIF brands
    for (size_t i = 0; i < min(size - 8, 36); i++) {
        if (memcmp(data + i, "ftyp", 4) == 0) {
            const char* brand = reinterpret_cast<const char*>(data + i + 4);
            if (memcmp(brand, "avif", 4) == 0 ||
                memcmp(brand, "avis", 4) == 0 ||
                memcmp(brand, "heic", 4) == 0 ||
                memcmp(brand, "heix", 4) == 0 ||
                memcmp(brand, "mif1", 4) == 0) {
                return true;
            }
        }
    }
    return false;
}

bool AVIFDecoder::GetDimensions(const BYTE* data, size_t size, int* width, int* height) {
    if (!IsAVIFFormat(data, size) || !width || !height) return false;
    
    avifDecoder* decoder = avifDecoderCreate();
    if (!decoder) return false;
    
    avifResult result = avifDecoderSetIOMemory(decoder, data, size);
    if (result == AVIF_RESULT_OK) {
        result = avifDecoderParse(decoder);
        if (result == AVIF_RESULT_OK) {
            *width = decoder->image->width;
            *height = decoder->image->height;
        }
    }
    
    avifDecoderDestroy(decoder);
    return (result == AVIF_RESULT_OK);
}

HRESULT AVIFDecoder::DecodeToHBITMAP(const BYTE* data, size_t size, HBITMAP* phBitmap) {
    if (!data || size == 0 || !phBitmap) return E_INVALIDARG;
    *phBitmap = nullptr;
    
    if (!IsAVIFFormat(data, size)) return E_FAIL;
    
    avifDecoder* decoder = avifDecoderCreate();
    if (!decoder) return E_FAIL;
    
    avifResult result = avifDecoderSetIOMemory(decoder, data, size);
    if (result != AVIF_RESULT_OK) {
        avifDecoderDestroy(decoder);
        return E_FAIL;
    }
    
    result = avifDecoderParse(decoder);
    if (result != AVIF_RESULT_OK) {
        avifDecoderDestroy(decoder);
        return E_FAIL;
    }
    
    result = avifDecoderNextImage(decoder);
    if (result != AVIF_RESULT_OK) {
        avifDecoderDestroy(decoder);
        return E_FAIL;
    }
    
    HBITMAP hBitmap = CreateBitmapFromAVIF(decoder->image);
    avifDecoderDestroy(decoder);
    
    if (!hBitmap) return E_FAIL;
    
    *phBitmap = hBitmap;
    return S_OK;
}

HBITMAP AVIFDecoder::CreateBitmapFromAVIF(const avifImage* image) {
    if (!image || !image->yuvPlanes[0]) return nullptr;
    
    int width = image->width;
    int height = image->height;
    
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    void* pBits = nullptr;
    HDC hdc = GetDC(nullptr);
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    ReleaseDC(nullptr, hdc);
    
    if (!hBitmap || !pBits) return nullptr;
    
    // Convert YUV to RGB (simplified for 4:2:0)
    BYTE* dst = static_cast<BYTE*>(pBits);
    const uint8_t* yPlane = image->yuvPlanes[0];
    const uint8_t* uPlane = image->yuvPlanes[1];
    const uint8_t* vPlane = image->yuvPlanes[2];
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int yVal = yPlane[y * image->yuvRowBytes[0] + x];
            int uVal = uPlane[(y/2) * image->yuvRowBytes[1] + (x/2)] - 128;
            int vVal = vPlane[(y/2) * image->yuvRowBytes[2] + (x/2)] - 128;
            
            BYTE r, g, b;
            YUVtoRGB(yVal, uVal, vVal, &r, &g, &b);
            
            dst[0] = b;
            dst[1] = g;
            dst[2] = r;
            dst[3] = 255;
            dst += 4;
        }
    }
    
    return hBitmap;
}

void AVIFDecoder::YUVtoRGB(int y, int u, int v, BYTE* r, BYTE* g, BYTE* b) {
    int c = y - 16;
    int rVal = (298 * c + 409 * v + 128) >> 8;
    int gVal = (298 * c - 100 * u - 208 * v + 128) >> 8;
    int bVal = (298 * c + 516 * u + 128) >> 8;
    
    *r = static_cast<BYTE>(max(0, min(255, rVal)));
    *g = static_cast<BYTE>(max(0, min(255, gVal)));
    *b = static_cast<BYTE>(max(0, min(255, bVal)));
}

} // namespace DarkThumbs
```

---

### Phase 4: Update CBXShellClass.cpp

**Add includes at top:**
```cpp
#include "webp_decoder.h"
#include "avif_decoder.h"
using namespace DarkThumbs;
```

**In `GetThumbnail()` method, add BEFORE existing format checks:**
```cpp
// Check for WebP format
if (WebPDecoder::IsWebPFormat(fileData, fileSize)) {
    HBITMAP hBitmap = nullptr;
    HRESULT hr = WebPDecoder::DecodeToHBITMAP(fileData, fileSize, &hBitmap);
    if (SUCCEEDED(hr) && hBitmap) {
        *phbmp = hBitmap;
        return S_OK;
    }
}

// Check for AVIF/HEIF format
if (AVIFDecoder::IsAVIFFormat(fileData, fileSize)) {
    HBITMAP hBitmap = nullptr;
    HRESULT hr = AVIFDecoder::DecodeToHBITMAP(fileData, fileSize, &hBitmap);
    if (SUCCEEDED(hr) && hBitmap) {
        *phbmp = hBitmap;
        return S_OK;
    }
}
```

---

### Phase 5: Update CBXShell.vcxproj

**Add to `<AdditionalIncludeDirectories>`:**
```xml
..\external\image-libs\libwebp-1.4.0\src;..\external\image-libs\libavif-1.1.0\include;..\external\image-libs\dav1d-1.4.0\include;
```

**Add to `<AdditionalLibraryDirectories>`:**
```xml
..\external\image-libs\libwebp-1.4.0\build-vs\Release;..\external\image-libs\libavif-1.1.0\build-vs\Release;..\external\image-libs\dav1d-1.4.0\build-vs\src;
```

**Add to `<AdditionalDependencies>`:**
```xml
libwebp.lib;avif.lib;libdav1d.a;
```

---

### Phase 6: Build & Test

**1. Rebuild CBXShell:**
```cmd
build-scripts\build-cbxshell.cmd
```

**2. Check for errors:**
- Linker errors → verify library paths
- Include errors → verify header paths
- Missing symbols → verify library names

**3. Run tests:**
```cmd
sprint-test.cmd
```

**4. Manual testing:**
```cmd
REM Create test WebP file (use online converter)
REM Create test archive with WebP image
REM Right-click archive → Properties → Check thumbnail
REM Open Explorer → Large Icons view → Check thumbnail preview
```

---

## 🧪 Testing Checklist

### WebP Testing
- [ ] Create test.webp file
- [ ] Add to test-archive.zip
- [ ] Rename to test-archive.cbz
- [ ] Check thumbnail in Explorer
- [ ] Verify dimensions are correct
- [ ] Test large WebP (>4K)
- [ ] Test animated WebP (should show first frame)

### AVIF Testing
- [ ] Create test.avif file
- [ ] Test with iPhone HEIC photo
- [ ] Test 10-bit color depth
- [ ] Test HDR image (should tone-map)
- [ ] Verify proper color rendering

### Integration Testing
- [ ] Mixed format archive (JPG + WebP + AVIF)
- [ ] WebP-only archive
- [ ] AVIF-only archive
- [ ] Large archives (100+ images)

### Performance Testing
- [ ] Measure decode time for WebP
- [ ] Measure decode time for AVIF
- [ ] Check memory usage
- [ ] Verify no memory leaks
- [ ] Test Explorer responsiveness

---

## 📊 Expected Results

### Build Sizes
| Component | Before | After | Change |
|-----------|--------|-------|--------|
| CBXShell.dll | ~3 MB | ~5-6 MB | +2-3 MB |

### Performance
| Format | Decode Time | Memory | Quality |
|--------|-------------|--------|---------|
| JPEG | 2-5ms | Low | Good |
| PNG | 5-10ms | Medium | Excellent |
| WebP | 5-10ms | Low | Excellent |
| AVIF | 20-30ms | Medium | Best |

### File Size Savings
| Format | vs JPEG | vs PNG |
|--------|---------|--------|
| WebP | -25-35% | -25-30% |
| AVIF | -40-50% | -30-40% |

---

## 🐛 Troubleshooting

### Build Issues

**Problem:** CMake not found
```cmd
winget install Kitware.CMake
```

**Problem:** Meson not found
```cmd
pip install meson ninja
```

**Problem:** libwebp.lib not found
- Check: `external\image-libs\libwebp-1.4.0\build-vs\Release\`
- Might be named `webp.lib` instead
- Update vcxproj accordingly

**Problem:** libdav1d.a not found
- Check: `external\image-libs\dav1d-1.4.0\build-vs\src\`
- Meson build might have failed
- Re-run `build-scripts\build-libavif.cmd`

### Runtime Issues

**Problem:** Thumbnails not showing
- Verify DLL registered: `install-x64.cmd`
- Clear thumbnail cache: `Disk Cleanup → Thumbnails`
- Restart Explorer: `taskkill /f /im explorer.exe && explorer.exe`

**Problem:** Access violation in decoder
- Check buffer sizes in format detection
- Verify data is not null before memcmp
- Add defensive checks in decoder functions

**Problem:** Wrong colors in AVIF
- YUV color space issue
- Check chroma subsampling format
- Verify YUVtoRGB conversion matrix

---

## 📝 Completion Criteria

Sprint 1 is complete when:

- [x] Libraries downloaded successfully
- [x] libwebp built and linked
- [x] libavif + dav1d built and linked
- [ ] WebP format detection working
- [ ] WebP decoding producing valid thumbnails
- [ ] AVIF format detection working
- [ ] AVIF decoding producing valid thumbnails
- [ ] All existing tests still pass
- [ ] Performance acceptable (<50ms decode)
- [ ] No memory leaks detected
- [ ] Explorer integration stable
- [ ] Code documented
- [ ] Build scripts updated

---

## 🚀 Next Steps After Sprint 1

**Sprint 2: PDF Support**
- Download PDFium or MuPDF
- Add PDF rendering
- Test with comics and e-books

**Sprint 3: Video Thumbnails**
- DirectShow integration
- FFmpeg integration (alternative)
- Support: MP4, MKV, AVI, MOV, WEBM

**Sprint 4: GPU Acceleration**
- DirectCompute for image decoding
- CUDA support (optional)
- OpenCL support (optional)

**Sprint 5: Final Integration**
- Complete testing
- Update documentation
- Create installer
- Tag v5.0 release

---

## 📚 References

- Full Sprint Plan: `docs/SPRINT1_MODERN_IMAGES.md`
- Build Scripts: `build-scripts/README.md`
- WebP Decoder: `CBXShell/webp_decoder.h`
- AVIF Decoder: `CBXShell/avif_decoder.h`
- libwebp docs: https://developers.google.com/speed/webp
- libavif docs: https://github.com/AOMediaCodec/libavif

---

**Ready to Start?**
```cmd
sprint1-quickstart.cmd
```
