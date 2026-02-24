//==============================================================================
// ExplorerLens Engine — ARM64 Performance Optimizer
// NEON/SVE2 SIMD acceleration profiles, efficiency core scheduling hints,
// memory bandwidth optimisation for ARM64 topology, and thermal-aware
// clock ramp strategy for thumbnail decode workloads.
//==============================================================================
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

enum class ARM64SIMDExt  : uint8_t { NEON=0, SVE, SVE2, SME, COUNT };
enum class ARM64CoreType : uint8_t { BigCore=0, LittleCore, MiddleCore, COUNT };
enum class ARM64ThermalHint : uint8_t { Burst=0, Sustained, Balanced, Efficiency, Silent, COUNT };

struct ARM64TopologyInfo {
    uint8_t    bigCores      = 0;
    uint8_t    littleCores   = 0;
    uint8_t    middleCores   = 0;
    uint32_t   cacheSizeKB   = 0;   // L2 per-cluster
    bool       simd128Bit    = false;
    ARM64SIMDExt bestSIMD    = ARM64SIMDExt::NEON;
};

struct ARM64DecodeProfile {
    ARM64SIMDExt   simdExt     = ARM64SIMDExt::NEON;
    ARM64CoreType  preferCore  = ARM64CoreType::BigCore;
    ARM64ThermalHint thermal   = ARM64ThermalHint::Balanced;
    float          expectedSpeedup = 1.0f; // vs scalar baseline
};

class ARM64PerformanceOptimizer {
public:
    static const wchar_t* SIMDExtName(ARM64SIMDExt s) {
        switch(s) {
            case ARM64SIMDExt::NEON: return L"NEON (128-bit)";
            case ARM64SIMDExt::SVE:  return L"SVE";
            case ARM64SIMDExt::SVE2: return L"SVE2";
            case ARM64SIMDExt::SME:  return L"SME";
            default: return L"Scalar";
        }
    }
    static const wchar_t* CoreTypeName(ARM64CoreType c) {
        switch(c) {
            case ARM64CoreType::BigCore:    return L"Performance Core";
            case ARM64CoreType::LittleCore: return L"Efficiency Core";
            case ARM64CoreType::MiddleCore: return L"Balance Core";
            default: return L"Unknown";
        }
    }
    static const wchar_t* ThermalHintName(ARM64ThermalHint h) {
        switch(h) {
            case ARM64ThermalHint::Burst:       return L"Burst";
            case ARM64ThermalHint::Sustained:   return L"Sustained";
            case ARM64ThermalHint::Balanced:    return L"Balanced";
            case ARM64ThermalHint::Efficiency:  return L"Efficiency";
            case ARM64ThermalHint::Silent:      return L"Silent";
            default: return L"Unknown";
        }
    }
    static constexpr size_t SIMDExtCount()    { return static_cast<size_t>(ARM64SIMDExt::COUNT); }
    static constexpr size_t CoreTypeCount()   { return static_cast<size_t>(ARM64CoreType::COUNT); }
    static constexpr size_t ThermalHintCount(){ return static_cast<size_t>(ARM64ThermalHint::COUNT); }
};

}} // namespace ExplorerLens::Engine

