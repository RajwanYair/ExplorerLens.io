// FormatMigrationAdvisor.h — File Format Migration Advisory Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Analyzes files and recommends modern format alternatives for better
// compression, quality, or compatibility. E.g., BMP->PNG, JPEG->AVIF,
// TIFF->JXL, ZIP->ZSTD. Provides estimated savings and migration paths.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct MigrationRecommendation {
    std::string sourceFormat;
    std::string targetFormat;
    double      estimatedSizeReduction = 0.0;  // 0.0-1.0 (50% = 0.5)
    double      qualityRetention = 1.0;        // 0.0-1.0
    std::string rationale;
    bool        isLossless = true;
    uint32_t    toolAvailability = 0;  // 0-5 stars
};

struct MigrationAnalysis {
    std::string currentFormat;
    uint64_t    currentSizeBytes = 0;
    uint32_t    recommendationCount = 0;
    std::vector<MigrationRecommendation> recommendations;
};

struct MigrationStats {
    uint32_t filesAnalyzed = 0;
    uint32_t migrationsRecommended = 0;
    uint64_t potentialSavingsBytes = 0;
};

class FormatMigrationAdvisor {
public:
    FormatMigrationAdvisor() = default;
    ~FormatMigrationAdvisor() = default;

    static const wchar_t* GetName() { return L"FormatMigrationAdvisor"; }

    /// Analyze a file and produce migration recommendations.
    MigrationAnalysis Analyze(const std::string& format, uint64_t sizeBytes) const {
        MigrationAnalysis analysis;
        analysis.currentFormat = format;
        analysis.currentSizeBytes = sizeBytes;

        if (format == "BMP") {
            analysis.recommendations.push_back({
                "BMP", "PNG", 0.70, 1.0, "Lossless compression, universally supported", true, 5
                });
            analysis.recommendations.push_back({
                "BMP", "WebP", 0.80, 0.99, "Superior lossless compression", true, 4
                });
        }
        else if (format == "JPEG" || format == "JPG") {
            analysis.recommendations.push_back({
                "JPEG", "AVIF", 0.40, 0.98, "30-50% smaller at equivalent quality", false, 3
                });
            analysis.recommendations.push_back({
                "JPEG", "JXL", 0.35, 1.0, "Lossless JPEG recompression possible", true, 2
                });
        }
        else if (format == "PNG") {
            analysis.recommendations.push_back({
                "PNG", "WebP", 0.30, 1.0, "Better lossless compression", true, 4
                });
            analysis.recommendations.push_back({
                "PNG", "JXL", 0.40, 1.0, "Best lossless compression ratio", true, 2
                });
        }
        else if (format == "TIFF") {
            analysis.recommendations.push_back({
                "TIFF", "JXL", 0.50, 1.0, "Modern replacement with rich metadata", true, 2
                });
        }
        else if (format == "GIF") {
            analysis.recommendations.push_back({
                "GIF", "WebP", 0.60, 1.0, "Animated WebP: better compression + more colors", true, 4
                });
            analysis.recommendations.push_back({
                "GIF", "AVIF", 0.70, 0.99, "Animated AVIF: best compression", false, 2
                });
        }
        else if (format == "ZIP") {
            analysis.recommendations.push_back({
                "ZIP", "ZSTD", 0.25, 1.0, "5-10x faster decompression, similar ratio", true, 3
                });
        }
        else if (format == "RAR") {
            analysis.recommendations.push_back({
                "RAR", "ZSTD", 0.10, 1.0, "Open format, faster decompression", true, 3
                });
        }

        analysis.recommendationCount = static_cast<uint32_t>(analysis.recommendations.size());

        m_stats.filesAnalyzed++;
        m_stats.migrationsRecommended += analysis.recommendationCount;
        for (const auto& rec : analysis.recommendations) {
            m_stats.potentialSavingsBytes +=
                static_cast<uint64_t>(sizeBytes * rec.estimatedSizeReduction);
        }

        return analysis;
    }

    MigrationStats GetStats() const { return m_stats; }

private:
    mutable MigrationStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
