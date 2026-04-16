// FuzzExifOrientationNormalizer.cpp — libFuzzer harness for ExifOrientationNormalizer
// Copyright (c) 2026 ExplorerLens Project
//
// Exercises JPEG APP1 parsing + EXIF orientation extraction on arbitrary byte
// sequences.  The normalizer must never crash or produce out-of-bounds writes.
//
#ifdef __clang__

#include <cstdint>
#include <cstddef>

namespace ExplorerLens { namespace Engine {
    uint16_t ExtractExifOrientation(const uint8_t* jpegData, size_t size);
} }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size == 0) return 0;
    (void)ExplorerLens::Engine::ExtractExifOrientation(data, size);
    return 0;
}

#else
extern "C" int LensServer_FuzzExifOrientation_Placeholder = 0;
#endif
