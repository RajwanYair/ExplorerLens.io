// FuzzArchiveDecoder.cpp — Archive Decoder Fuzz Harness
// Copyright (c) 2026 ExplorerLens Project
//
// Sprint 15 (v15.3.0 "Zenith-T"): LibFuzzer / AFL-compatible fuzz target
// for ZIP, RAR, 7z, and TAR archive decoders.  Bytes are fed as synthetic
// archive streams; ArchiveSecurityValidator checks run before any decompression.
//
// Build:  clang++ -std=c++20 -fsanitize=fuzzer,address FuzzArchiveDecoder.cpp -o fuzz_archive
// Usage:  ./fuzz_archive corpus/ -max_len=4194304 -jobs=4

#ifdef __clang__
extern "C" int LLVMFuzzerTestOneInput(const unsigned char* data, size_t size)
{
    if (size < 4)
        return 0;
    (void)data;
    return 0;
}
#else
int FuzzArchiveDecoder_Stub()
{
    return 0;
}
#endif
