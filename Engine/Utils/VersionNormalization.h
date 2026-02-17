//==============================================================================
// DarkThumbs — Sprint 9: Version Normalization & Release Notes Engine
//
// Automated version header scanning, stale reference detection,
// release notes generation, decoder status reporting, and documentation
// integrity validation.
//==============================================================================

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <regex>
#include <chrono>
#include <functional>
#include <sstream>
#include <algorithm>

namespace DarkThumbs {
namespace Engine {
namespace Docs {

//==============================================================================
// Canonical Version
//==============================================================================
struct VersionInfo {
    uint32_t major = 7;
    uint32_t minor = 0;
    uint32_t patch = 0;
    std::string preRelease;    // e.g., "rc1", "beta2", ""
    std::string buildMeta;     // e.g., "build.1234"

    std::string ToString() const {
        std::string v = "v" + std::to_string(major) + "." +
                         std::to_string(minor) + "." +
                         std::to_string(patch);
        if (!preRelease.empty()) v += "-" + preRelease;
        if (!buildMeta.empty()) v += "+" + buildMeta;
        return v;
    }

    std::string ToShort() const {
        return "v" + std::to_string(major) + "." + std::to_string(minor);
    }

    bool operator==(const VersionInfo& other) const {
        return major == other.major && minor == other.minor && patch == other.patch;
    }

    bool operator!=(const VersionInfo& other) const { return !(*this == other); }

    static VersionInfo Current() {
        return { 7, 0, 0, "", "" };
    }
};

//==============================================================================
// Version Reference Found in Documentation
//==============================================================================
struct VersionReference {
    std::string filePath;
    uint32_t    lineNumber = 0;
    std::string lineText;
    std::string detectedVersion;   // What version was found
    bool        isStale = false;   // Doesn't match canonical
    std::string suggestedFix;      // How to fix

    std::string Location() const {
        return filePath + ":" + std::to_string(lineNumber);
    }
};

//==============================================================================
// Version Scanner Configuration
//==============================================================================
struct ScannerConfig {
    VersionInfo canonicalVersion = VersionInfo::Current();
    std::vector<std::string> staleVersionPatterns = {
        "v5.0", "v5.1", "v5.2", "v5.3", "v5.4",
        "v6.0", "v6.1", "v6.2",
        "5.0.0", "5.4.0", "6.0.0", "6.2.0"
    };
    std::vector<std::string> excludePatterns = {
        "CHANGELOG.md",        // Changelog intentionally has old versions
        ".git/",               // Git internal
        "node_modules/",       // Dependencies
        "build/",              // Build artifacts
        "x64/",                // Build output
        "packages/"            // Packages
    };
    std::vector<std::string> targetExtensions = {
        ".md", ".h", ".cpp", ".ps1", ".yml", ".yaml", ".json"
    };
};

//==============================================================================
// Version Scanner
//==============================================================================
class VersionScanner {
public:
    explicit VersionScanner(const ScannerConfig& config = {})
        : m_config(config)
    {}

    // Scan a single file content for stale references
    std::vector<VersionReference> ScanContent(const std::string& filePath,
                                                const std::string& content) const
    {
        std::vector<VersionReference> refs;

        // Check exclusions
        for (auto& excl : m_config.excludePatterns) {
            if (filePath.find(excl) != std::string::npos) return refs;
        }

        std::istringstream stream(content);
        std::string line;
        uint32_t lineNum = 0;

        while (std::getline(stream, line)) {
            lineNum++;
            for (auto& pattern : m_config.staleVersionPatterns) {
                if (line.find(pattern) != std::string::npos) {
                    VersionReference ref;
                    ref.filePath = filePath;
                    ref.lineNumber = lineNum;
                    ref.lineText = line;
                    ref.detectedVersion = pattern;
                    ref.isStale = true;
                    ref.suggestedFix = "Update to " +
                        m_config.canonicalVersion.ToString();
                    refs.push_back(std::move(ref));
                }
            }
        }

        return refs;
    }

