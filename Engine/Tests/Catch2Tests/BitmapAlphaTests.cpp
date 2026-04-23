// BitmapAlphaTests.cpp — Catch2 tests for BGRA32 pixel layout + alpha compositing
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the BGRA32 pixel format contract (§7.4, §9.3):
//   - Byte layout: B @ offset 0, G @ 1, R @ 2, A @ 3
//   - Premultiplied alpha: R_pre = (R * A + 127) / 255 (round-to-nearest)
//   - Alpha range: 0 = fully transparent, 255 = fully opaque
//   - Premultiply identity: A=255 → R_pre == R
//   - Premultiply zero: A=0 → all channels zero
//   - Straight-alpha round-trip (premultiply then un-premultiply)
//   - Compositing: premultiplied BGRA over opaque white → expected sRGB value
//   - sRGB gamma linearize/encode formulae
//   - HDR float16 clamp before BGRA conversion
//
// All tests are self-contained — no Windows headers, no Engine headers.
//
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <array>

// ---------------------------------------------------------------------------
// BGRA32 pixel model (§7.4)
// ---------------------------------------------------------------------------

namespace ExplorerLens::Tests::BitmapAlpha {

// ── BGRA32 pixel (matches DXGI_FORMAT_B8G8R8A8_UNORM) ────────────────────

struct Bgra32 {
    uint8_t b, g, r, a;  // byte layout: B at [0], G at [1], R at [2], A at [3]

    constexpr bool operator==(const Bgra32& o) const noexcept {
        return b == o.b && g == o.g && r == o.r && a == o.a;
    }
};

static_assert(sizeof(Bgra32) == 4, "Bgra32 must be exactly 4 bytes");

// ── Premultiplied alpha operations ────────────────────────────────────────

/// Premultiply a single channel: chan_pre = (chan * alpha + 127) / 255
/// Uses round-to-nearest arithmetic (add 127 before dividing by 255)
inline constexpr uint8_t PremulChannel(uint8_t chan, uint8_t alpha) {
    return static_cast<uint8_t>((static_cast<uint32_t>(chan) * alpha + 127u) / 255u);
}

/// Premultiply all colour channels; alpha unchanged
inline constexpr Bgra32 Premultiply(Bgra32 px) {
    return {
        PremulChannel(px.b, px.a),
        PremulChannel(px.g, px.a),
        PremulChannel(px.r, px.a),
        px.a
    };
}

/// Un-premultiply a single channel (integer, lossy for low-alpha values)
inline constexpr uint8_t UnpremulChannel(uint8_t premul, uint8_t alpha) {
    if (alpha == 0) return 0;
    uint32_t v = (static_cast<uint32_t>(premul) * 255u + alpha / 2u) / alpha;
    return static_cast<uint8_t>((std::min)(v, 255u));
}

/// Un-premultiply all colour channels; alpha unchanged
inline constexpr Bgra32 Unpremultiply(Bgra32 px) {
    return {
        UnpremulChannel(px.b, px.a),
        UnpremulChannel(px.g, px.a),
        UnpremulChannel(px.r, px.a),
        px.a
    };
}

// ── Compositing: src (premultiplied) over opaque white ───────────────────

/// Porter-Duff "src over white":  out = src + white * (1 - src.a/255)
/// Input px must be premultiplied.  Output is opaque BGRA32.
inline Bgra32 ComposeOverWhite(Bgra32 srcPre) {
    auto blend = [&](uint8_t srcChan) -> uint8_t {
        // srcChan is already premultiplied; alpha complement applied to white (255)
        uint32_t comp = 255u - srcPre.a;
        uint32_t out  = static_cast<uint32_t>(srcChan) + (255u * comp + 127u) / 255u;
        return static_cast<uint8_t>((std::min)(out, 255u));
    };
    return { blend(srcPre.b), blend(srcPre.g), blend(srcPre.r), 255 };
}

// ── sRGB gamma model (approximate piecewise) ─────────────────────────────

/// Linearise sRGB → linear light (float [0,1])
inline float sRGBToLinear(float v) {
    if (v <= 0.04045f) return v / 12.92f;
    return std::pow((v + 0.055f) / 1.055f, 2.4f);
}

/// Linear light → sRGB (float [0,1])
inline float LinearToSRGB(float v) {
    if (v <= 0.0031308f) return v * 12.92f;
    return 1.055f * std::pow(v, 1.0f / 2.4f) - 0.055f;
}

// ── Half-float clamp model ────────────────────────────────────────────────

/// Clamp a float16 value (modelled as float here) to [0, 1] for BGRA conversion
inline float ClampHDRToSRGB(float hdr) {
    return (std::max)(0.0f, (std::min)(1.0f, hdr));
}

/// Convert clamped HDR float → uint8 sRGB channel
inline uint8_t HDRFloatToU8(float hdrLinear) {
    float clamped = ClampHDRToSRGB(hdrLinear);
    float srgb    = LinearToSRGB(clamped);
    return static_cast<uint8_t>(std::round(srgb * 255.0f));
}

} // namespace ExplorerLens::Tests::BitmapAlpha

