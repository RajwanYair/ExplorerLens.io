# Sprint 179 — Build System Cleanup

**Version:** v8.4.0  
**Date:** June 2025  
**Status:** ✅ Complete

---

## Objective

Clean up orphaned files in the Engine CMakeLists.txt, add missing feature flags for conditional decoder compilation.

## Changes

### 1. Registered Orphaned Headers in ENGINE_HEADERS

Four header files existed on disk in `Engine/Decoders/` but were not registered in `Engine/CMakeLists.txt`:

| File | Sprint | Action |
|------|--------|--------|
| `ColorSpaceManager.h` | Sprint 40 | Registered in ENGINE_HEADERS |
| `EPUBDecoder.h` | Sprint 15 | Registered in ENGINE_HEADERS |
| `OptimizedArchiveReader.h` | Sprint 14 | Registered in ENGINE_HEADERS |
| `ExampleDecoder.h` | Reference | Commented out (template, not compiled) |

### 2. Added Missing CMake Feature Flags

Existing flags: `HAS_LIBJXL`, `HAS_LIBHEIF`, `HAS_LIBRAW`

New flags added:

| Flag | Purpose | Default |
|------|---------|---------|
| `HAS_LIBAVIF` | Enable AVIF decoder (libavif) | OFF |
| `HAS_MUPDF` | Enable PDF decoder (MuPDF) | OFF |
| `ENABLE_VIDEO_DECODER` | Enable video frame extraction (Media Foundation) | OFF |
| `ENABLE_AUDIO_DECODER` | Enable audio album art/waveform (Media Foundation) | OFF |

### 3. Enhanced Build Summary

The CMake configuration summary now displays all 7 feature flags with their current values.

## Files Changed

| File | Action |
|------|--------|
| `Engine/CMakeLists.txt` | 3 headers registered, 4 feature flags added, summary enhanced |

## Usage

```powershell
# Enable all decoders
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DHAS_LIBJXL=ON -DHAS_LIBHEIF=ON -DHAS_LIBRAW=ON \
  -DHAS_LIBAVIF=ON -DHAS_MUPDF=ON \
  -DENABLE_VIDEO_DECODER=ON -DENABLE_AUDIO_DECODER=ON
```
