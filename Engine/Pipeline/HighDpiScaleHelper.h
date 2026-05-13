// Engine/Pipeline/HighDpiScaleHelper.h
// ExplorerLens — WM_DPICHANGED-aware thumbnail sizing and scale factor management
// Sprint S337.
//
// Purpose:
//   Windows Explorer requests thumbnails via IThumbnailProvider::GetThumbnail(cx)
//   where `cx` is the pixel size for a 100% DPI display (96 DPI).  On high-DPI
//   displays (150% = 144 DPI, 200% = 192 DPI), Explorer scales up the returned
//   bitmap with GDI — which causes blurry thumbnails on 4K monitors.
//
//   The correct behavior is:
//     1. Detect the monitor DPI for the folder window showing this file.
//     2. Multiply `cx` by the DPI scale factor before decoding.
//     3. Return a physically-sharp bitmap at the native pixel size.
//     4. Let the DWM compositor downscale if needed — DWM uses Lanczos; GDI doesn't.
//
//   HighDpiScaleHelper provides:
//     - Scale factor calculation from DPI value
//     - Adjusted thumbnail size for a given logical cx and DPI
//     - Maximum allowed physical size (prevents excessive RAM on >300% DPI)
//
//   Integration:
//     const auto info = HighDpiScaleHelper::Compute(cx, monitorDpi);
//     // Decode at info.physicalCx × info.physicalCy
//     // Return info.physicalCx to the shell
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_HIGH_DPI_SCALE_HELPER_H
#define EXPLORERLENS_ENGINE_HIGH_DPI_SCALE_HELPER_H

#include <cstdint>
#include <algorithm>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// DpiScaleInfo — result of a DPI calculation
// ---------------------------------------------------------------------------
struct DpiScaleInfo final {
    std::uint32_t logicalCx{};   ///< Requested logical size (from Explorer)
    std::uint32_t physicalCx{};  ///< Physical pixel size to decode at
    std::uint32_t physicalCy{};  ///< Physical pixel height (square thumbnails)
    std::uint32_t monitorDpi{};  ///< Actual monitor DPI used
    float         scaleFactor{}; ///< physicalCx / logicalCx
    bool          isHighDpi{};   ///< True if scale factor > 1.0
};

// ---------------------------------------------------------------------------
// HighDpiScaleHelper
// ---------------------------------------------------------------------------
class HighDpiScaleHelper final {
public:
    HighDpiScaleHelper() = delete;

    /// Standard "100% DPI" reference value (Windows default)
    static constexpr std::uint32_t kBaseDpi = 96u;

    /// Maximum physical thumbnail size: prevents extreme memory use at >300% scale.
    /// A 256×256 logical thumb at 300% would be 768×768 = ~2.25 MP — cap at 512.
    static constexpr std::uint32_t kMaxPhysicalCx = 512u;

    /// Minimum sensible thumbnail size
    static constexpr std::uint32_t kMinPhysicalCx = 16u;

    // ------------------------------------------------------------------
    // Compute() — calculate physical decode size from logical cx + DPI.
    //   monitorDpi = 0 means "use system default (96 DPI)"
    // ------------------------------------------------------------------
    [[nodiscard]]
    static DpiScaleInfo Compute(std::uint32_t logicalCx,
                                std::uint32_t monitorDpi = 0u) noexcept
    {
        const std::uint32_t effectiveDpi =
            (monitorDpi == 0u) ? kBaseDpi : monitorDpi;

        const float scale = static_cast<float>(effectiveDpi)
                          / static_cast<float>(kBaseDpi);

        const std::uint32_t rawPhysical =
            static_cast<std::uint32_t>(static_cast<float>(logicalCx) * scale + 0.5f);

        const std::uint32_t physCx =
            (std::max)(kMinPhysicalCx,
            (std::min)(rawPhysical, kMaxPhysicalCx));

        DpiScaleInfo info{};
        info.logicalCx   = logicalCx;
        info.physicalCx  = physCx;
        info.physicalCy  = physCx;  // thumbnails are always square
        info.monitorDpi  = effectiveDpi;
        info.scaleFactor = scale;
        info.isHighDpi   = (effectiveDpi > kBaseDpi);
        return info;
    }

    // ------------------------------------------------------------------
    // ScaleFactorFromDpi() — pure conversion helper
    // ------------------------------------------------------------------
    [[nodiscard]]
    static constexpr float ScaleFactorFromDpi(std::uint32_t dpi) noexcept
    {
        return static_cast<float>(dpi) / static_cast<float>(kBaseDpi);
    }

    // ------------------------------------------------------------------
    // DpiFromScaleFactor() — e.g., 1.5f → 144 DPI
    // ------------------------------------------------------------------
    [[nodiscard]]
    static constexpr std::uint32_t DpiFromScaleFactor(float scale) noexcept
    {
        return static_cast<std::uint32_t>(static_cast<float>(kBaseDpi) * scale + 0.5f);
    }

    // ------------------------------------------------------------------
    // CommonDpiValues — lookup table for standard Windows DPI settings
    // ------------------------------------------------------------------
    static constexpr std::uint32_t kDpi100Percent = 96u;
    static constexpr std::uint32_t kDpi125Percent = 120u;
    static constexpr std::uint32_t kDpi150Percent = 144u;
    static constexpr std::uint32_t kDpi175Percent = 168u;
    static constexpr std::uint32_t kDpi200Percent = 192u;
    static constexpr std::uint32_t kDpi250Percent = 240u;
    static constexpr std::uint32_t kDpi300Percent = 288u;

    // ------------------------------------------------------------------
    // SystemDpi() — query the primary monitor DPI via Windows API
    // ------------------------------------------------------------------
    [[nodiscard]]
    static std::uint32_t SystemDpi() noexcept
    {
#ifdef _WIN32
        // GetDpiForSystem() requires Windows 10 1607+
        using FnGetDpiForSystem = UINT(WINAPI*)();
        static const auto fn = reinterpret_cast<FnGetDpiForSystem>(
            GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForSystem"));
        if (fn) return static_cast<std::uint32_t>(fn());
        // Fallback: GetDeviceCaps from the screen DC
        HDC hdc = GetDC(nullptr);
        if (hdc) {
            const int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
            ReleaseDC(nullptr, hdc);
            if (dpi > 0) return static_cast<std::uint32_t>(dpi);
        }
#endif
        return kBaseDpi;
    }
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_HIGH_DPI_SCALE_HELPER_H
