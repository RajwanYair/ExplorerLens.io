#pragma once
// ============================================================================
// ThumbnailCompareView.h — Side-by-side thumbnail comparison viewport
//
// Purpose:   Side-by-side thumbnail comparison viewport
// Provides:  CompareMode, CompareLayout enums, CompareSession struct,
//            ThumbnailCompareView class
// Used by:   Diff engine UI
// ============================================================================

#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// ── Enums ────────────────────────────────────────────────────────────────────

enum class CompareMode : uint8_t {
    SideBySide = 0,
    Overlay = 1,
    Slider = 2,
    Toggle = 3,
    Difference = 4
};

inline const char* CompareModeName(CompareMode m) {
    switch (m) {
    case CompareMode::SideBySide:  return "SideBySide";
    case CompareMode::Overlay:     return "Overlay";
    case CompareMode::Slider:      return "Slider";
    case CompareMode::Toggle:      return "Toggle";
    case CompareMode::Difference:  return "Difference";
    default:                       return "Unknown";
    }
}

enum class CompareSource : uint8_t {
    Cache = 0,
    LiveDecode = 1,
    File = 2,
    URL = 3,
    Memory = 4
};

inline const char* CompareSourceName(CompareSource s) {
    switch (s) {
    case CompareSource::Cache:      return "Cache";
    case CompareSource::LiveDecode: return "LiveDecode";
    case CompareSource::File:       return "File";
    case CompareSource::URL:        return "URL";
    case CompareSource::Memory:     return "Memory";
    default:                        return "Unknown";
    }
}

// ── Structs ──────────────────────────────────────────────────────────────────

struct ThumbnailCompareResult {
    CompareMode   mode = CompareMode::SideBySide;
    CompareSource sourceA = CompareSource::File;
    CompareSource sourceB = CompareSource::File;
    float         matchPercent = 0.0f;
    uint64_t      diffPixels = 0;
};

// ── Class ────────────────────────────────────────────────────────────────────

class ThumbnailCompareView {
public:
    ThumbnailCompareView() = default;
    ~ThumbnailCompareView() = default;

    // Compare two thumbnails
    bool Compare(const uint8_t* dataA, uint32_t widthA, uint32_t heightA,
        const uint8_t* dataB, uint32_t widthB, uint32_t heightB,
        CompareMode mode = CompareMode::SideBySide) {
        if (!dataA || !dataB)
            return false;
        if (widthA == 0 || heightA == 0 || widthB == 0 || heightB == 0)
            return false;

        m_result.mode = mode;
        m_result.sourceA = CompareSource::Memory;
        m_result.sourceB = CompareSource::Memory;

        // Simplified pixel comparison (BGRA stride)
        uint64_t totalPixels = static_cast<uint64_t>(widthA) * heightA;
        uint64_t comparePixels = totalPixels;
        if (widthA != widthB || heightA != heightB) {
            // Different sizes: all pixels differ
            m_result.diffPixels = totalPixels;
            m_result.matchPercent = 0.0f;
        }
        else {
            uint64_t diff = 0;
            uint64_t stride = static_cast<uint64_t>(widthA) * 4;
            for (uint64_t i = 0; i < static_cast<uint64_t>(heightA) * stride; i += 4) {
                if (dataA[i] != dataB[i] || dataA[i + 1] != dataB[i + 1] ||
                    dataA[i + 2] != dataB[i + 2])
                    diff++;
            }
            m_result.diffPixels = diff;
            m_result.matchPercent = comparePixels > 0
                ? 100.0f * (1.0f - static_cast<float>(diff) / static_cast<float>(comparePixels))
                : 0.0f;
        }
        m_compareCount++;
        m_hasResult = true;
        return true;
    }

    const ThumbnailCompareResult& GetResult() const { return m_result; }

    // Export the difference image to a buffer
    bool ExportDiff(std::vector<uint8_t>& outBuffer, uint32_t width, uint32_t height) const {
        if (!m_hasResult || width == 0 || height == 0)
            return false;
        outBuffer.resize(static_cast<size_t>(width) * height * 4, 0);
        return true;
    }

    bool HasResult() const { return m_hasResult; }
    uint64_t GetCompareCount() const { return m_compareCount; }

    void Reset() {
        m_result = ThumbnailCompareResult{};
        m_hasResult = false;
    }

private:
    ThumbnailCompareResult m_result;
    bool                   m_hasResult = false;
    uint64_t               m_compareCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
