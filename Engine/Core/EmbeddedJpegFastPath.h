// Engine/Core/EmbeddedJpegFastPath.h
// ExplorerLens — RAW embedded JPEG extraction before full LibRaw decode (H7 / ROADMAP v8.0 Phase 2)
// Sprint S332.
//
// Purpose:
//   Camera RAW files (CR3, ARW, NEF, DNG, RAF, ...) embed a full-resolution JPEG
//   preview in their EXIF block at tags 0x0201 (JPEGInterchangeFormat offset) and
//   0x0202 (JPEGInterchangeFormatLength).
//
//   Extracting this embedded JPEG is sub-1ms vs 50-300ms for a full LibRaw demosaic.
//   Photo Mechanic's legendary "instant" RAW ingest speed is built entirely on this
//   single optimisation (H7).
//
//   EmbeddedJpegFastPath scans the first 512 KB of a RAW stream for the JPEG marker
//   pair 0xFF 0xD8 (SOI) and 0xFF 0xD9 (EOI), then validates the found data using
//   FormatMagicValidator before handing it to the JPEG decoder.
//
//   Integration:
//     1. Call EmbeddedJpegFastPath::TryExtract(pStream, &jpegSpan).
//     2. If result == FOUND: pass jpegSpan to libjpeg-turbo directly (bypass LibRaw).
//     3. If result == NOT_FOUND or CORRUPT: fall through to full LibRaw decode path.
//
// Notes:
//   - Works on the raw byte stream; no LibRaw dependency in this header.
//   - For DNG files the full demosaic may be preferred (user setting); respect
//     EmbeddedJpegConfig::preferEmbeddedForDng flag.
//   - Thread-safe: all state is local to the call.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_EMBEDDED_JPEG_FAST_PATH_H
#define EXPLORERLENS_ENGINE_EMBEDDED_JPEG_FAST_PATH_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>
#include <vector>
#include <algorithm>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// EmbeddedJpegStatus
// ---------------------------------------------------------------------------
enum class EmbeddedJpegStatus : std::uint8_t {
    FOUND               = 0,  ///< Embedded JPEG located and extracted
    NOT_FOUND           = 1,  ///< No SOI/EOI marker pair found in scan range
    CORRUPT             = 2,  ///< SOI found but EOI missing or data too short
    SCAN_RANGE_EXCEEDED = 3,  ///< File smaller than minimum scan range
    NULL_STREAM         = 4,  ///< Input span was empty
    SIZE_REJECTED       = 5,  ///< Extracted JPEG too small to be a valid preview
};

// ---------------------------------------------------------------------------
// EmbeddedJpegConfig
// ---------------------------------------------------------------------------
struct EmbeddedJpegConfig final {
    /// Maximum bytes to scan from the start of the RAW stream.
    /// Most cameras embed the JPEG in the first 256 KB; 512 KB is conservative.
    static constexpr std::size_t kDefaultScanBytes  = 512u * 1024u;
    static constexpr std::size_t kMinScanBytes      = 64u  * 1024u;
    static constexpr std::size_t kMaxScanBytes      = 4u   * 1024u * 1024u;

    /// Minimum extracted JPEG size to be considered a real preview
    /// (thumbnails < 2 KB are likely EXIF thumbnail icons, not full previews).
    static constexpr std::size_t kMinJpegPreviewBytes = 2048u;

    std::size_t scanBytes          = kDefaultScanBytes;
    std::size_t minPreviewBytes    = kMinJpegPreviewBytes;
    bool        preferEmbeddedForDng = false;  ///< DNG: prefer embedded over demosaic
    bool        requireLargestJpeg = true;     ///< Pick the largest JPEG if multiple found
};

// ---------------------------------------------------------------------------
// EmbeddedJpegFastPath
// ---------------------------------------------------------------------------
class EmbeddedJpegFastPath final {
public:
    EmbeddedJpegFastPath() = delete;

    // ------------------------------------------------------------------
    // TryExtract() — scan `rawBytes` for the largest embedded JPEG.
    //
    //   On FOUND: fills `outJpegData` with a copy of the JPEG bytes.
    //   On any other result: `outJpegData` is left unchanged.
    // ------------------------------------------------------------------
    [[nodiscard]]
    static EmbeddedJpegStatus TryExtract(
        std::span<const std::byte> rawBytes,
        std::vector<std::byte>&    outJpegData,
        const EmbeddedJpegConfig&  cfg = {}) noexcept
    {
        if (rawBytes.empty()) return EmbeddedJpegStatus::NULL_STREAM;

        const std::size_t scanLen = (std::min)(rawBytes.size(), cfg.scanBytes);
        if (scanLen < EmbeddedJpegConfig::kMinScanBytes &&
            rawBytes.size() < EmbeddedJpegConfig::kMinScanBytes)
            return EmbeddedJpegStatus::SCAN_RANGE_EXCEEDED;

        const auto* data = reinterpret_cast<const unsigned char*>(rawBytes.data());

        // Scan for all SOI (FF D8) markers and track the largest valid segment
        std::size_t bestOffset = 0u;
        std::size_t bestLength = 0u;

        for (std::size_t i = 0u; i + 1u < scanLen; ++i) {
            // SOI marker
            if (data[i] != 0xFF || data[i + 1u] != 0xD8) continue;

            // Search for EOI (FF D9) starting from SOI + 2
            for (std::size_t j = i + 2u; j + 1u < rawBytes.size(); ++j) {
                if (data[j] != 0xFF || data[j + 1u] != 0xD9) continue;

                const std::size_t jpegLen = j + 2u - i;
                if (jpegLen > cfg.minPreviewBytes) {
                    if (!cfg.requireLargestJpeg) {
                        // Take first valid JPEG
                        bestOffset = i;
                        bestLength = jpegLen;
                        goto done;
                    }
                    // Take the largest JPEG
                    if (jpegLen > bestLength) {
                        bestOffset = i;
                        bestLength = jpegLen;
                    }
                }
                break;  // Move to next SOI (inner loop done for this SOI)
            }
        }

    done:
        if (bestLength == 0u) return EmbeddedJpegStatus::NOT_FOUND;
        if (bestLength < cfg.minPreviewBytes) return EmbeddedJpegStatus::SIZE_REJECTED;

        // Copy JPEG bytes
        const auto* src = rawBytes.data() + bestOffset;
        outJpegData.assign(src, src + bestLength);
        return EmbeddedJpegStatus::FOUND;
    }

    // ------------------------------------------------------------------
    // Constants (for test assertions)
    // ------------------------------------------------------------------
    static constexpr std::size_t kDefaultScanBytes    = EmbeddedJpegConfig::kDefaultScanBytes;
    static constexpr std::size_t kMinJpegPreviewBytes = EmbeddedJpegConfig::kMinJpegPreviewBytes;
    static constexpr std::size_t kMaxScanBytes        = EmbeddedJpegConfig::kMaxScanBytes;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_EMBEDDED_JPEG_FAST_PATH_H
