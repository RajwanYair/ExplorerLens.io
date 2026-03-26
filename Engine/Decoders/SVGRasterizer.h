// SVGRasterizer.h — SVG to Bitmap via Direct2D Rasterization
// Copyright (c) 2026 ExplorerLens Project
//
// Rasterizes SVG vector graphics to BGRA bitmaps using Windows Direct2D
// ID2D1SvgDocument (Win10 1703+) with a fallback to Basic SVG parsing
// for older elements.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <d2d1_3.h>
#include <wrl/client.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct SVGRasterizeOptions {
    uint32_t outputWidth{256};
    uint32_t outputHeight{256};
    bool     preserveAspectRatio{true};
    bool     antiAlias{true};
    uint32_t backgroundBGRA{0x00000000};  // transparent by default
    float    dpiScale{1.0f};
};

struct SVGRasterizeResult {
    std::vector<uint8_t> bgra;
    uint32_t             width{0};
    uint32_t             height{0};
    uint32_t             stride{0};
    bool                 usedFallback{false};
    std::string          warningMessage;
};

class SVGRasterizer {
public:
    SVGRasterizer() = default;
    ~SVGRasterizer() { Shutdown(); }

    SVGRasterizer(const SVGRasterizer&) = delete;
    SVGRasterizer& operator=(const SVGRasterizer&) = delete;

    [[nodiscard]] bool Initialize();
    void Shutdown() noexcept;

    [[nodiscard]] std::optional<SVGRasterizeResult> Rasterize(
        const void* svgData, size_t svgSize,
        const SVGRasterizeOptions& opts = {}) const;

    // Quick check: is this valid SVG data?
    static bool LooksLikeSVG(const void* data, size_t size) noexcept;

    [[nodiscard]] bool IsInitialized() const noexcept { return m_d2dFactory != nullptr; }

private:
    std::optional<SVGRasterizeResult> RasterizeWithD2D(
        const void* data, size_t size, const SVGRasterizeOptions& opts) const;

    std::optional<SVGRasterizeResult> RasterizeFallback(
        const void* data, size_t size, const SVGRasterizeOptions& opts) const;

    Microsoft::WRL::ComPtr<ID2D1Factory3>       m_d2dFactory;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext5> m_deviceContext;
};

} // namespace Engine
} // namespace ExplorerLens
