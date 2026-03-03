// ThumbnailSignatureVerifier.h — Thumbnail Integrity Verification
// Copyright (c) 2026 ExplorerLens Project
//
// Verifies thumbnail integrity through CRC32-based content signatures.
// Uses a table-driven CRC32 implementation with no external crypto dependencies.
//
#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <algorithm>
#include <array>

namespace ExplorerLens {
namespace Engine {

// Verifies thumbnail integrity through CRC32-based content signatures.
// Provides checksum computation, verification, and corruption detection
// using a self-contained CRC32 table (no external dependencies).
class ThumbnailSignatureVerifier {
public:
    ThumbnailSignatureVerifier() {
        InitCRC32Table();
    }

    // Compute CRC32 checksum over arbitrary data
    uint32_t ComputeCRC32(const uint8_t* data, size_t size) const {
        if (!data || size == 0) return 0;

        uint32_t crc = 0xFFFFFFFF;
        for (size_t i = 0; i < size; ++i) {
            uint8_t index = static_cast<uint8_t>((crc ^ data[i]) & 0xFF);
            crc = (crc >> 8) ^ m_crc32Table[index];
        }
        return crc ^ 0xFFFFFFFF;
    }

    // Compute a checksum over a BGRA pixel buffer (w * h * 4 bytes)
    uint32_t ComputeChecksum(const uint8_t* pixels, uint32_t w, uint32_t h) const {
        if (!pixels || w == 0 || h == 0) return 0;
        size_t dataSize = static_cast<size_t>(w) * h * 4;
        return ComputeCRC32(pixels, dataSize);
    }

    // Verify that a pixel buffer matches an expected signature
    bool VerifySignature(const uint8_t* pixels, uint32_t w, uint32_t h, uint32_t expected) const {
        uint32_t actual = ComputeChecksum(pixels, w, h);
        return actual == expected;
    }

    // Convert a CRC32 checksum to an 8-character hex string
    static std::string GetSignatureString(uint32_t checksum) {
        static const char hexChars[] = "0123456789ABCDEF";
        std::string result(8, '0');

        for (int i = 7; i >= 0; --i) {
            result[i] = hexChars[checksum & 0xF];
            checksum >>= 4;
        }
        return result;
    }

    // Detect likely corruption in a pixel buffer.
    // Checks for degenerate patterns: all-zero, all-same-byte, or
    // statistically improbable uniformity in a non-trivial image.
    static bool IsCorrupted(const uint8_t* pixels, uint32_t w, uint32_t h) {
        if (!pixels || w == 0 || h == 0) return true;

        const size_t totalBytes = static_cast<size_t>(w) * h * 4;
        if (totalBytes < 16) return true;

        // Check all-zero (blank image)
        bool allZero = true;
        bool allSame = true;
        uint8_t firstByte = pixels[0];

        // Sample at regular intervals for efficiency on large images
        const size_t sampleStride = (std::max)(static_cast<size_t>(1), totalBytes / 1024);
        size_t sampledCount = 0;
        size_t zeroCount = 0;
        size_t sameCount = 0;

        for (size_t i = 0; i < totalBytes; i += sampleStride) {
            ++sampledCount;
            if (pixels[i] == 0) ++zeroCount;
            if (pixels[i] == firstByte) ++sameCount;
            if (pixels[i] != 0) allZero = false;
            if (pixels[i] != firstByte) allSame = false;
        }

        // All zeros = corrupted/blank
        if (allZero) return true;

        // All same byte = solid fill, likely corrupted thumbnail
        if (allSame && totalBytes > 64) return true;

        // If >99% of sampled bytes are zero, likely corrupted
        if (sampledCount > 0) {
            double zeroRatio = static_cast<double>(zeroCount) / static_cast<double>(sampledCount);
            if (zeroRatio > 0.99 && totalBytes > 256) return true;
        }

        return false;
    }

private:
    // Initialize the CRC32 lookup table (polynomial 0xEDB88320)
    void InitCRC32Table() {
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t crc = i;
            for (uint32_t j = 0; j < 8; ++j) {
                if (crc & 1)
                    crc = (crc >> 1) ^ 0xEDB88320;
                else
                    crc >>= 1;
            }
            m_crc32Table[i] = crc;
        }
    }

    std::array<uint32_t, 256> m_crc32Table;
};

} // namespace Engine
} // namespace ExplorerLens
