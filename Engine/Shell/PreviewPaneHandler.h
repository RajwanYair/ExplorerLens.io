//==============================================================================
// ExplorerLens Engine — Preview Pane & Rich Tooltip Integration
//
// Provides IPreviewHandler framework, property store handler for Explorer
// Details pane, EXIF metadata extraction for rich tooltips, column handler
// for additional Explorer columns, and tooltip content formatting.
//==============================================================================
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <functional>
#include <algorithm>

namespace ExplorerLens::Engine::Shell {

//==============================================================================
// Preview Mode — How the Preview Pane renders content
//==============================================================================

enum class PreviewMode : uint8_t {
    Thumbnail,      // Scaled-down static preview
    FullImage,      // Full decoded image (fit to pane)
    EXIF,           // Metadata-focused view with small preview
    SideBySide,     // Image + metadata panel
    Unsupported     // Format not previewable
};

inline const char* PreviewModeName(PreviewMode m) {
    switch (m) {
        case PreviewMode::Thumbnail:   return "Thumbnail";
        case PreviewMode::FullImage:   return "Full Image";
        case PreviewMode::EXIF:        return "EXIF Metadata";
        case PreviewMode::SideBySide:  return "Side-by-Side";
        case PreviewMode::Unsupported: return "Unsupported";
        default:                       return "Unknown";
    }
}

//==============================================================================
// EXIF Field — Single metadata key-value pair
//==============================================================================

struct EXIFField {
    std::string tag;       // e.g., "Make", "Model", "ExposureTime"
    std::string label;     // Human-readable: "Camera Make"
    std::string value;     // "Canon"
    std::string unit;      // "" (none), "px", "mm", "ISO", "sec"
    bool        isImportant = false; // Show in compact tooltip

    std::string FormattedValue() const {
        if (unit.empty()) return value;
        return value + " " + unit;
    }

    bool IsEmpty() const { return value.empty(); }
};

//==============================================================================
// Image Dimensions — Resolution info
//==============================================================================

struct ImageDimensions {
    uint32_t width  = 0;
    uint32_t height = 0;
    uint32_t dpiX   = 72;
    uint32_t dpiY   = 72;
    uint8_t  bitDepth   = 8;
    uint8_t  channels   = 3;
    bool     hasAlpha   = false;
    bool     isAnimated = false;
    uint32_t frameCount = 1;

    uint64_t PixelCount() const {
        return static_cast<uint64_t>(width) * static_cast<uint64_t>(height);
    }

    double MegaPixels() const {
        return static_cast<double>(PixelCount()) / 1000000.0;
    }

    std::string ResolutionText() const {
        return std::to_string(width) + " x " + std::to_string(height);
    }

    std::string DetailedText() const {
        std::ostringstream ss;
        ss << width << " x " << height
           << " (" << std::fixed << std::setprecision(1) << MegaPixels() << " MP)"
           << ", " << static_cast<int>(bitDepth) << "-bit";
        if (hasAlpha) ss << "+alpha";
        if (isAnimated) ss << ", " << frameCount << " frames";
        return ss.str();
    }

    double AspectRatio() const {
        if (height == 0) return 0.0;
        return static_cast<double>(width) / static_cast<double>(height);
    }

    bool IsPortrait()  const { return height > width; }
    bool IsLandscape() const { return width > height; }
    bool IsSquare()    const { return width == height && width > 0; }
};

//==============================================================================
// Camera Info — Extracted camera metadata
//==============================================================================

struct CameraInfo {
    std::string make;           // "Canon", "Nikon", "Sony"
    std::string model;          // "EOS R5", "Z9"
    std::string lens;           // "RF 24-70mm F2.8"
    std::string software;       // "Adobe Lightroom 15.0"
    std::string dateTime;       // "2026:02:17 14:30:00"
    double      focalLength = 0.0;
    double      aperture    = 0.0;
    double      exposureTime = 0.0;
    uint32_t    isoSpeed    = 0;
    bool        flashFired  = false;

    bool HasCamera() const { return !make.empty() || !model.empty(); }
    bool HasLens()   const { return !lens.empty(); }

    std::string CameraName() const {
        if (make.empty() && model.empty()) return "";
        if (make.empty()) return model;
        if (model.empty()) return make;
        return make + " " + model;
    }

    std::string ExposureDescription() const {
        std::ostringstream ss;
        if (aperture > 0) ss << "f/" << std::fixed << std::setprecision(1) << aperture;
        if (exposureTime > 0) {
            if (ss.tellp() > 0) ss << "  ";
            if (exposureTime >= 1.0) {
                ss << std::setprecision(1) << exposureTime << "s";
            } else {
                ss << "1/" << static_cast<int>(1.0 / exposureTime) << "s";
            }
        }
        if (isoSpeed > 0) {
            if (ss.tellp() > 0) ss << "  ";
            ss << "ISO " << isoSpeed;
        }
        if (focalLength > 0) {
            if (ss.tellp() > 0) ss << "  ";
            ss << std::setprecision(0) << focalLength << "mm";
        }
        return ss.str();
    }
};

//==============================================================================
// GPS Info — Location metadata
//==============================================================================

struct GPSInfo {
    double latitude  = 0.0;
    double longitude = 0.0;
    double altitude  = 0.0;
    bool   hasLocation = false;
    bool   hasAltitude = false;

    std::string LocationText() const {
        if (!hasLocation) return "";
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(6)
           << latitude << ", " << longitude;
        if (hasAltitude) {
            ss << " (" << std::setprecision(0) << altitude << "m)";
        }
        return ss.str();
    }
};

//==============================================================================
// File Metadata — Aggregated file + image info
//==============================================================================

struct FileMetadata {
    // File info
    std::string   filePath;
    std::string   fileName;
    std::string   extension;
    std::string   formatName;     // "JPEG XL", "HEIF", "WebP"
    std::string   codecName;      // Internal decoder name
    uint64_t      fileSize = 0;
    int64_t       modifiedTime = 0;

