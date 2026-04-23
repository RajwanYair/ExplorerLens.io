// MultiPageTests.cpp — Catch2 tests for multi-page document contracts
// Copyright (c) 2026 ExplorerLens Project
//
// Validates multi-page document model (§7.4, D38 MULTI_PAGE capability):
//   - Page count is ≥ 1 for all valid documents
//   - Page index 0 = cover page (first page / first frame)
//   - Out-of-range page index → error (not UB, not silent)
//   - PDF page-index contract
//   - TIFF IFD directory model
//   - GIF / APNG animation frame contract
//   - Cover thumbnail always extracted from page/frame 0
//   - Multi-page capability flag presence on supporting formats
//
// All tests are self-contained — no Windows headers, no Engine headers.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string_view>
#include <array>

// ---------------------------------------------------------------------------
// Multi-page document model (§7.4)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::MultiPage {

// ── Capability flags ────────────────────────────────────────────────────────

enum class DecoderCap : uint32_t {
    NONE              = 0x00000000,
    SINGLE_PAGE       = 0x00000001,  // always one image
    MULTI_PAGE        = 0x00000002,  // N pages (PDF, TIFF multi-IFD)
    ANIMATED          = 0x00000004,  // animated frames (GIF, APNG, WebP anim)
    STREAMING         = 0x00000008,  // IStreamingDecoder::ProbeHeader supported
    EMBEDDED_PREVIEW  = 0x00000010,  // RAW fast-path embedded JPEG
};

inline constexpr DecoderCap operator|(DecoderCap a, DecoderCap b) {
    return static_cast<DecoderCap>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline constexpr bool HasCap(DecoderCap caps, DecoderCap flag) {
    return (static_cast<uint32_t>(caps) & static_cast<uint32_t>(flag)) != 0;
}

// ── Format capability table ────────────────────────────────────────────────

struct FormatCaps {
    std::string_view name;
    DecoderCap       caps;
};

static constexpr std::array<FormatCaps, 12> FORMAT_CAPS = {{
    { "JPEG",  DecoderCap::SINGLE_PAGE | DecoderCap::STREAMING | DecoderCap::EMBEDDED_PREVIEW },
    { "PNG",   DecoderCap::SINGLE_PAGE | DecoderCap::STREAMING },
    { "APNG",  DecoderCap::ANIMATED   | DecoderCap::STREAMING },
    { "GIF",   DecoderCap::ANIMATED   | DecoderCap::STREAMING },
    { "WebP",  DecoderCap::ANIMATED   | DecoderCap::STREAMING },
    { "PDF",   DecoderCap::MULTI_PAGE | DecoderCap::STREAMING },
    { "TIFF",  DecoderCap::MULTI_PAGE | DecoderCap::STREAMING },
    { "PSD",   DecoderCap::SINGLE_PAGE },
    { "EXR",   DecoderCap::SINGLE_PAGE | DecoderCap::STREAMING },
    { "RAW",   DecoderCap::SINGLE_PAGE | DecoderCap::EMBEDDED_PREVIEW | DecoderCap::STREAMING },
    { "AVIF",  DecoderCap::SINGLE_PAGE | DecoderCap::STREAMING },
    { "HEIC",  DecoderCap::MULTI_PAGE  | DecoderCap::STREAMING },
}};

// ── Page count model ────────────────────────────────────────────────────────

struct PageRange {
    uint32_t pageCount;   // 1-based total count

    /// Returns true if a 0-based page index is within range
    constexpr bool IsValidIndex(uint32_t idx) const noexcept {
        return pageCount > 0 && idx < pageCount;
    }

    /// Cover page index — always 0
    static constexpr uint32_t CoverIndex() noexcept { return 0; }

    /// First page is always valid (as long as pageCount >= 1)
    constexpr bool HasCoverPage() const noexcept {
        return pageCount >= 1;
    }
};

// ── TIFF IFD model ─────────────────────────────────────────────────────────

struct TiffIFD {
    uint32_t ifdIndex;    // 0-based IFD index within the TIFF file
    uint32_t imageWidth;
    uint32_t imageHeight;
    bool     isReducedImage;  // true for TIFF sub-file type REDUCED_IMAGE
};

/// Validate that a TIFF IFD sequence is correctly ordered (IFDs 0..n-1)
inline bool TiffIFDsAreMonotonic(const TiffIFD* ifds, uint32_t count) {
    for (uint32_t i = 1; i < count; ++i) {
        if (ifds[i].ifdIndex != i) return false;
    }
    return true;
}

// ── GIF / APNG animation model ────────────────────────────────────────────

struct AnimationFrame {
    uint32_t frameIndex;    // 0-based
    uint32_t delayMs;       // display delay in milliseconds
    uint32_t width;
    uint32_t height;
};

/// Cover thumbnail = frame 0 of animation
static constexpr uint32_t ANIMATION_COVER_FRAME = 0;

/// Minimum valid animation delay (1 ms)
static constexpr uint32_t MIN_FRAME_DELAY_MS = 1;

/// GIF standard minimum delay: 10 cs (100ms); we enforce >= 1ms for APNG
static constexpr uint32_t GIF_MIN_DELAY_MS   = 20; // 20 ms (2 centiseconds)

} // namespace ExplorerLens::Tests::MultiPage

