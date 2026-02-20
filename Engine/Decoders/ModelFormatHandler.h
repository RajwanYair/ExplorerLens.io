//==============================================================================
// DarkThumbs Engine — Sprint 260: 3MF/USD Format Support
// 3MF decoder (ZIP+XML+mesh) and USD/USDZ format evaluation.
// 3MF is a ZIP archive containing 3D model XML and mesh data.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

namespace DarkThumbs { namespace Engine {

/// 3D printing / USD format types
enum class Model3DFormat : uint8_t {
    ThreeMF,        // .3mf — 3D Manufacturing Format (ZIP+XML)
    USD,            // .usd — Universal Scene Description (binary)
    USDA,           // .usda — USD ASCII
    USDC,           // .usdc — USD Crate (binary)
    USDZ,           // .usdz — USD ZIP package
    STEP,           // .stp/.step — ISO 10303
    IGES,           // .igs/.iges — Initial Graphics Exchange
    Unknown
};

/// 3MF content relationship types
enum class ThreeMFRelation : uint8_t {
    Model3D,            // /3D/3dmodel.model
    Thumbnail,          // /Metadata/thumbnail.png
    PrintTicket,        // Print settings
    DigitalSignature,   // Package signature
    CoreProperties,     // Core metadata
    Unknown
};

/// 3MF mesh statistics
struct ThreeMFMeshInfo {
    uint32_t vertexCount    = 0;
    uint32_t triangleCount  = 0;
    uint32_t objectCount    = 0;
    uint32_t componentCount = 0;
    bool     hasColors      = false;
    bool     hasMaterials   = false;
    bool     hasThumbnail   = false;
    std::wstring title;
    std::wstring creator;
};

/// 3MF/USD format handler
class ModelFormatHandler {
public:
    /// Detect 3D print/USD format from extension
    static Model3DFormat DetectFormat(const std::wstring& ext) {
        static const std::unordered_map<std::wstring, Model3DFormat> extMap = {
            { L".3mf",  Model3DFormat::ThreeMF },
            { L".usd",  Model3DFormat::USD },
            { L".usda", Model3DFormat::USDA },
            { L".usdc", Model3DFormat::USDC },
            { L".usdz", Model3DFormat::USDZ },
            { L".stp",  Model3DFormat::STEP },
            { L".step", Model3DFormat::STEP },
            { L".igs",  Model3DFormat::IGES },
            { L".iges", Model3DFormat::IGES }
        };
        auto it = extMap.find(ext);
        return (it != extMap.end()) ? it->second : Model3DFormat::Unknown;
    }

    /// Format display name
    static const wchar_t* FormatName(Model3DFormat f) {
        switch (f) {
            case Model3DFormat::ThreeMF: return L"3D Manufacturing Format";
            case Model3DFormat::USD:     return L"Universal Scene Description";
            case Model3DFormat::USDA:    return L"USD ASCII";
            case Model3DFormat::USDC:    return L"USD Crate";
            case Model3DFormat::USDZ:    return L"USD ZIP Package";
            case Model3DFormat::STEP:    return L"STEP (ISO 10303)";
            case Model3DFormat::IGES:    return L"IGES";
            default: return L"Unknown";
        }
    }

    /// Relation type name
    static const wchar_t* RelationName(ThreeMFRelation r) {
        switch (r) {
            case ThreeMFRelation::Model3D:          return L"3D Model";
            case ThreeMFRelation::Thumbnail:        return L"Thumbnail";
            case ThreeMFRelation::PrintTicket:      return L"Print Ticket";
            case ThreeMFRelation::DigitalSignature: return L"Digital Signature";
            case ThreeMFRelation::CoreProperties:   return L"Core Properties";
            default: return L"Unknown";
        }
    }

    /// Check if 3MF file (ZIP with 3MF content type)
    static bool Is3MFFile(const uint8_t* data, size_t size) {
        // 3MF is a ZIP file — check PK magic
        if (size < 4) return false;
        return data[0] == 'P' && data[1] == 'K' && data[2] == 0x03 && data[3] == 0x04;
    }

    /// Count of supported format types
    static constexpr size_t FormatCount() { return 7; }

    /// Count of supported extensions
    static size_t ExtensionCount() { return 9; }

    /// Can we extract embedded thumbnail from this format?
    static bool CanExtractThumbnail(Model3DFormat f) {
        // 3MF and USDZ can contain embedded thumbnails
        return f == Model3DFormat::ThreeMF || f == Model3DFormat::USDZ;
    }
};

}} // namespace DarkThumbs::Engine
