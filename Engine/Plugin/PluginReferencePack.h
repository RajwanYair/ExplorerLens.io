#pragma once
// Plugin Reference Pack
// Production-grade reference plugin descriptors for 3 format families:
// (1) Minimal image generator, (2) Metadata-only, (3) Watermark overlay.

#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <functional>

namespace ExplorerLens::Plugin {

// ─── Plugin capability flags ──────────────────────────────────────────────────

enum class PluginCapability : uint32_t {
    None            = 0x000,
    Decode          = 0x001,
    MetadataOnly    = 0x002,
    PostProcess     = 0x004,
    FormatConvert   = 0x008,
    Async           = 0x010,
    GPUAccelerated  = 0x020,
};

inline PluginCapability operator|(PluginCapability a, PluginCapability b) {
    return static_cast<PluginCapability>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline bool HasCapability(PluginCapability set, PluginCapability flag) {
    return (static_cast<uint32_t>(set) & static_cast<uint32_t>(flag)) != 0;
}

// ─── Reference plugin descriptor ─────────────────────────────────────────────

struct ReferencePluginDescriptor {
    std::string         id;                   // unique reverse-domain ID
    std::string         name;
    std::string         description;
    std::string         version;              // semantic version
    PluginCapability    capabilities;
    std::vector<std::string> supportedExtensions;
    std::string         ipcEndpoint;          // named pipe path template
    uint32_t            maxDecodeMs   { 100 };
    uint64_t            maxMemoryBytes{ 50ULL * 1024 * 1024 };

    bool Supports(const std::string& ext) const {
        for (const auto& e : supportedExtensions)
            if (e == ext) return true;
        return false;
    }
};

// ─── Reference Plugin 1 — Minimal Image Generator ────────────────────────────
// Generates a solid-color thumbnail for unrecognised/unsupported file types.

inline ReferencePluginDescriptor MinimalImageGeneratorPlugin() {
    return {
        "com.explorerlens.ref.minimal",
        "Minimal Image Generator",
        "Generates a solid-color placeholder thumbnail for unknown file types",
        "1.0.0",
        PluginCapability::Decode,
        { ".*" },   // wildcard — handles any extension as fallback
        "\\\\.\\pipe\\ExplorerLens_Plugin_Minimal",
        50,
        16ULL * 1024 * 1024
    };
}

struct SolidColorThumbnail {
    uint32_t    widthPx     { 256 };
    uint32_t    heightPx    { 256 };
    uint8_t     r { 64 }, g { 64 }, b { 96 };  // default: dark slate
    std::string labelText;                       // optional overlay label

    bool IsValid() const { return widthPx > 0 && heightPx > 0; }
};

// ─── Reference Plugin 2 — Metadata-Only Plugin ───────────────────────────────
// Reads EXIF/XMP metadata without performing full pixel decode.

inline ReferencePluginDescriptor MetadataOnlyPlugin() {
    return {
        "com.explorerlens.ref.metadata",
        "Metadata Reader",
        "Reads EXIF/XMP/IPTC metadata without full pixel decode",
        "1.0.0",
        PluginCapability::MetadataOnly | PluginCapability::Decode,
        { ".jpg", ".jpeg", ".tiff", ".tif", ".heic", ".heif", ".avif",
          ".png", ".webp", ".raw", ".cr2", ".cr3", ".nef", ".arw" },
        "\\\\.\\pipe\\ExplorerLens_Plugin_Metadata",
        200,    // metadata read allowed up to 200ms
        8ULL * 1024 * 1024
    };
}

struct EmbeddedMetadata {
    std::string             make;
    std::string             model;
    std::string             captureDateTime;
    std::optional<double>   focalLengthMm;
    std::optional<double>   apertureF;
    std::optional<double>   shutterSpeedSec;
    std::optional<int32_t>  iso;
    std::optional<double>   latitudeDeg;
    std::optional<double>   longitudeDeg;
    uint32_t                imageWidthPx    { 0 };
    uint32_t                imageHeightPx   { 0 };
    int32_t                 orientation     { 1 };   // EXIF orientation tag

    bool HasGPS() const { return latitudeDeg.has_value() && longitudeDeg.has_value(); }
    bool HasCameraInfo() const { return !make.empty() || !model.empty(); }
};

// ─── Reference Plugin 3 — Watermark Plugin ───────────────────────────────────
// Post-processes a thumbnail to apply a branding/copyright overlay.

inline ReferencePluginDescriptor WatermarkPlugin() {
    return {
        "com.explorerlens.ref.watermark",
        "Watermark Overlay",
        "Applies configurable branding or copyright overlay to thumbnails",
        "1.0.0",
        PluginCapability::PostProcess | PluginCapability::Decode,
        { ".jpg", ".jpeg", ".png", ".webp", ".tiff", ".bmp", ".heic",
          ".avif", ".jxl", ".gif", ".dng" },
        "\\\\.\\pipe\\ExplorerLens_Plugin_Watermark",
        80,
        32ULL * 1024 * 1024
    };
}

struct WatermarkConfig {
    std::string text            { "© ExplorerLens" };
    uint8_t     alpha           { 128 };            // 0=transparent, 255=opaque
    uint8_t     r { 255 }, g { 255 }, b { 255 };    // text colour

    enum class Position { TopLeft, TopRight, BottomLeft, BottomRight, Center };
    Position    position        { Position::BottomRight };
    uint32_t    fontSizePt      { 10 };
    bool        scaleWithImage  { true };

    static WatermarkConfig Subtle() {
        WatermarkConfig c;
        c.alpha = 64;
        return c;
    }
    static WatermarkConfig Bold() {
        WatermarkConfig c;
        c.alpha = 200;
        c.fontSizePt = 16;
        return c;
    }
};

// ─── Plugin pack registry ─────────────────────────────────────────────────────

struct ReferencePluginPack {
    ReferencePluginDescriptor minimalGenerator  { MinimalImageGeneratorPlugin() };
    ReferencePluginDescriptor metadataReader    { MetadataOnlyPlugin() };
    ReferencePluginDescriptor watermarkOverlay  { WatermarkPlugin() };

    uint32_t PluginCount() const { return 3; }

    std::vector<ReferencePluginDescriptor> All() const {
        return { minimalGenerator, metadataReader, watermarkOverlay };
    }

    const ReferencePluginDescriptor* FindById(const std::string& id) const {
        if (minimalGenerator.id == id) return &minimalGenerator;
        if (metadataReader.id   == id) return &metadataReader;
        if (watermarkOverlay.id == id) return &watermarkOverlay;
        return nullptr;
    }
};

} // namespace ExplorerLens::Plugin

