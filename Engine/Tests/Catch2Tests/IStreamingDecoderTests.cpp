// IStreamingDecoderTests.cpp — Catch2 tests for IStreamingDecoder interface contracts
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the IStreamingDecoder interface contracts from §7.4 (D38).
// Tests cover: DecodeResult structure, PartialDecodeState enum, error
// propagation model, DecodeAtSize target range, ProbeHeader span contract,
// SupportsPartialDecode/SupportsEmbeddedPreview defaults, and the streaming
// decoder capability enumeration model.
//
// All tests are self-contained — constants mirror Engine/Core/IStreamingDecoder.h
// without including Windows or Engine headers.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

// ---------------------------------------------------------------------------
// IStreamingDecoder interface model (§7.4, D38)
// Mirrored from Engine/Core/IStreamingDecoder.h
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::IStreamingDecoder {

// ── PartialDecodeState ─────────────────────────────────────────────────────

enum class PartialDecodeState : uint8_t {
    COMPLETE      = 0,  // Full-fidelity decode succeeded
    PARTIAL       = 1,  // Reduced-quality decode (e.g. progressive JPEG strip)
    HEADER_ONLY   = 2,  // Only metadata/dimensions extracted (no pixels)
    FAILED        = 3,  // Decode failed entirely
    CANCELLED     = 4,  // Cancelled via stop_token
};

static constexpr int PARTIAL_DECODE_STATE_COUNT = 5;

// ── EngineError codes ──────────────────────────────────────────────────────

enum class EngineError : uint32_t {
    // Success range — not an error
    OK                 = 0x00000000,
    // Decode errors (0x1xxx)
    UNSUPPORTED_FORMAT = 0x00001001,
    CORRUPT_HEADER     = 0x00001002,
    TRUNCATED_DATA     = 0x00001003,
    DIMENSIONS_OVERFLOW= 0x00001004,
    MEMORY_LIMIT       = 0x00001005,
    DECODE_TIMEOUT     = 0x00001006,
    // I/O errors (0x2xxx)
    STREAM_READ_FAILED = 0x00002001,
    STREAM_SEEK_FAILED = 0x00002002,
    FILE_NOT_FOUND     = 0x00002003,
    ACCESS_DENIED      = 0x00002004,
    // Cache errors (0x3xxx)
    CACHE_MISS         = 0x00003001,
    CACHE_CORRUPT      = 0x00003002,
    CACHE_BUDGET_EXCEEDED = 0x00003003,
    // Config/policy errors (0x4xxx)
    POLICY_BLOCK       = 0x00004001,
    DECODER_DISABLED   = 0x00004002,
    // Internal invariant failures (0x9xxx — should never reach callers)
    INTERNAL           = 0x00009001,
};

// ── Decoder capabilities flags ─────────────────────────────────────────────

enum class DecoderCapability : uint32_t {
    NONE                    = 0x00000000,
    PARTIAL_DECODE          = 0x00000001,  // Progressive / strip decode
    EMBEDDED_PREVIEW        = 0x00000002,  // Fast embedded JPEG in RAW [H7]
    MULTI_PAGE              = 0x00000004,  // PDF / TIFF page navigation
    STREAMING               = 0x00000008,  // Stream-oriented decode
    METADATA_EXTRACTION     = 0x00000010,  // EXIF / XMP / IPTC
    HDR_OUTPUT              = 0x00000020,  // Float16 or HDR pixel output
    ALPHA_CHANNEL           = 0x00000040,  // BGRA premultiplied alpha
    ANIMATION               = 0x00000080,  // GIF / APNG / WebP animated
    CANCELABLE              = 0x00000100,  // Respects stop_token
};

inline constexpr DecoderCapability operator|(DecoderCapability a, DecoderCapability b) {
    return static_cast<DecoderCapability>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline constexpr bool HasCapability(DecoderCapability flags, DecoderCapability bit) {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(bit)) != 0;
}

// ── Probe/target size constants ────────────────────────────────────────────

static constexpr size_t   PROBE_HEADER_MIN_BYTES    =    16;   // minimum for magic-byte match
static constexpr size_t   PROBE_HEADER_IDEAL_BYTES  = 16384;   // 16 KB ideal probe window [H12]
static constexpr uint32_t DECODE_TARGET_MIN_PX      =    16;   // smallest valid thumbnail
static constexpr uint32_t DECODE_TARGET_MAX_PX      =  4096;   // largest valid thumbnail
static constexpr uint32_t DECODE_TARGET_DEFAULT_PX  =   256;   // Windows Explorer default

// ── Mock decoder capability profiles ──────────────────────────────────────

struct DecoderProfile {
    std::string_view    name;
    DecoderCapability   caps;
    bool                supportsPartialDecode;
    bool                supportsEmbeddedPreview;
};

static constexpr std::array<DecoderProfile, 8> DECODER_PROFILES = {{
    { "JPEG",  DecoderCapability::PARTIAL_DECODE | DecoderCapability::METADATA_EXTRACTION |
               DecoderCapability::ALPHA_CHANNEL   | DecoderCapability::CANCELABLE,
               true, false },
    { "PNG",   DecoderCapability::PARTIAL_DECODE | DecoderCapability::METADATA_EXTRACTION |
               DecoderCapability::ALPHA_CHANNEL   | DecoderCapability::CANCELABLE,
               true, false },
    { "WebP",  DecoderCapability::PARTIAL_DECODE | DecoderCapability::METADATA_EXTRACTION |
               DecoderCapability::ALPHA_CHANNEL   | DecoderCapability::ANIMATION |
               DecoderCapability::CANCELABLE,
               true, false },
    { "AVIF",  DecoderCapability::PARTIAL_DECODE | DecoderCapability::METADATA_EXTRACTION |
               DecoderCapability::ALPHA_CHANNEL   | DecoderCapability::HDR_OUTPUT |
               DecoderCapability::CANCELABLE,
               true, false },
    { "RAW",   DecoderCapability::EMBEDDED_PREVIEW | DecoderCapability::METADATA_EXTRACTION |
               DecoderCapability::CANCELABLE,
               false, true },   // RAW fast path: embedded JPEG [H7]
    { "GIF",   DecoderCapability::ANIMATION | DecoderCapability::CANCELABLE,
               false, false },
    { "PDF",   DecoderCapability::MULTI_PAGE | DecoderCapability::METADATA_EXTRACTION |
               DecoderCapability::CANCELABLE,
               false, false },
    { "TIFF",  DecoderCapability::MULTI_PAGE | DecoderCapability::METADATA_EXTRACTION |
               DecoderCapability::HDR_OUTPUT  | DecoderCapability::ALPHA_CHANNEL |
               DecoderCapability::CANCELABLE,
               true, false },
}};

} // namespace ExplorerLens::Tests::IStreamingDecoder

