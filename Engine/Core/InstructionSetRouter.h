// InstructionSetRouter.h — Runtime CPU Feature Detection and Dispatch
// Copyright (c) 2026 ExplorerLens Project
//
// Detects CPU SIMD capabilities at runtime and routes to the best available
// code path. Supports SSE4.2, AVX2, AVX-512, and ARM64 NEON detection.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#ifdef _MSC_VER
    #include <intrin.h>
#endif

namespace ExplorerLens {
namespace Engine {

enum class ISAFeature : uint32_t {
    None = 0,
    SSE2 = 1 << 0,
    SSE3 = 1 << 1,
    SSSE3 = 1 << 2,
    SSE41 = 1 << 3,
    SSE42 = 1 << 4,
    AVX = 1 << 5,
    AVX2 = 1 << 6,
    FMA3 = 1 << 7,
    AVX512F = 1 << 8,
    AVX512BW = 1 << 9,
    AVX512VL = 1 << 10,
    POPCNT = 1 << 11,
    BMI1 = 1 << 12,
    BMI2 = 1 << 13,
    NEON = 1 << 14,
    F16C = 1 << 15
};

inline ISAFeature operator|(ISAFeature a, ISAFeature b)
{
    return static_cast<ISAFeature>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline ISAFeature operator&(ISAFeature a, ISAFeature b)
{
    return static_cast<ISAFeature>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

enum class ISACodePath : uint8_t {
    Scalar = 0,
    SSE42 = 1,
    AVX2 = 2,
    AVX512 = 3,
    NEON = 4
};

struct CPUFeatureReport
{
    ISAFeature features = ISAFeature::None;
    ISACodePath bestPath = ISACodePath::Scalar;
    std::string vendorId;
    std::string brandString;
    uint32_t family = 0;
    uint32_t model = 0;
    uint32_t stepping = 0;
    uint32_t logicalCores = 0;
};

class InstructionSetRouter
{
  public:
    static InstructionSetRouter& Instance()
    {
        static InstructionSetRouter s;
        return s;
    }

    CPUFeatureReport Detect()
    {
        CPUFeatureReport report{};
#if defined(_M_ARM64) || defined(__aarch64__)
        report.features = ISAFeature::NEON;
        report.bestPath = ISACodePath::NEON;
#elif defined(_MSC_VER)
        int cpuInfo[4] = {};
        __cpuid(cpuInfo, 0);
        report.vendorId.assign(reinterpret_cast<const char*>(&cpuInfo[1]), 4);
        report.vendorId.append(reinterpret_cast<const char*>(&cpuInfo[3]), 4);
        report.vendorId.append(reinterpret_cast<const char*>(&cpuInfo[2]), 4);

        __cpuid(cpuInfo, 1);
        report.stepping = cpuInfo[0] & 0xF;
        report.model = (cpuInfo[0] >> 4) & 0xF;
        report.family = (cpuInfo[0] >> 8) & 0xF;

        if (cpuInfo[3] & (1 << 26))
            report.features = report.features | ISAFeature::SSE2;
        if (cpuInfo[2] & (1 << 0))
            report.features = report.features | ISAFeature::SSE3;
        if (cpuInfo[2] & (1 << 9))
            report.features = report.features | ISAFeature::SSSE3;
        if (cpuInfo[2] & (1 << 19))
            report.features = report.features | ISAFeature::SSE41;
        if (cpuInfo[2] & (1 << 20))
            report.features = report.features | ISAFeature::SSE42;
        if (cpuInfo[2] & (1 << 23))
            report.features = report.features | ISAFeature::POPCNT;
        if (cpuInfo[2] & (1 << 28))
            report.features = report.features | ISAFeature::AVX;
        if (cpuInfo[2] & (1 << 12))
            report.features = report.features | ISAFeature::FMA3;
        if (cpuInfo[2] & (1 << 29))
            report.features = report.features | ISAFeature::F16C;

        __cpuidex(cpuInfo, 7, 0);
        if (cpuInfo[1] & (1 << 5))
            report.features = report.features | ISAFeature::AVX2;
        if (cpuInfo[1] & (1 << 3))
            report.features = report.features | ISAFeature::BMI1;
        if (cpuInfo[1] & (1 << 8))
            report.features = report.features | ISAFeature::BMI2;
        if (cpuInfo[1] & (1 << 16))
            report.features = report.features | ISAFeature::AVX512F;
        if (cpuInfo[1] & (1 << 30))
            report.features = report.features | ISAFeature::AVX512BW;
        if (cpuInfo[1] & (1 << 31))
            report.features = report.features | ISAFeature::AVX512VL;

        // Determine best code path
        if (HasFeature(report.features, ISAFeature::AVX512F))
            report.bestPath = ISACodePath::AVX512;
        else if (HasFeature(report.features, ISAFeature::AVX2))
            report.bestPath = ISACodePath::AVX2;
        else if (HasFeature(report.features, ISAFeature::SSE42))
            report.bestPath = ISACodePath::SSE42;
#endif
        m_report = report;
        return report;
    }

    bool HasFeature(ISAFeature feature) const
    {
        return HasFeature(m_report.features, feature);
    }

    ISACodePath GetBestPath() const
    {
        return m_report.bestPath;
    }
    const CPUFeatureReport& GetReport() const
    {
        return m_report;
    }

    bool Validate() const
    {
#if defined(_M_ARM64)
        return HasFeature(m_report.features, ISAFeature::NEON);
#elif defined(_M_IX86) || defined(_M_X64)
        return HasFeature(m_report.features, ISAFeature::SSE2);
#else
        return true;
#endif
    }

  private:
    InstructionSetRouter()
    {
        Detect();
    }
    ~InstructionSetRouter() = default;
    InstructionSetRouter(const InstructionSetRouter&) = delete;
    InstructionSetRouter& operator=(const InstructionSetRouter&) = delete;

    static bool HasFeature(ISAFeature set, ISAFeature test)
    {
        return (static_cast<uint32_t>(set) & static_cast<uint32_t>(test)) != 0;
    }

    CPUFeatureReport m_report{};
};

}  // namespace Engine
}  // namespace ExplorerLens
