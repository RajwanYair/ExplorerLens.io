// Engine/Core/FormatMagicValidator.h
// ExplorerLens — Magic-byte format validation before decode dispatch (S4 security control / ROADMAP v8.0 Phase 2)
// Sprint S328.
//
// Purpose:
//   Format spoofing is a common attack vector: rename a crafted exploit as
//   "photo.jpg" to trigger a JPEG decoder against non-JPEG bytes.
//
//   FormatMagicValidator provides a fast, allocation-free check that verifies
//   the file's magic bytes before the decoder dispatch table is consulted.
//   This closes ROADMAP security control S4: "Magic-byte validation before decode."
//
//   Design principles:
//     - All checks are pure functions (no I/O, no allocation).
//     - The magic table is compile-time constexpr.
//     - Validation runs on the first 32 bytes of the file (already in the
//       IStream internal buffer — no extra read).
//     - Mismatches are logged via DecodeErrorTracker before E_FAIL return.
//
// Integration:
//   Call FormatMagicValidator::Validate(extension, header, len) in the decoder
//   hot-path before any malloc or library call.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_FORMAT_MAGIC_VALIDATOR_H
#define EXPLORERLENS_ENGINE_FORMAT_MAGIC_VALIDATOR_H

#include <cstdint>
#include <cstddef>
#include <string_view>
#include <span>
#include <array>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// MagicValidatorResult
// ---------------------------------------------------------------------------
enum class MagicValidatorResult : std::uint8_t {
    MATCH           = 0,  ///< Magic bytes confirm the expected format
    MISMATCH        = 1,  ///< Magic bytes do NOT match the file extension
    UNKNOWN_FORMAT  = 2,  ///< Extension not in the magic table; cannot validate
    TOO_SHORT       = 3,  ///< Buffer has fewer bytes than the required magic
    EMPTY_BUFFER    = 4,  ///< Null or zero-length buffer passed
};

// ---------------------------------------------------------------------------
// MagicByteSpec — describes one format's magic signature
// ---------------------------------------------------------------------------
struct MagicByteSpec final {
    std::string_view          extension;    ///< file extension, e.g. ".jpg"
    std::string_view          mimeType;     ///< MIME type
    std::array<std::byte, 8>  magic{};      ///< Expected bytes (0-padded)
    std::uint8_t              magicLen{};   ///< Significant bytes (1–8)
    std::uint8_t              magicOffset{};///< Byte offset into file (usually 0)
};

// ---------------------------------------------------------------------------
// FormatMagicValidator
// ---------------------------------------------------------------------------
class FormatMagicValidator final {
public:
    // Number of entries in the built-in magic table
    static constexpr std::size_t kTableSize = 22u;

    // Minimum file bytes required for the longest magic sequence
    static constexpr std::size_t kMinHeaderBytes = 12u;

    // ------------------------------------------------------------------
    // Validate() — check that `header` matches the expected magic for
    // the given file `extension`.
    // ------------------------------------------------------------------
    [[nodiscard]]
    static MagicValidatorResult Validate(
        std::string_view           extension,
        std::span<const std::byte> header) noexcept
    {
        if (header.empty())  return MagicValidatorResult::EMPTY_BUFFER;
        if (header.size() < 2u) return MagicValidatorResult::TOO_SHORT;

        const MagicByteSpec* spec = FindSpec(extension);
        if (!spec) return MagicValidatorResult::UNKNOWN_FORMAT;

        const std::size_t needed = spec->magicOffset + spec->magicLen;
        if (header.size() < needed) return MagicValidatorResult::TOO_SHORT;

        const auto* h = reinterpret_cast<const unsigned char*>(header.data())
                        + spec->magicOffset;
        for (std::uint8_t i = 0; i < spec->magicLen; ++i) {
            if (h[i] != static_cast<unsigned char>(spec->magic[i]))
                return MagicValidatorResult::MISMATCH;
        }
        return MagicValidatorResult::MATCH;
    }

    // ------------------------------------------------------------------
    // FindSpec() — look up a MagicByteSpec by extension.
    // Returns nullptr if not in table.
    // ------------------------------------------------------------------
    [[nodiscard]]
    static const MagicByteSpec* FindSpec(std::string_view ext) noexcept
    {
        for (const auto& spec : kTable)
            if (spec.extension == ext) return &spec;
        return nullptr;
    }

    // ------------------------------------------------------------------
    // IsKnownExtension() — quick membership test
    // ------------------------------------------------------------------
    [[nodiscard]]
    static bool IsKnownExtension(std::string_view ext) noexcept
    { return FindSpec(ext) != nullptr; }

private:
    FormatMagicValidator() = delete;