using namespace ExplorerLens::Tests::BitmapAlpha;

// ===========================================================================
// BGRA32 byte layout
// ===========================================================================

TEST_CASE("Bgra32 — size is exactly 4 bytes",
          "[bgra][layout]") {
    REQUIRE(sizeof(Bgra32) == 4u);
}

TEST_CASE("Bgra32 — byte ordering: B=0, G=1, R=2, A=3",
          "[bgra][layout]") {
    Bgra32 px{0x01, 0x02, 0x03, 0x04};
    const auto* bytes = reinterpret_cast<const uint8_t*>(&px);
    CHECK(bytes[0] == 0x01); // B
    CHECK(bytes[1] == 0x02); // G
    CHECK(bytes[2] == 0x03); // R
    CHECK(bytes[3] == 0x04); // A
}

// ===========================================================================
// PremulChannel
// ===========================================================================

TEST_CASE("PremulChannel — alpha=255 is identity",
          "[bgra][premul]") {
    auto r = GENERATE(0u, 1u, 64u, 128u, 200u, 255u);
    // Cast to match expected type
    CHECK(PremulChannel(static_cast<uint8_t>(r), 255) == static_cast<uint8_t>(r));
}

TEST_CASE("PremulChannel — alpha=0 gives 0",
          "[bgra][premul]") {
    CHECK(PremulChannel(0,   0) == 0u);
    CHECK(PremulChannel(128, 0) == 0u);
    CHECK(PremulChannel(255, 0) == 0u);
}

TEST_CASE("PremulChannel — alpha=128 halves channel (approximately)",
          "[bgra][premul]") {
    uint8_t result = PremulChannel(255, 128);
    // 255 * 128 / 255 = 128.0; with rounding = 128
    CHECK(result == 128u);
}

TEST_CASE("PremulChannel — result never exceeds channel value",
          "[bgra][premul]") {
    // Premultiplied value ≤ original for any valid alpha
    auto chan = GENERATE(0u, 64u, 128u, 200u, 255u);
    auto alpha = GENERATE(0u, 64u, 128u, 200u, 255u);
    CHECK(PremulChannel(static_cast<uint8_t>(chan), static_cast<uint8_t>(alpha))
          <= static_cast<uint8_t>(chan));
}

// ===========================================================================
// Premultiply
// ===========================================================================

TEST_CASE("Premultiply — A=255 is identity on all channels",
          "[bgra][premul]") {
    Bgra32 px{100, 150, 200, 255};
    auto result = Premultiply(px);
    CHECK(result.b == 100u);
    CHECK(result.g == 150u);
    CHECK(result.r == 200u);
    CHECK(result.a == 255u);
}

TEST_CASE("Premultiply — A=0 zeros all colour channels",
          "[bgra][premul]") {
    Bgra32 px{100, 150, 200, 0};
    auto result = Premultiply(px);
    CHECK(result.b == 0u);
    CHECK(result.g == 0u);
    CHECK(result.r == 0u);
    CHECK(result.a == 0u);
}

TEST_CASE("Premultiply — alpha channel is preserved unchanged",
          "[bgra][premul]") {
    auto alpha = GENERATE(0u, 64u, 128u, 200u, 255u);
    Bgra32 px{200, 200, 200, static_cast<uint8_t>(alpha)};
    auto result = Premultiply(px);
    CHECK(result.a == static_cast<uint8_t>(alpha));
}

// ===========================================================================
// Unpremultiply
// ===========================================================================

TEST_CASE("Unpremultiply — A=255 is identity",
          "[bgra][unpremul]") {
    Bgra32 px{100, 150, 200, 255};
    auto result = Unpremultiply(px);
    CHECK(result.b == 100u);
    CHECK(result.g == 150u);
    CHECK(result.r == 200u);
}

TEST_CASE("Unpremultiply — A=0 returns all zeros",
          "[bgra][unpremul]") {
    Bgra32 px{0, 0, 0, 0};
    auto result = Unpremultiply(px);
    CHECK(result.b == 0u);
    CHECK(result.g == 0u);
    CHECK(result.r == 0u);
}

TEST_CASE("Unpremultiply — round-trip (premultiply then unpremultiply) is near-lossless for alpha=128",
          "[bgra][unpremul][roundtrip]") {
    Bgra32 original{200, 100, 50, 128};
    auto premulled = Premultiply(original);
    auto recovered = Unpremultiply(premulled);
    // Allow ±1 rounding error per channel
    CHECK(std::abs(static_cast<int>(recovered.b) - static_cast<int>(original.b)) <= 1);
    CHECK(std::abs(static_cast<int>(recovered.g) - static_cast<int>(original.g)) <= 1);
    CHECK(std::abs(static_cast<int>(recovered.r) - static_cast<int>(original.r)) <= 1);
}

