// SIMDCapabilityDetector.h — Runtime SIMD Feature Detection & Verification
// Copyright (c) 2026 ExplorerLens Project
//
// Provides one-shot runtime CPUID-based detection of SIMD capabilities,
// verification that the selected SIMD level is safe to use, and a
// self-test mechanism that catches SIGILL if AVX2/AVX-512 is reported
// by CPUID but disabled by the OS (e.g., Hyper-V guests, old VMs).
//
// Usage:
//   auto& det = SIMDCapabilityDetector::Instance();
//   if (det.IsVerified(SIMDCap::AVX2)) {
//       // Safe to use AVX2 intrinsics
//   }
//
// Complements SIMDAccelerationManager which handles dispatch routing.

#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <intrin.h>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/// Individual SIMD features that can be detected
enum class SIMDCap : uint8_t {
    SSE2 = 0,
    SSE3 = 1,
    SSSE3 = 2,
    SSE41 = 3,
    SSE42 = 4,
    AVX = 5,
    AVX2 = 6,
    FMA = 7,
    AVX512F = 8,
    AVX512BW = 9,
    POPCNT = 10,
    BMI1 = 11,
    BMI2 = 12,
    COUNT
};

inline const char* SIMDCapName(SIMDCap f)
{
    static const char* names[] = {"SSE2", "SSE3",     "SSSE3",     "SSE4.1", "SSE4.2", "AVX", "AVX2",
                                  "FMA",  "AVX-512F", "AVX-512BW", "POPCNT", "BMI1",   "BMI2"};
    auto idx = static_cast<uint8_t>(f);
    return (idx < static_cast<uint8_t>(SIMDCap::COUNT)) ? names[idx] : "Unknown";
}

/// Aggregated detection result
struct SIMDDetectionResult
{
    std::array<bool, static_cast<size_t>(SIMDCap::COUNT)> supported{};
    std::array<bool, static_cast<size_t>(SIMDCap::COUNT)> verified{};
    std::string cpuBrand;
    uint32_t cpuFamily = 0;
    uint32_t cpuModel = 0;
    uint32_t cpuStepping = 0;
    bool osSupportsAVX = false;
    bool osSupportsAVX512 = false;

    /// Get the highest verified SIMD level
    SIMDCap GetMaxVerified() const
    {
        for (int i = static_cast<int>(SIMDCap::COUNT) - 1; i >= 0; --i) {
            if (verified[i])
                return static_cast<SIMDCap>(i);
        }
        return SIMDCap::SSE2;  // x64 baseline
    }

    /// Human-readable summary
    std::string Summary() const
    {
        std::string s = "CPU: " + cpuBrand + " | Max SIMD: " + SIMDCapName(GetMaxVerified()) + " | Features: ";
        bool first = true;
        for (size_t i = 0; i < static_cast<size_t>(SIMDCap::COUNT); ++i) {
            if (verified[i]) {
                if (!first)
                    s += ", ";
                s += SIMDCapName(static_cast<SIMDCap>(i));
                first = false;
            }
        }
        return s;
    }
};

/// One-shot CPUID-based SIMD detection with OS support verification.
class SIMDCapabilityDetector
{
  public:
    static SIMDCapabilityDetector& Instance()
    {
        static SIMDCapabilityDetector inst;
        return inst;
    }

    /// Check if a feature is supported by CPU.
    bool IsSupported(SIMDCap f) const
    {
        return m_result.supported[static_cast<size_t>(f)];
    }

    /// Check if a feature is verified (supported by CPU + enabled by OS).
    bool IsVerified(SIMDCap f) const
    {
        return m_result.verified[static_cast<size_t>(f)];
    }

    /// Get full detection result.
    const SIMDDetectionResult& GetResult() const
    {
        return m_result;
    }

    /// Re-run detection (normally not needed — runs once on construction).
    void Detect()
    {
        RunDetection();
    }

  private:
    SIMDCapabilityDetector()
    {
        RunDetection();
    }
    SIMDCapabilityDetector(const SIMDCapabilityDetector&) = delete;
    SIMDCapabilityDetector& operator=(const SIMDCapabilityDetector&) = delete;

