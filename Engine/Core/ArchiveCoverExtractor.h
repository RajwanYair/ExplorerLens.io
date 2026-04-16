// ArchiveCoverExtractor.h — First-image extraction from ZIP/CBZ/RAR archives
// Copyright (c) 2026 ExplorerLens Project
//
// Comic books (CBZ, CBR) and generic archives often represent a collection of
// images. This extractor finds the first lexicographically-ordered image entry
// and returns its raw bytes for thumbnail generation, avoiding full extraction.
//
// Supported formats: ZIP (.zip, .cbz), RAR (.rar, .cbr), 7z (.7z, .cb7)
//
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// ArchiveCoverResult — the first image found inside an archive
// ---------------------------------------------------------------------------

struct ArchiveCoverResult {
    std::vector<uint8_t> imageData;   // Raw image bytes (JPEG, PNG, WebP, etc.)
    std::string          entryName;   // Path/name of the entry inside the archive
    std::string          mimeType;    // e.g. "image/jpeg"
};

// ---------------------------------------------------------------------------
// ArchiveCoverExtractor
// ---------------------------------------------------------------------------

class ArchiveCoverExtractor {
public:
    ArchiveCoverExtractor() = default;
    ~ArchiveCoverExtractor() = default;

    // Extract the cover image from an archive file.
    // Entries are sorted lexicographically; the first image is returned.
    // Returns nullopt if no image entry found or if extraction fails.
    std::optional<ArchiveCoverResult> Extract(std::string_view filePath) const;

    // Extract from an in-memory buffer (useful for nested archives).
    std::optional<ArchiveCoverResult> ExtractFromBuffer(
        const uint8_t* data, size_t size, std::string_view hint = "") const;

    // Maximum size of image to extract (bytes). Entries larger than this are
    // skipped to avoid OOM on malformed archives. Default: 32 MB.
    void SetMaxImageBytes(size_t maxBytes) noexcept { m_maxImageBytes = maxBytes; }

private:
    size_t m_maxImageBytes = 32 * 1024 * 1024;

    std::optional<ArchiveCoverResult> ExtractFirstImage(void* archiveHandle) const;

    static bool IsImageEntry(std::string_view entryName);
    static std::string InferMimeType(std::string_view entryName);
};

} // namespace Engine
} // namespace ExplorerLens
