#pragma once
//==============================================================================
// SIMDAccelerator — Sprint 208
// SIMD-accelerated image processing: SSE4.2, AVX2, NEON intrinsics
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>

namespace DarkThumbs { namespace Engine {

/// SIMD instruction set level
enum class SIMDLevel : uint8_t {
    None   = 0,
    SSE2   = 1,
    SSE41  = 2,
    SSE42  = 3,
    AVX    = 4,
    AVX2   = 5,
    AVX512 = 6,
    NEON   = 7   // ARM64
};

/// SIMD operation type
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

/// SIMD capability report
struct SIMDCapabilities {
    SIMDLevel maxLevel = SIMDLevel::None;
    bool hasSSE2 = false;
    bool hasSSE41 = false;
    bool hasSSE42 = false;
    bool hasAVX = false;
    bool hasAVX2 = false;
    bool hasAVX512 = false;
    bool hasNEON = false;
    uint32_t cacheLineSize = 64;
    uint32_t l1CacheKB = 0;
    uint32_t l2CacheKB = 0;
};

/// Benchmark result for a SIMD op
struct SIMDBenchmark {
    SIMDOp op = SIMDOp::Resize;
    SIMDLevel level = SIMDLevel::None;
    double scalarMs = 0.0;     // Scalar fallback time
    double simdMs = 0.0;       // SIMD time
    double speedup = 0.0;      // simd vs scalar
    uint64_t pixelsProcessed = 0;
};

//------------------------------------------------------------------------------
class SIMDAccelerator {
public:
    SIMDAccelerator();
    ~SIMDAccelerator() = default;

    // Detect capabilities
    SIMDCapabilities DetectCapabilities() const;
    SIMDLevel GetMaxLevel() const { return m_maxLevel; }

    // Image operations (dispatched to best SIMD level)
    bool ResizeBilinear(const uint8_t* src, uint32_t srcW, uint32_t srcH,
                        uint8_t* dst, uint32_t dstW, uint32_t dstH, uint32_t channels);
    bool AlphaBlend(const uint8_t* fg, const uint8_t* bg, uint8_t* dst,
                    uint32_t width, uint32_t height);
    bool ColorConvertRGBAToBGRA(const uint8_t* src, uint8_t* dst, uint32_t pixelCount);
    bool ApplySharpen(uint8_t* data, uint32_t width, uint32_t height, uint32_t channels,
                      float strength);
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

}} // namespace DarkThumbs::Engine
