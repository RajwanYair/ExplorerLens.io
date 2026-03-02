// FormatStatusProvider.h — Unified Format Decode Status for UI Layer
// Copyright (c) 2026 ExplorerLens Project
//
// Bridges the gap between engine-side decoder health monitoring and UI display.
// Aggregates status from DecoderHealthMonitor, FormatRegistry, and library
// availability flags (HAS_LIBJXL, HAS_LIBHEIF, etc.) into a single queryable
// interface that LENSManager can bind to its format status grid.
//
// Each format has a FormatStatusDetail containing:
//   - Decoder availability (enum: Available/Unavailable/Degraded)
//   - Library version string
//   - Decode success rate (last N attempts)
//   - Average decode time
//   - Last error message (if any)

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

/// Decoder availability status
enum class DecoderAvailability : uint8_t {
    Available    = 0,   // Library loaded, decoder functional
    Unavailable  = 1,   // Library not built / not present
    Degraded     = 2,   // Library present but some features missing
    Disabled     = 3,   // Disabled by user/policy
    Error        = 4    // Library loaded but decoder crashed/failed
};

inline const char* AvailabilityName(DecoderAvailability a) {
    static const char* names[] = { "Available", "Unavailable", "Degraded", "Disabled", "Error" };
    auto idx = static_cast<uint8_t>(a);
    return (idx < 5) ? names[idx] : "Unknown";
}

/// Complete status for a single format
struct FormatStatusDetail {
    std::string formatName;             // e.g., "JPEG XL", "HEIF", "PDF"
    std::string formatExtension;        // e.g., ".jxl", ".heif", ".pdf"
    std::string decoderName;            // e.g., "JXLDecoder", "PDFDecoder"
    std::string libraryName;            // e.g., "libjxl", "MuPDF"
    std::string libraryVersion;         // e.g., "0.11.1"
    DecoderAvailability availability = DecoderAvailability::Unavailable;
    uint32_t    totalDecodes    = 0;
    uint32_t    successCount    = 0;
    uint32_t    failureCount    = 0;
    double      successRate     = 0.0;  // 0.0 - 100.0
    double      avgDecodeTimeMs = 0.0;
    double      p95DecodeTimeMs = 0.0;
    std::string lastError;
    std::chrono::steady_clock::time_point lastDecodeTime{};
};

/// Summary across all formats
struct FormatStatusSummary {
    uint32_t totalFormats     = 0;
    uint32_t availableCount   = 0;
    uint32_t unavailableCount = 0;
    uint32_t degradedCount    = 0;
    uint32_t errorCount       = 0;
    double   overallSuccessRate = 0.0;
};

/// Provides unified format status queryable by the UI layer.
class FormatStatusProvider {
public:
    static FormatStatusProvider& Instance() {
        static FormatStatusProvider inst;
        return inst;
    }

    /// Initialize with all known formats and their compile-time availability.
    void Initialize() {
        if (m_initialized) return;
        RegisterBuiltinFormats();
        m_initialized = true;
    }

    /// Get status for a specific format (by extension, e.g., ".jxl").
    FormatStatusDetail GetStatus(const std::string& extension) const {
        auto it = m_statusMap.find(extension);
        if (it != m_statusMap.end()) return it->second;
        FormatStatusDetail empty;
        empty.formatExtension = extension;
        empty.availability = DecoderAvailability::Unavailable;
        return empty;
    }

    /// Get all format statuses.
    std::vector<FormatStatusDetail> GetAllStatuses() const {
        std::vector<FormatStatusDetail> result;
        result.reserve(m_statusMap.size());
        for (auto& [ext, status] : m_statusMap) {
            result.push_back(status);
        }
        return result;
    }

    /// Get summary across all formats.
    FormatStatusSummary GetSummary() const {
        FormatStatusSummary summary;
        summary.totalFormats = static_cast<uint32_t>(m_statusMap.size());
        uint32_t successTotal = 0, decodesTotal = 0;
        for (auto& [ext, s] : m_statusMap) {
            switch (s.availability) {
                case DecoderAvailability::Available: summary.availableCount++; break;
                case DecoderAvailability::Unavailable: summary.unavailableCount++; break;
                case DecoderAvailability::Degraded: summary.degradedCount++; break;
                case DecoderAvailability::Error: summary.errorCount++; break;
                default: break;
            }
            successTotal += s.successCount;
            decodesTotal += s.totalDecodes;
        }
        summary.overallSuccessRate = (decodesTotal > 0)
            ? (100.0 * successTotal / decodesTotal) : 0.0;
        return summary;
    }