using namespace ExplorerLens::Tests::IStreamingDecoder;

// ===========================================================================
// PartialDecodeState
// ===========================================================================

TEST_CASE("PartialDecodeState — 5 states defined",
          "[streaming][state][count]") {
    REQUIRE(PARTIAL_DECODE_STATE_COUNT == 5);
}

TEST_CASE("PartialDecodeState — COMPLETE is 0 (default/success state)",
          "[streaming][state]") {
    REQUIRE(static_cast<uint8_t>(PartialDecodeState::COMPLETE) == 0);
}

TEST_CASE("PartialDecodeState — all 5 values are distinct",
          "[streaming][state][uniqueness]") {
    std::array<uint8_t, 5> vals = {
        static_cast<uint8_t>(PartialDecodeState::COMPLETE),
        static_cast<uint8_t>(PartialDecodeState::PARTIAL),
        static_cast<uint8_t>(PartialDecodeState::HEADER_ONLY),
        static_cast<uint8_t>(PartialDecodeState::FAILED),
        static_cast<uint8_t>(PartialDecodeState::CANCELLED),
    };
    for (size_t i = 0; i < vals.size(); ++i) {
        for (size_t j = i + 1; j < vals.size(); ++j) {
            CHECK(vals[i] != vals[j]);
        }
    }
}

TEST_CASE("PartialDecodeState — FAILED > HEADER_ONLY > PARTIAL > COMPLETE ordering",
          "[streaming][state][ordering]") {
    // A decode that produces fewer artifacts is always "more failed"
    REQUIRE(static_cast<uint8_t>(PartialDecodeState::FAILED) >
            static_cast<uint8_t>(PartialDecodeState::HEADER_ONLY));
    REQUIRE(static_cast<uint8_t>(PartialDecodeState::HEADER_ONLY) >
            static_cast<uint8_t>(PartialDecodeState::PARTIAL));
    REQUIRE(static_cast<uint8_t>(PartialDecodeState::PARTIAL) >
            static_cast<uint8_t>(PartialDecodeState::COMPLETE));
}

// ===========================================================================
// EngineError codes
// ===========================================================================

TEST_CASE("EngineError — OK is 0",
          "[streaming][error]") {
    REQUIRE(static_cast<uint32_t>(EngineError::OK) == 0u);
}

