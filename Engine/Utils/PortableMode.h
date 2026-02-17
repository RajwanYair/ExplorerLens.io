#pragma once
//==============================================================================
// DarkThumbs — Sprint 42: Portable Mode & Thumbnail Overlay Badges
// Registry-free portable operation, INI-based config, file-based cache,
// format icon badges, file-size badges, overlay rendering.
//==============================================================================

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <algorithm>

namespace DarkThumbs::Engine::Utils {

//------------------------------------------------------------------------------
// Portable Mode Detection & Configuration
//------------------------------------------------------------------------------
enum class DeploymentMode : uint8_t {
    Installed = 0,     // Normal installed mode (registry + %LocalAppData%)
    Portable,          // Portable mode (portable.ini + local cache)
    Enterprise         // Enterprise GPO mode (HKLM policies override)
};

inline const char* DeploymentModeName(DeploymentMode mode) {
    switch (mode) {
        case DeploymentMode::Installed:  return "Installed";
        case DeploymentMode::Portable:   return "Portable";
        case DeploymentMode::Enterprise: return "Enterprise (GPO)";
        default:                         return "Unknown";
    }
}

// Check for portable.ini adjacent to the DLL
struct PortableDetector {
    std::string dllDirectory;    // Directory containing CBXShell.dll
    std::string iniFilePath;     // Full path to portable.ini

    // Detect portable mode: look for portable.ini next to the DLL
    bool IsPortable() const {
        // In real implementation: GetModuleFileName → extract dir → check file exists
        return !iniFilePath.empty();
    }

    DeploymentMode Detect() const {
        if (IsPortable()) return DeploymentMode::Portable;
        // In real implementation: check HKLM policies for enterprise
        return DeploymentMode::Installed;
    }
};

//------------------------------------------------------------------------------
// INI File Configuration — replaces registry in portable mode
//------------------------------------------------------------------------------
struct INISection {
    std::string name;
    std::unordered_map<std::string, std::string> values;

    std::string Get(const std::string& key, const std::string& defaultVal = "") const {
        auto it = values.find(key);
        return (it != values.end()) ? it->second : defaultVal;
    }

    int GetInt(const std::string& key, int defaultVal = 0) const {
        auto it = values.find(key);
        if (it != values.end()) {
            try { return std::stoi(it->second); } catch (...) {}
        }
        return defaultVal;
    }

    bool GetBool(const std::string& key, bool defaultVal = false) const {
        auto it = values.find(key);
        if (it != values.end()) {
            std::string v = it->second;
            return (v == "1" || v == "true" || v == "yes" || v == "on");
        }
        return defaultVal;
    }

    void Set(const std::string& key, const std::string& val) { values[key] = val; }
    bool HasKey(const std::string& key) const { return values.count(key) > 0; }
    uint32_t KeyCount() const { return static_cast<uint32_t>(values.size()); }
};

struct PortableConfig {
    std::vector<INISection> sections;
    std::string filePath;
    bool loaded = false;

    // Get a section by name
    const INISection* GetSection(const std::string& name) const {
        for (auto& s : sections) {
            if (s.name == name) return &s;
        }
        return nullptr;
    }

    // Add or get mutable section
    INISection& EnsureSection(const std::string& name) {
        for (auto& s : sections) {
            if (s.name == name) return s;
        }
        sections.push_back({name, {}});
        return sections.back();
    }

    uint32_t SectionCount() const { return static_cast<uint32_t>(sections.size()); }

