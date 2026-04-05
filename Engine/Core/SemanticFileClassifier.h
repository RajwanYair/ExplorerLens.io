// SemanticFileClassifier.h — ML-Based File Type Classification
// Copyright (c) 2026 ExplorerLens Project
//
// ML-based file type classification beyond extension. Uses magic bytes, header
// patterns, and entropy analysis to classify files into categories.
//
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SemanticCategory : uint8_t {
    Unknown,
    RasterImage,
    VectorGraphic,
    Document,
    Archive,
    Video,
    Audio,
    ThreeDModel,
    SourceCode,
    Executable,
    Font,
    Database,
    ScientificData,
    CADDrawing,
    Encrypted
};

struct SemanticClassificationResult
{
    SemanticCategory category = SemanticCategory::Unknown;
    double confidence = 0.0;
    std::string detectedFormat;
    double entropy = 0.0;
    bool isBinary = false;
};

struct SemanticMagicSignature
{
    std::vector<uint8_t> bytes;
    size_t offset = 0;
    SemanticCategory category = SemanticCategory::Unknown;
    std::string formatName;
};

class SemanticFileClassifier
{
  public:
    static SemanticFileClassifier& Instance()
    {
        static SemanticFileClassifier instance;
        return instance;
    }

    inline SemanticClassificationResult Classify(const uint8_t* data, size_t size) const
    {
        SemanticClassificationResult result;
        if (!data || size == 0)
            return result;

        result.entropy = ComputeShannonEntropy(data, size);
        result.isBinary = DetectBinary(data, size);

        auto magicResult = MatchMagicBytes(data, size);
        if (magicResult.confidence > 0.0) {
            result.category = magicResult.category;
            result.confidence = magicResult.confidence;
            result.detectedFormat = magicResult.detectedFormat;
            return result;
        }

        result = ClassifyByEntropy(result);
        return result;
    }

    inline double ComputeShannonEntropy(const uint8_t* data, size_t size) const
    {
        if (!data || size == 0)
            return 0.0;

        std::array<uint64_t, 256> freq{};
        for (size_t i = 0; i < size; ++i) {
            freq[data[i]]++;
        }

        double entropy = 0.0;
        for (auto count : freq) {
            if (count > 0) {
                double p = static_cast<double>(count) / static_cast<double>(size);
                entropy -= p * std::log2(p);
            }
        }
        return entropy;
    }

    inline bool DetectBinary(const uint8_t* data, size_t size) const
    {
        if (!data || size == 0)
            return false;
        size_t checkLen = (std::min)(size, static_cast<size_t>(8192));
        size_t nullCount = 0;
        size_t controlCount = 0;
        for (size_t i = 0; i < checkLen; ++i) {
            if (data[i] == 0)
                ++nullCount;
            else if (data[i] < 0x09 || (data[i] > 0x0D && data[i] < 0x20))
                ++controlCount;
        }
        double binaryRatio = static_cast<double>(nullCount + controlCount) / static_cast<double>(checkLen);
        return binaryRatio > 0.05;
    }

    inline std::string CategoryToString(SemanticCategory cat) const
    {
        switch (cat) {
            case SemanticCategory::RasterImage:
                return "Raster Image";
            case SemanticCategory::VectorGraphic:
                return "Vector Graphic";
            case SemanticCategory::Document:
                return "Document";
            case SemanticCategory::Archive:
                return "Archive";
            case SemanticCategory::Video:
                return "Video";
            case SemanticCategory::Audio:
                return "Audio";
            case SemanticCategory::ThreeDModel:
                return "3D Model";
            case SemanticCategory::SourceCode:
                return "Source Code";
            case SemanticCategory::Executable:
                return "Executable";
            case SemanticCategory::Font:
                return "Font";
            case SemanticCategory::Database:
                return "Database";
            case SemanticCategory::ScientificData:
                return "Scientific Data";
            case SemanticCategory::CADDrawing:
                return "CAD Drawing";
            case SemanticCategory::Encrypted:
                return "Encrypted";
            default:
                return "Unknown";
        }
    }

  private:
    SemanticFileClassifier() = default;

