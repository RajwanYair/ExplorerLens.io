// ColorSpaceTests.cpp — Catch2 tests for sRGB/linear/P3 color math
// Copyright (c) 2026 ExplorerLens Project
//
// Validates the color-space conversion math used in the GPU renderer and
// BGRA32 compositing path (§9.3, §10.3 GPU Phase 2 prep).
//
// Self-contained: does NOT include Engine headers — the math functions are
// defined in this translation unit to avoid linking the GPU pipeline library.
// The real production implementations in Engine/GPU/ColorSpaceTransform.h
// must remain consistent with these golden values.
//
// IEC 61966-2-1 (sRGB) piecewise transfer function reference:
//   linear = srgb / 12.92,                  srgb <= 0.04045
//   linear = ((srgb + 0.055) / 1.055)^2.4,  srgb >  0.04045
//
//   srgb = linear * 12.92,                   linear <= 0.0031308
//   srgb = 1.055 * linear^(1/2.4) - 0.055,  linear >  0.0031308
//
// DCI-P3 / Display-P3 matrix (D65 white point, from ICC v4.4):
//   P3 ≈ sRGB primaries rotated; for display-P3 the conversion is via the
//   linear-light matrix multiplication with a 3×3 matrix.
//
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

namespace ExplorerLens { namespace Tests { namespace ColorSpace {

// =============================================================================
// IEC 61966-2-1 sRGB transfer functions
// =============================================================================

/// sRGB → linear (inverse EOTF)
inline double SrgbToLinear(double srgb) noexcept {
    srgb = std::max(0.0, std::min(1.0, srgb));
    if (srgb <= 0.04045)
        return srgb / 12.92;
    return std::pow((srgb + 0.055) / 1.055, 2.4);
}

/// Linear → sRGB (EOTF)
inline double LinearToSrgb(double linear) noexcept {
    linear = std::max(0.0, std::min(1.0, linear));
    if (linear <= 0.0031308)
        return linear * 12.92;
    return 1.055 * std::pow(linear, 1.0 / 2.4) - 0.055;
}

/// Clamp to [0, 1]
inline double ClampF(double v) noexcept {
    return std::max(0.0, std::min(1.0, v));
}

/// 8-bit sRGB value → linear float
inline double Srgb8ToLinear(uint8_t byte) noexcept {
    return SrgbToLinear(byte / 255.0);
}

/// Linear float → 8-bit sRGB value (rounded)
inline uint8_t LinearToSrgb8(double linear) noexcept {
    return static_cast<uint8_t>(std::round(LinearToSrgb(linear) * 255.0));
}

// =============================================================================
// sRGB → Display-P3 matrix (linear-light, D65)
// Coefficients from ICC Profile Specification ICC.1:2022, Table 24
// =============================================================================

struct RGB { double r, g, b; };

inline RGB SrgbLinearToDisplayP3Linear(double r, double g, double b) noexcept {
    // sRGB primaries to XYZ(D65)
    const double xr = 0.4124564*r + 0.3575761*g + 0.1804375*b;
    const double yr = 0.2126729*r + 0.7151522*g + 0.0721750*b;
    const double zr = 0.0193339*r + 0.1191920*g + 0.9503041*b;
    // XYZ(D65) to Display-P3 (D65)
    const double pr =  2.4934969*xr - 0.9313836*yr - 0.4027107*zr;
    const double pg = -0.8294890*xr + 1.7626641*yr + 0.0236247*zr;
    const double pb =  0.0358458*xr - 0.0761724*yr + 0.9568845*zr;
    return { ClampF(pr), ClampF(pg), ClampF(pb) };
}

}}} // ExplorerLens::Tests::ColorSpace

using namespace ExplorerLens::Tests::ColorSpace;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

// =============================================================================
// §1 — Boundary values
// =============================================================================

TEST_CASE("ColorSpace: SrgbToLinear(0.0) == 0.0", "[color][srgb]") {
    REQUIRE_THAT(SrgbToLinear(0.0), WithinAbs(0.0, 1e-10));
}

TEST_CASE("ColorSpace: SrgbToLinear(1.0) == 1.0", "[color][srgb]") {
    REQUIRE_THAT(SrgbToLinear(1.0), WithinAbs(1.0, 1e-6));
}

