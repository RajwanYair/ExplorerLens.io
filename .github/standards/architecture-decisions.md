# ExplorerLens — Architecture Decision Records (ADRs)

**Version:** 15.0.0 "Zenith"
**Last Updated:** July 2025

---

## ADR Format

Each record captures: **Context** (why), **Decision** (what), **Consequences** (trade-offs).

---

## ADR-001: COM Shell Extension Architecture

**Date:** 2024-01  
**Status:** Accepted  

### Context

Windows Explorer discovers thumbnail providers via COM registration (IThumbnailProvider).
Alternative approaches (BHO, shell namespace extension) were considered but have
higher complexity and lower stability guarantees.

### Decision

Implement as an in-process COM DLL (`LENSShell.dll`) registered under CLSID
`{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}`. The DLL exports `DllGetClassObject`,
`DllCanUnloadNow`, `DllRegisterServer`, and `DllUnregisterServer`.

### Consequences

- **Pro:** Standard Windows pattern, low friction with Explorer
- **Pro:** Single DLL deployment (~2.9 MB)
- **Con:** Runs in Explorer's process — crashes can take down Explorer
- **Mitigation:** SEH exception wrapper + circuit breaker pattern in `GetThumbnail`

---

## ADR-002: Engine as Static Library

**Date:** 2024-03  
**Status:** Accepted  

### Context

The core decode/render engine could be a DLL or a static library.
DLL would allow independent updates but adds deployment complexity.

### Decision

Build `ExplorerLensEngine.lib` as a static library linked into `LENSShell.dll`.

### Consequences

- **Pro:** Single-file deployment (all code in LENSShell.dll)
- **Pro:** No DLL search path issues, no version conflicts
- **Pro:** Link-time optimization (LTCG) across engine + shell boundary
- **Con:** Full rebuild needed for engine changes
- **Con:** Larger DLL size (~2.9 MB)

---

## ADR-003: C++20 Standard with MSVC v145

**Date:** 2024-06  
**Status:** Accepted  

### Context

C++20 provides `std::format`, concepts, ranges, coroutines, and `<span>`.
C++23 was considered but MSVC support was incomplete at project start.

### Decision

Target C++20 (`/std:c++20`) with MSVC v145 toolset (cl.exe 19.50).

### Consequences

- **Pro:** `std::format` replaces `sprintf` for safe string formatting
- **Pro:** Concepts for template constraints (decoder interfaces)
- **Pro:** `std::span` for buffer-safe APIs
- **Con:** No `std::expected` (C++23) — use HRESULT error pattern instead

---

## ADR-004: GPU Pipeline — DirectX 11 Primary, DX12 + Vulkan Optional

**Date:** 2024-08  
**Status:** Accepted  

### Context

GPU acceleration for image scaling and format conversion.
DX12 offers lower overhead but higher complexity.
Vulkan provides vendor-neutral compute.

### Decision

- **Primary:** DirectX 11 (widest compatibility, simpler API)
- **Optional:** DirectX 12 for async compute on newer GPUs
- **Optional:** Vulkan compute for cross-vendor GPGPU
- **Fallback:** CPU-only path (GDI+ / WIC)

### Consequences

- **Pro:** Works on all Windows 10+ GPUs (DX11)
- **Pro:** DX12 path can exploit async copy/compute for throughput
- **Con:** Three GPU backends to maintain
- **Mitigation:** Abstracted via `IGPURenderer` interface

---

## ADR-005: MuPDF for PDF Rendering (AGPL-3.0)

**Date:** 2025-07  
**Status:** Accepted  

### Context

PDF thumbnail generation required a C/C++ library that:
1. Renders PDF pages to bitmaps
2. Handles complex documents (vector, fonts, transparency)
3. Builds with MSVC on Windows

Options evaluated:
- **MuPDF 1.24.11**: Complete renderer, AGPL-3.0 (or commercial license)
- **Poppler**: GPL, requires Cairo + fontconfig
- **PDFium**: BSD, but complex Chromium build system
- **Windows Shell fallback**: Depends on installed PDF viewer

### Decision

Use MuPDF 1.24.11 with AGPL-3.0 license. Build 6 sub-libraries via MSBuild
(`libmupdf`, `libthirdparty`, `libresources`, `libharfbuzz`, `libextract`, `libpkcs7`).
Gated behind `HAS_MUPDF` compile flag with shell-based fallback when not available.

### Consequences

- **Pro:** High-quality rendering with embedded fonts
- **Pro:** Self-contained (no external runtime dependencies)
- **Pro:** Handles vector PDF, transparency, CJK fonts
- **Con:** AGPL-3.0 requires open-source distribution or commercial license
- **Con:** Large libraries (~560 MB total in Release build)
- **Mitigation:** Feature-gated; fallback to Shell IThumbnailProvider

---

## ADR-006: Custom Test Framework (Not GTest)

**Date:** 2024-04  
**Status:** Accepted  

### Context

