// RuntimeSIMDDispatcher.h — Runtime SIMD Instruction Set Dispatcher
// Copyright (c) 2026 ExplorerLens Project
//
// Detects CPU SIMD capabilities at runtime (SSE2/SSE4.1/AVX2/AVX-512/NEON)
// and dispatches to the optimal code path for pixel processing, color conversion,
// and resize operations. Ensures graceful fallback on older hardware.
//
#pragma once

#include <immintrin.h>
#include <intrin.h>
#include <windows.h>
#include <array>
#include <cstdint>
#include <string>

#include "../Pipeline/SIMDDispatchRouter.h"
#include "../Utils/HardwareCapabilities.h"

namespace ExplorerLens {
namespace Engine {

/// Runtime SIMD detection and dispatch. Singleton — call Initialize() once at
/// engine startup (typically from DllMain or first thumbnail request).
///
/// Uses SIMDFeature and SIMDTier from SIMDDispatchRouter.h and
/// CPUCapabilities from HardwareCapabilities.h.
///
/// Usage:
///   RuntimeSIMDDispatcher::Instance().Initialize();
///   if (RuntimeSIMDDispatcher::Instance().GetTier() >= SIMDTier::AVX2) { ... }
///
class RuntimeSIMDDispatcher
{
  public:
    static RuntimeSIMDDispatcher& Instance()
    {
        static RuntimeSIMDDispatcher instance;
        return instance;
    }

    /// Detect CPU capabilities. Safe to call multiple times (no-op after first).
    void Initialize()
    {
        if (m_initialized)
            return;

#ifdef _M_ARM64
        DetectARM64();
#else
        DetectX86();
#endif
        // Core count
        SYSTEM_INFO si = {};
        GetSystemInfo(&si);
        m_caps.logicalCores = si.dwNumberOfProcessors;

        m_initialized = true;
    }

    const CPUCapabilities& GetCapabilities() const
    {
        return m_caps;
    }
    SIMDTier GetTier() const
    {
        return m_tier;
    }
    SIMDFeature GetFeatures() const
    {
        return m_features;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }

    /// Human-readable description of detected capabilities
    std::string Describe() const
    {
        std::string desc = "CPU: " + m_caps.brandString + "\n";
        desc += "Cores: " + std::to_string(m_caps.logicalCores) + "\n";
        desc += "Tier: ";
        desc += SIMDTierToString(m_tier);
        desc += "\nFeatures:";
        if (HasFeature(m_features, SIMDFeature::SSE2))
            desc += " SSE2";
        if (HasFeature(m_features, SIMDFeature::SSE41))
            desc += " SSE4.1";
        if (HasFeature(m_features, SIMDFeature::AVX2))
            desc += " AVX2";
        if (HasFeature(m_features, SIMDFeature::FMA))
            desc += " FMA";
        if (HasFeature(m_features, SIMDFeature::AVX512F))
            desc += " AVX-512F";
        return desc;
    }

    /// Dispatch-friendly: returns function pointers for optimized pixel ops
    /// based on detected tier. Callers get the fastest available implementation.
    using ResizeRowFn = void (*)(const uint8_t* src, uint8_t* dst, uint32_t srcW, uint32_t dstW);
    using BlendRowFn = void (*)(const uint8_t* src, uint8_t* dst, uint32_t width, uint8_t alpha);
    using ConvertRowFn = void (*)(const uint8_t* src, uint8_t* dst, uint32_t width);

    ResizeRowFn GetResizeRowFn() const
    {
        switch (m_tier) {
            case SIMDTier::AVX2:
            case SIMDTier::AVX512:
                return ResizeRow_AVX2;
            case SIMDTier::SSE:
                return ResizeRow_SSE2;
            default:
                return ResizeRow_Scalar;
        }
    }

    BlendRowFn GetBlendRowFn() const
    {
        switch (m_tier) {
            case SIMDTier::AVX2:
            case SIMDTier::AVX512:
                return BlendRow_AVX2;
            case SIMDTier::SSE:
                return BlendRow_SSE2;
            default:
                return BlendRow_Scalar;
        }
    }

