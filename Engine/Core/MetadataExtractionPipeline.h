#pragma once
// Sprint 407: Metadata Extraction Pipeline
// Unified EXIF/XMP/IPTC/ICC metadata extraction with overlay rendering on
// thumbnails — camera info, GPS, color profile, copyright, and ratings.
#include <cstdint>
#include <cstring>

namespace ExplorerLens {
namespace Engine {

/// Metadata source
enum class MetadataSource : uint8_t {
  EXIF = 0,
  XMP,
  IPTC,
  ICC,
  PNG_tEXt,
  TIFF_Tags,
  PDF_Info,
  ID3, // Audio
  COUNT
};

/// Overlay position for metadata badge on thumbnail
enum class MetaOverlayPos : uint8_t {
  None = 0,
  TopLeft,
  TopRight,
  BottomLeft,
  BottomRight,
  CenterBottom,
  COUNT
};

struct MetaFieldEntry {
  const wchar_t *key = nullptr;
  const wchar_t *value = nullptr;
  MetadataSource source = MetadataSource::EXIF;
};

struct MetadataOverlayConfig {
  MetaOverlayPos position = MetaOverlayPos::BottomLeft;
  bool showResolution = true;
  bool showColorSpace = false;
  bool showCamera = false;
  bool showFileSize = true;
  float badgeOpacity = 0.8f;
  uint32_t maxBadgeWidthPx = 120;
};

class MetadataExtractionPipeline {
public:
  static constexpr size_t SourceCount() {
    return static_cast<size_t>(MetadataSource::COUNT);
  }
  static constexpr size_t PositionCount() {
    return static_cast<size_t>(MetaOverlayPos::COUNT);
  }

  static const wchar_t *SourceName(MetadataSource s) {
    switch (s) {
    case MetadataSource::EXIF:
      return L"EXIF";
    case MetadataSource::XMP:
      return L"XMP";
    case MetadataSource::IPTC:
      return L"IPTC";
    case MetadataSource::ICC:
      return L"ICC Profile";
    case MetadataSource::PNG_tEXt:
      return L"PNG tEXt";
    case MetadataSource::TIFF_Tags:
      return L"TIFF Tags";
    case MetadataSource::PDF_Info:
      return L"PDF Info";
    case MetadataSource::ID3:
      return L"ID3";
    default:
      return L"Unknown";
    }
  }

  static const wchar_t *PositionName(MetaOverlayPos p) {
    switch (p) {
    case MetaOverlayPos::None:
      return L"None";
    case MetaOverlayPos::TopLeft:
      return L"Top Left";
    case MetaOverlayPos::TopRight:
      return L"Top Right";
    case MetaOverlayPos::BottomLeft:
      return L"Bottom Left";
    case MetaOverlayPos::BottomRight:
      return L"Bottom Right";
    case MetaOverlayPos::CenterBottom:
      return L"Center Bottom";
    default:
      return L"Unknown";
    }
  }

  /// Format file size to human-readable string
  static const wchar_t *FormatFileSize(uint64_t bytes) {
    if (bytes < 1024)
      return L"< 1 KB";
    if (bytes < 1024 * 1024)
      return L"KB";
    if (bytes < 1024ULL * 1024 * 1024)
      return L"MB";
    return L"GB";
  }
};

} // namespace Engine
} // namespace ExplorerLens
