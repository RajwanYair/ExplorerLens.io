//==============================================================================
// DarkThumbs Engine — Sprint 264: SIMD Acceleration
// Activate SIMDAccelerator/SIMDScaler for resize operations.
// AVX2 fast paths with SSE4.1 fallback and runtime detection.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// SIMD instruction set levels
enum class SIMDLevel : uint8_t {
    None    = 0,    // Scalar fallback
    SSE2    = 1,    // Baseline x64
    SSE41   = 2,    // SSE4.1 — blend, round, extract
    AVX     = 3,    // AVX — 256-bit float ops
    AVX2    = 4,    // AVX2 — 256-bit integer ops
    AVX512  = 5,    // AVX-512 — 512-bit ops
    NEON    = 6     // ARM NEON — ARM64 SIMD
};

/// SIMD-accelerated operations
enum class SIMDOperation : uint8_t {
    BilinearResize,     // Image resize (bilinear interpolation)
    LanczosResize,      // Image resize (Lanczos-3)
    BoxResize,          // Image resize (box/average filter)
    AlphaBlend,         // Alpha compositing
    ColorConvert,       // Color space conversion
    GammaCorrect,       // Gamma correction (sRGB ↔ linear)
    Sharpen,            // Unsharp mask post-resize
    Dither,             // Floyd-Steinberg dithering
    COUNT
};

/// SIMD capability detection results
struct SIMDCapabilities {
    SIMDLevel maxLevel          = SIMDLevel::SSE2;
    bool      hasSSE2           = false;
    bool      hasSSE41          = false;
    bool      hasAVX            = false;
    bool      hasAVX2           = false;
    bool      hasAVX512         = false;
    bool      hasNEON           = false;
    bool      hasFMA            = false;    // Fused multiply-add
    uint32_t  cacheLineSize     = 64;
    uint32_t  l1CacheSize       = 32768;    // 32 KB typical
};

/// SIMD acceleration configuration
struct SIMDConfig {
    SIMDLevel preferredLevel    = SIMDLevel::AVX2;
    bool      enableRuntimeDetect = true;
    bool      enableFallback    = true;
    uint32_t  minPixelsForSIMD  = 64;   // Below this, scalar is faster
    uint32_t  alignmentBytes    = 32;   // Memory alignment for AVX2
};

/// SIMD acceleration manager
class SIMDAccelerationManager {
public:
    /// SIMD level name
    static const wchar_t* LevelName(SIMDLevel level) {
        switch (level) {
            case SIMDLevel::None:   return L"Scalar";
            case SIMDLevel::SSE2:   return L"SSE2";
            case SIMDLevel::SSE41:  return L"SSE4.1";
            case SIMDLevel::AVX:    return L"AVX";
            case SIMDLevel::AVX2:   return L"AVX2";
            case SIMDLevel::AVX512: return L"AVX-512";
            case SIMDLevel::NEON:   return L"NEON";
            default: return L"Unknown";
        }
    }

    /// Operation name
    static const wchar_t* OperationName(SIMDOperation op) {
        switch (op) {
            case SIMDOperation::BilinearResize: return L"Bilinear Resize";
            case SIMDOperation::LanczosResize:  return L"Lanczos Resize";
            case SIMDOperation::BoxResize:      return L"Box Resize";
            case SIMDOperation::AlphaBlend:     return L"Alpha Blend";
            case SIMDOperation::ColorConvert:   return L"Color Convert";
            case SIMDOperation::GammaCorrect:   return L"Gamma Correct";
            case SIMDOperation::Sharpen:        return L"Sharpen";
            case SIMDOperation::Dither:         return L"Dither";
            default: return L"Unknown";
        }
    }

    /// Select best SIMD level for given capability set
    static SIMDLevel SelectLevel(const SIMDCapabilities& caps, const SIMDConfig& cfg) {
        SIMDLevel best = SIMDLevel::None;
        if (caps.hasSSE2)   best = SIMDLevel::SSE2;
        if (caps.hasSSE41)  best = SIMDLevel::SSE41;
        if (caps.hasAVX)    best = SIMDLevel::AVX;
        if (caps.hasAVX2)   best = SIMDLevel::AVX2;
        if (caps.hasAVX512) best = SIMDLevel::AVX512;
        if (caps.hasNEON)   best = SIMDLevel::NEON;
        // Clamp to preferred level
        if (static_cast<uint8_t>(best) > static_cast<uint8_t>(cfg.preferredLevel))
            best = cfg.preferredLevel;
        return best;
    }

    /// Speedup estimate for given SIMD level (relative to scalar)
    static float SpeedupEstimate(SIMDLevel level) {
        switch (level) {
            case SIMDLevel::SSE2:   return 2.0f;
            case SIMDLevel::SSE41:  return 2.5f;
            case SIMDLevel::AVX:    return 3.5f;
            case SIMDLevel::AVX2:   return 4.0f;
            case SIMDLevel::AVX512: return 6.0f;
            case SIMDLevel::NEON:   return 3.0f;
            default: return 1.0f;
        }
    }

    /// Level count
    static constexpr size_t LevelCount() { return 7; }

    /// Operation count
    static constexpr size_t OperationCount() { return static_cast<size_t>(SIMDOperation::COUNT); }
};

}} // namespace DarkThumbs::Engine
