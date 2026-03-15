// SIMDPixelConversion.h — AVX2/SSE4.2 Accelerated Color Space Transforms
// Copyright (c) 2026 ExplorerLens Project
//
// High-performance pixel format conversion using SIMD intrinsics.
// Processes 8-16 pixels per cycle for thumbnail color space transforms.
// Falls back to scalar path on older CPUs without AVX2.
//
#pragma once

#include <cstdint>
#include <cstddef>
#include <immintrin.h>
#include <algorithm>
#include "SIMDAccelerationManager.h"

namespace ExplorerLens {
namespace Engine {

//==============================================================================
// Pixel Format Enum
//==============================================================================
enum class SIMDPixelFormat : uint8_t {
    BGRA8,      // Windows native (DIBSection, HBITMAP)
    RGBA8,      // OpenGL/Vulkan native
    RGB8,       // Packed RGB (no alpha)
    BGR8,       // Windows GDI (no alpha)
    Gray8,      // Single-channel grayscale
    Gray16,     // 16-bit grayscale (scientific)
    RGBA16F,    // Half-float HDR
    RGBA32F,    // Full-float HDR (EXR, HDR)
};

//==============================================================================
// SIMD Pixel Conversion Engine
//==============================================================================
class SIMDPixelConverter {
public:
    SIMDPixelConverter() {
#if defined(_MSC_VER)
        int cpuInfo[4] = {};
        __cpuid(cpuInfo, 1);
        m_caps.hasSSE2   = (cpuInfo[3] & (1 << 26)) != 0;
        m_caps.hasSSE41  = (cpuInfo[2] & (1 << 19)) != 0;
        m_caps.hasSSE42  = (cpuInfo[2] & (1 << 20)) != 0;
        m_caps.hasAVX    = (cpuInfo[2] & (1 << 28)) != 0;
        m_caps.hasFMA    = (cpuInfo[2] & (1 << 12)) != 0;
        __cpuidex(cpuInfo, 7, 0);
        m_caps.hasAVX2   = (cpuInfo[1] & (1 << 5)) != 0;
        m_caps.hasAVX512 = (cpuInfo[1] & (1 << 16)) != 0;
#endif
    }

    /// Convert RGBA8 to BGRA8 (channel swap) — processes 8 pixels/cycle with AVX2
    void ConvertRGBA_to_BGRA(const uint8_t* src, uint8_t* dst, size_t pixelCount) const {
        size_t i = 0;

#if defined(__AVX2__) || defined(_MSC_VER)
        if (m_caps.hasAVX2) {
            // AVX2 path: process 8 pixels (32 bytes) per iteration
            const __m256i shuffleMask = _mm256_setr_epi8(
                2, 1, 0, 3,  6, 5, 4, 7,  10, 9, 8, 11,  14, 13, 12, 15,
                2, 1, 0, 3,  6, 5, 4, 7,  10, 9, 8, 11,  14, 13, 12, 15
            );

            for (; i + 8 <= pixelCount; i += 8) {
                __m256i pixels = _mm256_loadu_si256(
                    reinterpret_cast<const __m256i*>(src + i * 4));
                __m256i swapped = _mm256_shuffle_epi8(pixels, shuffleMask);
                _mm256_storeu_si256(
                    reinterpret_cast<__m256i*>(dst + i * 4), swapped);
            }
        }
#endif

        // Scalar fallback for remaining pixels
        for (; i < pixelCount; ++i) {
            size_t offset = i * 4;
            dst[offset + 0] = src[offset + 2]; // B = R
            dst[offset + 1] = src[offset + 1]; // G = G
            dst[offset + 2] = src[offset + 0]; // R = B
            dst[offset + 3] = src[offset + 3]; // A = A
        }
    }