TEST_CASE("ColorSpace: LinearToSrgb(0.0) == 0.0", "[color][srgb]") {
    REQUIRE_THAT(LinearToSrgb(0.0), WithinAbs(0.0, 1e-10));
}

TEST_CASE("ColorSpace: LinearToSrgb(1.0) == 1.0", "[color][srgb]") {
    REQUIRE_THAT(LinearToSrgb(1.0), WithinAbs(1.0, 1e-6));
}

// =============================================================================
// §2 — Roundtrip: sRGB → linear → sRGB
// =============================================================================

TEST_CASE("ColorSpace: roundtrip sRGB→linear→sRGB is identity", "[color][roundtrip]") {
    // Test a grid of sRGB values across [0, 1]
    for (int i = 0; i <= 100; ++i) {
        double s = i / 100.0;
        double reconstructed = LinearToSrgb(SrgbToLinear(s));
        REQUIRE_THAT(reconstructed, WithinAbs(s, 1e-9));
    }
}

TEST_CASE("ColorSpace: roundtrip linear→sRGB→linear is identity", "[color][roundtrip]") {
    for (int i = 0; i <= 100; ++i) {
        double lin = i / 100.0;
        double reconstructed = SrgbToLinear(LinearToSrgb(lin));
        REQUIRE_THAT(reconstructed, WithinAbs(lin, 1e-9));
    }
}

// =============================================================================
// §3 — Known golden values (IEC 61966-2-1 reference lookup table)
// =============================================================================

TEST_CASE("ColorSpace: SrgbToLinear(0.5) ≈ 0.2140 (IEC 61966-2-1 mid-gray)", "[color][golden]") {
    REQUIRE_THAT(SrgbToLinear(0.5), WithinAbs(0.2140, 0.001));
}

TEST_CASE("ColorSpace: LinearToSrgb(0.2140) ≈ 0.5 (inverse mid-gray)", "[color][golden]") {
    REQUIRE_THAT(LinearToSrgb(0.2140), WithinAbs(0.5, 0.001));
}

TEST_CASE("ColorSpace: SrgbToLinear(0.04045) boundary — piecewise knee point", "[color][golden]") {
    // At the knee: srgb/12.92 = ((srgb+0.055)/1.055)^2.4 ≈ 0.003131
    double lin = SrgbToLinear(0.04045);
    REQUIRE_THAT(lin, WithinAbs(0.003131, 0.0001));
}

TEST_CASE("ColorSpace: SrgbToLinear(0.2) ≈ 0.03310 (standard IEC reference point)", "[color][golden]") {
    // From IEC 61966-2-1 Table B.1
    REQUIRE_THAT(SrgbToLinear(0.2), WithinAbs(0.03310, 0.001));
}

TEST_CASE("ColorSpace: 8-bit sRGB 128 → linear ≈ 0.2158", "[color][8bit]") {
    // sRGB 128/255 ≈ 0.5020 → linear ≈ 0.2158
    REQUIRE_THAT(Srgb8ToLinear(128), WithinAbs(0.2158, 0.002));
}

TEST_CASE("ColorSpace: linear → 8-bit sRGB roundtrip for white (255)", "[color][8bit]") {
    REQUIRE(LinearToSrgb8(1.0) == 255);
}

TEST_CASE("ColorSpace: linear → 8-bit sRGB roundtrip for black (0)", "[color][8bit]") {
    REQUIRE(LinearToSrgb8(0.0) == 0);
}

// =============================================================================
// §4 — Gamma behavior: linear < sRGB for mid-tones (gamma < 1 input)
// =============================================================================

TEST_CASE("ColorSpace: linear mid-tone < sRGB mid-tone (gamma darkening)", "[color][gamma]") {
    // For 0 < sRGB < 1, linear < sRGB (gamma expansion darkens)
    double s = 0.5;
    REQUIRE(SrgbToLinear(s) < s);
}

TEST_CASE("ColorSpace: sRGB mid-tone > linear mid-tone (gamma encoding brightens)", "[color][gamma]") {
    double lin = 0.2;
    REQUIRE(LinearToSrgb(lin) > lin);
}

TEST_CASE("ColorSpace: monotonically increasing: larger sRGB → larger linear", "[color][gamma]") {
    for (int i = 1; i <= 100; ++i) {
        double s1 = (i - 1) / 100.0;
        double s2 = i       / 100.0;
        REQUIRE(SrgbToLinear(s1) < SrgbToLinear(s2));
    }
}

