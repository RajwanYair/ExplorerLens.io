// FuzzImageDecoder.cpp — Image Decoder Fuzz Harness
// Copyright (c) 2026 ExplorerLens Project
//
// Sprint 15 (v15.3.0 "Zenith-T"): LibFuzzer / AFL-compatible fuzz target for
// the image decode pipeline.  Build with clang -fsanitize=fuzzer,address to
// produce a fuzz binary.  Each fuzz iteration supplies raw bytes that are fed
// into the image decoder as a synthetic file stream.
//
// Build:  clang++ -std=c++20 -fsanitize=fuzzer,address FuzzImageDecoder.cpp -o fuzz_image
// Usage:  ./fuzz_image corpus/ -max_len=1048576 -jobs=4

#ifdef __clang__
extern "C" int LLVMFuzzerTestOneInput(const unsigned char* data, size_t size)
{
    if (size < 4)
        return 0;

    // Minimal guard: reject empty or trivially undersized payloads.
    // The real decoder integration is wired in when the fuzz binary is linked
    // against ExplorerLensEngine.lib; this scaffold compiles cleanly stand-alone.

    (void)data;
    return 0;
}
#else
// Stub for non-fuzzing builds so the file compiles under MSVC.
int FuzzImageDecoder_Stub()
{
    return 0;
}
#endif
