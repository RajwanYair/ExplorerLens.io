# DarkThumbs v5.0 - Sprint 3 Planning
**Date:** November 19, 2025  
**Current Status:** Sprints 1-2 Complete (WebP + HEIF)  
**Build:** CBXShell.dll 1.27 MB, ready for testing

---

## 🎯 Sprint 3 Evaluation

### Sprint 3A: PDF Support (Attempted)
**Goal:** Render first page of PDF files as thumbnails for comic archives

**Library Evaluated:** MuPDF 1.24.11  
**Download:** ✅ 51.5 MB source downloaded  
**Extract:** ❌ Windows long path limit exceeded  
**Issue:** `thirdparty/freeglut/progs/test-shapes-gles1/android_toolchain.cmake` path > 260 chars

**Alternative Approaches:**
1. **PDFium** (Google Chrome's PDF renderer)
   - Pros: Prebuilt binaries available, excellent rendering
   - Cons: MASSIVE (30-50 MB DLL), complex integration
   - Verdict: Too large for thumbnail DLL

2. **Windows PDF Platform**
   - Pros: Native Windows 10/11 API, zero dependencies
   - Cons: Requires Windows.Data.Pdf (UWP/C++/WinRT), complex COM
   - Verdict: Possible but requires significant refactoring

3. **Poppler** (based on Xpdf)
   - Pros: Smaller than MuPDF (~10 MB), good quality
   - Cons: Complex dependencies (FreeType, Cairo, etc.)
   - Verdict: Similar complexity to MuPDF

**Recommendation:** **DEFER PDF to future sprint** - Too complex for quick iteration

---

## 🚀 Sprint 3B: Performance & Polish (RECOMMENDED)

### Quick Wins That Add Value

#### 1. Thumbnail Cache Optimization ⚡
**Problem:** Regenerates thumbnails every time  
**Solution:** Cache thumbnails in `%LOCALAPPDATA%\DarkThumbs\cache\`

**Implementation:**
```cpp
// Generate cache key from file path + mtime + size
std::string cacheKey = MD5(archivePath + std::to_string(mtime) + std::to_string(size));
std::wstring cachePath = GetCachePath(cacheKey);

if (CacheExists(cachePath) && !IsStale(cachePath, mtime)) {
    return LoadFromCache(cachePath);
}

// Generate thumbnail...
SaveToCache(hBitmap, cachePath);
```

**Benefits:**
- 10-100x faster thumbnail display for revisited archives
- Reduces CPU/battery usage
- Better UX - instant thumbnails

**Time:** 2-3 hours  
**Files:** Add `thumbnail_cache.cpp/.h`  
**Value:** HIGH - improves everyday use

---

#### 2. Multi-threaded Decoding 🧵
**Problem:** Decodes images sequentially (slow for large archives)  
**Solution:** Thread pool for parallel image decoding

**Implementation:**
```cpp
// In cbxArchive.h
#include <thread>
#include <future>

// Decode WebP/HEIF in background thread
std::future<HBITMAP> future = std::async(std::launch::async, [&]() {
    return DecodeImage(buffer.data(), streamSize);
});

// Do other work...

HBITMAP hBitmap = future.get(); // Wait for result
```

**Benefits:**
- 2-4x faster on multi-core CPUs
- Better responsiveness
- Smoother Explorer experience

**Time:** 1-2 hours  
**Files:** Modify `cbxArchive.h`  
**Value:** MEDIUM - noticeable on large archives

---

#### 3. Error Handling & Logging 📝
**Problem:** Silent failures - users don't know why thumbnails fail  
**Solution:** Add logging to `%TEMP%\DarkThumbs.log`

**Implementation:**
```cpp
// Logger.h
class Logger {
public:
    static void Info(const std::string& msg);
    static void Error(const std::string& msg);
    static void Debug(const std::string& msg);
};

// In decoders
if (FAILED(hr)) {
    Logger::Error("WebP decode failed: " + GetErrorMessage(hr));
    return E_FAIL;
}
```

**Benefits:**
- Easier debugging for users
- Better issue reports
- Professional quality

**Time:** 1 hour  
**Files:** Add `logger.cpp/.h`  
**Value:** MEDIUM - helps support

---

#### 4. Modern Dark Mode Detection 🌙
**Problem:** Using registry hack for dark mode  
**Solution:** Use Windows 10/11 UISettings API

**Implementation:**
```cpp
// In DarkModeHelper.h
#include <winrt/Windows.UI.ViewManagement.h>

bool IsSystemDarkMode() {
    using namespace winrt::Windows::UI::ViewManagement;
    UISettings settings;
    auto foreground = settings.GetColorValue(UIColorType::Foreground);
    // Dark mode: foreground is white/light
    return (foreground.R + foreground.G + foreground.B) > 382; // > 50% brightness
}
```

**Benefits:**
- More reliable dark mode detection
- Works on all Windows 10/11 builds
- Future-proof

**Time:** 30 minutes  
**Files:** Modify `DarkModeHelper.h`  
**Value:** LOW - current method works fine

---

#### 5. Additional Image Format: AVIF (libaom) 🖼️
**Complexity:** HIGH (similar to dav1d issues)  
**Value:** MEDIUM (AVIF less common than HEIF)  
**Recommendation:** Skip for now

---

#### 6. Additional Image Format: BMP/PNG Optimization
**Problem:** CImage fallback is slow for large PNGs  
**Solution:** Use libpng for faster PNG, stb_image for BMP

**Benefits:**
- Faster PNG thumbnails
- Smaller code than CImage

**Time:** 2 hours  
**Value:** LOW - CImage works fine

---

## ✅ Recommended Sprint 3: Performance Sprint

### Implementation Plan

**Sprint 3: "Performance & Cache"**
1. ✅ **Thumbnail Cache System** (2-3 hours) - MUST HAVE
   - Cache directory: `%LOCALAPPDATA%\DarkThumbs\cache\`
   - Key: MD5(path + mtime + size)
   - Auto-cleanup: Delete if > 1000 cached thumbnails or > 500 MB
   
2. ✅ **Background Thread Decoding** (1-2 hours) - NICE TO HAVE
   - Thread pool for WebP/HEIF decoding
   - Async thumbnail generation
   
3. ✅ **Basic Logging** (1 hour) - DEBUGGING
   - Log file: `%TEMP%\DarkThumbs.log`
   - Log levels: ERROR, WARN, INFO
   - Size limit: 10 MB, rotate on overflow

**Total Time:** 4-6 hours  
**Value:** HIGH - dramatically improves user experience  
**Risk:** LOW - non-breaking enhancements  
**Testing:** Easy - just use Explorer and observe speed improvement

---

## 🎯 Alternative Sprint 3: UI Enhancements

### CBXManager Improvements

**Current State:** Basic registration tool  
**Enhancements:**
1. Show thumbnail preview in manager
2. Display registered formats list
3. Add "Repair Registration" button
4. Show cache statistics (size, count, clear)
5. Dark mode UI

**Time:** 3-4 hours  
**Value:** MEDIUM - better UX for configuration  
**Risk:** LOW - standalone exe changes

---

## 📊 Sprint Recommendation Matrix

| Sprint | Time | Value | Complexity | User Impact |
|--------|------|-------|------------|-------------|
| **Thumbnail Cache** | 2-3h | ⭐⭐⭐⭐⭐ | LOW | 🚀 HUGE |
| **Thread Pool Decoding** | 1-2h | ⭐⭐⭐ | MEDIUM | 📈 Good |
| **Logging System** | 1h | ⭐⭐ | LOW | 🔧 Debug |
| **PDF Support** | 6-8h | ⭐⭐⭐⭐ | HIGH | 📄 Great |
| **AVIF Support** | 4-6h | ⭐⭐ | HIGH | 🖼️ Niche |
| **UI Enhancements** | 3-4h | ⭐⭐⭐ | LOW | 🎨 Nice |

---

## ✅ Final Recommendation

### Sprint 3: **Performance & Cache System**

**Why?**
1. **Highest ROI** - 2-3 hours for 10-100x speed improvement
2. **Universal benefit** - Every user notices faster thumbnails
3. **Low risk** - Cache is optional, no breaking changes
4. **Easy to test** - Open same archive twice, see instant thumbnails
5. **Professional quality** - Real apps cache thumbnails

**Implementation Order:**
1. **Cache system** (must-have) - 2-3 hours
2. **Logging** (debugging) - 1 hour
3. **Thread pool** (if time allows) - 1-2 hours

**Defer to Sprint 4:**
- PDF support (needs simpler approach or prebuilt library)
- AVIF support (diminishing returns)
- UI enhancements (nice-to-have)

---

## 🚀 Next Steps

### Immediate Action
1. **Implement thumbnail cache system**
   - Create `CBXShell/thumbnail_cache.h/.cpp`
   - Add cache key generation (MD5 hash)
   - Implement cache load/save
   - Add cache directory management
   
2. **Test cache system**
   - Build CBXShell with cache
   - Open archive (slow - generates thumbnail)
   - Close and re-open (fast - loads from cache)
   
3. **Add logging for debugging**
   - Create `CBXShell/logger.h/.cpp`
   - Log cache hits/misses
   - Log decode errors

### Success Criteria
- ✅ Second access to archive is instant (< 50ms)
- ✅ Cache size stays reasonable (< 500 MB)
- ✅ Logs help debug issues
- ✅ No regressions in existing functionality

---

**Decision:** Proceed with **Sprint 3: Performance & Cache**  
**Reason:** Highest user value, lowest complexity, best ROI  
**Estimated Completion:** 3-5 hours total