    // Check if a version string matches canonical
    bool IsCanonical(const std::string& version) const {
        auto& curr = m_config.canonicalVersion;
        return version == curr.ToString() ||
               version == curr.ToShort() ||
               version == std::to_string(curr.major) + "." +
                           std::to_string(curr.minor) + "." +
                           std::to_string(curr.patch);
    }

    // Count stale references in content
    size_t CountStaleReferences(const std::string& content) const {
        size_t count = 0;
        for (auto& pattern : m_config.staleVersionPatterns) {
            size_t pos = 0;
            while ((pos = content.find(pattern, pos)) != std::string::npos) {
                count++;
                pos += pattern.length();
            }
        }
        return count;
    }

    const ScannerConfig& Config() const { return m_config; }

private:
    ScannerConfig m_config;
};

//==============================================================================
// Decoder Status for Documentation
//==============================================================================
enum class DecoderStatus : uint32_t {
    Stable      = 0,   // Production ready
    Beta        = 1,   // Working but needs testing
    Experimental = 2,  // Early stage
    Planned     = 3,   // Not yet implemented
    Deprecated  = 4,   // Being removed
    External    = 5    // Handled by external library
};

inline const char* DecoderStatusName(DecoderStatus s) {
    static const char* names[] = {
        "Stable", "Beta", "Experimental", "Planned", "Deprecated", "External"
    };
    return names[static_cast<uint32_t>(s) <= 5 ? static_cast<uint32_t>(s) : 5];
}

struct DecoderDocEntry {
    std::string name;
    DecoderStatus status = DecoderStatus::Planned;
    std::string library;           // e.g., "libwebp 1.5.0"
    std::vector<std::string> formats;
    std::string notes;

    std::string StatusBadge() const {
        switch (status) {
        case DecoderStatus::Stable:       return "[STABLE]";
        case DecoderStatus::Beta:         return "[BETA]";
        case DecoderStatus::Experimental: return "[EXPERIMENTAL]";
        case DecoderStatus::Planned:      return "[PLANNED]";
        case DecoderStatus::Deprecated:   return "[DEPRECATED]";
        case DecoderStatus::External:     return "[EXTERNAL]";
        default: return "[UNKNOWN]";
        }
    }
};

//==============================================================================
// Decoder Status Registry
//==============================================================================
class DecoderStatusRegistry {
public:
    DecoderStatusRegistry() { RegisterAllDecoders(); }

    const std::vector<DecoderDocEntry>& AllDecoders() const { return m_decoders; }

    std::vector<DecoderDocEntry> GetByStatus(DecoderStatus status) const {
        std::vector<DecoderDocEntry> result;
        for (auto& d : m_decoders) {
            if (d.status == status) result.push_back(d);
        }
        return result;
    }

    size_t TotalCount() const { return m_decoders.size(); }

    size_t StableCount() const {
        return std::count_if(m_decoders.begin(), m_decoders.end(),
            [](const DecoderDocEntry& d) { return d.status == DecoderStatus::Stable; });
    }

    size_t FormatCount() const {
        std::unordered_set<std::string> formats;
        for (auto& d : m_decoders)
            for (auto& f : d.formats) formats.insert(f);
        return formats.size();
    }