// ===========================================================================
// Compositing over white
// ===========================================================================

TEST_CASE("ComposeOverWhite — fully opaque src is unchanged",
          "[bgra][composite]") {
    Bgra32 src{100, 150, 200, 255}; // already premultiplied (alpha=255)
    auto out = ComposeOverWhite(src);
    CHECK(out.b == 100u);
    CHECK(out.g == 150u);
    CHECK(out.r == 200u);
    CHECK(out.a == 255u);
}

TEST_CASE("ComposeOverWhite — fully transparent src gives white",
          "[bgra][composite]") {
    Bgra32 src{0, 0, 0, 0}; // transparent black
    auto out = ComposeOverWhite(src);
    CHECK(out.b == 255u);
    CHECK(out.g == 255u);
    CHECK(out.r == 255u);
    CHECK(out.a == 255u);
}

TEST_CASE("ComposeOverWhite — output alpha is always 255",
          "[bgra][composite]") {
    auto alpha = GENERATE(0u, 64u, 128u, 200u, 255u);
    Bgra32 src{128, 128, 128, static_cast<uint8_t>(alpha)};
    auto out = ComposeOverWhite(src);
    CHECK(out.a == 255u);
}

// ===========================================================================
// sRGB gamma
// ===========================================================================

TEST_CASE("sRGBToLinear — value 0.0 maps to 0.0",
          "[bgra][srgb]") {
    REQUIRE(std::fabs(sRGBToLinear(0.0f)) < 1e-6f);
}

TEST_CASE("sRGBToLinear — value 1.0 maps to 1.0",
          "[bgra][srgb]") {
    REQUIRE(std::fabs(sRGBToLinear(1.0f) - 1.0f) < 1e-4f);
}

TEST_CASE("sRGBToLinear — value 0.5 maps to ~0.2140",
          "[bgra][srgb]") {
    float linear = sRGBToLinear(0.5f);
    CHECK(linear > 0.20f);
    CHECK(linear < 0.23f);
}

TEST_CASE("LinearToSRGB — value 0.0 maps to 0.0",
          "[bgra][srgb]") {
    REQUIRE(std::fabs(LinearToSRGB(0.0f)) < 1e-6f);
}

TEST_CASE("LinearToSRGB — value 1.0 maps to 1.0",
          "[bgra][srgb]") {
    REQUIRE(std::fabs(LinearToSRGB(1.0f) - 1.0f) < 1e-4f);
}

TEST_CASE("sRGB round-trip — linearise then encode returns original ±1/255",
          "[bgra][srgb][roundtrip]") {
    auto val = GENERATE(0.0f, 0.1f, 0.25f, 0.5f, 0.75f, 1.0f);
    float roundtrip = LinearToSRGB(sRGBToLinear(val));
    CHECK(std::fabs(roundtrip - val) < 0.005f);
}

// ===========================================================================
// HDR float clamp
// ===========================================================================

TEST_CASE("ClampHDRToSRGB — values in [0,1] are unchanged",
          "[bgra][hdr]") {
    CHECK(ClampHDRToSRGB(0.0f)  == 0.0f);
    CHECK(ClampHDRToSRGB(0.5f)  == 0.5f);
    CHECK(ClampHDRToSRGB(1.0f)  == 1.0f);
}

TEST_CASE("ClampHDRToSRGB — negative values clamp to 0",
          "[bgra][hdr]") {
    CHECK(ClampHDRToSRGB(-1.0f)  == 0.0f);
    CHECK(ClampHDRToSRGB(-100.f) == 0.0f);
}

TEST_CASE("ClampHDRToSRGB — values above 1 clamp to 1",
          "[bgra][hdr]") {
    CHECK(ClampHDRToSRGB(2.0f)  == 1.0f);
    CHECK(ClampHDRToSRGB(100.f) == 1.0f);
}

TEST_CASE("HDRFloatToU8 — linear 0.0 gives 0",
          "[bgra][hdr]") {
    REQUIRE(HDRFloatToU8(0.0f) == 0u);
}

TEST_CASE("HDRFloatToU8 — linear 1.0 gives 255",
          "[bgra][hdr]") {
    REQUIRE(HDRFloatToU8(1.0f) == 255u);
}

TEST_CASE("HDRFloatToU8 — linear 2.0 (HDR over-exposure) clamps to 255",
          "[bgra][hdr]") {
    REQUIRE(HDRFloatToU8(2.0f) == 255u);
}
