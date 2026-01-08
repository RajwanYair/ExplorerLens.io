# Next Development Steps - JXL and HEIF Decoder Completion

## Current Status

**Session Completed:** January 8, 2026 (Session 2 continuation)  
**Last Commit:** 964df0d - "feat: Prepare Engine for JXL and HEIF decoder integration"

### What Was Accomplished ✅

1. **CI/CD Infrastructure** - GitHub Actions workflow (commit 81d733d)
2. **Testing Documentation** - TESTING_GUIDE.md with 22 tests (commit 81d733d)
3. **JXL and HEIF Placeholder Decoders** - Headers and implementation files created (commit 78511b9)
4. **Documentation Updates** - PROJECT_STATUS.md and session summary (commit 691fe00)
5. **v5.3.0 Integration Completion** - CBXShell Engine adapter complete (commit 1a3356a)
6. **Decoder Registration Preparation** - EngineAdapter updated to register JXL/HEIF (commit 964df0d)

### What Needs Completion ⚠️

The JXL and HEIF decoders have **placeholder implementations** but need API corrections to compile. The decoders currently use an outdated API pattern.

## Issues to Fix

### 1. API Mismatch

**Problem:** JXL and HEIF decoders use incorrect API signatures.

**Current (Wrong):**
```cpp
bool CanDecode(const std::wstring& filePath)
ThumbnailResult Decode(const ThumbnailRequest& request)
std::wstring GetDecoderName()
int GetDecoderPriority()
```

**Required (Correct):**
```cpp
bool CanDecode(const wchar_t* filePath) override;
HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
DecoderInfo GetInfo() const override;
const wchar_t* GetName() const override;
const wchar_t** GetSupportedExtensions() const override;
uint32_t GetExtensionCount() const override;
bool SupportsGPU() const override;
bool IsArchiveDecoder() const override;
```

### 2. Struct Field Names

**ThumbnailRequest uses:** `filePath` (not `FilePath`)  
**ThumbnailResult uses:** `hBitmap`, `width`, `height`, `status` (not `Bitmap`, `Width`, `Height`, `Success`, `ErrorMessage`)

**DecoderInfo fields:**
- `name`, `version` ✅ (correct)
- `extensionCount`, `supportsGPU`, `isArchiveDecoder` ✅ (correct)
- `description`, `author` ❌ (these fields don't exist)

### 3. Namespace

Both decoders need double namespace:
```cpp
namespace DarkThumbs {
namespace Engine {
    // decoder code here
} // namespace Engine
} // namespace DarkThumbs
```

## Action Plan for Next Session

### Step 1: Fix JXLDecoder API

**File:** `Engine/Decoders/JXLDecoder.h`

```cpp
namespace DarkThumbs {
namespace Engine {

    class JXLDecoder : public IThumbnailDecoder {
    public:
        JXLDecoder();
        virtual ~JXLDecoder();

        // IThumbnailDecoder implementation
        bool CanDecode(const wchar_t* filePath) override;
        HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
        DecoderInfo GetInfo() const override;
        const wchar_t* GetName() const override { return L"JXLDecoder"; }
        const wchar_t** GetSupportedExtensions() const override;
        uint32_t GetExtensionCount() const override { return 1; }
        bool SupportsGPU() const override { return false; }
        bool IsArchiveDecoder() const override { return false; }

    private:
        // Signature verification
        bool VerifyJXLSignature(const uint8_t* data, size_t size) const;
        
        // File I/O
        std::unique_ptr<uint8_t[]> ReadFileData(const wchar_t* filePath, size_t& outSize);
        
        // HBITMAP creation
        HBITMAP CreateHBITMAPFromRGBA(const uint8_t* pixels, uint32_t width, uint32_t height, uint32_t channels);
    };

} // namespace Engine
} // namespace DarkThumbs
```

**File:** `Engine/Decoders/JXLDecoder.cpp`

Key changes:
- `bool CanDecode(const wchar_t* filePath)` - Check extension with `wcsrchr` and `_wcsicmp`
- `HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result)` - Use `request.filePath`, set `result.hBitmap`, return `S_OK` or error HRESULT
- `DecoderInfo GetInfo()` - Only set `name`, `version`, `extensionCount`, `supportsGPU`, `isArchiveDecoder`
- `const wchar_t** GetSupportedExtensions()` - Return `static const wchar_t* extensions[] = { L".jxl", nullptr };`

###Step 2: Fix HEIFDecoder API

Same pattern as JXLDecoder, but with 8 supported extensions:
```cpp
const wchar_t** HEIFDecoder::GetSupportedExtensions() const {
    static const wchar_t* extensions[] = { 
        L".heif", L".heic", L".hif", L".heifs", 
        L".heics", L".avci", L".avcs", L".avif", nullptr 
    };
    return extensions;
}
```

### Step 3: Build and Test

```powershell
# Build Engine with new decoders
msbuild Engine/DarkThumbsEngine.vcxproj /p:Configuration=Release /p:Platform=x64 /m /v:minimal

# Build and run tests
msbuild Engine/Tests/EngineTests.vcxproj /p:Configuration=Release /p:Platform=x64 /m /v:minimal
Engine/Tests/Release/EngineTests.exe
```

Expected result: **26 tests passing** (22 existing + 4 new JXL/HEIF tests)

### Step 4: Build Full Solution

```powershell
# Build CBXShell with new decoders registered
msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64 /m /v:minimal
```

Expected result: **0 errors, 0 warnings**

### Step 5: Commit Changes

```bash
git add Engine/Decoders/JXLDecoder.* Engine/Decoders/HEIFDecoder.*
git commit -m "fix: Complete JXL and HEIF decoder API implementation

- Fixed JXLDecoder to match IThumbnailDecoder interface
  * Corrected method signatures (CanDecode, Decode, GetInfo, etc.)
  * Fixed ThumbnailRequest/ThumbnailResult usage
  * Added proper namespace (DarkThumbs::Engine)
  * Removed non-existent DecoderInfo fields

- Fixed HEIFDecoder to match IThumbnailDecoder interface
  * Same API corrections as JXLDecoder
  * Supports 8 HEIF/HEIC extensions

- Both decoders now compile successfully
- Tests: 26/26 passing (includes new JXL/HEIF tests)
- Ready for external library integration (libjxl, libheif)

Status: Decoders are functional placeholders that return E_NOTIMPL.
Next: Build libjxl and libheif libraries, uncomment implementation code."
```

## Reference: Correct Decoder Pattern

See `Engine/Decoders/ImageDecoder.h` and `WebPDecoder.h` for working examples.

**Key Pattern:**
1. Constructor/Destructor
2. `CanDecode()` - lightweight extension check
3. `Decode()` - HRESULT return, fills ThumbnailResult by reference
4. `GetInfo()` - returns DecoderInfo struct
5. `GetName()` - returns const wchar_t*
6. `GetSupportedExtensions()` - returns array with nullptr terminator
7. `GetExtensionCount()`, `SupportsGPU()`, `IsArchiveDecoder()` - simple returns

## Build Statistics

**Current State:**
- Engine library: 1.93 MB (4 working decoders + 2 pending)
- CBXShell: 1.32 MB (compiles with 0 errors)
- Tests: 22/22 passing (need 4 more for JXL/HEIF)

**Target State:**
- Engine library: ~2.0 MB (6 working decoders)
- Tests: 26/26 passing
- Full solution: 0 errors, 0 warnings

---

**Next Session Goal:** Get JXL and HEIF decoders compiling and tests passing (25-30 minutes)