  private:
    RuntimeSIMDDispatcher() = default;
    RuntimeSIMDDispatcher(const RuntimeSIMDDispatcher&) = delete;
    RuntimeSIMDDispatcher& operator=(const RuntimeSIMDDispatcher&) = delete;

    bool m_initialized = false;
    SIMDFeature m_features = SIMDFeature::None;
    SIMDTier m_tier = SIMDTier::Scalar;
    CPUCapabilities m_caps{};

#ifndef _M_ARM64
    void DetectX86()
    {
        std::array<int, 4> cpuInfo = {};
        __cpuid(cpuInfo.data(), 0);
        int maxFunc = cpuInfo[0];

        if (maxFunc >= 1) {
            __cpuid(cpuInfo.data(), 1);
            uint32_t ecx = cpuInfo[2];
            uint32_t edx = cpuInfo[3];

            if (edx & (1 << 26)) {
                m_features = m_features | SIMDFeature::SSE2;
                m_caps.hasSSE2 = true;
            }
            if (ecx & (1 << 0)) {
                m_caps.hasSSE3 = true;
            }
            if (ecx & (1 << 9)) {
                m_caps.hasSSSE3 = true;
            }
            if (ecx & (1 << 19)) {
                m_features = m_features | SIMDFeature::SSE41;
                m_caps.hasSSE41 = true;
            }
            if (ecx & (1 << 20)) {
                m_features = m_features | SIMDFeature::SSE42;
                m_caps.hasSSE42 = true;
            }
            if (ecx & (1 << 23)) {
                m_features = m_features | SIMDFeature::POPCNT;
                m_caps.hasPOPCNT = true;
            }
            if (ecx & (1 << 28)) {
                m_features = m_features | SIMDFeature::AVX;
                m_caps.hasAVX = true;
            }
            if (ecx & (1 << 12)) {
                m_features = m_features | SIMDFeature::FMA;
                m_caps.hasFMA = true;
            }
            if (ecx & (1 << 29)) {
                m_caps.hasF16C = true;
            }
        }

        if (maxFunc >= 7) {
            __cpuidex(cpuInfo.data(), 7, 0);
            uint32_t ebx = cpuInfo[1];
            if (ebx & (1 << 5)) {
                m_features = m_features | SIMDFeature::AVX2;
                m_caps.hasAVX2 = true;
            }
            if (ebx & (1 << 3)) {
                m_caps.hasBMI1 = true;
            }
            if (ebx & (1 << 8)) {
                m_features = m_features | SIMDFeature::BMI2;
                m_caps.hasBMI2 = true;
            }
            if (ebx & (1 << 16)) {
                m_features = m_features | SIMDFeature::AVX512F;
                m_caps.hasAVX512F = true;
            }
            if (ebx & (1 << 30)) {
                m_features = m_features | SIMDFeature::AVX512BW;
                m_caps.hasAVX512BW = true;
            }

            uint32_t ecx7 = cpuInfo[2];
            (void)ecx7;  // Reserved for future use
        }

        // Brand string
        char brand[49] = {};
        for (int i = 0; i < 3; i++) {
            __cpuid(cpuInfo.data(), 0x80000002 + i);
            memcpy(brand + i * 16, cpuInfo.data(), 16);
        }
        brand[48] = '\0';
        m_caps.brandString = brand;

        // Determine tier
        if (HasFeature(m_features, SIMDFeature::AVX512F))
            m_tier = SIMDTier::AVX512;
        else if (HasFeature(m_features, SIMDFeature::AVX2))
            m_tier = SIMDTier::AVX2;
        else if (HasFeature(m_features, SIMDFeature::SSE41) || HasFeature(m_features, SIMDFeature::SSE2))
            m_tier = SIMDTier::SSE;
        else
            m_tier = SIMDTier::Scalar;

        // Cache line detection
        __cpuid(cpuInfo.data(), 1);
        m_caps.cacheLineSize = ((cpuInfo[1] >> 8) & 0xFF) * 8;
        if (m_caps.cacheLineSize == 0)
            m_caps.cacheLineSize = 64;
    }
#else
    void DetectARM64()
    {
        m_features = SIMDFeature::NEON;
        m_tier = SIMDTier::NEON;
        m_caps.brandString = "ARM64 Processor";
        m_caps.cacheLineSize = 64;
    }
#endif

