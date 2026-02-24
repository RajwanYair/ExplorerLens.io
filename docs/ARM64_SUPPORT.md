# ARM64 Support — ExplorerLens v15.0.0

## Overview

ExplorerLens v15.0.0 introduces foundational ARM64 support as part of the Sprints 155–159 block.
This document describes the build configuration, library compatibility matrix, runtime validation
approach, performance baseline targets, and CI integration strategy for Windows ARM64.

---

## Architecture

ARM64 support is implemented as a **cross-compile target** using the MSVC v145 toolset with the
`amd64_arm64` cross-compilation environment. The primary development and CI host remains x64;
ARM64 binaries are produced via cross-compilation.

```
Host (x64)  ──cross-compile──▶  Target (ARM64 / Windows on ARM)
   MSVC v145                       ExplorerLensEngine.lib (ARM64)
   cl.exe (amd64_arm64)            LENSShell.dll (ARM64) [future]
```

---

## CMake Toolchain

Use the provided toolchain file to configure an ARM64 build:

```powershell
cmake -B build-arm64 -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-windows-arm64.cmake `
  -DARM64_BUILD=ON `
  -DBUILD_TESTS=ON
cmake --build build-arm64 --config Release -j 4
```

Toolchain file: [`cmake/toolchain-windows-arm64.cmake`](../cmake/toolchain-windows-arm64.cmake)

---

## Library Compatibility Matrix (Sprint 156)

| Library       | Version  | ARM64 MSVC Support | Notes                              |
|--------------|----------|--------------------|------------------------------------|
| zlib          | 1.3.1    | ✅ Full            | CMake builds cleanly               |
| LZ4           | 1.10.0   | ✅ Full            | Portable C, no NEON intrinsics req |
| zstd          | 1.5.7    | ✅ Full            | CMake + portable fallback          |
| LZMA SDK      | 26.00    | ✅ Full            | Pure C, architecture-agnostic      |
| minizip-ng    | 4.0.10   | ✅ Full            | Depends on zlib                    |
| UnRAR         | 7.2.2    | ⚠️ Partial        | No ARM64 NMake project; needs CMake |
| libwebp       | 1.5.0    | ✅ Full            | NEON path available on ARM64       |
| libavif       | 1.3.0    | ✅ Full            | dav1d has ARM64 NEON optimizations |
| libjxl        | 0.11.1   | ✅ Full            | Highway library targets ARM64      |
| libheif       | 1.19.5   | ⚠️ Partial        | libde265 needs validation          |
| LibRaw        | 0.21.3   | ✅ Full            | CMake-based, portable              |
| dav1d         | latest   | ✅ Full            | Assembly NEON optimizations        |

Legend: ✅ Validated | ⚠️ Partial (needs extra steps) | ❌ Not supported

---

## Runtime Validation (Sprint 157)

The `ARM64RuntimeValidator` class (see `Engine/Utils/ARM64RuntimeValidator.h`) performs
at-runtime checks to detect:

- **NEON intrinsics availability** — verifies ARM Advanced SIMD at runtime
- **Cache line size** — ARM64 typically uses 64-byte cache lines
- **Memory page size** — typically 4KB or 16KB on some ARM64 platforms
- **Endianness** — always little-endian on Windows ARM64
- **Pointer size** — 8 bytes (64-bit ABI)
- **Alignment requirements** — validates natural alignment assumptions

```cpp
#include "Utils/ARM64RuntimeValidator.h"

ARM64Capabilities caps = ARM64RuntimeValidator::DetectCapabilities();
if (caps.hasNEON) {
    // Use NEON-optimized paths
}
auto report = ARM64RuntimeValidator::GenerateReport(caps);
```

---

## Performance Baseline (Sprint 158)

Target performance on a reference Surface Pro X (SQ2, 8 cores @ 3.15 GHz):

| Metric                        | x64 Baseline | ARM64 Target | Tolerance |
|-------------------------------|-------------|--------------|-----------|
| Single thumbnail decode       | 17 ms        | ≤ 25 ms      | +47%      |
| Batch throughput              | 235 img/s    | ≥ 160 img/s  | -32%      |
| Cache hit latency             | < 5 ms       | < 8 ms       | +60%      |
| Memory: peak per decode       | < 50 MB      | < 50 MB      | Same      |
| Memory: steady-state overhead | < 20 MB      | < 20 MB      | Same      |

> Note: ARM64 targets are conservative first-pass baselines. With NEON optimizations,
> actual performance may meet or exceed x64 baselines on high-end ARM64 hardware.

---

## CI Integration (Sprint 159 — `.github/workflows/arm64.yml`)

The ARM64 CI pipeline (`arm64.yml`) provides:

1. **ARM64 Windows native build** — full CMake + Ninja build on `windows-latest` runner
   using `amd64_arm64` cross-compile environment
2. **Cross-compile validation** — verifies toolchain availability and ARM64BuildConfig header
3. **Library matrix check** — validates presence of all external library sources

Triggers: push to `main`/`develop`/`release/*`, PRs to `main`/`develop`, manual dispatch.

---

## Headers (Engine/Utils/)

| Header                       | Sprint | Purpose                                         |
|------------------------------|--------|-------------------------------------------------|
| `ARM64BuildConfig.h`         | 155    | Compiler flags, CMake variables, platform macros |
| `ARM64LibraryMatrix.h`       | 156    | Per-library ARM64 compatibility records          |
| `ARM64RuntimeValidator.h`    | 157    | Runtime capability detection                     |
| `ARM64PerformanceBaseline.h` | 158    | Baseline metrics and acceptance thresholds       |
| `ARM64CIIntegration.h`       | 159    | CI stage descriptors and result types            |

---

## Known Limitations

- `LENSShell.dll` (COM shell extension) is **not yet packaged as ARM64** — the native shell
  extension requires ARM64EC or native ARM64 COM registration; planned for v9.0.0.
- `LENSManager.exe` (WTL GUI) has not been validated on ARM64 Windows; WTL 10 supports ARM64
  but build scripts need updating.
- UnRAR ARM64 build requires a CMake-based approach (the NMake build system used for x64
  does not support ARM64 targets).
- Hardware CI (running tests natively on ARM64 hardware) is planned for v9.0.0.

---

## Sprints

| Sprint | Title                          | Status |
|--------|-------------------------------|--------|
| 155    | ARM64 Build Configuration      | ✅ Done |
| 156    | ARM64 Library Compatibility Matrix | ✅ Done |
| 157    | ARM64 Runtime Validator        | ✅ Done |
| 158    | ARM64 Performance Baseline     | ✅ Done |
| 159    | ARM64 CI Integration           | ✅ Done |

---

*Document created: Sprint 159 | Version: v15.0.0 | Last updated: Sprint 159*

