// FileSummaryTooltipGenerator.h — File Summary Tooltip Content Generator
// Copyright (c) 2026 ExplorerLens Project
//
// Generates rich tooltip content for file hovering in Explorer. Provides
// format details, dimensions, compression ratio, embedded metadata summary,
// and decode performance info in a compact multi-line format.

#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>

namespace ExplorerLens {
namespace Engine {

struct FileMetadata {
    std::wstring formatName;
    uint32_t     width = 0;
    uint32_t     height = 0;
    uint32_t     bitsPerPixel = 0;
    uint64_t     fileSizeBytes = 0;
    uint64_t     rawSizeBytes = 0;  // Uncompressed size
    double       decodeTimeMs = 0.0;
    uint32_t     frameCount = 1;     // For animated formats
    uint32_t     layerCount = 1;
    bool         hasAlpha = false;
    bool         isAnimated = false;
    std::wstring colorSpace;
    std::wstring author;
    std::wstring camera;
    std::wstring dateCreated;
};

struct TooltipContent {
    std::wstring title;
    std::wstring body;
    std::wstring footer;
    uint32_t     lineCount = 0;
};

struct TooltipStats {
    uint32_t tooltipsGenerated = 0;
    double   avgGenerationUs = 0.0;
};

class FileSummaryTooltipGenerator {
public:
    FileSummaryTooltipGenerator() = default;
    ~FileSummaryTooltipGenerator() = default;

    static const wchar_t* GetName() { return L"FileSummaryTooltipGenerator"; }

    /// Generate tooltip content from file metadata.
    TooltipContent Generate(const FileMetadata& meta) const {
        TooltipContent tip;

        // Title: Format + Dimensions
        std::wostringstream title;
        title << meta.formatName;
        if (meta.width > 0 && meta.height > 0)
            title << L" — " << meta.width << L" × " << meta.height;
        tip.title = title.str();

        // Body: Details
        std::wostringstream body;
        uint32_t lines = 0;

        if (meta.bitsPerPixel > 0) {
            body << meta.bitsPerPixel << L"-bit";
            if (meta.hasAlpha) body << L" (with alpha)";
            body << L"\n"; lines++;
        }

        if (!meta.colorSpace.empty()) {
            body << L"Color: " << meta.colorSpace << L"\n"; lines++;
        }

        if (meta.isAnimated && meta.frameCount > 1) {
            body << meta.frameCount << L" frames\n"; lines++;
        }

        if (meta.layerCount > 1) {
            body << meta.layerCount << L" layers\n"; lines++;
        }

        // Compression ratio
        if (meta.rawSizeBytes > 0 && meta.fileSizeBytes > 0) {
            double ratio = static_cast<double>(meta.rawSizeBytes) / meta.fileSizeBytes;
            body << L"Compression: " << std::fixed << std::setprecision(1)
                << ratio << L":1\n"; lines++;
        }

        // File size
        body << L"Size: " << FormatSize(meta.fileSizeBytes) << L"\n"; lines++;

        if (!meta.camera.empty()) {
            body << L"Camera: " << meta.camera << L"\n"; lines++;
        }
        if (!meta.dateCreated.empty()) {
            body << L"Date: " << meta.dateCreated << L"\n"; lines++;
        }

        tip.body = body.str();

        // Footer: Decode performance
        std::wostringstream footer;
        if (meta.decodeTimeMs > 0) {
            footer << L"Decoded in " << std::fixed << std::setprecision(1)
                << meta.decodeTimeMs << L" ms";
        }
        tip.footer = footer.str();
        tip.lineCount = lines + 2; // title + footer

        m_stats.tooltipsGenerated++;
        return tip;
    }

    /// Format file size to human-readable string.
    static std::wstring FormatSize(uint64_t bytes) {
        if (bytes < 1024) return std::to_wstring(bytes) + L" B";
        if (bytes < 1048576) {
            double kb = bytes / 1024.0;
            std::wostringstream s;
            s << std::fixed << std::setprecision(1) << kb << L" KB";
            return s.str();
        }
        double mb = bytes / 1048576.0;
        std::wostringstream s;
        s << std::fixed << std::setprecision(1) << mb << L" MB";
        return s.str();
    }

    TooltipStats GetStats() const { return m_stats; }

private:
    mutable TooltipStats m_stats{};
};

} // namespace Engine
} // namespace ExplorerLens