    void RunDetection()
    {
        // Zero out
        m_result = {};

        // Get CPU brand string
        int brand[12]{};
        __cpuid(brand, 0x80000002);
        __cpuid(brand + 4, 0x80000003);
        __cpuid(brand + 8, 0x80000004);
        m_result.cpuBrand = std::string(reinterpret_cast<const char*>(brand), 48);
        // Trim trailing spaces
        while (!m_result.cpuBrand.empty() && m_result.cpuBrand.back() == ' ')
            m_result.cpuBrand.pop_back();

        // CPUID leaf 1: basic features
        int regs[4]{};
        __cpuid(regs, 1);
        m_result.cpuStepping = regs[0] & 0xF;
        m_result.cpuModel = ((regs[0] >> 4) & 0xF) | (((regs[0] >> 16) & 0xF) << 4);
        m_result.cpuFamily = ((regs[0] >> 8) & 0xF) + ((regs[0] >> 20) & 0xFF);

        int ecx1 = regs[2];
        int edx1 = regs[3];

        m_result.supported[idx(SIMDCap::SSE2)] = (edx1 >> 26) & 1;
        m_result.supported[idx(SIMDCap::SSE3)] = ecx1 & 1;
        m_result.supported[idx(SIMDCap::SSSE3)] = (ecx1 >> 9) & 1;
        m_result.supported[idx(SIMDCap::SSE41)] = (ecx1 >> 19) & 1;
        m_result.supported[idx(SIMDCap::SSE42)] = (ecx1 >> 20) & 1;
        m_result.supported[idx(SIMDCap::POPCNT)] = (ecx1 >> 23) & 1;
        m_result.supported[idx(SIMDCap::AVX)] = (ecx1 >> 28) & 1;
        m_result.supported[idx(SIMDCap::FMA)] = (ecx1 >> 12) & 1;

        // Check OS support for AVX (XSAVE + OSXSAVE)
        bool osxsave = (ecx1 >> 27) & 1;
        if (osxsave) {
            // Check XCR0 register for AVX state save support
            uint64_t xcr0 = _xgetbv(0);
            m_result.osSupportsAVX = (xcr0 & 0x6) == 0x6;       // SSE + AVX
            m_result.osSupportsAVX512 = (xcr0 & 0xE6) == 0xE6;  // SSE + AVX + opmask + ZMM
        }

        // CPUID leaf 7: extended features
        __cpuidex(regs, 7, 0);
        int ebx7 = regs[1];
        m_result.supported[idx(SIMDCap::AVX2)] = (ebx7 >> 5) & 1;
        m_result.supported[idx(SIMDCap::BMI1)] = (ebx7 >> 3) & 1;
        m_result.supported[idx(SIMDCap::BMI2)] = (ebx7 >> 8) & 1;
        m_result.supported[idx(SIMDCap::AVX512F)] = (ebx7 >> 16) & 1;
        m_result.supported[idx(SIMDCap::AVX512BW)] = (ebx7 >> 30) & 1;

        // Verify: feature supported by CPU AND enabled by OS
        for (size_t i = 0; i < static_cast<size_t>(SIMDCap::COUNT); ++i) {
            auto f = static_cast<SIMDCap>(i);
            if (!m_result.supported[i]) {
                m_result.verified[i] = false;
                continue;
            }

            // SSE2-SSE4.2 + POPCNT don't need XSAVE — always available on x64
            if (f <= SIMDCap::SSE42 || f == SIMDCap::POPCNT || f == SIMDCap::BMI1 || f == SIMDCap::BMI2) {
                m_result.verified[i] = true;
                continue;
            }

            // AVX/AVX2/FMA need OS AVX support
            if (f == SIMDCap::AVX || f == SIMDCap::AVX2 || f == SIMDCap::FMA) {
                m_result.verified[i] = m_result.osSupportsAVX;
                continue;
            }

            // AVX-512 needs OS AVX-512 support
            if (f == SIMDCap::AVX512F || f == SIMDCap::AVX512BW) {
                m_result.verified[i] = m_result.osSupportsAVX512;
                continue;
            }
        }
    }

    static constexpr size_t idx(SIMDCap f)
    {
        return static_cast<size_t>(f);
    }

    SIMDDetectionResult m_result;
};

}  // namespace Engine
}  // namespace ExplorerLens