    /// Record a decode result for a format.
    void RecordDecode(const std::string& extension, bool success, double decodeTimeMs,
                      const std::string& error = {}) {
        auto it = m_statusMap.find(extension);
        if (it == m_statusMap.end()) return;

        auto& s = it->second;
        s.totalDecodes++;
        if (success) {
            s.successCount++;
        } else {
            s.failureCount++;
            if (!error.empty()) s.lastError = error;
        }
        s.successRate = (s.totalDecodes > 0)
            ? (100.0 * s.successCount / s.totalDecodes) : 0.0;
        s.avgDecodeTimeMs = (s.avgDecodeTimeMs * (s.totalDecodes - 1) + decodeTimeMs) / s.totalDecodes;
        s.lastDecodeTime = std::chrono::steady_clock::now();
    }

    /// Get count of available formats.
    uint32_t GetAvailableCount() const {
        uint32_t count = 0;
        for (auto& [ext, s] : m_statusMap) {
            if (s.availability == DecoderAvailability::Available) count++;
        }
        return count;
    }

private:
    FormatStatusProvider() = default;

    void RegisterBuiltinFormats() {
        // Image formats — always available (built-in decoders)
        RegisterFormat(".bmp",  "BMP",      "ImageDecoder",   "Built-in",   "1.0", DecoderAvailability::Available);
        RegisterFormat(".png",  "PNG",      "ImageDecoder",   "Built-in",   "1.0", DecoderAvailability::Available);
        RegisterFormat(".jpg",  "JPEG",     "ImageDecoder",   "Built-in",   "1.0", DecoderAvailability::Available);
        RegisterFormat(".gif",  "GIF",      "ImageDecoder",   "Built-in",   "1.0", DecoderAvailability::Available);
        RegisterFormat(".tiff", "TIFF",     "ImageDecoder",   "Built-in",   "1.0", DecoderAvailability::Available);
        RegisterFormat(".ico",  "ICO",      "ICODecoder",     "Built-in",   "1.0", DecoderAvailability::Available);
        RegisterFormat(".tga",  "TGA",      "TGADecoder",     "Built-in",   "1.0", DecoderAvailability::Available);
        RegisterFormat(".pcx",  "PCX",      "PCXDecoder",     "Built-in",   "1.0", DecoderAvailability::Available);
        RegisterFormat(".ppm",  "PPM",      "PPMDecoder",     "Built-in",   "1.0", DecoderAvailability::Available);
        RegisterFormat(".dds",  "DDS",      "DDSDecoder",     "Built-in",   "1.0", DecoderAvailability::Available);
        RegisterFormat(".hdr",  "HDR",      "HDRDecoder",     "Built-in",   "1.0", DecoderAvailability::Available);
        RegisterFormat(".exr",  "OpenEXR",  "EXRDecoder",     "Built-in",   "1.0", DecoderAvailability::Available);
        RegisterFormat(".psd",  "PSD",      "PSDDecoder",     "Built-in",   "1.0", DecoderAvailability::Available);
        RegisterFormat(".svg",  "SVG",      "SVGDecoder",     "Built-in",   "1.0", DecoderAvailability::Available);
        RegisterFormat(".qoi",  "QOI",      "QOIDecoder",     "Built-in",   "1.0", DecoderAvailability::Available);
        RegisterFormat(".xpm",  "XPM",      "XPMDecoder",     "Built-in",   "1.0", DecoderAvailability::Available);

        // Library-dependent formats
#if defined(HAS_LIBWEBP) && HAS_LIBWEBP
        RegisterFormat(".webp", "WebP",     "WebPDecoder",    "libwebp",    "1.5.0", DecoderAvailability::Available);
#else
        RegisterFormat(".webp", "WebP",     "WebPDecoder",    "libwebp",    "",      DecoderAvailability::Unavailable);
#endif

#if defined(HAS_LIBJXL) && HAS_LIBJXL
        RegisterFormat(".jxl",  "JPEG XL",  "JXLDecoder",     "libjxl",     "0.11.1", DecoderAvailability::Available);
#else
        RegisterFormat(".jxl",  "JPEG XL",  "JXLDecoder",     "libjxl",     "",       DecoderAvailability::Unavailable);
#endif

#if defined(HAS_LIBHEIF) && HAS_LIBHEIF
        RegisterFormat(".heif", "HEIF",     "HEIFDecoder",    "libheif",    "1.19.5", DecoderAvailability::Available);
        RegisterFormat(".heic", "HEIC",     "HEIFDecoder",    "libheif",    "1.19.5", DecoderAvailability::Available);
#else
        RegisterFormat(".heif", "HEIF",     "HEIFDecoder",    "libheif",    "",       DecoderAvailability::Unavailable);
        RegisterFormat(".heic", "HEIC",     "HEIFDecoder",    "libheif",    "",       DecoderAvailability::Unavailable);
#endif

#if defined(HAS_LIBAVIF) && HAS_LIBAVIF
        RegisterFormat(".avif", "AVIF",     "AVIFDecoder",    "libavif",    "1.3.0", DecoderAvailability::Available);
#else
        RegisterFormat(".avif", "AVIF",     "AVIFDecoder",    "libavif",    "",      DecoderAvailability::Unavailable);
#endif

#if defined(HAS_LIBRAW) && HAS_LIBRAW
        RegisterFormat(".cr2",  "Canon RAW","RAWDecoder",     "LibRaw",     "0.21.3", DecoderAvailability::Available);
        RegisterFormat(".nef",  "Nikon RAW","RAWDecoder",     "LibRaw",     "0.21.3", DecoderAvailability::Available);
        RegisterFormat(".arw",  "Sony RAW", "RAWDecoder",     "LibRaw",     "0.21.3", DecoderAvailability::Available);
        RegisterFormat(".dng",  "DNG",      "RAWDecoder",     "LibRaw",     "0.21.3", DecoderAvailability::Available);
#else
        RegisterFormat(".cr2",  "Canon RAW","RAWDecoder",     "LibRaw",     "",       DecoderAvailability::Unavailable);
        RegisterFormat(".nef",  "Nikon RAW","RAWDecoder",     "LibRaw",     "",       DecoderAvailability::Unavailable);
        RegisterFormat(".arw",  "Sony RAW", "RAWDecoder",     "LibRaw",     "",       DecoderAvailability::Unavailable);
        RegisterFormat(".dng",  "DNG",      "RAWDecoder",     "LibRaw",     "",       DecoderAvailability::Unavailable);
#endif

#if defined(HAS_MUPDF) && HAS_MUPDF
        RegisterFormat(".pdf",  "PDF",      "PDFDecoder",     "MuPDF",      "1.25.5", DecoderAvailability::Available);
#else
        RegisterFormat(".pdf",  "PDF",      "PDFDecoder",     "MuPDF",      "",       DecoderAvailability::Unavailable);
#endif

#if defined(HAS_OPENJPEG) && HAS_OPENJPEG
        RegisterFormat(".jp2",  "JPEG 2000","JPEG2000Decoder", "OpenJPEG",  "2.5.3", DecoderAvailability::Available);
        RegisterFormat(".j2k",  "JPEG 2000","JPEG2000Decoder", "OpenJPEG",  "2.5.3", DecoderAvailability::Available);
#else
        RegisterFormat(".jp2",  "JPEG 2000","JPEG2000Decoder", "OpenJPEG",  "",      DecoderAvailability::Unavailable);
        RegisterFormat(".j2k",  "JPEG 2000","JPEG2000Decoder", "OpenJPEG",  "",      DecoderAvailability::Unavailable);
#endif

        // Archive formats — always available (built-in + zlib/lz4/zstd)
        RegisterFormat(".zip",  "ZIP",      "ArchiveDecoder", "minizip-ng", "4.0.10", DecoderAvailability::Available);
        RegisterFormat(".7z",   "7-Zip",    "ArchiveDecoder", "LZMA SDK",   "26.00",  DecoderAvailability::Available);
        RegisterFormat(".rar",  "RAR",      "ArchiveDecoder", "UnRAR",      "7.2.2",  DecoderAvailability::Available);
    }

    void RegisterFormat(const std::string& ext, const std::string& name,
                        const std::string& decoder, const std::string& lib,
                        const std::string& ver, DecoderAvailability avail) {
        FormatStatusDetail s;
        s.formatName = name;
        s.formatExtension = ext;
        s.decoderName = decoder;
        s.libraryName = lib;
        s.libraryVersion = ver;
        s.availability = avail;
        m_statusMap[ext] = std::move(s);
    }

    std::unordered_map<std::string, FormatStatusDetail> m_statusMap;
    bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
