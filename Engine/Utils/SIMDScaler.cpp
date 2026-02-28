//==============================================================================
// SIMDScaler.cpp - SIMD-accelerated image scaling implementation
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#include "SIMDScaler.h"
#include "HardwareCapabilities.h"
#include <algorithm>
#include <cstring>
#include <cmath>
#include <intrin.h>

using namespace ExplorerLens::Engine;

namespace ExplorerLens {
namespace SIMD {

//==============================================================================
// Main Scaling Function
//==============================================================================

bool SIMDScaler::ScaleBGRA(
 const uint8_t* srcData,
 uint32_t srcWidth,
 uint32_t srcHeight,
 uint32_t srcStride,
 uint8_t* dstData,
 uint32_t dstWidth,
 uint32_t dstHeight,
 uint32_t dstStride,
 ScalingQuality quality
) {
 if (!ValidateDimensions(srcWidth, srcHeight, dstWidth, dstHeight)) {
 return false;
 }

 if (!srcData || !dstData) {
 return false;
 }

 // Select implementation based on quality and CPU features
 const auto& hwCaps = HardwareCapabilities::Get();
 const auto& cpu = hwCaps.GetCPU();
 
 switch (quality) {
 case ScalingQuality::NearestNeighbor:
 // Nearest neighbor (very fast, pixelated)
 return ScaleBGRA_NearestNeighbor(
 srcData, srcWidth, srcHeight, srcStride,
 dstData, dstWidth, dstHeight, dstStride
 );
 
 case ScalingQuality::Bicubic:
 // Bicubic scaling (higher quality, sharp edges)
 if (cpu.hasAVX2) {
 return ScaleBGRA_Bicubic_AVX2(
 srcData, srcWidth, srcHeight, srcStride,
 dstData, dstWidth, dstHeight, dstStride
 );
 } else {
 return ScaleBGRA_Bicubic_Scalar(
 srcData, srcWidth, srcHeight, srcStride,
 dstData, dstWidth, dstHeight, dstStride
 );
 }
 
 case ScalingQuality::Lanczos:
 // Lanczos3 scaling (best quality, minimal ringing)
 return ScaleBGRA_Lanczos3_Scalar(
 srcData, srcWidth, srcHeight, srcStride,
 dstData, dstWidth, dstHeight, dstStride
 );
 
 case ScalingQuality::Bilinear:
 default:
 // Bilinear scaling (fast, good quality, balanced)
 if (cpu.hasAVX2) {
 return ScaleBGRA_Bilinear_AVX2(
 srcData, srcWidth, srcHeight, srcStride,
 dstData, dstWidth, dstHeight, dstStride
 );
 } else if (cpu.hasSSE41) {
 return ScaleBGRA_Bilinear_SSE41(
 srcData, srcWidth, srcHeight, srcStride,
 dstData, dstWidth, dstHeight, dstStride
 );
 } else {
 return ScaleBGRA_Bilinear_Scalar(
 srcData, srcWidth, srcHeight, srcStride,
 dstData, dstWidth, dstHeight, dstStride
 );
 }
 }
}

//==============================================================================
// AVX2 Bilinear Scaling
//==============================================================================

bool SIMDScaler::ScaleBGRA_Bilinear_AVX2(
 const uint8_t* srcData,
 uint32_t srcWidth,
 uint32_t srcHeight,
 uint32_t srcStride,
 uint8_t* dstData,
 uint32_t dstWidth,
 uint32_t dstHeight,
 uint32_t dstStride
) {
 const float xRatio = static_cast<float>(srcWidth - 1) / static_cast<float>(dstWidth);
 const float yRatio = static_cast<float>(srcHeight - 1) / static_cast<float>(dstHeight);

 for (uint32_t y = 0; y < dstHeight; ++y) {
 const float srcY = y * yRatio;
 const uint32_t y1 = static_cast<uint32_t>(srcY);
 const uint32_t y2 = std::min(y1 + 1, srcHeight - 1);
 const float yFrac = srcY - y1;
 const uint32_t yFracInt = static_cast<uint32_t>(yFrac * 256.0f);

 const uint8_t* srcRow1 = srcData + y1 * srcStride;
 const uint8_t* srcRow2 = srcData + y2 * srcStride;
 uint8_t* dstRow = dstData + y * dstStride;

 uint32_t x = 0;
 
 // Process 8 pixels at a time with AVX2
 const uint32_t simdWidth = 8;
 const uint32_t simdCount = (dstWidth / simdWidth) * simdWidth;

 for (; x < simdCount; x += simdWidth) {
 // Calculate source X positions for 8 destination pixels
 __m256 srcXVec = _mm256_set_ps(
 (x + 7) * xRatio, (x + 6) * xRatio, (x + 5) * xRatio, (x + 4) * xRatio,
 (x + 3) * xRatio, (x + 2) * xRatio, (x + 1) * xRatio, x * xRatio
 );

 // Convert to integer positions
 __m256i x1Vec = _mm256_cvttps_epi32(srcXVec);
 
 // For simplicity in this initial implementation, process pixels individually
 // Full SIMD gather operations would require more complex code
 alignas(32) int32_t x1Array[8];
 _mm256_store_si256((__m256i*)x1Array, x1Vec);

 for (int i = 0; i < 8; ++i) {
 const uint32_t x1 = x1Array[i];
 const uint32_t x2 = std::min(x1 + 1, srcWidth - 1);
 const float xFrac = (x + i) * xRatio - x1;
 const uint32_t xFracInt = static_cast<uint32_t>(xFrac * 256.0f);

 // Load 4 pixels (top-left, top-right, bottom-left, bottom-right)
 const uint32_t* p00 = reinterpret_cast<const uint32_t*>(srcRow1 + x1 * 4);
 const uint32_t* p01 = reinterpret_cast<const uint32_t*>(srcRow1 + x2 * 4);
 const uint32_t* p10 = reinterpret_cast<const uint32_t*>(srcRow2 + x1 * 4);
 const uint32_t* p11 = reinterpret_cast<const uint32_t*>(srcRow2 + x2 * 4);

 // Bilinear interpolation for each channel
 const uint32_t b00 = (*p00 >> 0) & 0xFF;
 const uint32_t g00 = (*p00 >> 8) & 0xFF;
 const uint32_t r00 = (*p00 >> 16) & 0xFF;
 const uint32_t a00 = (*p00 >> 24) & 0xFF;

 const uint32_t b01 = (*p01 >> 0) & 0xFF;
 const uint32_t g01 = (*p01 >> 8) & 0xFF;
 const uint32_t r01 = (*p01 >> 16) & 0xFF;
 const uint32_t a01 = (*p01 >> 24) & 0xFF;

 const uint32_t b10 = (*p10 >> 0) & 0xFF;
 const uint32_t g10 = (*p10 >> 8) & 0xFF;
 const uint32_t r10 = (*p10 >> 16) & 0xFF;
 const uint32_t a10 = (*p10 >> 24) & 0xFF;

 const uint32_t b11 = (*p11 >> 0) & 0xFF;
 const uint32_t g11 = (*p11 >> 8) & 0xFF;
 const uint32_t r11 = (*p11 >> 16) & 0xFF;
 const uint32_t a11 = (*p11 >> 24) & 0xFF;

 // Interpolate top row
 const uint32_t b0 = ((b00 * (256 - xFracInt)) + (b01 * xFracInt)) >> 8;
 const uint32_t g0 = ((g00 * (256 - xFracInt)) + (g01 * xFracInt)) >> 8;
 const uint32_t r0 = ((r00 * (256 - xFracInt)) + (r01 * xFracInt)) >> 8;
 const uint32_t a0 = ((a00 * (256 - xFracInt)) + (a01 * xFracInt)) >> 8;

 // Interpolate bottom row
 const uint32_t b1 = ((b10 * (256 - xFracInt)) + (b11 * xFracInt)) >> 8;
 const uint32_t g1 = ((g10 * (256 - xFracInt)) + (g11 * xFracInt)) >> 8;
 const uint32_t r1 = ((r10 * (256 - xFracInt)) + (r11 * xFracInt)) >> 8;
 const uint32_t a1 = ((a10 * (256 - xFracInt)) + (a11 * xFracInt)) >> 8;

 // Final vertical interpolation
 const uint32_t b = ((b0 * (256 - yFracInt)) + (b1 * yFracInt)) >> 8;
 const uint32_t g = ((g0 * (256 - yFracInt)) + (g1 * yFracInt)) >> 8;
 const uint32_t r = ((r0 * (256 - yFracInt)) + (r1 * yFracInt)) >> 8;
 const uint32_t a = ((a0 * (256 - yFracInt)) + (a1 * yFracInt)) >> 8;

 // Store result
 uint32_t* dst = reinterpret_cast<uint32_t*>(dstRow + (x + i) * 4);
 *dst = (a << 24) | (r << 16) | (g << 8) | b;
 }
 }

 // Handle remaining pixels
 for (; x < dstWidth; ++x) {
 const float srcX = x * xRatio;
 const uint32_t x1 = static_cast<uint32_t>(srcX);
 const uint32_t x2 = std::min(x1 + 1, srcWidth - 1);
 const float xFrac = srcX - x1;
 const uint32_t xFracInt = static_cast<uint32_t>(xFrac * 256.0f);

 const uint32_t* p00 = reinterpret_cast<const uint32_t*>(srcRow1 + x1 * 4);
 const uint32_t* p01 = reinterpret_cast<const uint32_t*>(srcRow1 + x2 * 4);
 const uint32_t* p10 = reinterpret_cast<const uint32_t*>(srcRow2 + x1 * 4);
 const uint32_t* p11 = reinterpret_cast<const uint32_t*>(srcRow2 + x2 * 4);

 const uint32_t b00 = (*p00 >> 0) & 0xFF;
 const uint32_t g00 = (*p00 >> 8) & 0xFF;
 const uint32_t r00 = (*p00 >> 16) & 0xFF;
 const uint32_t a00 = (*p00 >> 24) & 0xFF;

 const uint32_t b01 = (*p01 >> 0) & 0xFF;
 const uint32_t g01 = (*p01 >> 8) & 0xFF;
 const uint32_t r01 = (*p01 >> 16) & 0xFF;
 const uint32_t a01 = (*p01 >> 24) & 0xFF;

 const uint32_t b10 = (*p10 >> 0) & 0xFF;
 const uint32_t g10 = (*p10 >> 8) & 0xFF;
 const uint32_t r10 = (*p10 >> 16) & 0xFF;
 const uint32_t a10 = (*p10 >> 24) & 0xFF;

 const uint32_t b11 = (*p11 >> 0) & 0xFF;
 const uint32_t g11 = (*p11 >> 8) & 0xFF;
 const uint32_t r11 = (*p11 >> 16) & 0xFF;
 const uint32_t a11 = (*p11 >> 24) & 0xFF;

 const uint32_t b0 = ((b00 * (256 - xFracInt)) + (b01 * xFracInt)) >> 8;
 const uint32_t g0 = ((g00 * (256 - xFracInt)) + (g01 * xFracInt)) >> 8;
 const uint32_t r0 = ((r00 * (256 - xFracInt)) + (r01 * xFracInt)) >> 8;
 const uint32_t a0 = ((a00 * (256 - xFracInt)) + (a01 * xFracInt)) >> 8;

 const uint32_t b1 = ((b10 * (256 - xFracInt)) + (b11 * xFracInt)) >> 8;
 const uint32_t g1 = ((g10 * (256 - xFracInt)) + (g11 * xFracInt)) >> 8;
 const uint32_t r1 = ((r10 * (256 - xFracInt)) + (r11 * xFracInt)) >> 8;
 const uint32_t a1 = ((a10 * (256 - xFracInt)) + (a11 * xFracInt)) >> 8;

 const uint32_t b = ((b0 * (256 - yFracInt)) + (b1 * yFracInt)) >> 8;
 const uint32_t g = ((g0 * (256 - yFracInt)) + (g1 * yFracInt)) >> 8;
 const uint32_t r = ((r0 * (256 - yFracInt)) + (r1 * yFracInt)) >> 8;
 const uint32_t a = ((a0 * (256 - yFracInt)) + (a1 * yFracInt)) >> 8;

 uint32_t* dst = reinterpret_cast<uint32_t*>(dstRow + x * 4);
 *dst = (a << 24) | (r << 16) | (g << 8) | b;
 }
 }

 return true;
}

//==============================================================================
// SSE4.1 Bilinear Scaling (for older CPUs)
//==============================================================================

bool SIMDScaler::ScaleBGRA_Bilinear_SSE41(
 const uint8_t* srcData,
 uint32_t srcWidth,
 uint32_t srcHeight,
 uint32_t srcStride,
 uint8_t* dstData,
 uint32_t dstWidth,
 uint32_t dstHeight,
 uint32_t dstStride
) {
 // For now, fall back to scalar implementation
 // A full SSE4.1 implementation would be similar to AVX2 but with 128-bit registers
 return ScaleBGRA_Bilinear_Scalar(
 srcData, srcWidth, srcHeight, srcStride,
 dstData, dstWidth, dstHeight, dstStride
 );
}

//==============================================================================
// Scalar Bilinear Scaling (Fallback)
//==============================================================================

bool SIMDScaler::ScaleBGRA_Bilinear_Scalar(
 const uint8_t* srcData,
 uint32_t srcWidth,
 uint32_t srcHeight,
 uint32_t srcStride,
 uint8_t* dstData,
 uint32_t dstWidth,
 uint32_t dstHeight,
 uint32_t dstStride
) {
 const float xRatio = static_cast<float>(srcWidth - 1) / static_cast<float>(dstWidth);
 const float yRatio = static_cast<float>(srcHeight - 1) / static_cast<float>(dstHeight);

 for (uint32_t y = 0; y < dstHeight; ++y) {
 const float srcY = y * yRatio;
 const uint32_t y1 = static_cast<uint32_t>(srcY);
 const uint32_t y2 = std::min(y1 + 1, srcHeight - 1);
 const float yFrac = srcY - y1;
 const uint32_t yFracInt = static_cast<uint32_t>(yFrac * 256.0f);

 const uint8_t* srcRow1 = srcData + y1 * srcStride;
 const uint8_t* srcRow2 = srcData + y2 * srcStride;
 uint8_t* dstRow = dstData + y * dstStride;

 for (uint32_t x = 0; x < dstWidth; ++x) {
 const float srcX = x * xRatio;
 const uint32_t x1 = static_cast<uint32_t>(srcX);
 const uint32_t x2 = std::min(x1 + 1, srcWidth - 1);
 const float xFrac = srcX - x1;
 const uint32_t xFracInt = static_cast<uint32_t>(xFrac * 256.0f);

 const uint32_t* p00 = reinterpret_cast<const uint32_t*>(srcRow1 + x1 * 4);
 const uint32_t* p01 = reinterpret_cast<const uint32_t*>(srcRow1 + x2 * 4);
 const uint32_t* p10 = reinterpret_cast<const uint32_t*>(srcRow2 + x1 * 4);
 const uint32_t* p11 = reinterpret_cast<const uint32_t*>(srcRow2 + x2 * 4);

 const uint32_t b00 = (*p00 >> 0) & 0xFF;
 const uint32_t g00 = (*p00 >> 8) & 0xFF;
 const uint32_t r00 = (*p00 >> 16) & 0xFF;
 const uint32_t a00 = (*p00 >> 24) & 0xFF;

 const uint32_t b01 = (*p01 >> 0) & 0xFF;
 const uint32_t g01 = (*p01 >> 8) & 0xFF;
 const uint32_t r01 = (*p01 >> 16) & 0xFF;
 const uint32_t a01 = (*p01 >> 24) & 0xFF;

 const uint32_t b10 = (*p10 >> 0) & 0xFF;
 const uint32_t g10 = (*p10 >> 8) & 0xFF;
 const uint32_t r10 = (*p10 >> 16) & 0xFF;
 const uint32_t a10 = (*p10 >> 24) & 0xFF;

 const uint32_t b11 = (*p11 >> 0) & 0xFF;
 const uint32_t g11 = (*p11 >> 8) & 0xFF;
 const uint32_t r11 = (*p11 >> 16) & 0xFF;
 const uint32_t a11 = (*p11 >> 24) & 0xFF;

 const uint32_t b0 = ((b00 * (256 - xFracInt)) + (b01 * xFracInt)) >> 8;
 const uint32_t g0 = ((g00 * (256 - xFracInt)) + (g01 * xFracInt)) >> 8;
 const uint32_t r0 = ((r00 * (256 - xFracInt)) + (r01 * xFracInt)) >> 8;
 const uint32_t a0 = ((a00 * (256 - xFracInt)) + (a01 * xFracInt)) >> 8;

 const uint32_t b1 = ((b10 * (256 - xFracInt)) + (b11 * xFracInt)) >> 8;
 const uint32_t g1 = ((g10 * (256 - xFracInt)) + (g11 * xFracInt)) >> 8;
 const uint32_t r1 = ((r10 * (256 - xFracInt)) + (r11 * xFracInt)) >> 8;
 const uint32_t a1 = ((a10 * (256 - xFracInt)) + (a11 * xFracInt)) >> 8;

 const uint32_t b = ((b0 * (256 - yFracInt)) + (b1 * yFracInt)) >> 8;
 const uint32_t g = ((g0 * (256 - yFracInt)) + (g1 * yFracInt)) >> 8;
 const uint32_t r = ((r0 * (256 - yFracInt)) + (r1 * yFracInt)) >> 8;
 const uint32_t a = ((a0 * (256 - yFracInt)) + (a1 * yFracInt)) >> 8;

 uint32_t* dst = reinterpret_cast<uint32_t*>(dstRow + x * 4);
 *dst = (a << 24) | (r << 16) | (g << 8) | b;
 }
 }

 return true;
}

//==============================================================================
// Bicubic Scaling Implementations
//==============================================================================

// Bicubic interpolation kernel (Mitchell-Netravali filter with B=1/3, C=1/3)
static inline float CubicKernel(float x) {
 x = (x < 0.0f) ? -x : x;
 float x2 = x * x;
 float x3 = x2 * x;
 
 if (x < 1.0f) {
 return (1.5f * x3 - 2.5f * x2 + 1.0f);
 } else if (x < 2.0f) {
 return (-0.5f * x3 + 2.5f * x2 - 4.0f * x + 2.0f);
 }
 return 0.0f;
}

bool SIMDScaler::ScaleBGRA_Bicubic_Scalar(
 const uint8_t* srcData,
 uint32_t srcWidth,
 uint32_t srcHeight,
 uint32_t srcStride,
 uint8_t* dstData,
 uint32_t dstWidth,
 uint32_t dstHeight,
 uint32_t dstStride
) {
 const float xRatio = static_cast<float>(srcWidth) / static_cast<float>(dstWidth);
 const float yRatio = static_cast<float>(srcHeight) / static_cast<float>(dstHeight);

 for (uint32_t dy = 0; dy < dstHeight; ++dy) {
 const float srcY = (dy + 0.5f) * yRatio - 0.5f;
 const int32_t y0 = static_cast<int32_t>(std::floor(srcY));
 
 uint8_t* dstRow = dstData + dy * dstStride;

 for (uint32_t dx = 0; dx < dstWidth; ++dx) {
 const float srcX = (dx + 0.5f) * xRatio - 0.5f;
 const int32_t x0 = static_cast<int32_t>(std::floor(srcX));
 
 float b = 0, g = 0, r = 0, a = 0;
 float weightSum = 0.0f;
 
 // 4x4 bicubic kernel
 for (int32_t ky = -1; ky <= 2; ++ky) {
 int32_t sy = y0 + ky;
 if (sy < 0) sy = 0;
 if (sy >= static_cast<int32_t>(srcHeight)) sy = srcHeight - 1;
 
 const float wy = CubicKernel(srcY - (y0 + ky));
 const uint8_t* srcRow = srcData + sy * srcStride;
 
 for (int32_t kx = -1; kx <= 2; ++kx) {
 int32_t sx = x0 + kx;
 if (sx < 0) sx = 0;
 if (sx >= static_cast<int32_t>(srcWidth)) sx = srcWidth - 1;
 
 const float wx = CubicKernel(srcX - (x0 + kx));
 const float weight = wx * wy;
 weightSum += weight;
 
 const uint8_t* pixel = srcRow + sx * 4;
 b += pixel[0] * weight;
 g += pixel[1] * weight;
 r += pixel[2] * weight;
 a += pixel[3] * weight;
 }
 }
 
 // Normalize and clamp
 if (weightSum > 0.0f) {
 b /= weightSum;
 g /= weightSum;
 r /= weightSum;
 a /= weightSum;
 }
 
 dstRow[dx * 4 + 0] = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
 dstRow[dx * 4 + 1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
 dstRow[dx * 4 + 2] = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
 dstRow[dx * 4 + 3] = static_cast<uint8_t>(std::clamp(a, 0.0f, 255.0f));
 }
 }

 return true;
}

bool SIMDScaler::ScaleBGRA_Bicubic_AVX2(
 const uint8_t* srcData,
 uint32_t srcWidth,
 uint32_t srcHeight,
 uint32_t srcStride,
 uint8_t* dstData,
 uint32_t dstWidth,
 uint32_t dstHeight,
 uint32_t dstStride
) {
 // For now, fall back to scalar implementation
 // Full AVX2 bicubic is complex and requires careful horizontal SIMD optimization
 return ScaleBGRA_Bicubic_Scalar(
 srcData, srcWidth, srcHeight, srcStride,
 dstData, dstWidth, dstHeight, dstStride
 );
}

//==============================================================================
// Nearest Neighbor Scaling (fastest, suitable for pixel art)
//==============================================================================

bool SIMDScaler::ScaleBGRA_NearestNeighbor(
 const uint8_t* srcData,
 uint32_t srcWidth,
 uint32_t srcHeight,
 uint32_t srcStride,
 uint8_t* dstData,
 uint32_t dstWidth,
 uint32_t dstHeight,
 uint32_t dstStride
) {
 // Fixed-point scaling ratios (16.16 format for precision)
 const uint32_t xRatioFP = (srcWidth << 16) / dstWidth;
 const uint32_t yRatioFP = (srcHeight << 16) / dstHeight;

 for (uint32_t dy = 0; dy < dstHeight; ++dy) {
 const uint32_t sy = (dy * yRatioFP) >> 16;
 const uint8_t* srcRow = srcData + sy * srcStride;
 uint32_t* dstRow = reinterpret_cast<uint32_t*>(dstData + dy * dstStride);

 for (uint32_t dx = 0; dx < dstWidth; ++dx) {
 const uint32_t sx = (dx * xRatioFP) >> 16;
 dstRow[dx] = reinterpret_cast<const uint32_t*>(srcRow)[sx];
 }
 }

 return true;
}

//==============================================================================
// Lanczos3 Scaling (highest quality)
//==============================================================================

// Lanczos3 kernel: sinc(x) * sinc(x/3) for |x| < 3, else 0
static inline float Lanczos3Kernel(float x) {
 if (x == 0.0f) return 1.0f;
 if (x < -3.0f || x > 3.0f) return 0.0f;
 
 const float pi = 3.14159265358979323846f;
 const float pix = pi * x;
 const float pix3 = pix / 3.0f;
 
 return (std::sin(pix) / pix) * (std::sin(pix3) / pix3);
}

bool SIMDScaler::ScaleBGRA_Lanczos3_Scalar(
 const uint8_t* srcData,
 uint32_t srcWidth,
 uint32_t srcHeight,
 uint32_t srcStride,
 uint8_t* dstData,
 uint32_t dstWidth,
 uint32_t dstHeight,
 uint32_t dstStride
) {
 const float xRatio = static_cast<float>(srcWidth) / static_cast<float>(dstWidth);
 const float yRatio = static_cast<float>(srcHeight) / static_cast<float>(dstHeight);
 const int32_t radius = 3; // Lanczos3

 for (uint32_t dy = 0; dy < dstHeight; ++dy) {
 const float srcY = (dy + 0.5f) * yRatio - 0.5f;
 const int32_t y0 = static_cast<int32_t>(std::floor(srcY));
 
 uint8_t* dstRow = dstData + dy * dstStride;

 for (uint32_t dx = 0; dx < dstWidth; ++dx) {
 const float srcX = (dx + 0.5f) * xRatio - 0.5f;
 const int32_t x0 = static_cast<int32_t>(std::floor(srcX));
 
 float b = 0, g = 0, r = 0, a = 0;
 float weightSum = 0.0f;
 
 // 6x6 Lanczos3 kernel
 for (int32_t ky = -radius + 1; ky <= radius; ++ky) {
 int32_t sy = y0 + ky;
 if (sy < 0) sy = 0;
 if (sy >= static_cast<int32_t>(srcHeight)) sy = srcHeight - 1;
 
 const float wy = Lanczos3Kernel(srcY - (y0 + ky));
 const uint8_t* srcRow = srcData + sy * srcStride;
 
 for (int32_t kx = -radius + 1; kx <= radius; ++kx) {
 int32_t sx = x0 + kx;
 if (sx < 0) sx = 0;
 if (sx >= static_cast<int32_t>(srcWidth)) sx = srcWidth - 1;
 
 const float wx = Lanczos3Kernel(srcX - (x0 + kx));
 const float weight = wx * wy;
 weightSum += weight;
 
 const uint8_t* pixel = srcRow + sx * 4;
 b += pixel[0] * weight;
 g += pixel[1] * weight;
 r += pixel[2] * weight;
 a += pixel[3] * weight;
 }
 }
 
 // Normalize and clamp
 if (weightSum > 0.0f) {
 b /= weightSum;
 g /= weightSum;
 r /= weightSum;
 a /= weightSum;
 }
 
 dstRow[dx * 4 + 0] = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
 dstRow[dx * 4 + 1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
 dstRow[dx * 4 + 2] = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
 dstRow[dx * 4 + 3] = static_cast<uint8_t>(std::clamp(a, 0.0f, 255.0f));
 }
 }

 return true;
}

//==============================================================================
// Box-Filter Scaler - Area-average downscaling for superior quality at high ratios
//==============================================================================

bool SIMDScaler::ScaleBGRA_BoxFilter_Scalar(
 const uint8_t* srcData,
 uint32_t srcWidth,
 uint32_t srcHeight,
 uint32_t srcStride,
 uint8_t* dstData,
 uint32_t dstWidth,
 uint32_t dstHeight,
 uint32_t dstStride
) {
 if (!srcData || !dstData || srcWidth == 0 || srcHeight == 0 || dstWidth == 0 || dstHeight == 0) {
 return false;
 }

 const float xScale = static_cast<float>(srcWidth) / static_cast<float>(dstWidth);
 const float yScale = static_cast<float>(srcHeight) / static_cast<float>(dstHeight);

 for (uint32_t dy = 0; dy < dstHeight; ++dy) {
 // Calculate the source row range for this destination row
 float srcY0f = dy * yScale;
 float srcY1f = (dy + 1) * yScale;
 uint32_t srcY0 = static_cast<uint32_t>(srcY0f);
 uint32_t srcY1 = std::min(static_cast<uint32_t>(std::ceil(srcY1f)), srcHeight);
 if (srcY1 <= srcY0) srcY1 = srcY0 + 1;

 uint8_t* dstRow = dstData + dy * dstStride;

 for (uint32_t dx = 0; dx < dstWidth; ++dx) {
 // Calculate the source column range for this destination pixel
 float srcX0f = dx * xScale;
 float srcX1f = (dx + 1) * xScale;
 uint32_t srcX0 = static_cast<uint32_t>(srcX0f);
 uint32_t srcX1 = std::min(static_cast<uint32_t>(std::ceil(srcX1f)), srcWidth);
 if (srcX1 <= srcX0) srcX1 = srcX0 + 1;

 // Accumulate all source pixels in the box
 uint32_t sumB = 0, sumG = 0, sumR = 0, sumA = 0;
 uint32_t count = 0;

 for (uint32_t sy = srcY0; sy < srcY1; ++sy) {
 const uint8_t* srcRow = srcData + sy * srcStride;
 for (uint32_t sx = srcX0; sx < srcX1; ++sx) {
 const uint8_t* pixel = srcRow + sx * 4;
 sumB += pixel[0];
 sumG += pixel[1];
 sumR += pixel[2];
 sumA += pixel[3];
 ++count;
 }
 }

 // Average
 if (count > 0) {
 dstRow[dx * 4 + 0] = static_cast<uint8_t>(sumB / count);
 dstRow[dx * 4 + 1] = static_cast<uint8_t>(sumG / count);
 dstRow[dx * 4 + 2] = static_cast<uint8_t>(sumR / count);
 dstRow[dx * 4 + 3] = static_cast<uint8_t>(sumA / count);
 }
 }
 }

 return true;
}

bool SIMDScaler::ScaleBGRA_BoxFilter_SSE41(
 const uint8_t* srcData,
 uint32_t srcWidth,
 uint32_t srcHeight,
 uint32_t srcStride,
 uint8_t* dstData,
 uint32_t dstWidth,
 uint32_t dstHeight,
 uint32_t dstStride
) {
 if (!srcData || !dstData || srcWidth == 0 || srcHeight == 0 || dstWidth == 0 || dstHeight == 0) {
 return false;
 }

 const float xScale = static_cast<float>(srcWidth) / static_cast<float>(dstWidth);
 const float yScale = static_cast<float>(srcHeight) / static_cast<float>(dstHeight);

 for (uint32_t dy = 0; dy < dstHeight; ++dy) {
 float srcY0f = dy * yScale;
 float srcY1f = (dy + 1) * yScale;
 uint32_t srcY0 = static_cast<uint32_t>(srcY0f);
 uint32_t srcY1 = std::min(static_cast<uint32_t>(std::ceil(srcY1f)), srcHeight);
 if (srcY1 <= srcY0) srcY1 = srcY0 + 1;

 uint8_t* dstRow = dstData + dy * dstStride;

 for (uint32_t dx = 0; dx < dstWidth; ++dx) {
 float srcX0f = dx * xScale;
 float srcX1f = (dx + 1) * xScale;
 uint32_t srcX0 = static_cast<uint32_t>(srcX0f);
 uint32_t srcX1 = std::min(static_cast<uint32_t>(std::ceil(srcX1f)), srcWidth);
 if (srcX1 <= srcX0) srcX1 = srcX0 + 1;

 // SSE4.1 accumulation: process 4 source pixels at a time
 __m128i accumBG = _mm_setzero_si128(); // B,G accumulators (32-bit each)
 __m128i accumRA = _mm_setzero_si128(); // R,A accumulators (32-bit each)
 uint32_t count = 0;

 for (uint32_t sy = srcY0; sy < srcY1; ++sy) {
 const uint8_t* srcRow = srcData + sy * srcStride;
 uint32_t sx = srcX0;
 const uint32_t boxWidth = srcX1 - srcX0;
 const uint32_t simdWidth = (boxWidth / 4) * 4;

 // Process 4 pixels at a time with SSE
 for (uint32_t i = 0; i < simdWidth; i += 4) {
 // Load 4 BGRA pixels (16 bytes)
 __m128i pixels = _mm_loadu_si128(
 reinterpret_cast<const __m128i*>(srcRow + (sx + i) * 4));

 // Unpack to 16-bit: split low/high halves
 __m128i zero = _mm_setzero_si128();
 __m128i lo16 = _mm_unpacklo_epi8(pixels, zero); // pixels 0,1 as 16-bit
 __m128i hi16 = _mm_unpackhi_epi8(pixels, zero); // pixels 2,3 as 16-bit

 // Sum horizontally: add lo and hi 16-bit values
 __m128i sum16 = _mm_add_epi16(lo16, hi16);

 // Unpack to 32-bit and accumulate
 __m128i sumLo32 = _mm_unpacklo_epi16(sum16, zero); // B,G of pair sums
 __m128i sumHi32 = _mm_unpackhi_epi16(sum16, zero); // R,A of pair sums

 accumBG = _mm_add_epi32(accumBG, sumLo32);
 accumRA = _mm_add_epi32(accumRA, sumHi32);
 count += 4;
 }

 // Handle remaining pixels in this row with scalar
 for (uint32_t i = simdWidth; i < boxWidth; ++i) {
 const uint8_t* pixel = srcRow + (sx + i) * 4;
 // Add to SSE accumulators via scalar path
 accumBG = _mm_add_epi32(accumBG,
 _mm_setr_epi32(pixel[0], pixel[1], 0, 0));
 accumRA = _mm_add_epi32(accumRA,
 _mm_setr_epi32(pixel[2], pixel[3], 0, 0));
 ++count;
 }
 }

 if (count > 0) {
 // Extract accumulated values
 alignas(16) uint32_t bg[4], ra[4];
 _mm_store_si128(reinterpret_cast<__m128i*>(bg), accumBG);
 _mm_store_si128(reinterpret_cast<__m128i*>(ra), accumRA);

 // Sum the pairs: bg[0]+bg[2] = total B, bg[1]+bg[3] = total G
 uint32_t totalB = bg[0] + bg[2];
 uint32_t totalG = bg[1] + bg[3];
 uint32_t totalR = ra[0] + ra[2];
 uint32_t totalA = ra[1] + ra[3];

 dstRow[dx * 4 + 0] = static_cast<uint8_t>(totalB / count);
 dstRow[dx * 4 + 1] = static_cast<uint8_t>(totalG / count);
 dstRow[dx * 4 + 2] = static_cast<uint8_t>(totalR / count);
 dstRow[dx * 4 + 3] = static_cast<uint8_t>(totalA / count);
 }
 }
 }

 return true;
}

bool SIMDScaler::ScaleBGRA_BoxFilter(
 const uint8_t* srcData,
 uint32_t srcWidth,
 uint32_t srcHeight,
 uint32_t srcStride,
 uint8_t* dstData,
 uint32_t dstWidth,
 uint32_t dstHeight,
 uint32_t dstStride
) {
 if (HardwareCapabilities::Get().GetCPU().hasSSE41) {
 return ScaleBGRA_BoxFilter_SSE41(srcData, srcWidth, srcHeight, srcStride,
 dstData, dstWidth, dstHeight, dstStride);
 }
 return ScaleBGRA_BoxFilter_Scalar(srcData, srcWidth, srcHeight, srcStride,
 dstData, dstWidth, dstHeight, dstStride);
}

//==============================================================================
// SIMD Color Conversion - RGB<->BGR, RGBA<->BGRA
//==============================================================================

void SIMDScaler::ConvertRGBAtoBGRA_AVX2(
 const uint8_t* rgba,
 uint8_t* bgra,
 uint32_t pixelCount
) {
 const uint32_t simdPixels = (pixelCount / 8) * 8; // Process 8 pixels at a time with AVX2
 
 // AVX2 shuffle mask to swap R and B channels in RGBA -> BGRA
 // Input: R0 G0 B0 A0 R1 G1 B1 A1 ...
 // Output: B0 G0 R0 A0 B1 G1 R1 A1 ...
 const __m256i shuffleMask = _mm256_setr_epi8(
 2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8, 11, 14, 13, 12, 15,
 2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8, 11, 14, 13, 12, 15
 );
 
 // Process 8 pixels (32 bytes) at a time with AVX2
 for (uint32_t i = 0; i < simdPixels; i += 8) {
 __m256i pixels = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(rgba + i * 4));
 __m256i swapped = _mm256_shuffle_epi8(pixels, shuffleMask);
 _mm256_storeu_si256(reinterpret_cast<__m256i*>(bgra + i * 4), swapped);
 }
 
 // Handle remaining pixels with scalar code
 for (uint32_t i = simdPixels; i < pixelCount; i++) {
 bgra[i * 4 + 0] = rgba[i * 4 + 2]; // B = R
 bgra[i * 4 + 1] = rgba[i * 4 + 1]; // G = G
 bgra[i * 4 + 2] = rgba[i * 4 + 0]; // R = B
 bgra[i * 4 + 3] = rgba[i * 4 + 3]; // A = A
 }
}

void SIMDScaler::ConvertRGBAtoBGRA_Scalar(
 const uint8_t* rgba,
 uint8_t* bgra,
 uint32_t pixelCount
) {
 for (uint32_t i = 0; i < pixelCount; i++) {
 bgra[i * 4 + 0] = rgba[i * 4 + 2]; // B = R
 bgra[i * 4 + 1] = rgba[i * 4 + 1]; // G = G
 bgra[i * 4 + 2] = rgba[i * 4 + 0]; // R = B
 bgra[i * 4 + 3] = rgba[i * 4 + 3]; // A = A
 }
}

void SIMDScaler::ConvertRGBtoBGR_AVX2(
 const uint8_t* rgb,
 uint8_t* bgr,
 uint32_t pixelCount
) {
 // RGB24 to BGR24 is more complex due to non-aligned 3-byte pixels
 // Fall back to scalar for now (proper SIMD requires careful handling)
 ConvertRGBtoBGR_Scalar(rgb, bgr, pixelCount);
}

void SIMDScaler::ConvertRGBtoBGR_Scalar(
 const uint8_t* rgb,
 uint8_t* bgr,
 uint32_t pixelCount
) {
 for (uint32_t i = 0; i < pixelCount; i++) {
 bgr[i * 3 + 0] = rgb[i * 3 + 2]; // B = R
 bgr[i * 3 + 1] = rgb[i * 3 + 1]; // G = G
 bgr[i * 3 + 2] = rgb[i * 3 + 0]; // R = B
 }
}

} // namespace SIMD
} // namespace ExplorerLens

