// DecoderOutputValidator.h — HBITMAP Output Validation
// Copyright (c) 2026 ExplorerLens Project
//
// Post-decode validator that examines each HBITMAP output for structural
// correctness (null handle, zero dimensions, invalid bit depth, corrupted
// BITMAPINFOHEADER) and content quality (all-black / all-white detection
// via sparse pixel sampling). Dimension tolerance is configurable. Stats
// per failure reason are tracked atomically for thread safety.
//
// Thread-safe singleton.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <atomic>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class OutputValidationFailure : uint32_t {
    None = 0,
    NullBitmap = 1,
    ZeroDimensions = 2,
    DimensionMismatch = 3,
    NullPixelData = 4,
    AllBlackPixels = 5,
    AllWhitePixels = 6,
    InvalidBitDepth = 7,
    CorruptedHeader = 8,
    TruncatedData = 9,
    Count = 10
};

static const wchar_t* OutputValidationFailureName(OutputValidationFailure f)
{
    static const wchar_t* names[] = {
        L"None",           L"NullBitmap",     L"ZeroDimensions",  L"DimensionMismatch", L"NullPixelData",
        L"AllBlackPixels", L"AllWhitePixels", L"InvalidBitDepth", L"CorruptedHeader",   L"TruncatedData"};
    auto idx = static_cast<uint32_t>(f);
    return (idx < static_cast<uint32_t>(OutputValidationFailure::Count)) ? names[idx] : L"Unknown";
}

struct OutputValidationResult
{
    bool passed = false;
    OutputValidationFailure failure = OutputValidationFailure::None;
    uint32_t actualWidth = 0;
    uint32_t actualHeight = 0;
    uint32_t bitDepth = 0;
    double uniqueColorRatio = 0.0;  // Fraction of sampled pixels that differ
};

struct OutputValidatorConfig
{
    bool checkAllBlack = true;
    bool checkAllWhite = true;
    bool checkDimensions = true;
    uint32_t dimensionTolerance = 2;    // Pixels tolerance for dimension check
    uint32_t samplePixelCount = 64;     // How many pixels to sample for color check
    double monochromeThreshold = 0.01;  // <1% unique colors = monochrome
};

struct OutputValidatorStats
{
    std::atomic<uint64_t> totalValidated{0};
    std::atomic<uint64_t> totalPassed{0};
    std::atomic<uint64_t> totalFailed{0};
    std::atomic<uint64_t> failureCounts[static_cast<size_t>(OutputValidationFailure::Count)]{};
};

// ========================================================================
// DecoderOutputValidator — Validates decoder output correctness
// ========================================================================
class DecoderOutputValidator
{
  public:
    static DecoderOutputValidator& Instance()
    {
        static DecoderOutputValidator instance;
        return instance;
    }

    void Initialize(const OutputValidatorConfig& config = {})
    {
        m_config = config;
        m_initialized = true;
    }

    bool IsInitialized() const
    {
        return m_initialized;
    }

    // Validate an HBITMAP output
    OutputValidationResult ValidateBitmap(HBITMAP hBitmap, uint32_t requestedWidth, uint32_t requestedHeight)
    {
        OutputValidationResult result;
        m_stats.totalValidated.fetch_add(1, std::memory_order_relaxed);

        // Null check
        if (!hBitmap) {
            result.failure = OutputValidationFailure::NullBitmap;
            RecordFailure(result.failure);
            return result;
        }

        // Get bitmap info
        BITMAP bm = {};
        if (GetObject(hBitmap, sizeof(BITMAP), &bm) == 0) {
            result.failure = OutputValidationFailure::CorruptedHeader;
            RecordFailure(result.failure);
            return result;
        }

        result.actualWidth = static_cast<uint32_t>(bm.bmWidth);
        result.actualHeight = static_cast<uint32_t>(std::abs(bm.bmHeight));
        result.bitDepth = static_cast<uint32_t>(bm.bmBitsPixel);

        // Zero dimensions
        if (result.actualWidth == 0 || result.actualHeight == 0) {
            result.failure = OutputValidationFailure::ZeroDimensions;
            RecordFailure(result.failure);
            return result;
        }

        // Dimension mismatch check
        if (m_config.checkDimensions && requestedWidth > 0 && requestedHeight > 0) {
            int32_t dw = static_cast<int32_t>(result.actualWidth) - static_cast<int32_t>(requestedWidth);
            int32_t dh = static_cast<int32_t>(result.actualHeight) - static_cast<int32_t>(requestedHeight);
            if (static_cast<uint32_t>(std::abs(dw)) > m_config.dimensionTolerance
                || static_cast<uint32_t>(std::abs(dh)) > m_config.dimensionTolerance) {
                result.failure = OutputValidationFailure::DimensionMismatch;
                RecordFailure(result.failure);
                return result;
            }
        }

        // Bit depth validation
        if (result.bitDepth != 32 && result.bitDepth != 24 && result.bitDepth != 16 && result.bitDepth != 8) {
            result.failure = OutputValidationFailure::InvalidBitDepth;
            RecordFailure(result.failure);
            return result;
        }

        // Pixel data check (if accessible)
        if (bm.bmBits) {
            ValidatePixelContent(bm, result);
            if (result.failure != OutputValidationFailure::None) {
                RecordFailure(result.failure);
                return result;
            }
        }

        result.passed = true;
        m_stats.totalPassed.fetch_add(1, std::memory_order_relaxed);
        return result;
    }

