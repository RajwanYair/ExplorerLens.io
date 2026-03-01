#pragma once
// ThumbnailExportEngine.h — Export thumbnails to various formats/destinations
// Sprint 417 · Batch 6 · ExplorerLens v15.0.0

#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// ── Enums ────────────────────────────────────────────────────────────────────

enum class ThumbnailExportFormat : uint8_t {
    PNG = 0,
    JPEG = 1,
    WebP = 2,
    BMP = 3,
    TIFF = 4
};

inline const char* ThumbnailExportFormatName(ThumbnailExportFormat f) {
    switch (f) {
    case ThumbnailExportFormat::PNG:  return "PNG";
    case ThumbnailExportFormat::JPEG: return "JPEG";
    case ThumbnailExportFormat::WebP: return "WebP";
    case ThumbnailExportFormat::BMP:  return "BMP";
    case ThumbnailExportFormat::TIFF: return "TIFF";
    default:                          return "Unknown";
    }
}

enum class ExportDestination : uint8_t {
    File = 0,
    Clipboard = 1,
    Stream = 2,
    Memory = 3,
    Network = 4
};

inline const char* ExportDestinationName(ExportDestination d) {
    switch (d) {
    case ExportDestination::File:      return "File";
    case ExportDestination::Clipboard: return "Clipboard";
    case ExportDestination::Stream:    return "Stream";
    case ExportDestination::Memory:    return "Memory";
    case ExportDestination::Network:   return "Network";
    default:                           return "Unknown";
    }
}

// ── Structs ──────────────────────────────────────────────────────────────────

struct ThumbnailExportConfig {
    ThumbnailExportFormat format = ThumbnailExportFormat::PNG;
    ExportDestination     destination = ExportDestination::File;
    uint32_t              quality = 90;
    uint32_t              maxWidth = 1024;
    uint32_t              maxHeight = 1024;
    bool                  preserveAspect = true;
};

// ── Class ────────────────────────────────────────────────────────────────────

class ThumbnailExportEngine {
public:
    ThumbnailExportEngine() = default;
    ~ThumbnailExportEngine() = default;

    // Export a single thumbnail
    bool Export(const uint8_t* pixelData, uint32_t width, uint32_t height,
        const ThumbnailExportConfig& config,
        const std::string& outputPath = "") {
        if (!pixelData || width == 0 || height == 0)
            return false;
        if (config.destination == ExportDestination::File && outputPath.empty())
            return false;

        // Apply dimension constraints
        uint32_t outW = width, outH = height;
        if (config.preserveAspect) {
            if (outW > config.maxWidth) {
                outH = outH * config.maxWidth / outW;
                outW = config.maxWidth;
            }
            if (outH > config.maxHeight) {
                outW = outW * config.maxHeight / outH;
                outH = config.maxHeight;
            }
        }
        else {
            if (outW > config.maxWidth)  outW = config.maxWidth;
            if (outH > config.maxHeight) outH = config.maxHeight;
        }

        m_lastWidth = outW;
        m_lastHeight = outH;
        m_lastFormat = config.format;
        m_exportCount++;
        return true;
    }

    // Batch export multiple thumbnails (returns count of successful exports)
    uint32_t BatchExport(const std::vector<std::pair<const uint8_t*, std::pair<uint32_t, uint32_t>>>& items,
        const ThumbnailExportConfig& config,
        const std::string& outputDir = "") {
        uint32_t success = 0;
        uint32_t idx = 0;
        for (const auto& item : items) {
            std::string path = outputDir.empty()
                ? ""
                : outputDir + "/thumb_" + std::to_string(idx) + ".png";
            if (Export(item.first, item.second.first, item.second.second, config, path))
                success++;
            idx++;
        }
        return success;
    }

    std::vector<ThumbnailExportFormat> GetSupportedFormats() const {
        return {
            ThumbnailExportFormat::PNG,
            ThumbnailExportFormat::JPEG,
            ThumbnailExportFormat::WebP,
            ThumbnailExportFormat::BMP,
            ThumbnailExportFormat::TIFF
        };
    }

    uint64_t GetExportCount() const { return m_exportCount; }
    uint32_t GetLastWidth() const { return m_lastWidth; }
    uint32_t GetLastHeight() const { return m_lastHeight; }
    ThumbnailExportFormat GetLastFormat() const { return m_lastFormat; }

    void Reset() {
        m_exportCount = 0;
        m_lastWidth = 0;
        m_lastHeight = 0;
        m_lastFormat = ThumbnailExportFormat::PNG;
    }

private:
    uint64_t              m_exportCount = 0;
    uint32_t              m_lastWidth = 0;
    uint32_t              m_lastHeight = 0;
    ThumbnailExportFormat m_lastFormat = ThumbnailExportFormat::PNG;
};

} // namespace Engine
} // namespace ExplorerLens
