# Sprint 15C Summary - Code Audit & Quality Sprint
**Date:** February 2026 (Session 2)  
**Version:** DarkThumbs v5.4.0 (bumped from v5.3.0)  
**Focus:** Deep code audit, TODO/FIXME resolution, scaling algorithms, enterprise foundations  

---

## ✅ Completed Tasks (18/25)

### Critical Fixes (Tasks 1-3)
1. **✅ Fix JXLDecoder.cpp corruption** - Removed duplicated code block (lines 122-143 were copy of Decode() tail), added proper `#ifdef HAS_LIBJXL` / `#else` / `#endif` guards around Decode() and DecodeJXLImage(). Without library, returns E_NOTIMPL.
2. **✅ Add HEIFDecoder #ifdef guards** - Wrapped all libheif API calls (heif_context_alloc, heif_decode_image, etc.) with `#ifdef HAS_LIBHEIF` guards in Decode(), DecodeHEIFImage(), and ExtractEmbeddedThumbnail().
3. **✅ CMake conditional feature flags** - Added `option(HAS_LIBJXL ...)`, `option(HAS_LIBHEIF ...)`, `option(HAS_LIBRAW ...)` to Engine/CMakeLists.txt. Library linking is now conditional.

### Test Re-enablement (Tasks 4-5)
4. **✅ Re-enable JXL unit tests** - Uncommented TestJXLDecoder_Create and TestJXLDecoder_CanDecode in EngineTests.cpp. These test interface methods (GetName, CanDecode) that don't need the actual JXL library.
5. **✅ Re-enable HEIF unit tests** - Uncommented TestHEIFDecoder_Create and TestHEIFDecoder_CanDecode. Test count: 38 → 42.

### Code Cleanup (Tasks 6-7)
6. **✅ Clean EngineAdapter.cpp stale TODO** - Replaced `// TODO: Complete JXL and HEIF decoder integration` and commented-out includes with clarifying comment about pipeline auto-registration.
7. **✅ Update CMake version to 5.4.0** - Bumped `DARKTHUMBS_VERSION` in Engine/CMakeLists.txt from 5.3.0 to 5.4.0.

### RAW Decoder Scaling (Tasks 8-9)
8. **✅ RAW thumbnail size scaling** - Implemented `ScaleToTarget()` method in RAWDecoder using GDI StretchBlt with HALFTONE mode. Integrated into ExtractEmbeddedThumbnail path. Preserves aspect ratio.
9. **✅ RAW full decode size scaling** - Integrated ScaleToTarget into DecodeFullRAW path. Removed `(void)targetWidth` TODO casts.

### SIMD Scaling Algorithms (Tasks 10-11)
10. **✅ SIMD NearestNeighbor scaling** - Implemented `ScaleBGRA_NearestNeighbor` in SIMDScaler.cpp using fixed-point integer arithmetic for fast pixel mapping. Added declaration to SIMDScaler.h.
11. **✅ Lanczos3 scaling implementation** - Implemented `ScaleBGRA_Lanczos3_Scalar` with 6×6 windowed sinc kernel (radius=3). Float precision with clamped output. Added declaration to SIMDScaler.h.

### New Components (Tasks 12-13)
12. **✅ Create PluginTypes.h** - Created `Engine/Core/PluginTypes.h` with:
    - `IPCTransferMode` enum (Pipe vs SharedMemory)
    - `PluginDecodeStatus` enum (Pending/InProgress/Completed/Failed/Timeout/Crashed/Cancelled)
    - `SerializableDecodeRequest` - flat, fixed-size IPC wire format for cross-process decode requests
    - `SerializableDecodeResult` - flat IPC response header with pixel data size
    - `PluginTypeConvert` struct with static conversion methods between Engine types, Plugin SDK types, and IPC serializable types
13. **✅ Create AuditLogger utility** - Created `Engine/Utils/AuditLogger.h` and `AuditLogger.cpp`:
    - 13 audit event types (thumbnail, plugin, cache, security, config events)
    - ISO 8601 timestamps with milliseconds
    - Log rotation (10MB threshold, 5 rotated files)
    - Registry-controlled enable/disable (HKLM/HKCU `Software\DarkThumbs\AuditLogging`)
    - Thread-safe singleton with mutex
    - Non-fatal: silently disabled if log file cannot be created

### Include & Comment Cleanup (Tasks 14-18)
14. **✅ Clean cbxArchive.h includes** - Removed commented-out `#include "gdiplus.h"`, `#pragma comment(lib,"gdiplus.lib")`, and `#include <atlstr.h>`.
15. **✅ Clean tools.h includes** - Replaced commented-out `#include "resource.h"` / `#include "Htmlhelp.h"` / `#pragma comment(lib,"Htmlhelp.lib")` with clear documentation comment about removal.
16. **✅ Document dark mode debug state** - Updated all 7 disabled dark mode features in MainDlg.cpp with structured tracking comments referencing Sprint 18 (WinUI Manager) and specific root causes.
17. **✅ PluginHostClient bitmap helper** - Extended `CreateHBITMAPFromPixels` in PluginDecoder.cpp to support RGB24, BGR24, and GRAY8 pixel formats in addition to existing BGRA32 and RGBA32.
18. **✅ Fix video_thumbnail include** - Updated comment explaining qedit.h was removed from Windows SDK 7.0+ and interfaces are defined inline.

