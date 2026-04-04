// ColorBlindFilter.h — Deuteranopia / Protanopia / Tritanopia Simulation
// Copyright (c) 2026 ExplorerLens Project
//
// Applies color-blindness simulation matrices to thumbnail pixel data,
// enabling QA verification that UI colors remain distinguishable for all users.
// Can also generate accessible palette suggestions.
//
#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string_view>

namespace ExplorerLens {
namespace Engine {

enum class ColorBlindType : uint8_t {
    None = 0,
    Deuteranopia = 1,   // red-green (green deficient), ~5% of males
    Protanopia = 2,     // red-green (red deficient),   ~1% of males
    Tritanopia = 3,     // blue-yellow,                 <0.01% of population
    Achromatopsia = 4,  // full color blindness (monochromacy)
    DeuterAnomalyMild = 5,
    ProtanomalyMild = 6,
};

// 3×3 RGB transform matrix (row-major, float)
using SimMatrix = std::array<float, 9>;

struct ColorBlindConfig
{
    ColorBlindType type{ColorBlindType::Deuteranopia};
    float severity{1.0f};  // 0.0 = no effect, 1.0 = full simulation
};

class ColorBlindFilter
{
  public:
    explicit ColorBlindFilter(ColorBlindConfig cfg = {}) : m_cfg(cfg) {}

    // Apply simulation in-place to a BGRA pixel buffer.
    void Apply(std::span<uint8_t> bgra, uint32_t width, uint32_t height, uint32_t stride) const noexcept;

    // Apply to a copy — leaves original intact.
    [[nodiscard]] std::vector<uint8_t> ApplyCopy(std::span<const uint8_t> bgra, uint32_t width, uint32_t height,
                                                 uint32_t stride) const;

    // Get the simulation matrix for a given type.
    static SimMatrix GetMatrix(ColorBlindType type) noexcept;

    // Compute a distinguishability score (0-1) between two BGRA colors under simulation.
    static float Distinguishability(uint32_t bgra1, uint32_t bgra2, ColorBlindType type) noexcept;

    // Check if a UI color pair is WCAG 2.1 AA distinguishable for the given deficiency.
    static bool MeetsWCAGContrastUnderSimulation(uint32_t fgBGRA, uint32_t bgBGRA, ColorBlindType type,
                                                 float minContrast = 4.5f) noexcept;

    [[nodiscard]] ColorBlindConfig Config() const noexcept
    {
        return m_cfg;
    }
    void SetConfig(ColorBlindConfig cfg) noexcept
    {
        m_cfg = cfg;
    }

  private:
    void TransformPixel(uint8_t& b, uint8_t& g, uint8_t& r, const SimMatrix& m, float severity) const noexcept;

    ColorBlindConfig m_cfg;
};

// Standard simulation matrices (Brettel 1997 / Viénot 1999)
namespace ColorMatrices {
inline constexpr SimMatrix Deuteranopia = {0.625f, 0.375f, 0.0f, 0.700f, 0.300f, 0.0f, 0.0f, 0.300f, 0.700f};
inline constexpr SimMatrix Protanopia = {0.567f, 0.433f, 0.0f, 0.558f, 0.442f, 0.0f, 0.0f, 0.242f, 0.758f};
inline constexpr SimMatrix Tritanopia = {0.950f, 0.050f, 0.0f, 0.0f, 0.433f, 0.567f, 0.0f, 0.475f, 0.525f};
inline constexpr SimMatrix Achromatopsia = {0.299f, 0.587f, 0.114f, 0.299f, 0.587f, 0.114f, 0.299f, 0.587f, 0.114f};
}  // namespace ColorMatrices

}  // namespace Engine
}  // namespace ExplorerLens