    // Image info
    ImageDimensions dimensions;
    CameraInfo      camera;
    GPSInfo         gps;
    std::string     colorSpace;   // "sRGB", "Display P3"
    std::string     colorProfile; // ICC profile name
    bool            isHDR = false;

    // Custom EXIF fields
    std::vector<EXIFField> exifFields;

    //--- Derived info ---

    std::string FileSizeHuman() const {
        if (fileSize >= 1024ULL * 1024ULL * 1024ULL) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1)
               << (static_cast<double>(fileSize) / (1024.0 * 1024.0 * 1024.0)) << " GB";
            return ss.str();
        }
        if (fileSize >= 1024ULL * 1024ULL) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1)
               << (static_cast<double>(fileSize) / (1024.0 * 1024.0)) << " MB";
            return ss.str();
        }
        if (fileSize >= 1024ULL) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1)
               << (static_cast<double>(fileSize) / 1024.0) << " KB";
            return ss.str();
        }
        return std::to_string(fileSize) + " B";
    }

    double CompressionRatio() const {
        uint64_t rawSize = dimensions.PixelCount() * dimensions.channels *
                           (dimensions.bitDepth / 8);
        if (rawSize == 0 || fileSize == 0) return 0.0;
        return static_cast<double>(rawSize) / static_cast<double>(fileSize);
    }
};

//==============================================================================
// Tooltip Content — Formatted tooltip sections
//==============================================================================

struct TooltipContent {
    std::string title;      // File name
    std::string subtitle;   // Format + dimensions
    std::vector<std::pair<std::string, std::string>> fields;  // label: value pairs

    void AddField(const std::string& label, const std::string& value) {
        if (!value.empty()) {
            fields.emplace_back(label, value);
        }
    }

    size_t FieldCount() const { return fields.size(); }
    bool   IsEmpty()    const { return title.empty() && fields.empty(); }

    std::string AsPlainText() const {
        std::ostringstream ss;
        if (!title.empty()) ss << title << "\n";
        if (!subtitle.empty()) ss << subtitle << "\n";
        if (!fields.empty()) ss << "---\n";
        for (const auto& [label, value] : fields) {
            ss << label << ": " << value << "\n";
        }
        return ss.str();
    }

    static TooltipContent FromMetadata(const FileMetadata& meta) {
        TooltipContent tip;
        tip.title    = meta.fileName;
        tip.subtitle = meta.formatName + "  " + meta.dimensions.ResolutionText();

        tip.AddField("Size", meta.FileSizeHuman());
        tip.AddField("Dimensions", meta.dimensions.DetailedText());
        tip.AddField("Color Space", meta.colorSpace);

        if (meta.camera.HasCamera()) {
            tip.AddField("Camera", meta.camera.CameraName());
        }
        if (!meta.camera.ExposureDescription().empty()) {
            tip.AddField("Exposure", meta.camera.ExposureDescription());
        }
        if (meta.camera.HasLens()) {
            tip.AddField("Lens", meta.camera.lens);
        }
        if (meta.gps.hasLocation) {
            tip.AddField("Location", meta.gps.LocationText());
        }
        if (meta.isHDR) {
            tip.AddField("HDR", "Yes");
        }

        return tip;
    }
};

//==============================================================================
// Property Column — Explorer "Details" column definition
//==============================================================================

struct PropertyColumn {
    std::string id;       // Property system ID
    std::string label;    // Column header text
    uint32_t    widthPx;  // Default width
    bool        visible;  // Show by default

    static std::vector<PropertyColumn> DefaultColumns() {
        return {
            {"ExplorerLens.Format",     "Image Format",  90, true},
            {"ExplorerLens.Dimensions", "Dimensions",   120, true},
            {"ExplorerLens.Codec",      "Decoder",       80, false},
            {"ExplorerLens.ColorSpace", "Color Space",   90, false},
            {"ExplorerLens.DecodeTime", "Decode Time",   80, false},
            {"ExplorerLens.Camera",     "Camera",       120, false},
            {"ExplorerLens.Exposure",   "Exposure",     150, false},
        };
    }

    static size_t VisibleCount(const std::vector<PropertyColumn>& cols) {
        return std::count_if(cols.begin(), cols.end(),
                            [](const PropertyColumn& c) { return c.visible; });
    }
};

//==============================================================================
// Preview Pane Config — Settings for preview rendering
//==============================================================================

struct PreviewPaneConfig {
    PreviewMode defaultMode      = PreviewMode::FullImage;
    uint32_t    maxPreviewWidth   = 1920;
    uint32_t    maxPreviewHeight  = 1080;
    bool        showEXIFOverlay   = true;
    bool        showHistogram     = false;
    bool        enableZoom        = true;
    double      maxZoomFactor     = 8.0;
    bool        showColorProfile  = false;

    static PreviewPaneConfig Default() {
        return {};
    }

    static PreviewPaneConfig Minimal() {
        PreviewPaneConfig c;
        c.showEXIFOverlay  = false;
        c.showHistogram    = false;
        c.enableZoom       = false;
        c.showColorProfile = false;
        return c;
    }

    static PreviewPaneConfig Photographer() {
        PreviewPaneConfig c;
        c.showEXIFOverlay  = true;
        c.showHistogram    = true;
        c.enableZoom       = true;
        c.maxZoomFactor    = 16.0;
        c.showColorProfile = true;
        return c;
    }
};

} // namespace ExplorerLens::Engine::Shell

