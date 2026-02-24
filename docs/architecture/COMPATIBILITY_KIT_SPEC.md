# Compatibility Kit Specification v1.0 (Sprint 15)

**Date:** January 6, 2026
**Status:** Design

## 1. Purpose

The Compatibility Kit (CK) is a standardized harness to validate that a Format Implementation (Core or Plugin) is robust, performant, and correct. **No format ships without a passing CK.**

## 2. The Test Corpus Structure

For every supported extension (e.g., `.psd`), the CK expects a corresponding folder in the test data:

```
tests/data/corpus/psd/
├── valid/
│   ├── minimal_1x1.psd          (Smallest valid file)
│   ├── typical_1080p.psd        (Standard use case)
│   ├── large_4k_16bit.psd       (Stress test)
│   ├── transparent_layer.psd    (Success criteria: Check Alpha)
│   └── cmyk_color.psd           (Success criteria: Color conversion)
├── invalid/
│   ├── truncated.psd            (Decoder must return Error, not Crash)
│   ├── garbage_header.psd       (Decoder must reject quickly)
│   └── zero_byte.psd
└── security/
    ├── zip_bomb_layers.psd      (Memory limit check)
    └── deep_recursion.psd       (Stack overflow check)
```

## 3. The Validation Tool: `ExplorerLens.Validator.exe`

The validator runs against a plugin or the core engine.

### usage

`ExplorerLens.Validator.exe --format psd --plugin path/to/plugin.dll --corpus tests/data/corpus/psd`

### Checks Performed

1. **Basic Render:** Can generate a thumbnail for all `valid/*` files?
2. **Visual Diff:** Does the output match the reference image (within 2% SSIM)?
3. **Leak Check:** Run 100 iterations. Does memory usage grow?
4. **Fuzzing Response:** Run against `invalid/*`.
   - **Pass:** Returns `E_FAIL` or `E_INVALIDARG`.
   - **Fail:** Crash / Access Violation / Timeout (>5s).
5. **Metadata:** Does it correctly extract dimensions/transparency info?

## 4. Performance Baseline

The CK records metrics for the `typical` and `large` samples:

- **Time to First Pixel (TTFP)**
- **Peak Heap Memory**
- **File Handle Hold Time**

**Gate:** If a plugin takes > 500ms for a 1080p image on reference hardware, it is flagged as "Slow" in the UI.

## 5. Automation

The CK is integrated into GitHub Actions.

- Prerequisite: Download corpus (Git LFS / Azure Blob).
- Trigger: PR modifying `src/Plugins/Psd/*`.
- Result: Pass/Fail check on the PR.