using namespace ExplorerLens::Tests::MultiPage;

// ===========================================================================
// Capability flag model
// ===========================================================================

TEST_CASE("MultiPage — MULTI_PAGE cap flag is defined and non-zero",
          "[multipage][caps]") {
    REQUIRE(static_cast<uint32_t>(DecoderCap::MULTI_PAGE) != 0u);
}

TEST_CASE("MultiPage — ANIMATED cap flag is distinct from MULTI_PAGE",
          "[multipage][caps]") {
    REQUIRE(DecoderCap::ANIMATED != DecoderCap::MULTI_PAGE);
}

TEST_CASE("MultiPage — HasCap returns correct result",
          "[multipage][caps]") {
    auto caps = DecoderCap::MULTI_PAGE | DecoderCap::STREAMING;
    CHECK(HasCap(caps, DecoderCap::MULTI_PAGE));
    CHECK(HasCap(caps, DecoderCap::STREAMING));
    CHECK_FALSE(HasCap(caps, DecoderCap::ANIMATED));
    CHECK_FALSE(HasCap(caps, DecoderCap::SINGLE_PAGE));
}

// ===========================================================================
// Format capability table
// ===========================================================================

TEST_CASE("MultiPage — format caps table has 12 entries",
          "[multipage][table]") {
    REQUIRE(FORMAT_CAPS.size() == 12u);
}

TEST_CASE("MultiPage — all format names are non-empty",
          "[multipage][table]") {
    for (const auto& f : FORMAT_CAPS) {
        CHECK_FALSE(f.name.empty());
    }
}

TEST_CASE("MultiPage — no duplicate format names",
          "[multipage][table]") {
    for (size_t i = 0; i < FORMAT_CAPS.size(); ++i)
        for (size_t j = i + 1; j < FORMAT_CAPS.size(); ++j)
            CHECK(FORMAT_CAPS[i].name != FORMAT_CAPS[j].name);
}

TEST_CASE("MultiPage — PDF has MULTI_PAGE capability",
          "[multipage][table][pdf]") {
    for (const auto& f : FORMAT_CAPS) {
        if (f.name == "PDF") {
            REQUIRE(HasCap(f.caps, DecoderCap::MULTI_PAGE));
            return;
        }
    }
    FAIL("PDF not found in caps table");
}

TEST_CASE("MultiPage — TIFF has MULTI_PAGE capability",
          "[multipage][table][tiff]") {
    for (const auto& f : FORMAT_CAPS) {
        if (f.name == "TIFF") {
            REQUIRE(HasCap(f.caps, DecoderCap::MULTI_PAGE));
            return;
        }
    }
    FAIL("TIFF not found in caps table");
}

TEST_CASE("MultiPage — GIF has ANIMATED capability (not MULTI_PAGE)",
          "[multipage][table][gif]") {
    for (const auto& f : FORMAT_CAPS) {
        if (f.name == "GIF") {
            CHECK(HasCap(f.caps, DecoderCap::ANIMATED));
            CHECK_FALSE(HasCap(f.caps, DecoderCap::MULTI_PAGE));
            return;
        }
    }
    FAIL("GIF not found in caps table");
}

TEST_CASE("MultiPage — JPEG has EMBEDDED_PREVIEW capability",
          "[multipage][table][jpeg]") {
    for (const auto& f : FORMAT_CAPS) {
        if (f.name == "JPEG") {
            REQUIRE(HasCap(f.caps, DecoderCap::EMBEDDED_PREVIEW));
            return;
        }
    }
    FAIL("JPEG not found in caps table");
}