    // Default portable.ini template
    static PortableConfig DefaultTemplate() {
        PortableConfig cfg;
        cfg.loaded = true;

        auto& general = cfg.EnsureSection("General");
        general.Set("Version", "7.0.0");
        general.Set("PortableMode", "true");
        general.Set("CacheDirectory", ".\\cache");

        auto& formats = cfg.EnsureSection("Formats");
        formats.Set("EnableJPEG", "1");
        formats.Set("EnablePNG", "1");
        formats.Set("EnableWebP", "1");
        formats.Set("EnableHEIF", "1");
        formats.Set("EnableJXL", "1");
        formats.Set("EnableAVIF", "1");
        formats.Set("EnableRAW", "1");
        formats.Set("EnablePSD", "1");
        formats.Set("EnableSVG", "1");
        formats.Set("EnableArchive", "1");
        formats.Set("EnableVideo", "0");
        formats.Set("EnableAudio", "0");

        auto& cache = cfg.EnsureSection("Cache");
        cache.Set("MaxSizeMB", "256");
        cache.Set("MaxEntries", "50000");
        cache.Set("CleanupDays", "30");

        auto& badges = cfg.EnsureSection("Badges");
        badges.Set("ShowFormatBadge", "1");
        badges.Set("ShowSizeBadge", "0");
        badges.Set("BadgePosition", "BottomLeft");
        badges.Set("BadgeFontSize", "10");

        auto& gpu = cfg.EnsureSection("GPU");
        gpu.Set("UseGPU", "1");
        gpu.Set("PreferD3D12", "1");
        gpu.Set("FallbackToSoftware", "1");

        return cfg;
    }
};

//------------------------------------------------------------------------------
// Portable Cache Paths
//------------------------------------------------------------------------------
struct PortablePaths {
    std::string baseDirectory;     // DLL directory
    std::string cacheDirectory;    // ./cache/
    std::string configFile;        // ./portable.ini
    std::string logDirectory;      // ./logs/
    std::string pluginDirectory;   // ./plugins/

    static PortablePaths FromDllDir(const std::string& dllDir) {
        PortablePaths p;
        p.baseDirectory = dllDir;
        p.cacheDirectory = dllDir + "\\cache";
        p.configFile = dllDir + "\\portable.ini";
        p.logDirectory = dllDir + "\\logs";
        p.pluginDirectory = dllDir + "\\plugins";
        return p;
    }

    static PortablePaths FromLocalAppData(const std::string& appDataDir) {
        PortablePaths p;
        p.baseDirectory = appDataDir + "\\DarkThumbs";
        p.cacheDirectory = p.baseDirectory + "\\Cache";
        p.configFile = "";  // Uses registry
        p.logDirectory = p.baseDirectory + "\\Logs";
        p.pluginDirectory = p.baseDirectory + "\\Plugins";
        return p;
    }
};

//------------------------------------------------------------------------------
// Thumbnail Overlay Badges
//------------------------------------------------------------------------------
enum class BadgePosition : uint8_t {
    TopLeft = 0,
    TopRight,
    BottomLeft,
    BottomRight
};

inline const char* BadgePositionName(BadgePosition pos) {
    switch (pos) {
        case BadgePosition::TopLeft:     return "Top-Left";
        case BadgePosition::TopRight:    return "Top-Right";
        case BadgePosition::BottomLeft:  return "Bottom-Left";
        case BadgePosition::BottomRight: return "Bottom-Right";
        default:                         return "Unknown";
    }
}

// Format badge: shows codec name (e.g., "JXL", "HEIF", "RAW")
struct FormatBadge {
    std::string formatLabel;     // "JXL", "HEIF", "RAW", "PSD", etc.
    BadgePosition position = BadgePosition::BottomLeft;
    uint32_t fontSize = 10;
    uint32_t bgColor = 0x80000000;  // Semi-transparent black
    uint32_t fgColor = 0xFFFFFFFF;  // White text
    float cornerRadius = 3.0f;
    float paddingX = 4.0f;
    float paddingY = 2.0f;

    bool IsEmpty() const { return formatLabel.empty(); }