    /// Convert RGB8 to BGRA8 (add alpha, swap channels)
    void ConvertRGB_to_BGRA(const uint8_t* src, uint8_t* dst, size_t pixelCount) const {
        size_t i = 0;

        // Scalar path (RGB→BGRA requires stride change, SIMD complex)
        for (; i < pixelCount; ++i) {
            size_t srcOff = i * 3;
            size_t dstOff = i * 4;
            dst[dstOff + 0] = src[srcOff + 2]; // B
            dst[dstOff + 1] = src[srcOff + 1]; // G
            dst[dstOff + 2] = src[srcOff + 0]; // R
            dst[dstOff + 3] = 0xFF;            // A (opaque)
        }
    }

    /// Convert Grayscale to BGRA8
    void ConvertGray_to_BGRA(const uint8_t* src, uint8_t* dst, size_t pixelCount) const {
        size_t i = 0;

#if defined(__AVX2__) || defined(_MSC_VER)
        if (m_caps.hasAVX2) {
            for (; i + 32 <= pixelCount; i += 32) {
                __m256i gray = _mm256_loadu_si256(
                    reinterpret_cast<const __m256i*>(src + i));

                // Unpack 8-bit gray to 32-bit BGRA (gray,gray,gray,0xFF)
                __m128i lo16 = _mm256_castsi256_si128(gray);
                __m128i hi16 = _mm256_extracti128_si256(gray, 1);

                // Process lower 16 bytes
                __m128i lo8 = _mm_unpacklo_epi8(lo16, lo16);
                __m128i hi8 = _mm_unpackhi_epi8(lo16, lo16);

                __m128i a = _mm_unpacklo_epi16(lo8, _mm_set1_epi16(static_cast<short>(0x00FF)));
                __m128i b = _mm_unpackhi_epi16(lo8, _mm_set1_epi16(static_cast<short>(0x00FF)));
                __m128i c = _mm_unpacklo_epi16(hi8, _mm_set1_epi16(static_cast<short>(0x00FF)));
                __m128i d = _mm_unpackhi_epi16(hi8, _mm_set1_epi16(static_cast<short>(0x00FF)));

                _mm_storeu_si128(reinterpret_cast<__m128i*>(dst + (i + 0) * 4), a);
                _mm_storeu_si128(reinterpret_cast<__m128i*>(dst + (i + 4) * 4), b);
                _mm_storeu_si128(reinterpret_cast<__m128i*>(dst + (i + 8) * 4), c);
                _mm_storeu_si128(reinterpret_cast<__m128i*>(dst + (i + 12) * 4), d);

                // Process upper 16 bytes
                __m128i lo8_2 = _mm_unpacklo_epi8(hi16, hi16);
                __m128i hi8_2 = _mm_unpackhi_epi8(hi16, hi16);

                __m128i e = _mm_unpacklo_epi16(lo8_2, _mm_set1_epi16(static_cast<short>(0x00FF)));
                __m128i f = _mm_unpackhi_epi16(lo8_2, _mm_set1_epi16(static_cast<short>(0x00FF)));
                __m128i g = _mm_unpacklo_epi16(hi8_2, _mm_set1_epi16(static_cast<short>(0x00FF)));
                __m128i h = _mm_unpackhi_epi16(hi8_2, _mm_set1_epi16(static_cast<short>(0x00FF)));

                _mm_storeu_si128(reinterpret_cast<__m128i*>(dst + (i + 16) * 4), e);
                _mm_storeu_si128(reinterpret_cast<__m128i*>(dst + (i + 20) * 4), f);
                _mm_storeu_si128(reinterpret_cast<__m128i*>(dst + (i + 24) * 4), g);
                _mm_storeu_si128(reinterpret_cast<__m128i*>(dst + (i + 28) * 4), h);
            }
        }
#endif

        for (; i < pixelCount; ++i) {
            size_t dstOff = i * 4;
            uint8_t v = src[i];
            dst[dstOff + 0] = v;    // B
            dst[dstOff + 1] = v;    // G
            dst[dstOff + 2] = v;    // R
            dst[dstOff + 3] = 0xFF; // A
        }
    }