    // Scalar fallback implementations
    static void ResizeRow_Scalar(const uint8_t* src, uint8_t* dst, uint32_t srcW, uint32_t dstW)
    {
        for (uint32_t x = 0; x < dstW; x++) {
            uint32_t sx = x * srcW / dstW;
            memcpy(dst + x * 4, src + sx * 4, 4);
        }
    }

    static void ResizeRow_SSE2(const uint8_t* src, uint8_t* dst, uint32_t srcW, uint32_t dstW)
    {
        // SSE2-optimized nearest-neighbor row resize — processes 4 BGRA pixels per iteration
        const uint32_t step = 4;  // 4 pixels x 4 bytes = 16 bytes = one __m128i
        const uint32_t aligned = dstW & ~(step - 1);
        for (uint32_t x = 0; x < aligned; x += step) {
            // Compute 4 source x-coordinates
            alignas(16) uint32_t indices[4];
            for (uint32_t i = 0; i < step; i++) {
                indices[i] = (x + i) * srcW / dstW;
            }
            // Gather 4 source pixels into a single 128-bit register
            alignas(16) uint8_t pixels[16];
            memcpy(pixels + 0, src + indices[0] * 4, 4);
            memcpy(pixels + 4, src + indices[1] * 4, 4);
            memcpy(pixels + 8, src + indices[2] * 4, 4);
            memcpy(pixels + 12, src + indices[3] * 4, 4);
            __m128i px = _mm_load_si128(reinterpret_cast<const __m128i*>(pixels));
            _mm_storeu_si128(reinterpret_cast<__m128i*>(dst + x * 4), px);
        }
        // Handle remaining pixels
        for (uint32_t x = aligned; x < dstW; x++) {
            uint32_t sx = x * srcW / dstW;
            memcpy(dst + x * 4, src + sx * 4, 4);
        }
    }

    static void ResizeRow_AVX2(const uint8_t* src, uint8_t* dst, uint32_t srcW, uint32_t dstW)
    {
        // AVX2-optimized nearest-neighbor row resize — processes 8 BGRA pixels per iteration
        const uint32_t step = 8;  // 8 pixels x 4 bytes = 32 bytes = one __m256i
        const uint32_t aligned = dstW & ~(step - 1);
        for (uint32_t x = 0; x < aligned; x += step) {
            // Compute 8 source indices and use VGATHERDD for true AVX2 gather
            __m256i idx =
                _mm256_set_epi32(static_cast<int>((x + 7) * srcW / dstW), static_cast<int>((x + 6) * srcW / dstW),
                                 static_cast<int>((x + 5) * srcW / dstW), static_cast<int>((x + 4) * srcW / dstW),
                                 static_cast<int>((x + 3) * srcW / dstW), static_cast<int>((x + 2) * srcW / dstW),
                                 static_cast<int>((x + 1) * srcW / dstW), static_cast<int>((x + 0) * srcW / dstW));
            __m256i px = _mm256_i32gather_epi32(reinterpret_cast<const int*>(src), idx, 4);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst + x * 4), px);
        }
        // Handle remaining pixels
        for (uint32_t x = aligned; x < dstW; x++) {
            uint32_t sx = x * srcW / dstW;
            memcpy(dst + x * 4, src + sx * 4, 4);
        }
    }

    static void BlendRow_Scalar(const uint8_t* src, uint8_t* dst, uint32_t width, uint8_t alpha)
    {
        for (uint32_t x = 0; x < width; x++) {
            for (int c = 0; c < 4; c++) {
                dst[x * 4 + c] = static_cast<uint8_t>((src[x * 4 + c] * alpha + dst[x * 4 + c] * (255 - alpha)) / 255);
            }
        }
    }