    // Determine format label from file extension
    static std::string LabelForExtension(const std::string& ext) {
        std::string lower = ext;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower == ".jxl") return "JXL";
        if (lower == ".heif" || lower == ".heic") return "HEIF";
        if (lower == ".avif") return "AVIF";
        if (lower == ".webp") return "WebP";
        if (lower == ".psd" || lower == ".psb") return "PSD";
        if (lower == ".dds") return "DDS";
        if (lower == ".hdr") return "HDR";
        if (lower == ".exr") return "EXR";
        if (lower == ".svg") return "SVG";
        if (lower == ".qoi") return "QOI";
        if (lower == ".dng" || lower == ".cr2" || lower == ".cr3" ||
            lower == ".nef" || lower == ".arw" || lower == ".orf" ||
            lower == ".gpr") return "RAW";
        if (lower == ".cbz" || lower == ".cbr" || lower == ".cb7") return "CBX";
        if (lower == ".epub") return "EPUB";
        if (lower == ".pdf") return "PDF";
        if (lower == ".mp4" || lower == ".mkv" || lower == ".avi" ||
            lower == ".mov" || lower == ".webm") return "VID";
        if (lower == ".mp3" || lower == ".flac" || lower == ".wav" ||
            lower == ".ogg") return "AUD";
        if (lower == ".ttf" || lower == ".otf" || lower == ".woff") return "FONT";
        if (lower == ".obj" || lower == ".stl" || lower == ".fbx" ||
            lower == ".gltf") return "3D";
        return "";
    }
};

// File size badge: shows human-readable size (e.g., "2.4 MB")
struct FileSizeBadge {
    uint64_t fileSize = 0;
    BadgePosition position = BadgePosition::BottomRight;
    uint32_t fontSize = 9;
    uint32_t bgColor = 0x80333333;
    uint32_t fgColor = 0xFFCCCCCC;

    bool ShouldShow() const { return fileSize > 0; }

    std::string SizeText() const {
        if (fileSize < 1024) return std::to_string(fileSize) + " B";
        if (fileSize < 1048576) {
            double kb = fileSize / 1024.0;
            char buf[32];
            snprintf(buf, sizeof(buf), "%.1f KB", kb);
            return buf;
        }
        if (fileSize < 1073741824) {
            double mb = fileSize / 1048576.0;
            char buf[32];
            snprintf(buf, sizeof(buf), "%.1f MB", mb);
            return buf;
        }
        double gb = fileSize / 1073741824.0;
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f GB", gb);
        return buf;
    }
};

//------------------------------------------------------------------------------
// Badge Overlay Configuration
//------------------------------------------------------------------------------
struct BadgeOverlayConfig {
    bool showFormatBadge = true;
    bool showSizeBadge = false;
    BadgePosition formatPosition = BadgePosition::BottomLeft;
    BadgePosition sizePosition = BadgePosition::BottomRight;
    uint32_t minThumbnailSize = 128;  // Don't show badges on very small thumbs
    float opacity = 0.8f;

    static BadgeOverlayConfig Default() { return {}; }

    static BadgeOverlayConfig Disabled() {
        BadgeOverlayConfig c;
        c.showFormatBadge = false;
        c.showSizeBadge = false;
        return c;
    }

    static BadgeOverlayConfig AllBadges() {
        BadgeOverlayConfig c;
        c.showFormatBadge = true;
        c.showSizeBadge = true;
        return c;
    }

    bool HasAnyBadge() const { return showFormatBadge || showSizeBadge; }
};

//------------------------------------------------------------------------------
// No-Install Deployment Info
//------------------------------------------------------------------------------
struct DeploymentInfo {
    DeploymentMode mode = DeploymentMode::Installed;
    std::string version = "7.0.0";
    std::string dllPath;
    std::string cacheLocation;
    std::string configSource;   // "Registry" or "portable.ini"
    bool registeredWithShell = false;  // regsvr32 done?

    std::string Summary() const {
        return "DarkThumbs v" + version + " [" +
               DeploymentModeName(mode) + "] config=" + configSource +
               " registered=" + (registeredWithShell ? "yes" : "no");
    }
};

} // namespace DarkThumbs::Engine::Utils
