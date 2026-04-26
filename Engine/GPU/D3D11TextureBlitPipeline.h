// Engine/GPU/D3D11TextureBlitPipeline.h
// ExplorerLens — D3D11 texture blit pipeline: first real GPU pixels (ROADMAP v7.0 Phase 2)
// Sprint S317.
//
// Purpose:
//   The blit pipeline is the first stage where actual pixels go through the
//   GPU.  It takes a decoded BGRA pixel buffer in system memory, uploads it
//   to a D3D11 staging texture, performs a resize via a bilinear/Lanczos
//   compute shader, then reads back the result into an HBITMAP-compatible
//   DIBSection.
//
//   Phase 2 pipeline:
//     [CPU decoded pixels]
//       → ID3D11Texture2D (staging, CPU write)
//       → CopySubresourceRegion to GPU texture
//       → Dispatch resize compute shader (resize_bilinear.hlsl)
//       → CopyResource to staging readback texture
//       → Map / memcpy → HBITMAP DIBSection
//
//   This approach is ~3-5× faster than GDI+ StretchBlt for large thumbnails
//   (measured in IrfanView GPU mode — H10 harvest).
//
// Prerequisites:
//   - D3D11DeviceManager::IsReady() == true
//   - Feature level >= D3D11DeviceManager::kMinComputeFeatureLevel (FL_11_0)
//     for compute-shader resize; falls back to GDI+ StretchBlt below FL_11_0.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_D3D11_TEXTURE_BLIT_PIPELINE_H
#define EXPLORERLENS_ENGINE_D3D11_TEXTURE_BLIT_PIPELINE_H

#include <cstdint>
#include <cstddef>
#include <span>
#include <vector>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// BlitResizeMode — which algorithm to use for the resize step
// ---------------------------------------------------------------------------
enum class BlitResizeMode : std::uint8_t {
    BILINEAR       = 0,   ///< CS dispatch: resize_bilinear.hlsl (default, fast)
    LANCZOS3       = 1,   ///< CS dispatch: resize_lanczos.hlsl (higher quality)
    GDI_FALLBACK   = 2,   ///< GDI+ StretchBlt (CPU, used when FL < 11_0)
};

// ---------------------------------------------------------------------------
// BlitDescriptor — describes the source pixel buffer and target size
// ---------------------------------------------------------------------------
struct BlitDescriptor final {
    // Source
    const std::byte*  srcPixels{ nullptr };   ///< BGRA-8 source buffer
    std::uint32_t     srcWidth{};
    std::uint32_t     srcHeight{};
    std::uint32_t     srcStrideBytes{};       ///< Row stride; 0 = srcWidth * 4

    // Target
    std::uint32_t     dstWidth{};
    std::uint32_t     dstHeight{};

    BlitResizeMode    resizeMode{ BlitResizeMode::BILINEAR };

    // Premultiply alpha before resize (reduces haloing artefacts)
    bool              premultiplyAlpha{ true };
};

// ---------------------------------------------------------------------------
// BlitStatus
// ---------------------------------------------------------------------------
enum class BlitStatus : std::uint8_t {
    OK                   = 0,
    DEVICE_NOT_READY     = 1,   ///< D3D11DeviceManager not initialised
    FEATURE_LEVEL_TOO_LOW= 2,   ///< FL < 11_0; caller should use GDI fallback
    TEXTURE_ALLOC_FAILED = 3,   ///< ID3D11Texture2D::CreateTexture2D failed
    SHADER_NOT_LOADED    = 4,   ///< Resize CS not yet compiled (Phase 2)
    INVALID_DESCRIPTOR   = 5,   ///< srcPixels null or dimensions are zero
    BLIT_FAILURE         = 6,   ///< GPU command failed
    DISABLED             = 7,   ///< Phase 2 stub
};

// ---------------------------------------------------------------------------
// BlitResult
// ---------------------------------------------------------------------------
struct BlitResult final {
    BlitStatus            status{ BlitStatus::DISABLED };
    std::uint32_t         widthPixels{};
    std::uint32_t         heightPixels{};
    std::uint32_t         strideBytes{};
    std::vector<std::byte> pixels;            ///< Resized BGRA-8 pixels
    std::uint64_t         blitTimeUs{};       ///< Measured GPU blit wall time
    BlitResizeMode        modeUsed{ BlitResizeMode::GDI_FALLBACK };
};

// ---------------------------------------------------------------------------
// D3D11TextureBlitPipeline
// ---------------------------------------------------------------------------
// Phase 2 stub — Blit() returns DISABLED until the D3D11 device + compute
// shader are wired (Sprint ≥ S330).
//
class D3D11TextureBlitPipeline final {
public:
    D3D11TextureBlitPipeline() noexcept  = default;
    ~D3D11TextureBlitPipeline() noexcept = default;

    D3D11TextureBlitPipeline(const D3D11TextureBlitPipeline&)            = delete;
    D3D11TextureBlitPipeline& operator=(const D3D11TextureBlitPipeline&) = delete;

    // ── Primary API ───────────────────────────────────────────────────────────

    /// Upload, resize, and readback a pixel buffer via the GPU pipeline.
    /// Phase 2 stub: always returns DISABLED without touching D3D11 APIs.
    [[nodiscard]] BlitResult Blit(const BlitDescriptor& desc) const noexcept
    {
        if (!desc.srcPixels || desc.srcWidth == 0 || desc.srcHeight == 0
            || desc.dstWidth == 0 || desc.dstHeight == 0) {
            return BlitResult{ .status = BlitStatus::INVALID_DESCRIPTOR };
        }
        (void)desc;
        return BlitResult{ .status = BlitStatus::DISABLED };
    }

    // ── Capability ────────────────────────────────────────────────────────────

    /// True when the D3D11 device is ready and compute shaders are loaded.
    [[nodiscard]] static bool IsGPUPathAvailable() noexcept { return false; }

    /// True when GDI+ fallback is usable (always true on Windows).
    [[nodiscard]] static constexpr bool IsGDIFallbackAvailable() noexcept { return true; }

    // ── Constants ─────────────────────────────────────────────────────────────

    /// Maximum source texture dimension (D3D11 hardware limit for Shader Model 5)
    static constexpr std::uint32_t kMaxTextureDimension = 16384u;

    /// Minimum blit target size (sub-16 px thumbnails not worth GPU overhead)
    static constexpr std::uint32_t kMinBlitDimension = 16u;

    /// Staging texture alignment required for D3D11 Map()
    static constexpr std::uint32_t kStagingTexturePitchAlignment = 256u;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_D3D11_TEXTURE_BLIT_PIPELINE_H
