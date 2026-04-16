// FuzzArchiveCoverExtractor.cpp — libFuzzer harness for ArchiveCoverExtractor
// Copyright (c) 2026 ExplorerLens Project
//
// Exercises ArchiveCoverExtractor::ExtractFromBuffer() against arbitrary byte
// sequences purporting to be archive data.  The extractor must never crash,
// buffer-overflow, or enter an infinite loop regardless of input.
//
// Build (clang-cl, see FuzzFormatDetection.cpp for flags):
//   clang-cl -fsanitize=fuzzer,address -O1 -c FuzzArchiveCoverExtractor.cpp
//   link against ExplorerLensEngine.lib + libarchive.lib
//
#ifdef __clang__

#include <cstdint>
#include <cstddef>
#include <vector>

// Forward-declare the Engine API we are fuzzing
namespace ExplorerLens { namespace Engine {
    struct CoverImage {
        std::vector<uint8_t> pixels;
        uint32_t width  = 0;
        uint32_t height = 0;
        uint32_t bpp    = 4;
    };
    class ArchiveCoverExtractor {
    public:
        static CoverImage ExtractFromBuffer(const uint8_t* data, size_t size,
                                             uint32_t minWidth  = 32,
                                             uint32_t minHeight = 32);
    };
} }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 4) return 0;

    // Fuzz with varying minimum resolution to exercise different code paths
    uint32_t minW = (static_cast<uint32_t>(data[0]) % 8) * 32 + 32;  // 32–288
    uint32_t minH = (static_cast<uint32_t>(data[1]) % 8) * 32 + 32;

    (void)ExplorerLens::Engine::ArchiveCoverExtractor::ExtractFromBuffer(
        data + 2, size - 2, minW, minH);

    return 0;
}

#else // MSVC stub

extern "C" int LensServer_FuzzArchiveCoverExtractor_Placeholder = 0;

#endif