TEST_CASE("MultiPage — RAW has EMBEDDED_PREVIEW capability",
          "[multipage][table][raw]") {
    for (const auto& f : FORMAT_CAPS) {
        if (f.name == "RAW") {
            REQUIRE(HasCap(f.caps, DecoderCap::EMBEDDED_PREVIEW));
            return;
        }
    }
    FAIL("RAW not found in caps table");
}

// ===========================================================================
// PageRange model
// ===========================================================================

TEST_CASE("PageRange — cover page index is always 0",
          "[multipage][pagerange]") {
    REQUIRE(PageRange::CoverIndex() == 0u);
}

TEST_CASE("PageRange — single-page document has valid cover",
          "[multipage][pagerange]") {
    PageRange pr{1};
    CHECK(pr.HasCoverPage());
    CHECK(pr.IsValidIndex(0));
    CHECK_FALSE(pr.IsValidIndex(1));
}

TEST_CASE("PageRange — multi-page PDF: pages 0..N-1 are valid",
          "[multipage][pagerange][pdf]") {
    PageRange pr{10};
    for (uint32_t i = 0; i < 10; ++i) {
        CHECK(pr.IsValidIndex(i));
    }
    CHECK_FALSE(pr.IsValidIndex(10));
    CHECK_FALSE(pr.IsValidIndex(100));
}

TEST_CASE("PageRange — page count 0 has no valid index",
          "[multipage][pagerange][edge]") {
    PageRange pr{0};
    CHECK_FALSE(pr.HasCoverPage());
    CHECK_FALSE(pr.IsValidIndex(0));
}

// ===========================================================================
// TIFF IFD model
// ===========================================================================

TEST_CASE("TiffIFD — IFD0 is the primary image (first image)",
          "[multipage][tiff]") {
    TiffIFD ifd0{0, 1920, 1080, false};
    REQUIRE(ifd0.ifdIndex == 0u);
}

TEST_CASE("TiffIFDsAreMonotonic — correctly ordered IFDs return true",
          "[multipage][tiff]") {
    TiffIFD ifds[] = {
        {0, 1920, 1080, false},
        {1, 960,  540,  true},
        {2, 480,  270,  true},
    };
    REQUIRE(TiffIFDsAreMonotonic(ifds, 3));
}

TEST_CASE("TiffIFDsAreMonotonic — out-of-order IFDs return false",
          "[multipage][tiff]") {
    TiffIFD ifds[] = {
        {0, 1920, 1080, false},
        {2, 960,  540,  true},  // gap: should be 1
    };
    REQUIRE_FALSE(TiffIFDsAreMonotonic(ifds, 2));
}

TEST_CASE("TiffIFD — reduced-image IFDs are auxiliary (not primary)",
          "[multipage][tiff]") {
    TiffIFD primary{0, 3200, 2400, false};
    TiffIFD reduced{1, 800,  600,  true};
    CHECK_FALSE(primary.isReducedImage);
    CHECK(reduced.isReducedImage);
}

// ===========================================================================
// Animation frame model
// ===========================================================================

TEST_CASE("Animation — cover frame index is 0",
          "[multipage][animation]") {
    REQUIRE(ANIMATION_COVER_FRAME == 0u);
}

TEST_CASE("Animation — valid GIF frame has index < totalFrames",
          "[multipage][animation][gif]") {
    uint32_t totalFrames = 20;
    for (uint32_t i = 0; i < totalFrames; ++i) {
        CHECK(i < totalFrames);
    }
    CHECK_FALSE(totalFrames < totalFrames); // boundary: index == count is invalid
}

TEST_CASE("Animation — frame delay >= 1 ms",
          "[multipage][animation][delay]") {
    AnimationFrame f{0, 100, 320, 240};
    REQUIRE(f.delayMs >= MIN_FRAME_DELAY_MS);
}

TEST_CASE("Animation — GIF frame delay should be >= 20 ms",
          "[multipage][animation][gif][delay]") {
    // GIF spec: centiseconds → multiply by 10. min browser-honoured: 20ms
    uint32_t gifDelayMs = 30; // reasonable 30 fps frame
    CHECK(gifDelayMs >= GIF_MIN_DELAY_MS);
}

TEST_CASE("Animation — zero delay frame is clamped to minimum",
          "[multipage][animation][delay][edge]") {
    uint32_t declared = 0;
    uint32_t effective = (std::max)(declared, MIN_FRAME_DELAY_MS);
    REQUIRE(effective == MIN_FRAME_DELAY_MS);
}
