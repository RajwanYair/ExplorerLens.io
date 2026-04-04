#pragma once
//==============================================================================
// SIMDAccelerator
// SIMD-accelerated image processing: SSE4.2, AVX2, NEON intrinsics
// Types (SIMDLevel, SIMDCapabilities) are defined in SIMDAccelerationManager.h
//==============================================================================

#include "SIMDAccelerationManager.h"

namespace ExplorerLens {
namespace Engine {

/// SIMD operation type (unique to SIMDAccelerator)
enum class SIMDOp : uint8_t {
    Resize = 0,
    ColorConvert,
    AlphaBlend,
    Sharpen,
    GammaCorrect,
    BilinearFilter,
    BoxFilter,
    Transpose,
    OpCount
};

/// Benchmark result for a SIMD op
struct SIMDBenchmark
{
    SIMDOp op = SIMDOp::Resize;
    SIMDLevel level = SIMDLevel::None;
    double scalarMs = 0.0;  // Scalar fallback time
    double simdMs = 0.0;    // SIMD time
    double speedup = 0.0;   // simd vs scalar
    uint64_t pixelsProcessed = 0;
};

//------------------------------------------------------------------------------
class SIMDAccelerator
{
  public:
    SIMDAccelerator();
    ~SIMDAccelerator() = default;

    // Detect capabilities
    SIMDCapabilities DetectCapabilities() const;
    SIMDLevel GetMaxLevel() const
    {
        return m_maxLevel;
    }

    // Image operations (dispatched to best SIMD level)
    bool ResizeBilinear(const uint8_t* src, uint32_t srcW, uint32_t srcH, uint8_t* dst, uint32_t dstW, uint32_t dstH,
                        uint32_t channels);
    bool AlphaBlend(const uint8_t* fg, const uint8_t* bg, uint8_t* dst, uint32_t width, uint32_t height);
    bool ColorConvertRGBAToBGRA(const uint8_t* src, uint8_t* dst, uint32_t pixelCount);
    bool ApplySharpen(uint8_t* data, uint32_t width, uint32_t height, uint32_t channels, float strength);
    bool ApplyGammaCorrect(uint8_t* data, uint32_t pixelCount, float gamma);

    // Benchmarking
    SIMDBenchmark BenchmarkOp(SIMDOp op, uint32_t width, uint32_t height);
    std::vector<SIMDBenchmark> BenchmarkAll(uint32_t width, uint32_t height);

    // Info
    static const wchar_t* GetLevelName(SIMDLevel level);
    static const wchar_t* GetOpName(SIMDOp op);
    static uint32_t GetOpCount();

    // Alignment helpers
    static bool IsAligned(const void* ptr, size_t alignment = 16);
    static size_t GetOptimalAlignment(SIMDLevel level);

  private:
    SIMDLevel m_maxLevel = SIMDLevel::None;
    void DetectCPUFeatures();
    SIMDCapabilities m_caps;
};

}  // namespace Engine
}  // namespace ExplorerLens