### Remaining Tasks (19-25) - Documentation Sprint
19. **⬜ Update SPRINT_SUMMARY.md** - This document
20. **⬜ Create CODEBASE_AUDIT.md** - Comprehensive audit findings
21. **⬜ Create DECODER_STATUS.md** - Per-decoder status matrix
22. **⬜ Update README.md version** - Version to 5.4.0
23. **⬜ Create updated ROADMAP v4** - Post-audit roadmap refresh
24. **⬜ Create decoder verify script** - PowerShell script to verify decoder compilation
25. **⬜ Engine decoder status reporting** - Runtime decoder availability reporting

---

## 📝 Files Modified

### Engine/Decoders/
- [JXLDecoder.cpp](Engine/Decoders/JXLDecoder.cpp) - Fixed corruption, added #ifdef HAS_LIBJXL guards
- [HEIFDecoder.cpp](Engine/Decoders/HEIFDecoder.cpp) - Added #ifdef HAS_LIBHEIF guards
- [RAWDecoder.cpp](Engine/Decoders/RAWDecoder.cpp) - Added ScaleToTarget scaling method
- [RAWDecoder.h](Engine/Decoders/RAWDecoder.h) - Added ScaleToTarget declaration

### Engine/Utils/
- [SIMDScaler.cpp](Engine/Utils/SIMDScaler.cpp) - NearestNeighbor + Lanczos3 implementations
- [SIMDScaler.h](Engine/Utils/SIMDScaler.h) - Added NearestNeighbor + Lanczos3 declarations
- [AuditLogger.h](Engine/Utils/AuditLogger.h) - **NEW** - Enterprise audit logger header
- [AuditLogger.cpp](Engine/Utils/AuditLogger.cpp) - **NEW** - Enterprise audit logger implementation

### Engine/Core/
- [PluginTypes.h](Engine/Core/PluginTypes.h) - **NEW** - Unified plugin type bridge + IPC serialization

### Engine/
- [CMakeLists.txt](Engine/CMakeLists.txt) - Version bump 5.4.0, conditional feature flags
- [Tests/EngineTests.cpp](Engine/Tests/EngineTests.cpp) - Re-enabled JXL + HEIF tests (42 total)

### Engine/Plugin/
- [PluginDecoder.cpp](Engine/Plugin/PluginDecoder.cpp) - Extended CreateHBITMAPFromPixels for RGB24/BGR24/GRAY8

### CBXShell/
- [EngineAdapter.cpp](CBXShell/EngineAdapter.cpp) - Cleaned stale TODO comments
- [cbxArchive.h](CBXShell/cbxArchive.h) - Removed commented-out includes
- [video_thumbnail.cpp](CBXShell/video_thumbnail.cpp) - Updated qedit.h documentation

### CBXManager/
- [MainDlg.cpp](CBXManager/MainDlg.cpp) - Documented all 7 disabled dark mode features with Sprint 18 tracking
- [tools.h](CBXManager/tools.h) - Cleaned commented-out HTML Help includes

---

## 🔍 Key Findings from Code Audit

### Critical Issues Found & Fixed
1. **JXLDecoder had corrupted duplicate code** - ~20 lines duplicated, causing compilation errors
2. **HEIF/JXL decoders had no conditional compilation** - Would fail without libraries
3. **CMake linked libraries unconditionally** - Build would fail without all external libs present

### Issues Documented (Not Fixed - Intentional)
1. **MainDlg.cpp 7 disabled dark mode features** - Root causes documented, deferred to Sprint 18 WinUI rewrite
2. **unzip.cpp is a stub** - All 5 functions return NULL/E_NOTIMPL. Minizip-ng integration pending.
3. **svg_decoder.h is a placeholder** - Returns gradient bitmap with "SVG" text. Awaiting lunasvg integration.
4. **pdf_decoder.cpp returns E_NOTIMPL** - C++/WinRT implementation plan commented out.
5. **Plugin system commented out** - LoadPlugins() disabled in ThumbnailPipeline.cpp. Sprint 17 task.

### False Positives from Scan
- IsolationModeSelector.h `<mutex>` - Already included (line 14)
- ThumbnailCache.cpp GDI+ init - Already uses lazy singleton pattern
- RAWDecoder EXIF orientation - Already fully implemented (all 8 cases)

---

## 📊 Metrics
| Metric | Value |
|--------|-------|
| Files modified | 17 |
| Files created | 3 |
| Critical bugs fixed | 3 (JXL corruption, HEIF guards, CMake linking) |
| Tests re-enabled | 4 (JXL Create/CanDecode, HEIF Create/CanDecode) |
| Total test count | 42 |
| New algorithms | 2 (NearestNeighbor, Lanczos3) |
| Pixel format conversions added | 3 (RGB24, BGR24, GRAY8) |
| Version | 5.3.0 → 5.4.0 |
