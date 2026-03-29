# ADR-002: NPU & Heterogeneous Compute Routing

**Status:** Accepted  
**Date:** 2026-03-01  
**Implemented in:** v25.2.0 "Rigel-S"  
**Affected components:** `Engine/GPU/`, `Engine/Pipeline/PowerAwareScheduler.h`, `Engine/AI/NPUWarmupEngine.h`

---

## Context

AI inference in the thumbnail pipeline (smart crop, scene classification, format fingerprinting,
upscaling) ran exclusively on CPU in v24.x. On ARM64 Windows (Snapdragon X Elite) and
modern Intel platforms (Meteor Lake+), a dedicated Neural Processing Unit (NPU) sits idle.
The NPU can execute INT8 ONNX models at 48–45 TOPS — 4–8× faster than CPU-side inference
with < 1/5 the power draw.

Additionally, the Snapdragon X Elite's ARM64 CPU has NEON/SVE2 vector units that must
use dedicated decode paths — the existing x86 AVX-optimised paths are not executed.

## Decision

Implement a **multi-EP routing layer** (`ONNXEPRouter`) that selects the best available
ONNX Execution Provider at runtime:

```
Priority order:
  1. OpenVINO NPU EP   (Intel Meteor Lake / Arrow Lake)
  2. QNN EP            (Qualcomm Hexagon HTP/HTA)
  3. DirectML EP       (Any GPU — NVIDIA, AMD, Intel Arc)
  4. CUDA EP           (NVIDIA — if CUDA 12+ available)
  5. CPU EP            (Fallback, always available)
```

A `PowerAwareScheduler` further adjusts EP selection based on Windows power state:
- **AC power**: route to NPU/GPU aggressively
- **Battery**: route to CPU-only for formats below a TOPS threshold
- **Battery Saver**: CPU-only for all inference tasks

`HardwareCapabilityProfiler` fingerprints the system at startup (once, cached), producing
a ranked `HardwareProfile` used by the router at every dispatch.

## Rationale

| Approach | Throughput | Power | Complexity |
|----------|-----------|-------|-----------|
| CPU-only | 1× | high | low |
| GPU-only (DirectML) | 4× | medium | medium |
| NPU (this ADR) | 6–8× | very low | medium |
| Heterogeneous routing (this ADR) | 6–8× best path | optimal | medium |

Heterogeneous routing is superior because:
- Not all users have NPUs — fallback to GPU/CPU required anyway
- Power-aware routing extends battery life (critical for Surface/laptop users)
- `ONNXEPRouter` adds only ~0.5 ms overhead per dispatch decision

## Consequences

**Positive:**
- 6–8× inference speedup on Meteor Lake / Snapdragon X Elite platforms
- Battery life improvements of ~15% on AI-heavy thumbnail workflows
- ARM64-native decode paths eliminate the x86 emulation penalty on WoA

**Negative:**
- `HardwareCapabilityProfiler` adds ~25 ms to first-launch cold start (cached thereafter)
- NPU drivers must be up-to-date; graceful degradation required for outdated drivers
- QNN EP requires Qualcomm AI SDK DLLs — redistributed or detected-at-runtime

## Alternatives Considered

1. **DirectML-only** — Universal GPU EP, good but misses NPU power advantage and has
   higher latency than NPU for small INT8 models. Rejected as sole strategy.

2. **Vendor-specific SDKs** — Use OpenVINO C++ API directly (not ONNX). Rejected:
   creates vendor lock-in; ONNX Runtime EP abstracts all vendors uniformly.

3. **CPU-only with SIMD** — AVX-512 + AMX on server-class CPUs can match NPU throughput
   but at 5× the power draw. Accepted as fallback only.
