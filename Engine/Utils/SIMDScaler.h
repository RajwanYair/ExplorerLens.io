#pragma once

//==============================================================================
// SIMDScaler.h - SIMD-accelerated image scaling
// Copyright (c) 2026 - ExplorerLens Project
//
// High-performance image scaling using AVX2/AVX-512 intrinsics
// Provides 3-4x speedup over naive implementations
// Uses HardwareCapabilities for CPU feature detection
//==============================================================================

#include "HardwareCapabilities.h"
#include <cstdint>
#include <immintrin.h> // SIMD intrinsics
#include <memory>

namespace ExplorerLens {
namespace SIMD {

//==============================================================================
// SIMD Image Scaler
//==============================================================================

enum class ScalingQuality {
 NearestNeighbor, // Fastest, lowest quality
 Bilinear, // Fast, good quality (SIMD optimized)
 Bicubic, // Slower, best quality (SIMD optimized)
 Lanczos // Slowest, excellent quality
};

class SIMDScaler {
public:
 /// SIMD execution path
 enum class SIMDPath : uint8_t { Scalar = 0, SSE42, AVX2, AVX512, COUNT };

 static constexpr size_t PathCount() {
 return static_cast<size_t>(SIMDPath::COUNT);
 }

 static const wchar_t *PathName(SIMDPath p) {
 switch (p) {
 case SIMDPath::Scalar:
 return L"Scalar";
 case SIMDPath::SSE42:
 return L"SSE 4.2";
 case SIMDPath::AVX2:
 return L"AVX2";
 case SIMDPath::AVX512:
 return L"AVX-512";
 default:
 return L"Unknown";
 }
 }

 //==========================================================================
 // Scale BGRA image (Windows format)
 //==========================================================================

 // Scale with automatic algorithm selection based on CPU features
 static bool ScaleBGRA(const uint8_t *srcData, uint32_t srcWidth,
 uint32_t srcHeight, uint32_t srcStride,
 uint8_t *dstData, uint32_t dstWidth, uint32_t dstHeight,
 uint32_t dstStride,
 ScalingQuality quality = ScalingQuality::Bilinear);

 //==========================================================================
 // Specialized scaling algorithms
 //==========================================================================

 // Bilinear scaling with AVX2 (3-4x faster than scalar)
 static bool ScaleBGRA_Bilinear_AVX2(const uint8_t *srcData, uint32_t srcWidth,
 uint32_t srcHeight, uint32_t srcStride,
 uint8_t *dstData, uint32_t dstWidth,
 uint32_t dstHeight, uint32_t dstStride);

 // Bilinear scaling with SSE4.1
 static bool ScaleBGRA_Bilinear_SSE41(const uint8_t *srcData,
 uint32_t srcWidth, uint32_t srcHeight,
 uint32_t srcStride, uint8_t *dstData,
 uint32_t dstWidth, uint32_t dstHeight,
 uint32_t dstStride);

 // Fallback scalar implementation
 static bool ScaleBGRA_Bilinear_Scalar(const uint8_t *srcData,
 uint32_t srcWidth, uint32_t srcHeight,
 uint32_t srcStride, uint8_t *dstData,
 uint32_t dstWidth, uint32_t dstHeight,
 uint32_t dstStride);

 // Bicubic scaling (higher quality, slower)
 static bool ScaleBGRA_Bicubic_AVX2(const uint8_t *srcData, uint32_t srcWidth,
 uint32_t srcHeight, uint32_t srcStride,
 uint8_t *dstData, uint32_t dstWidth,
 uint32_t dstHeight, uint32_t dstStride);

 static bool ScaleBGRA_Bicubic_Scalar(const uint8_t *srcData,
 uint32_t srcWidth, uint32_t srcHeight,
 uint32_t srcStride, uint8_t *dstData,
 uint32_t dstWidth, uint32_t dstHeight,
 uint32_t dstStride);

 // Nearest neighbor scaling (fastest, suitable for pixel art / icons)
 static bool ScaleBGRA_NearestNeighbor(const uint8_t *srcData,
 uint32_t srcWidth, uint32_t srcHeight,
 uint32_t srcStride, uint8_t *dstData,
 uint32_t dstWidth, uint32_t dstHeight,
 uint32_t dstStride);

 // Lanczos3 scaling (highest quality, 6-tap windowed sinc filter)
 static bool ScaleBGRA_Lanczos3_Scalar(const uint8_t *srcData,
 uint32_t srcWidth, uint32_t srcHeight,
 uint32_t srcStride, uint8_t *dstData,
 uint32_t dstWidth, uint32_t dstHeight,
 uint32_t dstStride);

 //==========================================================================
 // Box Filter / Area Average
 // Best for downscaling — averages all source pixels contributing to each
 // destination pixel. Prevents aliasing that bilinear produces on high
 // reduction ratios (e.g., 4000x3000 → 256x192).
 //==========================================================================

