// FuzzFormatDetection.cpp — libFuzzer harness for FormatDetector
// Copyright (c) 2026 ExplorerLens Project
//
// Exercises the Engine's format detection logic against arbitrary byte
// sequences.  FormatDetector should never crash or throw an uncaught exception
// regardless of input.
//
// Build with clang-cl (LLVM/libFuzzer) or AFL++ on non-Windows:
//   clang-cl -fsanitize=fuzzer,address -O1 -c FuzzFormatDetection.cpp
//   clang-cl -fsanitize=fuzzer,address -o FuzzFormatDetection.exe \
//       FuzzFormatDetection.obj ExplorerLensEngine.lib
//
// Run:
//   FuzzFormatDetection.exe corpus/ -max_len=65536 -timeout=10
//
// MSVC note: This file compiles under MSVC as a no-op stub (the
// LLVMFuzzerTestOneInput export is guarded by __clang__), allowing it to
// be registered in the VS solution without build errors.
//
#ifdef __clang__

#include <cstdint>
#include <cstddef>

// Forward-declare the Engine API we are fuzzing
namespace ExplorerLens { namespace Engine { namespace Pipeline {
    int DetectFormat(const uint8_t* data, size_t size);
} } }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Feed arbitrary bytes to the format detector.
    // Any return value is valid; we only care about no crash/hang.
    if (size == 0) return 0;
    (void)ExplorerLens::Engine::Pipeline::DetectFormat(data, size);
    return 0;
}

#else // MSVC stub — compiles but produces no fuzz entry point

extern "C" int LensServer_FuzzFormatDetection_Placeholder = 0;

#endif