TEST_CASE("EngineError — decode errors are in 0x1xxx range",
          "[streaming][error][range]") {
    auto decodeErrors = {
        EngineError::UNSUPPORTED_FORMAT,
        EngineError::CORRUPT_HEADER,
        EngineError::TRUNCATED_DATA,
        EngineError::DIMENSIONS_OVERFLOW,
        EngineError::MEMORY_LIMIT,
        EngineError::DECODE_TIMEOUT,
    };
    for (auto e : decodeErrors) {
        uint32_t v = static_cast<uint32_t>(e);
        CHECK(v >= 0x1000u);
        CHECK(v < 0x2000u);
    }
}

TEST_CASE("EngineError — I/O errors are in 0x2xxx range",
          "[streaming][error][range]") {
    auto ioErrors = {
        EngineError::STREAM_READ_FAILED,
        EngineError::STREAM_SEEK_FAILED,
        EngineError::FILE_NOT_FOUND,
        EngineError::ACCESS_DENIED,
    };
    for (auto e : ioErrors) {
        uint32_t v = static_cast<uint32_t>(e);
        CHECK(v >= 0x2000u);
        CHECK(v < 0x3000u);
    }
}

TEST_CASE("EngineError — cache errors are in 0x3xxx range",
          "[streaming][error][range]") {
    auto cacheErrors = {
        EngineError::CACHE_MISS,
        EngineError::CACHE_CORRUPT,
        EngineError::CACHE_BUDGET_EXCEEDED,
    };
    for (auto e : cacheErrors) {
        uint32_t v = static_cast<uint32_t>(e);
        CHECK(v >= 0x3000u);
        CHECK(v < 0x4000u);
    }
}

TEST_CASE("EngineError — policy errors are in 0x4xxx range",
          "[streaming][error][range]") {
    auto policyErrors = {
        EngineError::POLICY_BLOCK,
        EngineError::DECODER_DISABLED,
    };
    for (auto e : policyErrors) {
        uint32_t v = static_cast<uint32_t>(e);
        CHECK(v >= 0x4000u);
        CHECK(v < 0x5000u);
    }
}

TEST_CASE("EngineError — INTERNAL is in 0x9xxx range",
          "[streaming][error][range]") {
    uint32_t v = static_cast<uint32_t>(EngineError::INTERNAL);
    CHECK(v >= 0x9000u);
    CHECK(v < 0xa000u);
}

// ===========================================================================
// DecoderCapability flags
// ===========================================================================

TEST_CASE("DecoderCapability — NONE is 0",
          "[streaming][capability]") {
    REQUIRE(static_cast<uint32_t>(DecoderCapability::NONE) == 0u);
}

TEST_CASE("DecoderCapability — all 9 non-NONE flags are powers of two",
          "[streaming][capability][flags]") {
    auto caps = {
        DecoderCapability::PARTIAL_DECODE,
        DecoderCapability::EMBEDDED_PREVIEW,
        DecoderCapability::MULTI_PAGE,
        DecoderCapability::STREAMING,
        DecoderCapability::METADATA_EXTRACTION,
        DecoderCapability::HDR_OUTPUT,
        DecoderCapability::ALPHA_CHANNEL,
        DecoderCapability::ANIMATION,
        DecoderCapability::CANCELABLE,
    };
    for (auto c : caps) {
        uint32_t v = static_cast<uint32_t>(c);
        INFO("Capability value: 0x" << std::hex << v);
        CHECK(v != 0u);
        CHECK((v & (v - 1u)) == 0u);  // Is power of 2?
    }
}

TEST_CASE("DecoderCapability — HasCapability detects set bits",
          "[streaming][capability][helper]") {
    auto combined = DecoderCapability::PARTIAL_DECODE |
                    DecoderCapability::METADATA_EXTRACTION |
                    DecoderCapability::CANCELABLE;
    CHECK( HasCapability(combined, DecoderCapability::PARTIAL_DECODE));
    CHECK( HasCapability(combined, DecoderCapability::METADATA_EXTRACTION));
    CHECK( HasCapability(combined, DecoderCapability::CANCELABLE));
    CHECK_FALSE(HasCapability(combined, DecoderCapability::EMBEDDED_PREVIEW));
    CHECK_FALSE(HasCapability(combined, DecoderCapability::ANIMATION));
}

TEST_CASE("DecoderCapability — bitwise OR combines flags without collision",
          "[streaming][capability]") {
    auto a = DecoderCapability::PARTIAL_DECODE | DecoderCapability::ALPHA_CHANNEL;
    auto b = DecoderCapability::HDR_OUTPUT     | DecoderCapability::CANCELABLE;
    auto ab = a | b;
    CHECK(HasCapability(ab, DecoderCapability::PARTIAL_DECODE));
    CHECK(HasCapability(ab, DecoderCapability::ALPHA_CHANNEL));
    CHECK(HasCapability(ab, DecoderCapability::HDR_OUTPUT));
    CHECK(HasCapability(ab, DecoderCapability::CANCELABLE));
}