// =============================================================================
// §5 — Clamp behavior
// =============================================================================

TEST_CASE("ColorSpace: SrgbToLinear clamps negative input to 0", "[color][clamp]") {
    REQUIRE_THAT(SrgbToLinear(-0.5), WithinAbs(0.0, 1e-10));
}

TEST_CASE("ColorSpace: SrgbToLinear clamps input > 1 to linear(1.0)", "[color][clamp]") {
    REQUIRE_THAT(SrgbToLinear(2.0), WithinAbs(1.0, 1e-6));
}

TEST_CASE("ColorSpace: LinearToSrgb clamps negative to 0", "[color][clamp]") {
    REQUIRE_THAT(LinearToSrgb(-1.0), WithinAbs(0.0, 1e-10));
}

TEST_CASE("ColorSpace: LinearToSrgb clamps > 1 to sRGB(1.0)", "[color][clamp]") {
    REQUIRE_THAT(LinearToSrgb(5.0), WithinAbs(1.0, 1e-6));
}

TEST_CASE("ColorSpace: ClampF behavior", "[color][clamp]") {
    REQUIRE_THAT(ClampF(-99.0), WithinAbs(0.0, 1e-10));
    REQUIRE_THAT(ClampF(0.5),   WithinAbs(0.5, 1e-10));
    REQUIRE_THAT(ClampF(99.0),  WithinAbs(1.0, 1e-10));
}

// =============================================================================
// §6 — Display-P3 gamut (sRGB white → P3 white)
// =============================================================================

TEST_CASE("ColorSpace: sRGB(1,1,1) white → P3(1,1,1) white", "[color][p3]") {
    auto [r, g, b] = SrgbLinearToDisplayP3Linear(1.0, 1.0, 1.0);
    REQUIRE_THAT(r, WithinAbs(1.0, 0.01));
    REQUIRE_THAT(g, WithinAbs(1.0, 0.01));
    REQUIRE_THAT(b, WithinAbs(1.0, 0.01));
}

TEST_CASE("ColorSpace: sRGB(0,0,0) black → P3(0,0,0) black", "[color][p3]") {
    auto [r, g, b] = SrgbLinearToDisplayP3Linear(0.0, 0.0, 0.0);
    REQUIRE_THAT(r, WithinAbs(0.0, 1e-8));
    REQUIRE_THAT(g, WithinAbs(0.0, 1e-8));
    REQUIRE_THAT(b, WithinAbs(0.0, 1e-8));
}

TEST_CASE("ColorSpace: P3 output values are clamped to [0,1]", "[color][p3]") {
    // All-red sRGB (linear): P3 red will be near 1 but may clip to 1
    auto [r, g, b] = SrgbLinearToDisplayP3Linear(1.0, 0.0, 0.0);
    REQUIRE(r >= 0.0); REQUIRE(r <= 1.0);
    REQUIRE(g >= 0.0); REQUIRE(g <= 1.0);
    REQUIRE(b >= 0.0); REQUIRE(b <= 1.0);
}

TEST_CASE("ColorSpace: mid-gray sRGB→P3 is achromatic (equal RGB)", "[color][p3]") {
    // D65 neutral gray is metamerically neutral in both spaces
    double s = SrgbToLinear(0.5);  // ≈ 0.2140
    auto [r, g, b] = SrgbLinearToDisplayP3Linear(s, s, s);
    REQUIRE_THAT(r, WithinAbs(g, 0.01));
    REQUIRE_THAT(g, WithinAbs(b, 0.01));
}

// =============================================================================
// §7 — Parametric: NaN/Inf clamping safety
// =============================================================================

TEST_CASE("ColorSpace: SrgbToLinear clamps NaN-free input grid", "[color][parametric]") {
    // Verify output is never NaN or Inf for valid [0,1] inputs
    int steps = GENERATE(10, 50, 100);
    for (int i = 0; i <= steps; ++i) {
        double s = static_cast<double>(i) / static_cast<double>(steps);
        double lin = SrgbToLinear(s);
        REQUIRE(std::isfinite(lin));
        REQUIRE(lin >= 0.0);
        REQUIRE(lin <= 1.0);
    }
}