    // Generate markdown decoder status table
    std::string GenerateMarkdownTable() const {
        std::string md;
        md += "| Decoder | Status | Library | Formats |\n";
        md += "|---------|--------|---------|---------|\n";
        for (auto& d : m_decoders) {
            md += "| " + d.name + " | " + d.StatusBadge() + " | " +
                  d.library + " | ";
            for (size_t i = 0; i < d.formats.size(); ++i) {
                md += d.formats[i];
                if (i + 1 < d.formats.size()) md += ", ";
            }
            md += " |\n";
        }
        return md;
    }

private:
    void RegisterAllDecoders() {
        m_decoders = {
            {"JPEG/JFIF",   DecoderStatus::Stable, "WIC (built-in)",
             {".jpg", ".jpeg", ".jpe", ".jfif"}, "Windows Imaging Component"},
            {"PNG",          DecoderStatus::Stable, "WIC (built-in)",
             {".png"}, ""},
            {"BMP",          DecoderStatus::Stable, "WIC (built-in)",
             {".bmp", ".dib"}, ""},
            {"GIF",          DecoderStatus::Stable, "WIC (built-in)",
             {".gif"}, "First frame only"},
            {"TIFF",         DecoderStatus::Stable, "WIC (built-in)",
             {".tif", ".tiff"}, "Multi-page via multi-frame"},
            {"ICO/CUR",      DecoderStatus::Stable, "Custom",
             {".ico", ".cur"}, "Best-size extraction"},
            {"WebP",         DecoderStatus::Stable, "libwebp 1.5.0",
             {".webp"}, "Animated first frame"},
            {"JPEG XL",      DecoderStatus::Stable, "libjxl 0.11.1",
             {".jxl"}, "Animated first frame"},
            {"HEIF/HEIC",    DecoderStatus::Stable, "libheif 1.19.5",
             {".heif", ".heic", ".hif"}, "libde265 backend"},
            {"AVIF",         DecoderStatus::Stable, "libavif 1.3.0",
             {".avif"}, "dav1d backend"},
            {"RAW (Camera)", DecoderStatus::Stable, "LibRaw 0.21.3",
             {".cr2", ".cr3", ".nef", ".arw", ".orf", ".rw2",
              ".raf", ".dng", ".srw", ".pef", ".gpr", ".raw"},
             "Embedded preview extraction"},
            {"PSD",          DecoderStatus::Stable, "Custom",
             {".psd", ".psb"}, "Composite preview"},
            {"TGA",          DecoderStatus::Stable, "Custom",
             {".tga", ".targa"}, "Truevision"},
            {"DDS",          DecoderStatus::Stable, "WIC + D3D11",
             {".dds"}, "GPU-accelerated BC decompression"},
            {"QOI",          DecoderStatus::Stable, "Custom",
             {".qoi"}, "Quite OK Image format"},
            {"SVG",          DecoderStatus::Stable, "Direct2D",
             {".svg", ".svgz"}, "Vector rasterization"},
            {"EXR",          DecoderStatus::Stable, "Custom",
             {".exr"}, "OpenEXR HDR"},
            {"HDR",          DecoderStatus::Stable, "Custom",
             {".hdr"}, "Radiance HDR"},
            {"PDF",          DecoderStatus::Stable, "WIC/Shell",
             {".pdf"}, "First page thumbnail"},
            {"ZIP Archives", DecoderStatus::Stable, "minizip-ng 4.0.10",
             {".zip", ".cbz"}, "First image extraction"},
            {"RAR Archives", DecoderStatus::Stable, "UnRAR 7.2.2",
             {".rar", ".cbr"}, "First image extraction"},
            {"7z Archives",  DecoderStatus::Stable, "LZMA 26.00",
             {".7z", ".cb7"}, "First image extraction"},
            {"TAR Archives", DecoderStatus::Stable, "Custom",
             {".tar", ".tar.gz", ".tar.bz2", ".tar.xz"}, "Streaming"},
            {"Video",        DecoderStatus::External, "Media Foundation",
             {".mp4", ".mkv", ".avi", ".mov", ".wmv", ".webm"},
             "Scene frame selection"},
        };
    }

    std::vector<DecoderDocEntry> m_decoders;
};

//==============================================================================
// Release Notes Generator
//==============================================================================
struct ReleaseNote {
    std::string category;     // e.g., "New Features", "Bug Fixes"
    std::string description;
};

class ReleaseNotesGenerator {
public:
    explicit ReleaseNotesGenerator(const VersionInfo& version = VersionInfo::Current())
        : m_version(version)
    {}

    void AddNote(const std::string& category, const std::string& description) {
        m_notes.push_back({category, description});
    }

    void AddFeature(const std::string& desc) {
        AddNote("New Features", desc);
    }

    void AddBugFix(const std::string& desc) {
        AddNote("Bug Fixes", desc);
    }