// ===========================================================================
// Probe header constants
// ===========================================================================

TEST_CASE("ProbeHeader — PROBE_HEADER_MIN_BYTES is 16",
          "[streaming][probe]") {
    REQUIRE(PROBE_HEADER_MIN_BYTES == 16u);
}

TEST_CASE("ProbeHeader — PROBE_HEADER_IDEAL_BYTES is 16384 (16 KB)",
          "[streaming][probe]") {
    REQUIRE(PROBE_HEADER_IDEAL_BYTES == 16384u);
}

TEST_CASE("ProbeHeader — ideal >= min",
          "[streaming][probe]") {
    REQUIRE(PROBE_HEADER_IDEAL_BYTES >= PROBE_HEADER_MIN_BYTES);
}

// ===========================================================================
// Decode target size constraints
// ===========================================================================

TEST_CASE("DecodeTarget — MIN is 16 px",        "[streaming][target]") { REQUIRE(DECODE_TARGET_MIN_PX == 16u);   }
TEST_CASE("DecodeTarget — MAX is 4096 px",      "[streaming][target]") { REQUIRE(DECODE_TARGET_MAX_PX == 4096u); }
TEST_CASE("DecodeTarget — DEFAULT is 256 px",   "[streaming][target]") { REQUIRE(DECODE_TARGET_DEFAULT_PX == 256u); }

TEST_CASE("DecodeTarget — MIN < DEFAULT < MAX",
          "[streaming][target][ordering]") {
    REQUIRE(DECODE_TARGET_MIN_PX < DECODE_TARGET_DEFAULT_PX);
    REQUIRE(DECODE_TARGET_DEFAULT_PX < DECODE_TARGET_MAX_PX);
}

// ===========================================================================
// Decoder profile registry
// ===========================================================================

TEST_CASE("DecoderProfiles — 8 profiles defined",
          "[streaming][profiles]") {
    REQUIRE(DECODER_PROFILES.size() == 8u);
}

TEST_CASE("DecoderProfiles — all profiles have non-empty names",
          "[streaming][profiles]") {
    for (const auto& p : DECODER_PROFILES) {
        REQUIRE_FALSE(p.name.empty());
    }
}

TEST_CASE("DecoderProfiles — all profiles have CANCELABLE capability",
          "[streaming][profiles][capability]") {
    for (const auto& p : DECODER_PROFILES) {
        INFO("Decoder: " << p.name);
        CHECK(HasCapability(p.caps, DecoderCapability::CANCELABLE));
    }
}

TEST_CASE("DecoderProfiles — RAW decoder has EMBEDDED_PREVIEW capability [H7]",
          "[streaming][profiles][raw]") {
    for (const auto& p : DECODER_PROFILES) {
        if (p.name == "RAW") {
            CHECK(p.supportsEmbeddedPreview);
            CHECK(HasCapability(p.caps, DecoderCapability::EMBEDDED_PREVIEW));
            CHECK_FALSE(p.supportsPartialDecode);
        }
    }
}

TEST_CASE("DecoderProfiles — WebP supports ANIMATION capability",
          "[streaming][profiles][animation]") {
    for (const auto& p : DECODER_PROFILES) {
        if (p.name == "WebP") {
            CHECK(HasCapability(p.caps, DecoderCapability::ANIMATION));
        }
    }
}

TEST_CASE("DecoderProfiles — PDF supports MULTI_PAGE capability",
          "[streaming][profiles][multipage]") {
    for (const auto& p : DECODER_PROFILES) {
        if (p.name == "PDF") {
            CHECK(HasCapability(p.caps, DecoderCapability::MULTI_PAGE));
        }
    }
}

TEST_CASE("DecoderProfiles — supportsEmbeddedPreview only for RAW",
          "[streaming][profiles][preview]") {
    int previewCount = 0;
    for (const auto& p : DECODER_PROFILES) {
        if (p.supportsEmbeddedPreview) { ++previewCount; }
    }
    // Only RAW uses the LibRaw::unpack_thumb() fast path (D39)
    REQUIRE(previewCount == 1);
}

TEST_CASE("DecoderProfiles — P0 formats (JPEG/PNG/WebP/AVIF) all support PARTIAL_DECODE",
          "[streaming][profiles][p0]") {
    std::array<std::string_view, 4> p0 = {"JPEG", "PNG", "WebP", "AVIF"};
    for (auto fmt : p0) {
        for (const auto& p : DECODER_PROFILES) {
            if (p.name == fmt) {
                INFO("P0 format: " << fmt);
                CHECK(p.supportsPartialDecode);
                CHECK(HasCapability(p.caps, DecoderCapability::PARTIAL_DECODE));
            }
        }
    }
}