 /// SSE4.1 accelerated box-filter downscaler
 static bool ScaleBGRA_BoxFilter_SSE41(const uint8_t *srcData,
 uint32_t srcWidth, uint32_t srcHeight,
 uint32_t srcStride, uint8_t *dstData,
 uint32_t dstWidth, uint32_t dstHeight,
 uint32_t dstStride);

 /// Scalar fallback for box-filter downscaler
 static bool ScaleBGRA_BoxFilter_Scalar(const uint8_t *srcData,
 uint32_t srcWidth, uint32_t srcHeight,
 uint32_t srcStride, uint8_t *dstData,
 uint32_t dstWidth, uint32_t dstHeight,
 uint32_t dstStride);

 /// Auto-select best box filter implementation
 static bool ScaleBGRA_BoxFilter(const uint8_t *srcData, uint32_t srcWidth,
 uint32_t srcHeight, uint32_t srcStride,
 uint8_t *dstData, uint32_t dstWidth,
 uint32_t dstHeight, uint32_t dstStride);

 //==========================================================================
 // SIMD Color Conversion (RGB <-> BGR, RGBA <-> BGRA)
 //==========================================================================

 // Convert RGB to BGR (swap R and B channels) - AVX2 accelerated
 static void ConvertRGBtoBGR_AVX2(const uint8_t *rgb, uint8_t *bgr,
 uint32_t pixelCount);

 // Convert RGBA to BGRA (swap R and B channels) - AVX2 accelerated
 static void ConvertRGBAtoBGRA_AVX2(const uint8_t *rgba, uint8_t *bgra,
 uint32_t pixelCount);

 // Scalar fallback versions
 static void ConvertRGBtoBGR_Scalar(const uint8_t *rgb, uint8_t *bgr,
 uint32_t pixelCount);

 static void ConvertRGBAtoBGRA_Scalar(const uint8_t *rgba, uint8_t *bgra,
 uint32_t pixelCount);

 //==========================================================================
 // Utility functions
 //==========================================================================

 // Calculate optimal thumbnail size preserving aspect ratio
 static void CalculateScaledSize(uint32_t srcWidth, uint32_t srcHeight,
 uint32_t maxSize, uint32_t &outWidth,
 uint32_t &outHeight);

 // Check if dimensions are valid
 static bool ValidateDimensions(uint32_t srcWidth, uint32_t srcHeight,
 uint32_t dstWidth, uint32_t dstHeight);

private:
 // Helper: Bilinear interpolation using AVX2 (processes 8 pixels)
 static __m256i BilinearInterpolate_AVX2(__m256i p00, __m256i p01, __m256i p10,
 __m256i p11, __m256i fracX,
 __m256i fracY);

 // Helper: Convert BGRA to planar for SIMD processing
 static void DeinterleaveBGRA_AVX2(const uint8_t *bgra, uint8_t *b, uint8_t *g,
 uint8_t *r, uint8_t *a, uint32_t count);

 // Helper: Convert planar to BGRA
 static void InterleaveBGRA_AVX2(const uint8_t *b, const uint8_t *g,
 const uint8_t *r, const uint8_t *a,
 uint8_t *bgra, uint32_t count);
};

//==============================================================================
// Inline implementations
//==============================================================================

inline void SIMDScaler::CalculateScaledSize(uint32_t srcWidth,
 uint32_t srcHeight,
 uint32_t maxSize,
 uint32_t &outWidth,
 uint32_t &outHeight) {
 if (srcWidth <= maxSize && srcHeight <= maxSize) {
 outWidth = srcWidth;
 outHeight = srcHeight;
 return;
 }

 const float aspectRatio =
 static_cast<float>(srcWidth) / static_cast<float>(srcHeight);

 if (srcWidth > srcHeight) {
 outWidth = maxSize;
 outHeight = static_cast<uint32_t>(maxSize / aspectRatio);
 } else {
 outHeight = maxSize;
 outWidth = static_cast<uint32_t>(maxSize * aspectRatio);
 }

 // Ensure minimum size
 if (outWidth == 0)
 outWidth = 1;
 if (outHeight == 0)
 outHeight = 1;
}

inline bool SIMDScaler::ValidateDimensions(uint32_t srcWidth,
 uint32_t srcHeight,
 uint32_t dstWidth,
 uint32_t dstHeight) {
 return (srcWidth > 0 && srcHeight > 0 && dstWidth > 0 && dstHeight > 0 &&
 srcWidth <= 65536 && srcHeight <= 65536 && dstWidth <= 65536 &&
 dstHeight <= 65536);
}

} // namespace SIMD
} // namespace ExplorerLens

// Expose to ExplorerLens::Engine namespace for test compatibility
namespace ExplorerLens {
namespace Engine {
using ::ExplorerLens::SIMD::ScalingQuality;
using ::ExplorerLens::SIMD::SIMDScaler;
using SIMDPath = SIMDScaler::SIMDPath;
} // namespace Engine
} // namespace ExplorerLens
