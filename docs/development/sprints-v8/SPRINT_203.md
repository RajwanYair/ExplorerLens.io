# Sprint 203 — Python SDK

**Date:** 2026-01-20  
**Version:** v10.0.0  
**Status:** ✅ Complete

## Objective
Provide a Python SDK enabling scripted and programmatic access to DarkThumbs
thumbnail generation via ctypes C ABI and pybind11 bindings.

## Deliverables
| Artifact | Path |
|----------|------|
| Header | `Engine/Utils/PythonSDK.h` |
| Source | `Engine/Utils/PythonSDK.cpp` |
| Tests | 5 tests in `Engine/Tests/EngineTests.cpp` |
| CMake | Registered in `Engine/CMakeLists.txt` |

## Key Features
- C ABI exports: `DarkThumbs_Init`, `DarkThumbs_GenerateThumbnail`, `DarkThumbs_Shutdown`, etc.
- `PythonSDKConfig` with max concurrency, default dimensions, batch mode, GPU toggle
- `PythonDecoderInfo` for exposing decoder metadata to Python
- `GenerateCtypesStub()` — generates ready-to-use Python ctypes wrapper
- `GeneratePybind11Wrapper()` — generates pybind11 module source
- Batch processing support with concurrent limit

## Tests Added (5)
1. `TestPythonSDK_DefaultConfig` — default config validation
2. `TestPythonSDK_DecoderInfo` — decoder list enumeration
3. `TestPythonSDK_CtypesStub` — ctypes code generation
4. `TestPythonSDK_Pybind11Wrapper` — pybind11 code generation
5. `TestPythonSDK_BatchConfig` — configuration round-trip

## Impact
- Enables automation/scripting of thumbnail generation
- Supports both ctypes (no build) and pybind11 (compiled) paths
