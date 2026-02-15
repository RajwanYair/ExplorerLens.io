# External Library Update Plan - DarkThumbs v5.1.0

**Date**: November 23, 2025
**Goal**: Download latest versions, build as static libraries, achieve 100% static linking

## Current Library Versions

| Library          | Current | Latest (Nov 2025) | License       | Status           |
| ---------------- | ------- | ----------------- | ------------- | ---------------- |
| zlib             | 1.3.1   | 1.3.1             | Zlib          | ✅ Current       |
| minizip-ng       | 4.0.7   | 4.0.7             | Zlib          | ✅ Current       |
| bzip2            | 1.0.8   | 1.0.8             | BSD-like      | ✅ Current       |
| zstd             | 1.5.7   | 1.5.7             | BSD/GPLv2     | ✅ Current       |
| lz4              | 1.10.0  | 1.10.0            | BSD           | ✅ Current       |
| lzma (xz)        | 24.08   | 5.6.3             | Public Domain | 🔄 Check version |
| libwebp          | 1.5.0   | 1.5.0             | BSD           | ✅ Current       |
| libavif          | N/A     | 1.1.1             | BSD           | 🆕 Add support   |
| libjxl (JPEG XL) | N/A     | 0.11.0            | BSD           | 🆕 Add support   |
| UnRAR            | 7.2.1   | 7.2.1             | Proprietary   | ⚠️ DLL-only    |

## UnRAR Static Linking Solution

### Problem Analysis

The previous attempt failed because UnRAR's DLL interface (`dll.hpp`) uses `PASCAL` calling convention and expects `__declspec(dllexport)`, but the static library build doesn't provide this.

### Solution: Build UnRAR without DLL interface

**Approach**: Use UnRAR's **library interface** instead of DLL interface:

1. **Remove RARDLL preprocessor define** - This switches from DLL mode to static library mode
2. **Use static library functions** - UnRAR provides a different API for static linking
3. **Rebuild with proper exports** - Create .def file or modify source to export symbols

### Implementation Steps

#### Option 1: Use UnRAR Library Mode (RECOMMENDED)

```cpp
// Instead of dll.hpp functions:
// RAROpenArchiveEx(), RARCloseArchive(), etc.

// Use library mode functions:
#include "rar.hpp"
// Direct access to Archive class and extraction methods
```

**Advantages**:

- No DLL dependencies
- Full static linking
- Better integration with C++ code
- More control over extraction process

**Disadvantages**:

- Requires code refactoring in cbxArchive.h
- Different API than current implementation

#### Option 2: Create Custom Static Export (ALTERNATIVE)

```cpp
// Create wrapper that exports DLL-style functions from static lib
// unrar_static_wrapper.cpp:

extern "C" {
    __declspec(dllexport) HANDLE WINAPI RAROpenArchiveEx(RAROpenArchiveDataEx* data) {
        // Implementation using Archive class
    }
    // ... other functions
}
```

**Advantages**:

- Minimal code changes in cbxArchive.h
- Maintains current API

**Disadvantages**:

- Additional wrapper layer
- More complex build process

#### Option 3: Use Alternative RAR Library (SAFEST)

Use **libarchive** or **7-Zip SDK** for RAR support:

**libarchive** (v3.7.6):

- BSD license (safe for static linking)
- Supports RAR reading via libarchive_read_support_format_rar()
- Multi-format (ZIP, 7z, TAR, RAR, etc.)
- Actively maintained

**7-Zip SDK** (v24.08):

- LGPL/BSD dual license
- Native RAR5 support
- Smaller footprint than libarchive
- Used by many commercial products

## Recommended Implementation Plan

### Phase 1: Update Build Infrastructure

1. Create `build-scripts/download-latest-libs.ps1` - Automated download script
2. Create `build-scripts/build-static-libs.ps1` - Build all libraries as static
3. Update all .vcxproj files to disable DLL builds

### Phase 2: Library Updates

1. **Keep current versions** (already latest):

   - zlib 1.3.1
   - minizip-ng 4.0.7
   - bzip2 1.0.8
   - zstd 1.5.7
   - lz4 1.10.0
   - libwebp 1.5.0
2. **Verify LZMA/XZ version**:

   - Current: "24.08" (might be date-based)
   - Latest: 5.6.3
   - Check and update if needed

### Phase 3: UnRAR Static Linking - OPTION 3 (RECOMMENDED)

**Replace UnRAR with libarchive for RAR support:**

```powershell
# Download libarchive
Invoke-WebRequest -Uri "https://github.com/libarchive/libarchive/releases/download/v3.7.6/libarchive-3.7.6.zip" -OutFile "libarchive.zip"

# Build as static library
mkdir external/compression/libarchive-3.7.6
cd external/compression/libarchive-3.7.6
cmake -G "Visual Studio 17 2022" -A x64 `
    -DENABLE_WERROR=OFF `
    -DENABLE_TEST=OFF `
    -DBUILD_SHARED_LIBS=OFF `
    -DCMAKE_BUILD_TYPE=Release `
    ..
msbuild libarchive.sln /p:Configuration=Release /p:Platform=x64
```

**Code Changes** (cbxArchive.h):

```cpp
// Replace UnRAR includes
#include <archive.h>
#include <archive_entry.h>

// Replace CUnRar class with CArchiveReader class
class CArchiveReader {
    struct archive* m_archive;
public:
    BOOL Open(LPCTSTR filename) {
        m_archive = archive_read_new();
        archive_read_support_format_rar(m_archive);
        archive_read_support_format_rar5(m_archive);
        // ... implementation
    }
    // ... methods
};
```

**Benefits**:

- ✅ 100% static linking
- ✅ BSD license (no restrictions)
- ✅ Multi-format support (future-proof)
- ✅ Active maintenance and updates
- ✅ Used in production by FileZilla, 7-Zip, etc.

### Phase 4: Verification


1. Clean rebuild of all libraries
2. Update CBXShell.vcxproj with new library paths
3. Remove all DLL references
4. Test RAR extraction with libarchive
5. Verify binary size and performance

## Expected Results

### Before (Current):

- **Static Libraries**: 7/8 (87.5%)
- **DLL Dependencies**: UnRAR64.dll (338 KB)
- **Total Distribution**: CBXShell.dll (1.45 MB) + UnRAR64.dll (338 KB) = 1.78 MB

### After (Full Static):

- **Static Libraries**: 8/8 (100%) using libarchive
- **DLL Dependencies**: None
- **Total Distribution**: CBXShell.dll (~1.6-1.7 MB) = Single file deployment

### Binary Size Impact:

- libarchive static: ~400-500 KB (compressed code)
- Net increase: ~150-200 KB (libarchive is more efficient than UnRAR DLL)
- **Total CBXShell.dll**: Estimated 1.6-1.7 MB (all-in-one)

## Timeline

1. **Download libarchive** - 5 minutes
2. **Build static library** - 10 minutes
3. **Refactor cbxArchive.h** - 30-45 minutes
4. **Test and verify** - 15 minutes
5. **Update documentation** - 10 minutes

**Total**: ~90 minutes for complete static linking migration

## Approval Required

Shall I proceed with **Option 3** (libarchive replacement)?

Alternative: Proceed with **Option 1** (refactor to use UnRAR library mode)?

**Recommendation**: Option 3 (libarchive) for legal safety, better licensing, and multi-format support.