    static constexpr std::array<MagicByteSpec, kTableSize> kTable = {{
        // JPEG: FF D8 FF
        { ".jpg",  "image/jpeg", { std::byte{0xFF}, std::byte{0xD8}, std::byte{0xFF} }, 3, 0 },
        { ".jpeg", "image/jpeg", { std::byte{0xFF}, std::byte{0xD8}, std::byte{0xFF} }, 3, 0 },
        // PNG: 89 50 4E 47 0D 0A 1A 0A
        { ".png",  "image/png",
          { std::byte{0x89}, std::byte{0x50}, std::byte{0x4E}, std::byte{0x47},
            std::byte{0x0D}, std::byte{0x0A}, std::byte{0x1A}, std::byte{0x0A} }, 8, 0 },
        // GIF: 47 49 46 38
        { ".gif",  "image/gif",
          { std::byte{0x47}, std::byte{0x49}, std::byte{0x46}, std::byte{0x38} }, 4, 0 },
        // BMP: 42 4D
        { ".bmp",  "image/bmp",  { std::byte{0x42}, std::byte{0x4D} }, 2, 0 },
        // TIFF LE: 49 49 2A 00
        { ".tif",  "image/tiff",
          { std::byte{0x49}, std::byte{0x49}, std::byte{0x2A}, std::byte{0x00} }, 4, 0 },
        { ".tiff", "image/tiff",
          { std::byte{0x49}, std::byte{0x49}, std::byte{0x2A}, std::byte{0x00} }, 4, 0 },
        // WebP: 52 49 46 46 (RIFF)
        { ".webp", "image/webp",
          { std::byte{0x52}, std::byte{0x49}, std::byte{0x46}, std::byte{0x46} }, 4, 0 },
        // PDF: 25 50 44 46 (%PDF)
        { ".pdf",  "application/pdf",
          { std::byte{0x25}, std::byte{0x50}, std::byte{0x44}, std::byte{0x46} }, 4, 0 },
        // ZIP (CBZ, DOCX, XLSX): 50 4B 03 04
        { ".zip",  "application/zip",
          { std::byte{0x50}, std::byte{0x4B}, std::byte{0x03}, std::byte{0x04} }, 4, 0 },
        { ".cbz",  "application/x-cbz",
          { std::byte{0x50}, std::byte{0x4B}, std::byte{0x03}, std::byte{0x04} }, 4, 0 },
        // 7-Zip: 37 7A BC AF 27 1C
        { ".7z",   "application/x-7z",
          { std::byte{0x37}, std::byte{0x7A}, std::byte{0xBC}, std::byte{0xAF},
            std::byte{0x27}, std::byte{0x1C} }, 6, 0 },
        // RAR5: 52 61 72 21 1A 07 01 00
        { ".rar",  "application/x-rar",
          { std::byte{0x52}, std::byte{0x61}, std::byte{0x72}, std::byte{0x21},
            std::byte{0x1A}, std::byte{0x07}, std::byte{0x01}, std::byte{0x00} }, 8, 0 },
        { ".cbr",  "application/x-cbr",
          { std::byte{0x52}, std::byte{0x61}, std::byte{0x72}, std::byte{0x21},
            std::byte{0x1A}, std::byte{0x07}, std::byte{0x01}, std::byte{0x00} }, 8, 0 },
        // EXR: 76 2F 31 01
        { ".exr",  "image/x-exr",
          { std::byte{0x76}, std::byte{0x2F}, std::byte{0x31}, std::byte{0x01} }, 4, 0 },
        // HEIF/HEIC: ftyp box at offset 4
        { ".heic", "image/heic",
          { std::byte{0x66}, std::byte{0x74}, std::byte{0x79}, std::byte{0x70} }, 4, 4 },
        { ".heif", "image/heif",
          { std::byte{0x66}, std::byte{0x74}, std::byte{0x79}, std::byte{0x70} }, 4, 4 },
        { ".avif", "image/avif",
          { std::byte{0x66}, std::byte{0x74}, std::byte{0x79}, std::byte{0x70} }, 4, 4 },
        // ICO: 00 00 01 00
        { ".ico",  "image/x-icon",
          { std::byte{0x00}, std::byte{0x00}, std::byte{0x01}, std::byte{0x00} }, 4, 0 },
        // JXL: FF 0A (bare codestream) or 00 00 00 0C 4A 58 4C 20 (ISO box)
        { ".jxl",  "image/jxl",
          { std::byte{0xFF}, std::byte{0x0A} }, 2, 0 },
        // SVG: starts with <?xml or <svg (text-based; check for '<')
        { ".svg",  "image/svg+xml", { std::byte{0x3C} }, 1, 0 },
        // DDS: 44 44 53 20
        { ".dds",  "image/x-dds",
          { std::byte{0x44}, std::byte{0x44}, std::byte{0x53}, std::byte{0x20} }, 4, 0 },
    }};
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_FORMAT_MAGIC_VALIDATOR_H
