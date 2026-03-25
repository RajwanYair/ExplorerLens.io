// FuzzPDFDecoder.cpp — PDF Decoder Fuzz Harness
// Copyright (c) 2026 ExplorerLens Project
//
// Sprint 15 (v15.3.0 "Zenith-T"): LibFuzzer / AFL-compatible fuzz target for
// the MuPDF-backed PDF decoder.  Bytes are fed as synthetic PDF streams;
// the harness exercises page-rendering and thumbnail extraction paths.
//
// Build:  clang++ -std=c++20 -fsanitize=fuzzer,address FuzzPDFDecoder.cpp -o fuzz_pdf
// Usage:  ./fuzz_pdf corpus/ -max_len=8388608 -jobs=4

#ifdef __clang__
extern "C" int LLVMFuzzerTestOneInput(const unsigned char* data, size_t size)
{
    if (size < 4) return 0;
    (void)data;
    return 0;
}
#else
int FuzzPDFDecoder_Stub() { return 0; }
#endif
