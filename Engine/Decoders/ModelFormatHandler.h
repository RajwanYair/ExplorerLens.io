//==============================================================================
// ExplorerLens Engine — 3MF/USD Format Support
// 3MF decoder (ZIP+XML+mesh) and USD/USDZ format evaluation.
// 3MF is a ZIP archive containing 3D model XML and mesh data.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// 3D printing / CAD format types (distinct from Model3DFormat in
/// Model3DRendererV2.h)
enum class PrintModel3DFormat : uint8_t {
  ThreeMF, // .3mf — 3D Manufacturing Format (ZIP+XML)
  USD,     // .usd — Universal Scene Description (binary)
  USDA,    // .usda — USD ASCII
  USDC,    // .usdc — USD Crate (binary)
  USDZ,    // .usdz — USD ZIP package
  STEP,    // .stp/.step — ISO 10303
  IGES,    // .igs/.iges — Initial Graphics Exchange
  Unknown
};

/// 3MF content relationship types
enum class ThreeMFRelation : uint8_t {
  Model3D,          // /3D/3dmodel.model
  Thumbnail,        // /Metadata/thumbnail.png
  PrintTicket,      // Print settings
  DigitalSignature, // Package signature
  CoreProperties,   // Core metadata
  Unknown
};

/// 3MF mesh statistics
struct ThreeMFMeshInfo {
  uint32_t vertexCount = 0;
  uint32_t triangleCount = 0;
  uint32_t objectCount = 0;
  uint32_t componentCount = 0;
  bool hasColors = false;
  bool hasMaterials = false;
  bool hasThumbnail = false;
  std::wstring title;
  std::wstring creator;
};

/// 3MF/USD format handler
class ModelFormatHandler {
public:
  /// Detect 3D print/USD format from extension
  static PrintModel3DFormat DetectFormat(const std::wstring &ext) {
    static const std::unordered_map<std::wstring, PrintModel3DFormat> extMap = {
        {L".3mf", PrintModel3DFormat::ThreeMF},
        {L".usd", PrintModel3DFormat::USD},
        {L".usda", PrintModel3DFormat::USDA},
        {L".usdc", PrintModel3DFormat::USDC},
        {L".usdz", PrintModel3DFormat::USDZ},
        {L".stp", PrintModel3DFormat::STEP},
        {L".step", PrintModel3DFormat::STEP},
        {L".igs", PrintModel3DFormat::IGES},
        {L".iges", PrintModel3DFormat::IGES}};
    auto it = extMap.find(ext);
    return (it != extMap.end()) ? it->second : PrintModel3DFormat::Unknown;
  }

  /// Format display name
  static const wchar_t *FormatName(PrintModel3DFormat f) {
    switch (f) {
    case PrintModel3DFormat::ThreeMF:
      return L"3D Manufacturing Format";
    case PrintModel3DFormat::USD:
      return L"Universal Scene Description";
    case PrintModel3DFormat::USDA:
      return L"USD ASCII";
    case PrintModel3DFormat::USDC:
      return L"USD Crate";
    case PrintModel3DFormat::USDZ:
      return L"USD ZIP Package";
    case PrintModel3DFormat::STEP:
      return L"STEP (ISO 10303)";
    case PrintModel3DFormat::IGES:
      return L"IGES";
    default:
      return L"Unknown";
    }
  }

  /// Relation type name
  static const wchar_t *RelationName(ThreeMFRelation r) {
    switch (r) {
    case ThreeMFRelation::Model3D:
      return L"3D Model";
    case ThreeMFRelation::Thumbnail:
      return L"Thumbnail";
    case ThreeMFRelation::PrintTicket:
      return L"Print Ticket";
    case ThreeMFRelation::DigitalSignature:
      return L"Digital Signature";
    case ThreeMFRelation::CoreProperties:
      return L"Core Properties";
    default:
      return L"Unknown";
    }
  }

  /// Check if 3MF file (ZIP with 3MF content type)
  static bool Is3MFFile(const uint8_t *data, size_t size) {
    // 3MF is a ZIP file — check PK magic
    if (size < 4)
      return false;
    return data[0] == 'P' && data[1] == 'K' && data[2] == 0x03 &&
           data[3] == 0x04;
  }

  /// Count of supported format types
  static constexpr size_t FormatCount() { return 7; }

  /// Count of supported extensions
  static size_t ExtensionCount() { return 9; }

  /// Can we extract embedded thumbnail from this format?
  static bool CanExtractThumbnail(PrintModel3DFormat f) {
    // 3MF and USDZ can contain embedded thumbnails
    return f == PrintModel3DFormat::ThreeMF || f == PrintModel3DFormat::USDZ;
  }
};

} // namespace Engine
} // namespace ExplorerLens
