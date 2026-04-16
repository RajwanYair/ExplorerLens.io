// MagicBytesDatabase.h — Centralized Format Magic-Byte Registry
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a compile-time and runtime lookup of magic bytes for all 200+
// supported formats.  This is the single authoritative source for:
//   - File signature validation (magic bytes at offset)
//   - Extension→format mapping
//   - MIME type resolution
//
// All decoders should call MagicBytesDatabase::Detect() as the first step
// of ProbeHeader() rather than re-implementing magic byte checks locally.
//
#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// FormatSignature — one magic-byte rule for a format
// ---------------------------------------------------------------------------

struct FormatSignature {
    std::string_view  formatId;       // canonical id: "JPEG", "PNG", "WEBP", ...
    std::string_view  mimeType;
    uint32_t          offset;         // byte offset of the magic pattern
    std::span<const uint8_t> magic;   // bytes to match at that offset
    // optional mask: 0xFF = exact, other = bitwise AND before compare
    std::span<const uint8_t> mask;    // empty = exact match (0xFF for all)
};

// ---------------------------------------------------------------------------
// DetectResult — output of a magic-byte probe
// ---------------------------------------------------------------------------

struct DetectResult {
    std::string formatId;   // empty → not recognized
    std::string mimeType;
    float       confidence; // 1.0 = certain (magic match), 0.5 = extension only
};

// ---------------------------------------------------------------------------
// MagicBytesDatabase — static registry with runtime lookup
// ---------------------------------------------------------------------------

class MagicBytesDatabase {
public:
    // Probe the first N bytes of a file to detect its format.
    // Call with at least 32 bytes; 512 bytes gives best results.
    // Falls back to extension hint if magic bytes are ambiguous.
    static DetectResult Detect(std::span<const uint8_t> header,
                               std::string_view extensionHintLower = {});

    // Map lowercase extension → canonical formatId.
    // Returns empty string if the extension is unknown.
    static std::string ExtensionToFormatId(std::string_view extLower);

    // Map formatId → MIME type.
    static std::string FormatIdToMimeType(std::string_view formatId);

    // Return all known extensions for a formatId (e.g. "JPEG" → [".jpg",".jpeg",...]).
    static std::vector<std::string> ExtensionsForFormat(std::string_view formatId);

    // Get the full signature table (for testing / enumeration).
    static std::span<const FormatSignature> Signatures();

    // Total number of registered format families.
    static size_t FormatCount();

private:
    MagicBytesDatabase() = delete;
};

} // namespace Engine
} // namespace ExplorerLens