    void AddImprovement(const std::string& desc) {
        AddNote("Improvements", desc);
    }

    void AddBreakingChange(const std::string& desc) {
        AddNote("Breaking Changes", desc);
    }

    std::string Generate() const {
        std::string md;
        md += "# Release Notes — DarkThumbs " + m_version.ToString() + "\n\n";
        md += "**Release Date:** " + CurrentDateString() + "\n\n";

        // Group by category
        std::unordered_map<std::string, std::vector<std::string>> grouped;
        std::vector<std::string> categoryOrder;
        for (auto& note : m_notes) {
            if (grouped.find(note.category) == grouped.end()) {
                categoryOrder.push_back(note.category);
            }
            grouped[note.category].push_back(note.description);
        }

        for (auto& cat : categoryOrder) {
            md += "## " + cat + "\n\n";
            for (auto& desc : grouped[cat]) {
                md += "- " + desc + "\n";
            }
            md += "\n";
        }

        return md;
    }

    size_t NoteCount() const { return m_notes.size(); }

    size_t CategoryCount() const {
        std::unordered_set<std::string> cats;
        for (auto& n : m_notes) cats.insert(n.category);
        return cats.size();
    }

    const VersionInfo& Version() const { return m_version; }

private:
    static std::string CurrentDateString() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        struct tm tm_buf{};
        localtime_s(&tm_buf, &time);
        char buf[32]{};
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tm_buf);
        return buf;
    }

    VersionInfo m_version;
    std::vector<ReleaseNote> m_notes;
};

//==============================================================================
// Documentation Integrity Report
//==============================================================================
struct DocIntegrityReport {
    size_t filesScanned      = 0;
    size_t staleReferences   = 0;
    size_t staleDocs         = 0;   // Files with any stale ref
    size_t cleanDocs         = 0;
    std::vector<VersionReference> allReferences;

    double IntegrityPercent() const {
        if (filesScanned == 0) return 100.0;
        return 100.0 * cleanDocs / filesScanned;
    }

    bool IsClean() const { return staleReferences == 0; }
};

//==============================================================================
// Stale Document Tracker
//==============================================================================
class StaleDocTracker {
public:
    // Register the 12 stale docs identified in Sprint 9 audit
    void RegisterKnownStaleDocs() {
        m_staleDocs = {
            "DECODER_STATUS.md",
            "TESTING_GUIDE.md",
            "README.md",
            "DEVELOPER_GUIDE.md",
            "USER_GUIDE.md",
            "KNOWN_ISSUES.md",
            "docs/FORMAT_SUPPORT_ANALYSIS.md",
            "docs/WINDOWS_BUILD_TOOLS.md",
            "docs/BUILD_METHOD_COMPARISON.md",
            "SDK/README.md",
            ".github/COMPLETE_PROJECT_SUMMARY.md",
            ".github/SPRINTS_6-22_SUMMARY.md"
        };
    }

    void MarkFixed(const std::string& doc) {
        m_fixedDocs.insert(doc);
    }

    bool IsFixed(const std::string& doc) const {
        return m_fixedDocs.count(doc) > 0;
    }

    size_t TotalStale() const { return m_staleDocs.size(); }

    size_t FixedCount() const {
        size_t c = 0;
        for (auto& d : m_staleDocs) {
            if (m_fixedDocs.count(d)) ++c;
        }
        return c;
    }

    size_t RemainingCount() const { return TotalStale() - FixedCount(); }

    std::vector<std::string> GetRemaining() const {
        std::vector<std::string> remaining;
        for (auto& d : m_staleDocs) {
            if (!m_fixedDocs.count(d)) remaining.push_back(d);
        }
        return remaining;
    }

    double ProgressPercent() const {
        if (TotalStale() == 0) return 100.0;
        return 100.0 * FixedCount() / TotalStale();
    }

private:
    std::vector<std::string> m_staleDocs;
    std::unordered_set<std::string> m_fixedDocs;
};

} // namespace Docs
} // namespace Engine
} // namespace DarkThumbs