Google Test (GTest) is the standard C++ test framework, but it adds build complexity
and another external dependency.

### Decision

Use custom macros: `TEST(name)`, `RUN_TEST(name)`, `ASSERT(cond)` with
`g_testsRun/g_testsPassed/g_testsFailed` counters in `EngineTests.cpp`.

### Consequences

- **Pro:** Zero additional dependencies
- **Pro:** Extremely fast compile (single .cpp file, ~10K lines)
- **Pro:** Simple CTest integration
- **Con:** No test fixtures, parameterized tests, or death tests
- **Con:** No IDE test explorer integration (unlike GTest/Catch2)

---

## ADR-007: AVX2 as Default SIMD Target

**Date:** 2025-01  
**Status:** Accepted  

### Context

SIMD instruction sets: SSE2 (baseline), AVX, AVX2, AVX-512.
AVX2 (2013+) covers 95%+ of active Windows PCs.

### Decision

Compile with `/arch:AVX2` globally. No SSE2-only fallback path.

### Consequences

- **Pro:** 2-4x speedup for pixel operations, color conversion, scaling
- **Pro:** Simplified codegen (no runtime CPUID dispatch needed)
- **Con:** Won't run on pre-Haswell CPUs (2013 and older)
- **Mitigation:** Acceptable given Windows 10+ requirement

---

## ADR-008: Zero-Warnings Policy (/W4 /WX-)

**Date:** 2025-02  
**Status:** Accepted  

### Context

Warning discipline prevents latent bugs. `/WX` (warnings as errors) is ideal
but impractical with third-party headers that emit warnings.

### Decision

Build with `/W4 /WX-` (level 4 warnings, not treated as errors).
Target: zero warnings in CI. Use `#pragma warning(push/pop)` to suppress
known third-party header warnings (e.g., MuPDF C4100/C4611).

### Consequences

- **Pro:** High warning visibility without blocking on third-party issues
- **Pro:** Pragmatic balance between strictness and build stability
- **Con:** Not enforced by compiler — relies on CI/review discipline

---

## ADR-009: Plugin System via C ABI

**Date:** 2025-03  
**Status:** Accepted  

### Context

Plugins need ABI stability across compiler versions.
C++ ABI is fragile; COM is heavy for simple decode plugins.

### Decision

Plugins export C functions via `plugin_api.h` (SDK/).
Host loads via `LoadLibrary` + `GetProcAddress`.
Trust chain validates digital signatures before loading.

### Consequences

- **Pro:** ABI-stable across MSVC versions
- **Pro:** Plugins can be built with any C/C++ compiler
- **Con:** No C++ classes across boundary (use opaque handles)
- **Con:** Manual memory management at boundary

---

## ADR-010: Memory-Mapped I/O for Large Archives

**Date:** 2025-04  
**Status:** Accepted  

### Context

Extracting thumbnails from 500MB+ archives was slow (2.5s).
Traditional `fread()` copies data through kernel buffers.

### Decision

Use `CreateFileMapping` + `MapViewOfFile` for archive I/O.
Only map the sections needed (central directory + first image entry).

### Consequences

- **Pro:** 68% latency reduction (2.5s → 0.8s for 500MB archives)
- **Pro:** OS manages paging, no explicit buffer management
- **Con:** 32-bit address space limit (not relevant — x64 only)
- **Con:** Memory pressure with many simultaneous mappings
- **Mitigation:** Unmap immediately after extraction

---

## ADR-011: Multi-Threaded lcms2 (lcms2mt) for MuPDF

**Date:** 2025-07  
**Status:** Accepted  

### Context

MuPDF requires lcms2mt (multi-threaded fork of Little CMS 2)
rather than standard lcms2. The standard version uses global state
that isn't thread-safe for concurrent PDF rendering.

### Decision

Use ArtifexSoftware's lcms2mt fork (bundled in MuPDF thirdparty).
Compile with `HAVE_LCMS2MT=1` preprocessor definition.

### Consequences

- **Pro:** Thread-safe color management for concurrent PDF rendering
- **Pro:** Bundled with MuPDF source (no extra download)
- **Con:** Diverges from upstream lcms2 (Artifex maintains fork)

---

## ADR-012: Dynamic CRT Linkage (/MD)

**Date:** 2025-02  
**Status:** Accepted  

### Context

MSVC CRT can be linked statically (`/MT`) or dynamically (`/MD`).
Mixing causes linker errors (`LIBCMT` vs `MSVCRT` conflicts).

### Decision

All targets use `/MD` (dynamic CRT — `MultiThreadedDLL`).
Exception: libwebp was built with `/MT` — uses `/NODEFAULTLIB:LIBCMT` workaround.

### Consequences

- **Pro:** Smaller DLL (shares CRT with other loaded DLLs)
- **Pro:** CRT security updates applied system-wide
- **Con:** Requires MSVC redistributable on target machines
- **Con:** libwebp CRT mismatch requires linker workaround

---

**Document Version:** 1.0  
**Total ADRs:** 12