    /// Convert HDR float RGBA to 8-bit BGRA with tone mapping
    void ConvertRGBA32F_to_BGRA(const float* src, uint8_t* dst, size_t pixelCount,
                                 float exposure = 1.0f) const {
        size_t i = 0;

#if defined(__AVX2__) || defined(_MSC_VER)
        if (m_caps.hasAVX2 && m_caps.hasFMA) {
            __m256 vExposure = _mm256_set1_ps(exposure);
            __m256 vOne = _mm256_set1_ps(1.0f);
            __m256 v255 = _mm256_set1_ps(255.0f);
            __m256 vZero = _mm256_setzero_ps();

            for (; i + 2 <= pixelCount; i += 2) {
                // Load 2 pixels (8 floats)
                __m256 px = _mm256_loadu_ps(src + i * 4);

                // Apply exposure
                px = _mm256_mul_ps(px, vExposure);

                // Reinhard tone mapping: x / (1 + x)
                __m256 denom = _mm256_add_ps(vOne, px);
                px = _mm256_div_ps(px, denom);

                // Scale to [0, 255] and clamp
                px = _mm256_mul_ps(px, v255);
                px = _mm256_max_ps(px, vZero);
                px = _mm256_min_ps(px, v255);

                // Extract and swizzle to BGRA
                // Pixel 0
                float tmp[8];
                _mm256_storeu_ps(tmp, px);

                size_t off0 = i * 4;
                dst[off0 + 0] = static_cast<uint8_t>(tmp[2]); // B
                dst[off0 + 1] = static_cast<uint8_t>(tmp[1]); // G
                dst[off0 + 2] = static_cast<uint8_t>(tmp[0]); // R
                dst[off0 + 3] = static_cast<uint8_t>(tmp[3]); // A

                size_t off1 = (i + 1) * 4;
                dst[off1 + 0] = static_cast<uint8_t>(tmp[6]); // B
                dst[off1 + 1] = static_cast<uint8_t>(tmp[5]); // G
                dst[off1 + 2] = static_cast<uint8_t>(tmp[4]); // R
                dst[off1 + 3] = static_cast<uint8_t>(tmp[7]); // A
            }
        }
#endif

        for (; i < pixelCount; ++i) {
            size_t srcOff = i * 4;
            size_t dstOff = i * 4;

            for (int c = 0; c < 3; ++c) {
                float v = src[srcOff + c] * exposure;
                v = v / (1.0f + v); // Reinhard tone map
                int mapped = (c == 0) ? 2 : (c == 2) ? 0 : 1; // RGBA→BGRA swizzle
                dst[dstOff + mapped] = static_cast<uint8_t>(
                    (std::min)(255.0f, (std::max)(0.0f, v * 255.0f)));
            }
            // Alpha
            float a = src[srcOff + 3];
            dst[dstOff + 3] = static_cast<uint8_t>(
                (std::min)(255.0f, (std::max)(0.0f, a * 255.0f)));
        }
    }

    /// Premultiply alpha (BGRA in-place)
    void PremultiplyAlpha(uint8_t* pixels, size_t pixelCount) const {
        for (size_t i = 0; i < pixelCount; ++i) {
            size_t off = i * 4;
            uint8_t a = pixels[off + 3];
            if (a == 255) continue;
            if (a == 0) {
                pixels[off + 0] = 0;
                pixels[off + 1] = 0;
                pixels[off + 2] = 0;
                continue;
            }
            pixels[off + 0] = static_cast<uint8_t>((pixels[off + 0] * a + 127) / 255);
            pixels[off + 1] = static_cast<uint8_t>((pixels[off + 1] * a + 127) / 255);
            pixels[off + 2] = static_cast<uint8_t>((pixels[off + 2] * a + 127) / 255);
        }
    }

    /// Get detected SIMD capabilities
    const SIMDCapabilities& GetCapabilities() const { return m_caps; }

private:
    SIMDCapabilities m_caps;
};

} // namespace Engine
} // namespace ExplorerLens
