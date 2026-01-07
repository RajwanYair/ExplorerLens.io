# Test Strategy v1.0 (Sprint 11)

**Date:** January 6, 2026
**Status:** Draft / Implementation Guide

## 1. Overview

This document outlines the testing strategy for the DarkThumbs v6.0 era. We are moving from ad-hoc testing to a rigorous, layered testing pyramid.

## 2. Test Pyramid

### 2.1 Unit Tests (GoogleTest)

- **Scope:** Individual classes and functions (Decoders, Parsers, Math helpers).
- **Location:** `tests/UnitTests/`
- **Framework:** GoogleTest + GoogleMock
- **Policy:**
  - Every new class must have a corresponding test suite.
  - 80% code coverage target for core engine logic.
  - Mocks should be used for I/O and System calls (FileSystem, Registry).

### 2.2 Integration Tests

- **Scope:** Components working together (Pipeline + Decoder + Cache).
- **Location:** `tests/IntegrationTests/`
- **Framework:** GoogleTest (creating real engine instances)
- **Policy:**
  - Test complete thumbnail generation workflow.
  - Use real file system fixtures (small sample files).
  - Validate interactions between C++ Engine and simulated COM calls.

### 2.3 Performance Smoke Tests

- **Scope:** Detect regressions in speed or memory usage.
- **Location:** `tests/PerfTests/`
- **Framework:** GoogleBenchmark
- **Metrics:**
  - Time to First Pixel (TTFP)
  - Peak Memory Usage
  - GPU Utilization (via counters)
- **Baseline DB:** Stored in `tests/data/perf_baseline.json`. CI compares current run vs baseline.

## 3. Test Data Strategy

We will maintain a `tests/data/corpus` directory containing minimal, valid examples of every supported format.

Structure:

```
tests/data/corpus/
  valid/
    image_avif/
      sample_400x400.avif
    archive_cbz/
      comic_simple.cbz
  corrupt/
    fuzz_truncated.avif
    fuzz_invalid_header.cbz
```

**Note:** Git LFS or a download script (`scripts/download_test_corpus.ps1`) will be used for large files to keep the repo light.

## 4. CI Pipeline Wiring

- **Trigger:** Pull Request & Push to Main.
- **Jobs:**
    1. **Build (Debug/Release x64/ARM64)**
    2. **Unit Tests** (Must pass 100%)
    3. **Integration Tests** (Must pass 100%)
    4. **Perf Sanity** (Warn if >10% regression)

## 5. Mocking Strategy

We will introduce `IFileSystem` and `IRegistry` interfaces to allow mocking OS interactions, ensuring unit tests are fast and deterministic.
