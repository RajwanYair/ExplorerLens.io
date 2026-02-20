# Sprint 202 — Vulkan Compute Pipeline

**Date:** 2026-01-20  
**Version:** v10.0.0  
**Status:** ✅ Complete

## Objective
Add Vulkan compute shader support for GPU-accelerated image processing with
automatic fallback through D3D12 → D3D11 → CPU pipeline.

## Deliverables
| Artifact | Path |
|----------|------|
| Header | `Engine/GPU/VulkanComputePipeline.h` |
| Source | `Engine/GPU/VulkanComputePipeline.cpp` |
| Tests | 5 tests in `Engine/Tests/EngineTests.cpp` |
| CMake | Registered in `Engine/CMakeLists.txt` |

## Key Features
- `GPUBackend` enum: None, Vulkan, D3D12, D3D11, CPU
- `ComputeShaderType` enum: Resize, ColorConvert, ToneMap, Sharpen, Denoise, EdgeDetect
- Runtime Vulkan detection via `LoadLibraryW(L"vulkan-1.dll")`
- `VulkanDevice` info with device name, VRAM, compute queue support
- `CPUFallbackResize` — bilinear resize when GPU unavailable
- Pipeline cache with hit/miss statistics

## Tests Added (5)
1. `TestVulkan_BackendNames` — backend→name mapping
2. `TestVulkan_ShaderTypeNames` — shader type→name mapping
3. `TestVulkan_CPUFallbackResize` — bilinear CPU resize validation
4. `TestVulkan_PipelineCacheStats` — empty cache initial stats
5. `TestVulkan_ActiveBackend` — backend auto-detection

## Impact
- GPU pipeline: D3D11 only → D3D11 + D3D12 + Vulkan + CPU fallback
- Prepares for cross-vendor GPU acceleration