    // Lightweight validation (no pixel sampling)
    OutputValidationResult ValidateBasic(HBITMAP hBitmap)
    {
        return ValidateBitmap(hBitmap, 0, 0);
    }

    // Get stats
    uint64_t GetTotalValidated() const
    {
        return m_stats.totalValidated.load(std::memory_order_relaxed);
    }
    uint64_t GetTotalPassed() const
    {
        return m_stats.totalPassed.load(std::memory_order_relaxed);
    }
    uint64_t GetTotalFailed() const
    {
        return m_stats.totalFailed.load(std::memory_order_relaxed);
    }

    double GetPassRate() const
    {
        uint64_t total = GetTotalValidated();
        return (total > 0) ? (static_cast<double>(GetTotalPassed()) / static_cast<double>(total)) : 0.0;
    }

  private:
    DecoderOutputValidator() = default;

    void RecordFailure(OutputValidationFailure f)
    {
        m_stats.totalFailed.fetch_add(1, std::memory_order_relaxed);
        m_stats.failureCounts[static_cast<uint32_t>(f)].fetch_add(1, std::memory_order_relaxed);
    }

    void ValidatePixelContent(const BITMAP& bm, OutputValidationResult& result)
    {
        if (!bm.bmBits || bm.bmBitsPixel < 24)
            return;

        uint32_t bytesPerPixel = bm.bmBitsPixel / 8;
        uint32_t stride = static_cast<uint32_t>(bm.bmWidthBytes);
        uint32_t width = static_cast<uint32_t>(bm.bmWidth);
        uint32_t height = static_cast<uint32_t>(std::abs(bm.bmHeight));
        uint32_t totalPixels = width * height;

        if (totalPixels == 0)
            return;

        uint32_t sampleCount = (std::min)(m_config.samplePixelCount, totalPixels);
        uint32_t step = totalPixels / sampleCount;
        if (step == 0)
            step = 1;

        bool allBlack = true;
        bool allWhite = true;
        uint32_t uniqueColors = 0;
        uint32_t lastColor = 0xFFFFFFFF;
        const uint8_t* bits = static_cast<const uint8_t*>(bm.bmBits);

        for (uint32_t i = 0; i < sampleCount; ++i) {
            uint32_t pixelIndex = i * step;
            uint32_t y = pixelIndex / width;
            uint32_t x = pixelIndex % width;
            if (y >= height)
                break;

            const uint8_t* pixel = bits + y * stride + x * bytesPerPixel;
            uint8_t b = pixel[0], g = pixel[1], r = pixel[2];

            if (r > 5 || g > 5 || b > 5)
                allBlack = false;
            if (r < 250 || g < 250 || b < 250)
                allWhite = false;

            uint32_t color = (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | b;
            if (color != lastColor) {
                uniqueColors++;
                lastColor = color;
            }
        }

        result.uniqueColorRatio =
            (sampleCount > 0) ? (static_cast<double>(uniqueColors) / static_cast<double>(sampleCount)) : 0.0;

        if (m_config.checkAllBlack && allBlack) {
            result.failure = OutputValidationFailure::AllBlackPixels;
        } else if (m_config.checkAllWhite && allWhite) {
            result.failure = OutputValidationFailure::AllWhitePixels;
        }
    }

    OutputValidatorConfig m_config;
    OutputValidatorStats m_stats;
    bool m_initialized = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
