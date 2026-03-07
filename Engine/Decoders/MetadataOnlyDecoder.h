// MetadataOnlyDecoder.h — EXIF/XMP Thumbnail Extraction
// Copyright (c) 2026 ExplorerLens Project
//
// Extracts embedded EXIF thumbnails and XMP preview images from files
// without full decode, providing near-instant thumbnails for photos.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class MetadataDecodeSource : uint8_t {
    EXIF = 0,
    XMP = 1,
    IPTC = 2,
    ICCProfile = 3,
    None = 255
};

struct EmbeddedThumbnail {
    MetadataDecodeSource source = MetadataDecodeSource::None;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t offset = 0;
    uint32_t size = 0;
    std::string format; // "JPEG", "TIFF", etc.
    bool isValid = false;
};

struct MetadataDecodeResult {
    bool found = false;
    EmbeddedThumbnail thumbnail;
    std::vector<uint8_t> thumbnailData;
    double extractionMs = 0.0;
    bool isRotated = false;
    uint16_t orientation = 1; // EXIF orientation tag
};

struct MetadataDecodeConfig {
    bool preferEXIFThumbnail = true;
    uint32_t minThumbnailDimension = 32;
    uint32_t maxThumbnailSize = 1024 * 1024; // 1MB
    bool respectOrientation = true;
    bool fallbackToICCPreview = false;
};

struct MetadataDecoderStats {
    uint64_t totalChecked = 0;
    uint64_t thumbnailsFound = 0;
    uint64_t exifHits = 0;
    uint64_t xmpHits = 0;
    double avgExtractionMs = 0.0;
    double foundRate() const { return totalChecked > 0 ? 100.0 * thumbnailsFound / totalChecked : 0.0; }
};

class MetadataOnlyDecoder {
public:
    void Configure(const MetadataDecodeConfig& config) { m_config = config; }

    EmbeddedThumbnail FindEXIFThumbnail(const uint8_t* fileData, size_t fileSize) const {
        EmbeddedThumbnail result;
        if (!fileData || fileSize < 12) return result;

        // Check for JPEG with EXIF
        if (fileData[0] == 0xFF && fileData[1] == 0xD8) {
            // JPEG — scan for APP1 (EXIF) marker
            size_t pos = 2;
            while (pos + 4 < fileSize) {
                if (fileData[pos] == 0xFF && fileData[pos + 1] == 0xE1) {
                    // Found APP1 segment
                    uint32_t segLen = (fileData[pos + 2] << 8) | fileData[pos + 3];
                    if (pos + 4 + 6 < fileSize &&
                        fileData[pos + 4] == 'E' && fileData[pos + 5] == 'x' &&
                        fileData[pos + 6] == 'i' && fileData[pos + 7] == 'f') {
                        result.source = MetadataDecodeSource::EXIF;
                        result.offset = static_cast<uint32_t>(pos + 4);
                        result.size = segLen;
                        result.format = "JPEG";
                        result.isValid = true;
                    }
                    break;
                }
                if (fileData[pos] == 0xFF) {
                    uint32_t skip = (fileData[pos + 2] << 8) | fileData[pos + 3];
                    pos += 2 + skip;
                }
                else {
                    break;
                }
            }
        }
        return result;
    }

    bool MeetsMinimumSize(const EmbeddedThumbnail& thumb) const {
        return thumb.width >= m_config.minThumbnailDimension &&
            thumb.height >= m_config.minThumbnailDimension;
    }

    MetadataDecoderStats GetStats() const { return m_stats; }

private:
    MetadataDecodeConfig m_config;
    MetadataDecoderStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
