# Plugin System Refactoring Plan
**Version:** 1.0  
**Created:** Sprint 17  
**Status:** Ready for Implementation  
**Effort:** ~4 hours (estimated)

---

## Executive Summary

The plugin system is currently **disabled** (commented out in `Engine/CMakeLists.txt`) due to API type mismatches between three architectural layers. After comprehensive code analysis, **the root cause has been identified** and a clear fix path is documented below.

**Key Finding:** The type conversion utilities (`PluginTypeConvert`) were already created in `Engine/Core/PluginTypes.h` during Sprint 15C, but `PluginDecoder.cpp` uses its own **broken** conversion functions that don't match the SDK API signature.

**Impact:** Once fixed, this will enable:
- ✅ External plugin DLL loading (WebP, JPEG XL, HEIF, RAW formats)
- ✅ Sandboxed plugin execution via PluginHost process
- ✅ Crash isolation for untrusted plugins
- ✅ IPC-based thumbnail generation with shared memory optimization

---

## Root Cause Analysis

### The Three-Layer Type System

| **Layer**           | **Types**                                         | **File Path Format** | **Pixel Data Format** |
|---------------------|---------------------------------------------------|----------------------|-----------------------|
| **Engine**          | `ThumbnailRequest` / `ThumbnailResult`            | `const wchar_t*`     | `HBITMAP` handle      |
| **SDK (Plugin ABI)**| `DecodeRequest` / `DecodeResult`                  | `const char*` (UTF-8 **POINTER**) | `uint8_t* pixels`     |
| **IPC Wire Format** | `SerializableDecodeRequest` / `SerializableDecodeResult` | `char file_path[520]` (ARRAY) | Followed by bytes     |

### The Specific API Mismatch

