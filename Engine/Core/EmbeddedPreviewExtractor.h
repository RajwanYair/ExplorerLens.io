// EmbeddedPreviewExtractor.h — Fast embedded preview extraction from camera RAW files
// Copyright (c) 2026 ExplorerLens Project
//
// Uses LibRaw's unpack_thumb() path to decode the embedded JPEG thumbnail that
// modern cameras embed in RAW files (CR2, NEF, ARW, DNG, etc.) without decoding
// the full RAW sensor data. Falls back to full LibRaw decode if no JPEG thumb found.
//
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// EmbeddedPreviewResult — output from a successful extraction
// ---------------------------------------------------------------------------

struct EmbeddedPreviewResult {
    std::vector<uint8_t> jpegData;   // Raw JPEG bytes ready to decode with TurboJPEG/LibJPEG
    uint32_t             width  = 0;
    uint32_t             height = 0;
    bool                 fromEmbedded = true;  // false → came from full decode fallback
};

// ---------------------------------------------------------------------------
// EmbeddedPreviewExtractor
// ---------------------------------------------------------------------------

class EmbeddedPreviewExtractor {
public:
    EmbeddedPreviewExtractor();
    ~EmbeddedPreviewExtractor();

    EmbeddedPreviewExtractor(const EmbeddedPreviewExtractor&) = delete;
    EmbeddedPreviewExtractor& operator=(const EmbeddedPreviewExtractor&) = delete;

    // Extract an embedded JPEG from a RAW file.
    // Returns nullopt if the file is unreadable or no suitable preview found.
    std::optional<EmbeddedPreviewResult> Extract(std::string_view filePath) const;

    // Extract from in-memory buffer (useful for archive/stream contexts).
    std::optional<EmbeddedPreviewResult> ExtractFromBuffer(
        const uint8_t* data, size_t size) const;

    // Minimum embedded preview resolution to accept (default: 256x256).
    // Previews smaller than this trigger a full LibRaw decode fallback.
    void SetMinimumResolution(uint32_t width, uint32_t height) noexcept;

private:
    uint32_t m_minWidth  = 256;
    uint32_t m_minHeight = 256;

    std::optional<EmbeddedPreviewResult> DoExtract(
        void* librawHandle) const;
};

} // namespace Engine
} // namespace ExplorerLens