    static void BlendRow_SSE2(const uint8_t* src, uint8_t* dst, uint32_t width, uint8_t alpha)
    {
        // SSE2 alpha blend — processes 4 BGRA pixels (16 bytes) per iteration
        // Formula per channel: out = (src * alpha + dst * (255 - alpha)) / 255
        // Approximated as: out = (src * alpha + dst * invAlpha + 128) >> 8 using _mm_mulhi_epu16
        const __m128i zero = _mm_setzero_si128();
        const __m128i alphaVec = _mm_set1_epi16(static_cast<short>(alpha));
        const __m128i invAlphaVec = _mm_set1_epi16(static_cast<short>(255 - alpha));
        const __m128i half = _mm_set1_epi16(128);

        const uint32_t step = 4;
        const uint32_t aligned = width & ~(step - 1);
        for (uint32_t x = 0; x < aligned; x += step) {
            __m128i s = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src + x * 4));
            __m128i d = _mm_loadu_si128(reinterpret_cast<const __m128i*>(dst + x * 4));

            // Unpack low 8 pixels (first 2 BGRA pixels) to 16-bit
            __m128i sLo = _mm_unpacklo_epi8(s, zero);
            __m128i dLo = _mm_unpacklo_epi8(d, zero);
            // Unpack high 8 pixels (next 2 BGRA pixels) to 16-bit
            __m128i sHi = _mm_unpackhi_epi8(s, zero);
            __m128i dHi = _mm_unpackhi_epi8(d, zero);

            // Blend: (src * alpha + dst * invAlpha + 128) >> 8
            __m128i rLo = _mm_srli_epi16(
                _mm_add_epi16(_mm_add_epi16(_mm_mullo_epi16(sLo, alphaVec), _mm_mullo_epi16(dLo, invAlphaVec)), half),
                8);
            __m128i rHi = _mm_srli_epi16(
                _mm_add_epi16(_mm_add_epi16(_mm_mullo_epi16(sHi, alphaVec), _mm_mullo_epi16(dHi, invAlphaVec)), half),
                8);

            // Pack 16-bit results back to 8-bit with unsigned saturation
            __m128i result = _mm_packus_epi16(rLo, rHi);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(dst + x * 4), result);
        }
        // Handle remaining pixels
        for (uint32_t x = aligned; x < width; x++) {
            for (int c = 0; c < 4; c++) {
                dst[x * 4 + c] = static_cast<uint8_t>((src[x * 4 + c] * alpha + dst[x * 4 + c] * (255 - alpha)) / 255);
            }
        }
    }

    static void BlendRow_AVX2(const uint8_t* src, uint8_t* dst, uint32_t width, uint8_t alpha)
    {
        // AVX2 alpha blend — processes 8 BGRA pixels (32 bytes) per iteration
        const __m256i zero = _mm256_setzero_si256();
        const __m256i alphaVec = _mm256_set1_epi16(static_cast<short>(alpha));
        const __m256i invAlphaVec = _mm256_set1_epi16(static_cast<short>(255 - alpha));
        const __m256i half = _mm256_set1_epi16(128);

        const uint32_t step = 8;
        const uint32_t aligned = width & ~(step - 1);
        for (uint32_t x = 0; x < aligned; x += step) {
            __m256i s = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + x * 4));
            __m256i d = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(dst + x * 4));

            // Unpack to 16-bit (AVX2 works in two 128-bit lanes)
            __m256i sLo = _mm256_unpacklo_epi8(s, zero);
            __m256i dLo = _mm256_unpacklo_epi8(d, zero);
            __m256i sHi = _mm256_unpackhi_epi8(s, zero);
            __m256i dHi = _mm256_unpackhi_epi8(d, zero);

            // Blend: (src * alpha + dst * invAlpha + 128) >> 8
            __m256i rLo = _mm256_srli_epi16(
                _mm256_add_epi16(
                    _mm256_add_epi16(_mm256_mullo_epi16(sLo, alphaVec), _mm256_mullo_epi16(dLo, invAlphaVec)), half),
                8);
            __m256i rHi = _mm256_srli_epi16(
                _mm256_add_epi16(
                    _mm256_add_epi16(_mm256_mullo_epi16(sHi, alphaVec), _mm256_mullo_epi16(dHi, invAlphaVec)), half),
                8);

            __m256i result = _mm256_packus_epi16(rLo, rHi);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst + x * 4), result);
        }
        // Handle remaining pixels
        for (uint32_t x = aligned; x < width; x++) {
            for (int c = 0; c < 4; c++) {
                dst[x * 4 + c] = static_cast<uint8_t>((src[x * 4 + c] * alpha + dst[x * 4 + c] * (255 - alpha)) / 255);
            }
        }
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
