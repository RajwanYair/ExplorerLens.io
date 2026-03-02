//==============================================================================
// ExplorerLens Engine — Format Conversion & Export Pipeline
//
// Provides image format conversion between supported formats, batch
// conversion with queue management, quality/size optimization,
// EXIF metadata preservation, output profile configuration, and
// conversion result tracking.
//==============================================================================
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace ExplorerLens::Engine::Codec {

//==============================================================================
// Output Format — Supported conversion targets
//==============================================================================

enum class OutputFormat : uint8_t {
    JPEG,
    PNG,
    WebP,
    JXL,
    HEIF,
    AVIF,
    TIFF,
    BMP,
    QOI
};

inline const char* OutputFormatName(OutputFormat f) {
    switch (f) {
    case OutputFormat::JPEG: return "JPEG";
    case OutputFormat::PNG: return "PNG";
    case OutputFormat::WebP: return "WebP";
    case OutputFormat::JXL: return "JPEG XL";
    case OutputFormat::HEIF: return "HEIF";
    case OutputFormat::AVIF: return "AVIF";
    case OutputFormat::TIFF: return "TIFF";
    case OutputFormat::BMP: return "BMP";
    case OutputFormat::QOI: return "QOI";
    default: return "Unknown";
    }
}

inline const char* OutputFormatExtension(OutputFormat f) {
    switch (f) {
    case OutputFormat::JPEG: return ".jpg";
    case OutputFormat::PNG: return ".png";
    case OutputFormat::WebP: return ".webp";
    case OutputFormat::JXL: return ".jxl";
    case OutputFormat::HEIF: return ".heif";
    case OutputFormat::AVIF: return ".avif";
    case OutputFormat::TIFF: return ".tiff";
    case OutputFormat::BMP: return ".bmp";
    case OutputFormat::QOI: return ".qoi";
    default: return "";
    }
}

inline bool SupportsLossless(OutputFormat f) {
    switch (f) {
    case OutputFormat::PNG:
    case OutputFormat::WebP:
    case OutputFormat::JXL:
    case OutputFormat::AVIF:
    case OutputFormat::TIFF:
    case OutputFormat::BMP:
    case OutputFormat::QOI:
        return true;
    case OutputFormat::JPEG:
    case OutputFormat::HEIF:
    default:
        return false;
    }
}

inline bool SupportsAlpha(OutputFormat f) {
    switch (f) {
    case OutputFormat::PNG:
    case OutputFormat::WebP:
    case OutputFormat::JXL:
    case OutputFormat::AVIF:
    case OutputFormat::TIFF:
    case OutputFormat::QOI:
        return true;
    case OutputFormat::JPEG:
    case OutputFormat::HEIF:
    case OutputFormat::BMP:
    default:
        return false;
    }
}

inline bool SupportsAnimation(OutputFormat f) {
    return f == OutputFormat::WebP || f == OutputFormat::JXL || f == OutputFormat::AVIF;
}

inline bool SupportsHDR(OutputFormat f) {
    return f == OutputFormat::JXL || f == OutputFormat::AVIF;
}

//==============================================================================
// Quality Preset — Named quality levels
//==============================================================================

enum class QualityPreset : uint8_t {
    Lossless, // 100% quality, no information loss
    Maximum, // 95-98% quality, near-visually-lossless
    High, // 85-90% quality, good general use
    Medium, // 70-80% quality, balanced size/quality
    Low, // 50-60% quality, smaller files
    Thumbnail // 40-50% quality, tiny files for previews
};

inline const char* QualityPresetName(QualityPreset p) {
    switch (p) {
    case QualityPreset::Lossless: return "Lossless";
    case QualityPreset::Maximum: return "Maximum";
    case QualityPreset::High: return "High";
    case QualityPreset::Medium: return "Medium";
    case QualityPreset::Low: return "Low";
    case QualityPreset::Thumbnail: return "Thumbnail";
    default: return "Unknown";
    }
}

inline int QualityPresetValue(QualityPreset p) {
    switch (p) {
    case QualityPreset::Lossless: return 100;
    case QualityPreset::Maximum: return 95;
    case QualityPreset::High: return 85;
    case QualityPreset::Medium: return 75;
    case QualityPreset::Low: return 55;
    case QualityPreset::Thumbnail: return 45;
    default: return 85;
    }
}

//==============================================================================
// Resize Mode — How to handle resolution changes
//==============================================================================

enum class ResizeMode : uint8_t {
    None, // Keep original dimensions
    FitWithin, // Scale down if larger, maintain aspect ratio
    ExactSize, // Force exact dimensions (may distort)
    FillCrop, // Fill target size, crop overflow
    Percentage // Scale by percentage
};

inline const char* ResizeModeName(ResizeMode m) {
    switch (m) {
    case ResizeMode::None: return "None";
    case ResizeMode::FitWithin: return "Fit Within";
    case ResizeMode::ExactSize: return "Exact Size";
    case ResizeMode::FillCrop: return "Fill & Crop";
    case ResizeMode::Percentage: return "Percentage";
    default: return "Unknown";
    }
}

//==============================================================================
// Metadata Handling — What to do with EXIF/XMP during conversion
//==============================================================================

enum class MetadataHandling : uint8_t {
    Preserve, // Copy all metadata
    Strip, // Remove all metadata
    Essential, // Keep only orientation, color profile
    Anonymize // Remove GPS, camera serial, keep technical
};

inline const char* MetadataHandlingName(MetadataHandling m) {
    switch (m) {
    case MetadataHandling::Preserve: return "Preserve All";
    case MetadataHandling::Strip: return "Strip All";
    case MetadataHandling::Essential: return "Essential Only";
    case MetadataHandling::Anonymize: return "Anonymize";
    default: return "Unknown";
    }
}

//==============================================================================
// Conversion Profile — Full conversion settings
//==============================================================================

struct ConversionProfile {
    std::string name;
    OutputFormat format = OutputFormat::JPEG;
    QualityPreset quality = QualityPreset::High;
    int qualityOverride = -1; // -1 = use preset
    ResizeMode resize = ResizeMode::None;
    uint32_t maxWidth = 0; // 0 = no limit
    uint32_t maxHeight = 0;
    double scalePercent = 100.0;
    MetadataHandling metadata = MetadataHandling::Preserve;
    bool preserveColorProfile = true;
    std::string outputDirectory;
    std::string fileNamePattern; // "{name}_converted.{ext}"

    int EffectiveQuality() const {
        return (qualityOverride >= 0) ? qualityOverride : QualityPresetValue(quality);
    }

    bool NeedsResize() const {
        return resize != ResizeMode::None;
    }

    //--- Preset profiles ---

    static ConversionProfile WebOptimized() {
        ConversionProfile p;
        p.name = "Web Optimized";
        p.format = OutputFormat::WebP;
        p.quality = QualityPreset::High;
        p.resize = ResizeMode::FitWithin;
        p.maxWidth = 2048;
        p.maxHeight = 2048;
        p.metadata = MetadataHandling::Essential;
        return p;
    }

    static ConversionProfile ArchiveQuality() {
        ConversionProfile p;
        p.name = "Archive Quality";
        p.format = OutputFormat::JXL;
        p.quality = QualityPreset::Lossless;
        p.metadata = MetadataHandling::Preserve;
        return p;
    }

    static ConversionProfile SocialMedia() {
        ConversionProfile p;
        p.name = "Social Media";
        p.format = OutputFormat::JPEG;
        p.quality = QualityPreset::High;
        p.resize = ResizeMode::FitWithin;
        p.maxWidth = 4096;
        p.maxHeight = 4096;
        p.metadata = MetadataHandling::Anonymize;
        return p;
    }

    static ConversionProfile ThumbnailExport() {
        ConversionProfile p;
        p.name = "Thumbnail Export";
        p.format = OutputFormat::JPEG;
        p.quality = QualityPreset::Thumbnail;
        p.resize = ResizeMode::FitWithin;
        p.maxWidth = 256;
        p.maxHeight = 256;
        p.metadata = MetadataHandling::Strip;
        return p;
    }
};

//==============================================================================
// Conversion Result — Outcome of a single conversion
//==============================================================================

struct ConversionResult {
    std::string inputPath;
    std::string outputPath;
    OutputFormat format = OutputFormat::JPEG;
    bool success = false;
    std::string error;
    uint64_t inputSize = 0;
    uint64_t outputSize = 0;
    double conversionTimeMs = 0.0;
    uint32_t outputWidth = 0;
    uint32_t outputHeight = 0;

    double SizeReduction() const {
        if (inputSize == 0) return 0.0;
        return (1.0 - static_cast<double>(outputSize) / static_cast<double>(inputSize)) * 100.0;
    }

    double CompressionRatio() const {
        if (outputSize == 0) return 0.0;
        return static_cast<double>(inputSize) / static_cast<double>(outputSize);
    }

    bool IsSmallerOutput() const {
        return outputSize > 0 && outputSize < inputSize;
    }
};

//==============================================================================
// Batch Conversion Result — Summary of a batch operation
//==============================================================================

struct BatchConversionResult {
    size_t totalFiles = 0;
    size_t succeeded = 0;
    size_t failed = 0;
    size_t skipped = 0;
    uint64_t totalInputSize = 0;
    uint64_t totalOutputSize = 0;
    double totalTimeMs = 0.0;
    std::vector<ConversionResult> results;
    std::vector<std::string> errors;

    double SuccessRate() const {
        if (totalFiles == 0) return 0.0;
        return (static_cast<double>(succeeded) / static_cast<double>(totalFiles)) * 100.0;
    }

    double OverallSizeReduction() const {
        if (totalInputSize == 0) return 0.0;
        return (1.0 - static_cast<double>(totalOutputSize)
            / static_cast<double>(totalInputSize)) * 100.0;
    }

    double ThroughputPerSecond() const {
        if (totalTimeMs <= 0.0) return 0.0;
        return (static_cast<double>(succeeded) / totalTimeMs) * 1000.0;
    }

    std::string Summary() const {
        std::ostringstream ss;
        ss << succeeded << "/" << totalFiles << " converted"
            << " (" << std::fixed << std::setprecision(1) << SuccessRate() << "%)";
        if (failed > 0) ss << ", " << failed << " failed";
        if (skipped > 0) ss << ", " << skipped << " skipped";
        ss << ", " << std::setprecision(1) << OverallSizeReduction() << "% smaller";
        return ss.str();
    }
};

//==============================================================================
// Format Compatibility Matrix — Which conversions are supported
//==============================================================================

class FormatCompatibility {
public:
    static bool CanConvertTo(const std::string& inputExt, [[maybe_unused]] OutputFormat target) {
        // All image formats can be decoded and re-encoded to any target
        // except animated → non-animated target
        static const std::unordered_set<std::string> imageExtensions = {
        ".jpg", ".jpeg", ".png", ".bmp", ".tiff", ".tif",
        ".webp", ".jxl", ".heif", ".heic", ".avif",
        ".psd", ".hdr", ".exr", ".svg", ".qoi",
        ".dng", ".cr2", ".cr3", ".nef", ".arw", ".orf",
        ".tga", ".ico", ".dds", ".gif"
        };

        auto ext = inputExt;
        std::transform(ext.begin(), ext.end(), ext.begin(),
            [](char c) { return static_cast<char>(::tolower(static_cast<unsigned char>(c))); });
        return imageExtensions.count(ext) > 0;
    }

    static std::vector<OutputFormat> SupportedOutputFormats() {
        return {
        OutputFormat::JPEG, OutputFormat::PNG, OutputFormat::WebP,
        OutputFormat::JXL, OutputFormat::HEIF, OutputFormat::AVIF,
        OutputFormat::TIFF, OutputFormat::BMP, OutputFormat::QOI
        };
    }

    static std::vector<OutputFormat> ModernFormats() {
        return {
        OutputFormat::WebP, OutputFormat::JXL,
        OutputFormat::HEIF, OutputFormat::AVIF
        };
    }

    static std::vector<OutputFormat> LosslessFormats() {
        std::vector<OutputFormat> result;
        for (auto f : SupportedOutputFormats()) {
            if (SupportsLossless(f)) result.push_back(f);
        }
        return result;
    }
};

} // namespace ExplorerLens::Engine::Codec