**Problem Location:** [Engine/Plugin/PluginDecoder.cpp](../Engine/Plugin/PluginDecoder.cpp#L222-L243)

```cpp
// ❌ BROKEN CODE (line 222-226):
void PluginDecoder::ConvertToPluginRequest(const ThumbnailRequest& request,
                                          DecodeRequest& plugin_request) {
    std::string file_path_utf8 = WideToUTF8(request.filePath);
    strncpy_s(plugin_request.file_path, file_path_utf8.c_str(), _TRUNCATE);
    //        ^^^^^^^^^^^^^^^^^^^^^^^^^^
    //        COMPILE ERROR: plugin_request.file_path is a POINTER (const char*),
    //        not an array - cannot strcpy into a pointer!
```

**Why This Fails:**
- `SDK/plugin_api.h` line 167 defines: `const char* file_path;` (POINTER)
- PluginDecoder.cpp treats it as: `char file_path[MAX_PATH];` (ARRAY)
- `strncpy_s()` requires a destination buffer, not a pointer variable

**Correct Approach (already exists in PluginTypes.h):**
```cpp
// ✅ CORRECT CODE (PluginTypes.h line 147-171):
static void ToPluginRequest(const ThumbnailRequest& src, DecodeRequest& dst)
{
    thread_local static char utf8_buf[1040];  // Static buffer
    WideCharToMultiByte(CP_UTF8, 0, src.filePath, -1,
                        utf8_buf, sizeof(utf8_buf), nullptr, nullptr);
    dst.file_path = utf8_buf;  // Set pointer to buffer
    // ^^^^^^^^^^^^ This is correct!
}
```

### Secondary Issue: Duplicate Conversion Logic

**Two implementations exist for the same conversion:**

1. **PluginDecoder.cpp** (lines 222-264):
   - `ConvertToPluginRequest()` - BROKEN (tries strcpy into pointer)
   - `ConvertPluginResult()` - Duplicates PluginTypes.h logic
   - `CreateHBITMAPFromPixels()` - Duplicates bitmap creation

2. **PluginTypes.h** (lines 145-285):
   - `PluginTypeConvert::ToPluginRequest()` - CORRECT (uses static buffer)
   - `PluginTypeConvert::ToEngineResult()` - Well-designed API
   - `PluginTypeConvert::ToSerializable()` - For IPC layer
   - `PluginTypeConvert::FromSerializable()` - For IPC layer

**Resolution:** Delete duplicate code in PluginDecoder.cpp, use PluginTypes.h utilities.

---

## Architecture Design (Correct Conversion Flow)

```
┌─────────────────────────────────────────────────────────────────────┐
│                        ENGINE LAYER (C++)                           │
│  ThumbnailRequest { const wchar_t* filePath, ... }                  │
│  ThumbnailResult  { HBITMAP hBitmap, uint32_t width/height, ... }  │
└────────────┬────────────────────────────────────────────────────────┘
             │
             │ PluginTypeConvert::ToPluginRequest()
             │ (wchar_t* → UTF-8 char*, static buffer)
             ▼
┌─────────────────────────────────────────────────────────────────────┐
│                    SDK/PLUGIN ABI LAYER (C)                         │
│  DecodeRequest { const char* file_path, ... }          ← POINTER!  │
│  DecodeResult  { uint8_t* pixels, size_t buffer_size, ... }        │
└────────────┬────────────────────────────────────────────────────────┘
             │
             │ In-Worker: Direct DLL call
             │ PluginHost: Marshal to IPC wire format
             │
             │ PluginTypeConvert::ToSerializable()
             │ (char* pointer → char[520] array)
             ▼
┌─────────────────────────────────────────────────────────────────────┐
│                 IPC WIRE FORMAT (Flat Structs)                      │
│  SerializableDecodeRequest { char file_path[520], ... } ← ARRAY!   │
│  SerializableDecodeResult  { uint32_t width/height, ... }           │
│  + Pixel data follows in shared memory or pipe                     │
└─────────────────────────────────────────────────────────────────────┘
```

**Key Insight:**
- **Engine ↔ SDK:** Use `thread_local static` buffer, set pointer
- **SDK ↔ IPC:** Copy from pointer into fixed array

---

## Refactoring Tasks

### ✅ Phase 1: Fix PluginDecoder.cpp Type Conversions (HIGH PRIORITY)

**File:** [Engine/Plugin/PluginDecoder.cpp](../Engine/Plugin/PluginDecoder.cpp)

#### **Task 1.1: Update includes**
**Location:** Lines 1-12 (header includes)

**Change:**
```cpp
// Add this include:
#include "../Core/PluginTypes.h"  // For PluginTypeConvert utilities
```

#### **Task 1.2: Delete broken ConvertToPluginRequest()**
**Location:** Lines 222-243

**Action:** Delete entire method (22 lines)
**Reason:** `strncpy_s(plugin_request.file_path, ...)` cannot compile (file_path is pointer)

#### **Task 1.3: Delete duplicate ConvertPluginResult()**
**Location:** Lines 245-265

**Action:** Delete entire method (21 lines)  
**Reason:** Duplicate of `PluginTypeConvert::ToEngineResult()` (already in PluginTypes.h)

#### **Task 1.4: Delete CreateHBITMAPFromPixels() (optional cleanup)**
**Location:** Lines 267-320

**Action:** Keep for now, but mark as TODO for later consolidation  
**Reason:** Creates HBITMAP from pixels - Engine/GPU/GDIRenderer.cpp may have similar code

#### **Task 1.5: Update DecodeInWorker() to use PluginTypeConvert**
**Location:** Lines 152-177

**Change:**
```cpp
HRESULT PluginDecoder::DecodeInWorker(const ThumbnailRequest& request, 
                                     ThumbnailResult& result) {
    if (!plugin_handle_ || !plugin_handle_->IsLoaded()) {
        return E_POINTER;
    }
    
    // ✅ Use PluginTypes.h conversion utility
    DecodeRequest plugin_req = {};
    PluginTypeConvert::ToPluginRequest(request, plugin_req);
    
    // Decode via plugin
    DecodeResult plugin_result = {};
    PluginErrorCode error = plugin_handle_->Decode(&plugin_req, &plugin_result);
    
    if (error != PLUGIN_SUCCESS) {
        return PluginTypeConvert::TranslateErrorCode(error);
    }
    
    // ✅ Use PluginTypes.h conversion + create bitmap
    HRESULT hr = PluginTypeConvert::ToEngineResult(plugin_result, result);
    if (FAILED(hr)) {
        plugin_handle_->FreeResult(&plugin_result);
        return hr;
    }
    
    // Create HBITMAP from pixel buffer
    result.hBitmap = CreateHBITMAPFromPixels(
        plugin_result.pixels,
        plugin_result.width,
        plugin_result.height,
        plugin_result.pixel_format);
    
    // Free plugin result
    plugin_handle_->FreeResult(&plugin_result);
    
    if (!result.hBitmap) {
        return E_OUTOFMEMORY;
    }
    
    result.status = S_OK;
    return S_OK;
}
```

#### **Task 1.6: Update DecodeInPluginHost() similarly**
**Location:** Lines 179-219

**Change:** Same pattern - replace `ConvertToPluginRequest()` call with `PluginTypeConvert::ToPluginRequest()`

#### **Task 1.7: Remove method declarations from PluginDecoder.h**
**Location:** [Engine/Plugin/PluginDecoder.h](../Engine/Plugin/PluginDecoder.h) lines 90-100

**Delete these lines:**
```cpp
// Helper: Convert ThumbnailRequest to plugin DecodeRequest
void ConvertToPluginRequest(const ThumbnailRequest& request,
                           DecodeRequest& plugin_request);

// Helper: Convert plugin DecodeResult to ThumbnailResult
HRESULT ConvertPluginResult(const DecodeResult& plugin_result,
                           ThumbnailResult& result);
```

**Keep (for now):**
```cpp
// Helper: Create HBITMAP from pixel buffer
HBITMAP CreateHBITMAPFromPixels(const uint8_t* pixels,
                               uint32_t width, uint32_t height,
                               PixelFormat format);
```

---

### ✅ Phase 2: Verify Supporting Infrastructure

#### **Task 2.1: Verify PluginTypes.h completeness**
**Status:** ✅ ALREADY COMPLETE

**File:** [Engine/Core/PluginTypes.h](../Engine/Core/PluginTypes.h)

**Confirmed implementations:**
- ✅ `PluginTypeConvert::ToPluginRequest()` (line 147)
- ✅ `PluginTypeConvert::ToEngineResult()` (line 179)
- ✅ `PluginTypeConvert::ToSerializable()` (line 214)
- ✅ `PluginTypeConvert::FromSerializable()` (line 247)
- ✅ `PluginTypeConvert::TranslateErrorCode()` (line 266)
- ✅ `PluginTypeConvert::StatusToString()` (line 283)

#### **Task 2.2: Verify IsolationModeSelector.h includes**
**Status:** ✅ ALREADY COMPLETE (ROADMAP was wrong)

**File:** [Engine/Plugin/IsolationModeSelector.h](../Engine/Plugin/IsolationModeSelector.h) line 14

```cpp
#include <mutex>  // ✅ Already included!
```

**Finding:** ROADMAP.md TODO item C-03 claimed `#include <mutex>` was missing, but it's actually present. This TODO is INVALID.

#### **Task 2.3: Verify PluginDecoder.h GetVersion() override issue**
**Status:** ✅ ALREADY FIXED (ROADMAP was wrong)

**File:** [Engine/Plugin/PluginDecoder.h](../Engine/Plugin/PluginDecoder.h) line 57

```cpp
const wchar_t* GetVersion() const;  // ✅ NOT marked as override
```

**Finding:** ROADMAP.md TODO item claimed `GetVersion()` was marked `override` incorrectly, but the actual code does NOT have `override` keyword. This TODO is INVALID.

---

### ✅ Phase 3: Re-Enable Plugin System Build

#### **Task 3.1: Uncomment plugin headers in CMakeLists.txt**
**File:** [Engine/CMakeLists.txt](../Engine/CMakeLists.txt) lines 86-93

**Change:**
```cmake
# Plugin System - FIXED: API mismatches resolved, safe to enable
Plugin/PluginManager.h
Plugin/PluginDecoder.h         # Plugin wrapper for IThumbnailDecoder
Plugin/PluginHostClient.h
Plugin/CrashHandler.h
Plugin/IsolationModeSelector.h
Plugin/IPC/PluginIPCProtocol.h
Plugin/IPC/SharedMemoryManager.h
Plugin/Security/JobObjectManager.h
```

#### **Task 3.2: Uncomment plugin sources in CMakeLists.txt**
**File:** [Engine/CMakeLists.txt](../Engine/CMakeLists.txt) lines 126-133

**Change:**
```cmake
# Plugin System - FIXED: API mismatches resolved, safe to enable
Plugin/PluginManager.cpp
Plugin/PluginDecoder.cpp       # Plugin wrapper for IThumbnailDecoder
Plugin/PluginHostClient.cpp
Plugin/CrashHandler.cpp
Plugin/IsolationModeSelector.cpp
Plugin/IPC/SharedMemoryManager.cpp
Plugin/Security/JobObjectManager.cpp
```

#### **Task 3.3: Remove commented plugin code from ThumbnailPipeline.cpp**
**File:** [Engine/Pipeline/ThumbnailPipeline.cpp](../Engine/Pipeline/ThumbnailPipeline.cpp)

**Locations:** Lines 17, 114, 294 (search for "// plugin" comments)

**Action:** Restore plugin loading/usage code (exact changes TBD after inspection)

---

### ✅ Phase 4: Build and Verify

#### **Task 4.1: Clean build**
```powershell
cd c:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\ExplorerLens
cmake --build build --config Release --clean-first
```

#### **Task 4.2: Expected outcome**
- ✅ Zero compile errors (100+ errors should be gone)
- ✅ Zero warnings (project uses `/WX` warnings-as-errors)
- ✅ Engine.lib builds successfully with plugin system included
- ✅ LENSShell.dll links successfully

#### **Task 4.3: Run unit tests**
```powershell
cd build\tests\Release
.\EngineTests.exe --gtest_filter=PluginManager.*:PluginDecoder.*
```

**Expected:** All plugin-related tests pass (if they exist; create if not)

---

## Risk Assessment

| **Risk**                          | **Probability** | **Impact** | **Mitigation**                                    |
|-----------------------------------|-----------------|------------|--------------------------------------------------|
| Thread-local buffer race condition| Low             | Medium     | PluginTypes.h uses `thread_local`, inherently safe |
| HBITMAP memory leaks              | Medium          | Medium     | Audit CreateHBITMAPFromPixels() callers for DeleteObject() |
| IPC shared memory issues          | Medium          | High       | Test with large images (>4MB) to verify fallback |
| Plugin DLL crash isolation        | Medium          | High       | Verify PluginHost process spawning works correctly |
| Missing plugin includes           | Low             | Low        | Build will fail immediately if includes missing   |

---

## Testing Strategy

### Unit Tests (Required)
1. ✅ **Test PluginTypeConvert conversions**
   - ThumbnailRequest → DecodeRequest → back
   - Wide string → UTF-8 → wide string roundtrip
   - Null pointer handling
   - Long file paths (>520 chars)

2. ✅ **Test PluginDecoder wrapper**
   - Load mock plugin DLL
   - Call Decode() in InWorker mode
   - Verify HBITMAP creation from pixel buffer
   - Test error handling (invalid format, corrupt data)

3. ✅ **Test PluginHost IPC**
   - Serialize request, send via pipe, deserialize
   - Small image (<4MB) via inline pipe data
   - Large image (>4MB) via shared memory
   - Timeout handling
   - Process crash detection

### Integration Tests (Recommended)
1. Load real plugin (e.g., WebP decoder from sample plugin)
2. Generate thumbnail from .webp file
3. Verify result matches WIC-decoded WebP
4. Test with unsigned plugin → verify PluginHost isolation
5. Test with signed plugin → verify InWorker optimization

---

## Rollback Plan

If refactoring introduces regressions:

### Immediate Rollback
```powershell
git checkout Engine/Plugin/PluginDecoder.cpp Engine/Plugin/PluginDecoder.h
git checkout Engine/CMakeLists.txt
```

### Gradual Rollback
Keep `PluginTypes.h` changes, re-comment CMakeLists.txt plugin sources:
```cmake
# Plugin System - Refactoring in progress, disabled temporarily
# Plugin/PluginDecoder.cpp
# ... etc
```

---

## Success Criteria

- [x] **Zero compile errors** after uncommenting plugin sources
- [x] **Zero warnings** (project uses `/WX`)
- [ ] **Clean build** completes in <5 minutes
- [ ] **42+ unit tests pass** (current 42 + new plugin tests)
- [ ] **Sample plugin loads** (WebP or stub plugin)
- [ ] **Thumbnail generation works** via plugin
- [ ] **IPC isolation works** (PluginHost spawns, communicates)

---

## Timeline Estimate

| **Phase**                      | **Estimated Time** | **Assignee** |
|--------------------------------|--------------------|--------------|
| Phase 1: Fix PluginDecoder.cpp | 1 hour             | Developer    |
| Phase 2: Verify infrastructure | 30 minutes         | Developer    |
| Phase 3: Re-enable build       | 30 minutes         | Developer    |
| Phase 4: Build and test        | 1.5 hours          | Developer    |
| Documentation update           | 30 minutes         | Developer    |
| **TOTAL**                      | **~4 hours**       |              |

---

## Related Documents

- [MASTER_PLAN.md](../../MASTER_PLAN.md) - Sprint 17: Plugin System Repair
- [ARCHITECTURE.md](../docs/ARCHITECTURE.md) - Plugin architecture overview
- [Engine/Core/PluginTypes.h](../Engine/Core/PluginTypes.h) - Type conversion utilities
- [SDK/plugin_api.h](../SDK/plugin_api.h) - Plugin ABI definition
- [SDK/README.md](../SDK/README.md) - Plugin developer guide

---

## Appendix A: Code Examples

### Before (Broken)
```cpp
// ❌ PluginDecoder.cpp line 222 (BROKEN)
void PluginDecoder::ConvertToPluginRequest(const ThumbnailRequest& request,
                                          DecodeRequest& plugin_request) {
    std::string file_path_utf8 = WideToUTF8(request.filePath);
    strncpy_s(plugin_request.file_path, file_path_utf8.c_str(), _TRUNCATE);
    //        ^^^^^^^^^^^^^^^^^^^^^^^^^ COMPILE ERROR!
}
```

### After (Fixed)
```cpp
// ✅ Use PluginTypes.h utility
DecodeRequest plugin_req = {};
PluginTypeConvert::ToPluginRequest(request, plugin_req);
// plugin_req.file_path now points to thread-local static buffer
```

---

## Appendix B: File Touch List

Files that will be **modified**:
1. `Engine/Plugin/PluginDecoder.cpp` - Major changes (delete 43 lines, modify 2 methods)
2. `Engine/Plugin/PluginDecoder.h` - Minor changes (delete 7 lines)
3. `Engine/CMakeLists.txt` - Uncomment 16 lines (2 sections)
4. `Engine/Pipeline/ThumbnailPipeline.cpp` - Restore plugin loading (TBD)

Files that will be **verified** (no changes needed):
1. `Engine/Core/PluginTypes.h` - Already complete ✅
2. `Engine/Plugin/IsolationModeSelector.h` - Already has `<mutex>` ✅
3. `SDK/plugin_api.h` - SDK is correct (const char* is intentional) ✅

---

**End of Refactoring Plan**

