//==============================================================================
// ExplorerLens Engine — D3D11 resize pipeline scaffold (Sprint S249)
// Copyright (c) 2026 — ExplorerLens Project
// ROADMAP v6.0 §2.1 A6 / ADR-017 — Direct3D 11 replaces GDI+ for batch resize.
//==============================================================================
//
// GDI+ is single-threaded-per-image and leaves big GPUs idle. D3D11 lets us
// batch 16-64 thumbnails through a single staging texture ring, then copy
// back with a single readback. This header is the _contract_ layer:
//   * Resize options POD the caller fills in
//   * Quality knob enum (nearest / bilinear / catmull-rom / lanczos4)
//   * Result / reason enum
//   * Size + alignment limits the implementation enforces
//
// Implementation is deferred — this sprint only locks the public shape so
// other Engine modules can reference the types.
//==============================================================================
#pragma once

#include <cstdint>
#include <string_view>
#include <type_traits>

namespace ExplorerLens {
namespace Engine {

/// <summary>Resampling kernel quality.</summary>
enum class ResizeQuality : std::uint8_t
{
    NEAREST       = 0,
    BILINEAR      = 1,
    CATMULL_ROM   = 2,   // default — good balance
    LANCZOS_4     = 3,
    LANCZOS_6     = 4
};

/// <summary>Colour pipeline mode.</summary>
enum class ResizeColorMode : std::uint8_t
{
    BGRA8_SRGB     = 0,
    BGRA8_LINEAR   = 1,
    RGBA16_LINEAR  = 2,
    RGBA32F_LINEAR = 3
};

/// <summary>Single resize request — POD, cross-boundary safe.</summary>
struct D3D11ResizeRequest
{
    std::uint32_t    srcWidth   = 0;
    std::uint32_t    srcHeight  = 0;
    std::uint32_t    srcStride  = 0;  // bytes per row
    std::uint32_t    dstWidth   = 0;
    std::uint32_t    dstHeight  = 0;
    ResizeQuality    quality    = ResizeQuality::CATMULL_ROM;
    ResizeColorMode  colorMode  = ResizeColorMode::BGRA8_SRGB;
    bool             preserveAspect = true;
    bool             premultipliedAlpha = true;
};

/// <summary>Resize outcome.</summary>
enum class ResizeStatus : std::uint8_t
{
    OK              = 0,
    BAD_PARAMETER   = 1,
    OUT_OF_BUDGET   = 2,
    DEVICE_LOST     = 3,
    STAGING_FAILED  = 4,
    FALLBACK_TO_CPU = 5
};

/// <summary>Result / diagnostics record.</summary>
struct D3D11ResizeResult
{
    ResizeStatus  status = ResizeStatus::BAD_PARAMETER;
    std::uint32_t bytesUploaded = 0;
    std::uint32_t bytesReadback = 0;
    std::uint32_t gpuMicros     = 0;  // timestamp delta
    std::uint32_t cpuMicros     = 0;
};

/// <summary>Batch parameters — upper bounds on the command list.</summary>
inline constexpr std::uint32_t kResizeMaxBatch         = 64;
inline constexpr std::uint32_t kResizeMaxDim           = 16384;
inline constexpr std::uint32_t kResizeMinDim           = 1;
inline constexpr std::uint32_t kResizeStagingBytes     = 64u * 1024u * 1024u;
inline constexpr std::string_view kResizeShaderModel   = "cs_5_0";

static_assert(std::is_trivially_copyable_v<D3D11ResizeRequest>,
              "D3D11ResizeRequest must be trivially copyable");
static_assert(std::is_trivially_copyable_v<D3D11ResizeResult>,
              "D3D11ResizeResult must be trivially copyable");

} // namespace Engine
} // namespace ExplorerLens
