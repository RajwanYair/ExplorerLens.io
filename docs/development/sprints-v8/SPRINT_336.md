# Sprint 336: ARM64 Performance Optimizer

**Status:** ✅ Complete
**Component:** `Engine/Utils/ARM64PerformanceOptimizer.h`
**Tests:** 5 (TestARM64Opt_SIMDExtNames, TestARM64Opt_CoreTypeNames, TestARM64Opt_ThermalNames, TestARM64Opt_SIMDCount, TestARM64Opt_CoreTypeCount)

## Overview
ARM64-specific SIMD (NEON/SVE) optimisation and big.LITTLE heterogeneous scheduling for Snapdragon X, Apple Silicon via Parallels, and Ampere platforms.

## Key Features
- ARM64SIMDExt: NEON, SVE, SVE2, SME, Crypto, CRC32, DotProd (7 extensions)
- ARM64CoreType: Performance, Efficiency, Balanced, GPU, NPU
- ARM64ThermalHint: Cool, Nominal, Warm, Hot, Throttled
- Runtime CPUID probing and dispatch table selection for optimal code path
