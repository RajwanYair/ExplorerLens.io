// CompressedStreamAnalyzer.h — Compression Characteristic Analysis
// Copyright (c) 2026 ExplorerLens Project
//
// Analyzes compression characteristics of archive streams including ratio
// calculations, efficiency scoring, and compression classification.
//
#pragma once

#include <string>
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// Classification of compression effectiveness
enum class CompressionClass : uint8_t {
    None,           // No compression (ratio >= 1.0)
    Minimal,        // < 10% savings
    Low,            // 10-30% savings
    Moderate,       // 30-60% savings
    High,           // 60-80% savings
    Extreme         // > 80% savings
};

// Analyzes compression characteristics of archive streams.
// Provides ratio calculations, efficiency scoring, and classification
// based on compressed vs original sizes.
class CompressedStreamAnalyzer {
public:
    CompressedStreamAnalyzer()
        : m_compressedSize(0)
        , m_originalSize(0) {
    }

    // Set the compressed (on-disk) size in bytes
    void SetCompressedSize(uint64_t bytes) {
        m_compressedSize = bytes;
    }

    // Set the original (uncompressed) size in bytes
    void SetOriginalSize(uint64_t bytes) {
        m_originalSize = bytes;
    }

    // Get the compression ratio (original / compressed).
    // Returns 1.0 if sizes are zero or invalid.
    // Higher values mean better compression.
    double GetCompressionRatio() const {
        if (m_compressedSize == 0 || m_originalSize == 0) return 1.0;
        return static_cast<double>(m_originalSize) / static_cast<double>(m_compressedSize);
    }

    // Get compression efficiency as a 0.0 - 1.0 score.
    // 0.0 = no compression benefit, 1.0 = perfect compression.
    double GetCompressionEfficiency() const {
        if (m_originalSize == 0) return 0.0;
        if (m_compressedSize >= m_originalSize) return 0.0;

        double ratio = 1.0 - (static_cast<double>(m_compressedSize) / static_cast<double>(m_originalSize));
        return (std::max)(0.0, (std::min)(1.0, ratio));
    }

    // Classify the compression level based on savings percentage
    CompressionClass ClassifyCompression() const {
        double savings = GetSavingsPercent();

        if (savings <= 0.0)   return CompressionClass::None;
        if (savings < 10.0)   return CompressionClass::Minimal;
        if (savings < 30.0)   return CompressionClass::Low;
        if (savings < 60.0)   return CompressionClass::Moderate;
        if (savings < 80.0)   return CompressionClass::High;
        return CompressionClass::Extreme;
    }

    // Get the space savings as a percentage (0.0 - 100.0).
    // Returns 0.0 if no compression benefit.
    double GetSavingsPercent() const {
        if (m_originalSize == 0) return 0.0;
        if (m_compressedSize >= m_originalSize) return 0.0;

        double savings = (1.0 - static_cast<double>(m_compressedSize) / static_cast<double>(m_originalSize)) * 100.0;
        return (std::max)(0.0, (std::min)(100.0, savings));
    }

    // Get a human-readable classification string
    static std::string ClassifyToString(CompressionClass cls) {
        switch (cls) {
        case CompressionClass::None:     return "None";
        case CompressionClass::Minimal:  return "Minimal";
        case CompressionClass::Low:      return "Low";
        case CompressionClass::Moderate: return "Moderate";
        case CompressionClass::High:     return "High";
        case CompressionClass::Extreme:  return "Extreme";
        default:                         return "Unknown";
        }
    }

    // Get current sizes
    uint64_t GetCompressedSize() const { return m_compressedSize; }
    uint64_t GetOriginalSize() const { return m_originalSize; }

    // Reset analyzer state
    void Reset() {
        m_compressedSize = 0;
        m_originalSize = 0;
    }

private:
    uint64_t m_compressedSize;
    uint64_t m_originalSize;
};

} // namespace Engine
} // namespace ExplorerLens