    inline SemanticClassificationResult MatchMagicBytes(const uint8_t* data, size_t size) const
    {
        SemanticClassificationResult result;

        struct MagicEntry
        {
            const uint8_t* sig;
            size_t sigLen;
            size_t offset;
            SemanticCategory category;
            const char* format;
        };

        static const uint8_t SIG_PNG[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
        static const uint8_t SIG_JPEG[] = {0xFF, 0xD8, 0xFF};
        static const uint8_t SIG_GIF[] = {0x47, 0x49, 0x46, 0x38};
        static const uint8_t SIG_BMP[] = {0x42, 0x4D};
        static const uint8_t SIG_WEBP[] = {0x57, 0x45, 0x42, 0x50};
        static const uint8_t SIG_PDF[] = {0x25, 0x50, 0x44, 0x46};
        static const uint8_t SIG_ZIP[] = {0x50, 0x4B, 0x03, 0x04};
        static const uint8_t SIG_RAR[] = {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07};
        static const uint8_t SIG_7Z[] = {0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C};
        static const uint8_t SIG_EXE[] = {0x4D, 0x5A};
        static const uint8_t SIG_TIFF_LE[] = {0x49, 0x49, 0x2A, 0x00};
        static const uint8_t SIG_TIFF_BE[] = {0x4D, 0x4D, 0x00, 0x2A};
        static const uint8_t SIG_EXR[] = {0x76, 0x2F, 0x31, 0x01};
        static const uint8_t SIG_GZIP[] = {0x1F, 0x8B};
        static const uint8_t SIG_OGG[] = {0x4F, 0x67, 0x67, 0x53};

        static const MagicEntry entries[] = {
            {SIG_PNG, 8, 0, SemanticCategory::RasterImage, "PNG"},
            {SIG_JPEG, 3, 0, SemanticCategory::RasterImage, "JPEG"},
            {SIG_GIF, 4, 0, SemanticCategory::RasterImage, "GIF"},
            {SIG_BMP, 2, 0, SemanticCategory::RasterImage, "BMP"},
            {SIG_WEBP, 4, 8, SemanticCategory::RasterImage, "WebP"},
            {SIG_PDF, 4, 0, SemanticCategory::Document, "PDF"},
            {SIG_ZIP, 4, 0, SemanticCategory::Archive, "ZIP"},
            {SIG_RAR, 6, 0, SemanticCategory::Archive, "RAR"},
            {SIG_7Z, 6, 0, SemanticCategory::Archive, "7z"},
            {SIG_EXE, 2, 0, SemanticCategory::Executable, "PE/EXE"},
            {SIG_TIFF_LE, 4, 0, SemanticCategory::RasterImage, "TIFF"},
            {SIG_TIFF_BE, 4, 0, SemanticCategory::RasterImage, "TIFF"},
            {SIG_EXR, 4, 0, SemanticCategory::RasterImage, "OpenEXR"},
            {SIG_GZIP, 2, 0, SemanticCategory::Archive, "GZIP"},
            {SIG_OGG, 4, 0, SemanticCategory::Audio, "OGG"},
        };

        for (const auto& entry : entries) {
            if (size >= entry.offset + entry.sigLen) {
                bool match = true;
                for (size_t i = 0; i < entry.sigLen; ++i) {
                    if (data[entry.offset + i] != entry.sig[i]) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    result.category = entry.category;
                    result.detectedFormat = entry.format;
                    result.confidence = 0.95;
                    return result;
                }
            }
        }
        return result;
    }

    inline SemanticClassificationResult ClassifyByEntropy(SemanticClassificationResult partial) const
    {
        if (partial.entropy > 7.5) {
            partial.category = SemanticCategory::Encrypted;
            partial.confidence = 0.6;
            partial.detectedFormat = "High-entropy (encrypted/compressed)";
        } else if (!partial.isBinary && partial.entropy < 5.0) {
            partial.category = SemanticCategory::SourceCode;
            partial.confidence = 0.4;
            partial.detectedFormat = "Text-like content";
        } else if (partial.isBinary) {
            partial.category = SemanticCategory::Unknown;
            partial.confidence = 0.2;
            partial.detectedFormat = "Binary (unrecognized)";
        }
        return partial;
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
