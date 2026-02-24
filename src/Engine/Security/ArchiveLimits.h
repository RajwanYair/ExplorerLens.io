#pragma once

#include <cstdint>

namespace ExplorerLens::Engine::Security {

    // Hard limits to prevent Denial of Service (DoS) via "Zip Bombs" or massive images
    
    struct ProcessingLimits {
        // Maximum allowed file size for loading entirely into memory (256 MB)
        static constexpr uint64_t MaxInputFileSize = 256 * 1024 * 1024;

        // Maximum dimension (Width or Height) to attempt decoding (16k)
        // Prevents allocation of massive buffers for headers lying about size
        static constexpr uint32_t MaxDimensionPx = 16384;

        // Maximum pixel count (Width * Height) (64 Megapixels)
        static constexpr uint64_t MaxPixelCount = 64 * 1024 * 1024;

        // Archive Scanners
        struct Archive {
            // Maximum number of entries to scan in a zip/rar to find a cover image
            static constexpr uint32_t MaxEntriesToScan = 100;

            // Maximum expansion ratio (Uncompressed / Compressed)
            // If we extract 1MB and it becomes > 100MB, abort (Zip Bomb)
            static constexpr double MaxCompressionRatio = 100.0;

            // Max size of a single extracted file (e.g. the cover image inside the zip) (50 MB)
            static constexpr uint64_t MaxExtractedFileSize = 50 * 1024 * 1024;
        };

        // Recursive formats (e.g. SVG referencing external entities)
        static constexpr uint32_t MaxRecursionDepth = 4;
    };

}

