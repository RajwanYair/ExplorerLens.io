//==============================================================================
// DarkThumbs Engine — Sprint 285: CDR/Visio Vector Decoder
// CorelDRAW (.cdr) and Visio (.vsd/.vsdx) vector format thumbnail generation.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace DarkThumbs { namespace Engine {

/// Vector format type
enum class VectorFormat : uint8_t {
    CDR,        // CorelDRAW
    CMX,        // Corel Metafile Exchange
    VSD,        // Visio binary
    VSDX,       // Visio Open XML
    AI,         // Adobe Illustrator
    WMF,        // Windows Metafile
    EMF,        // Enhanced Metafile
    COUNT
};

/// Vector element type
enum class VectorElement : uint8_t {
    Path,
    Rectangle,
    Ellipse,
    Text,
    Image,
    Group,
    Layer,
    COUNT
};

/// Vector decoder config
struct VectorDecoderConfig {
    uint32_t renderWidth    = 256;
    uint32_t renderHeight   = 256;
    bool     antiAlias      = true;
    bool     showGrid       = false;
    float    strokeWidthMin = 0.5f;
    uint32_t bgColor        = 0xFFFFFFFF;  // White
};

/// Vector file info
struct VectorFileInfo {
    VectorFormat format     = VectorFormat::CDR;
    uint32_t    pageCount   = 1;
    uint32_t    layerCount  = 1;
    uint32_t    objectCount = 0;
    double      widthMM     = 210.0;   // A4 default
    double      heightMM    = 297.0;
    bool        hasText     = false;
    bool        hasImages   = false;
};

/// CDR/Visio vector decoder
class VectorFormatDecoder {
public:
    static const wchar_t* FormatName(VectorFormat f) {
        switch (f) {
            case VectorFormat::CDR:  return L"CorelDRAW";
            case VectorFormat::CMX:  return L"Corel CMX";
            case VectorFormat::VSD:  return L"Visio Binary";
            case VectorFormat::VSDX: return L"Visio OOXML";
            case VectorFormat::AI:   return L"Illustrator";
            case VectorFormat::WMF:  return L"Windows Metafile";
            case VectorFormat::EMF:  return L"Enhanced Metafile";
            default: return L"Unknown";
        }
    }

    static const wchar_t* ElementName(VectorElement e) {
        switch (e) {
            case VectorElement::Path:      return L"Path";
            case VectorElement::Rectangle: return L"Rectangle";
            case VectorElement::Ellipse:   return L"Ellipse";
            case VectorElement::Text:      return L"Text";
            case VectorElement::Image:     return L"Image";
            case VectorElement::Group:     return L"Group";
            case VectorElement::Layer:     return L"Layer";
            default: return L"Unknown";
        }
    }

    static VectorFormat DetectFormat(const std::wstring& ext) {
        if (ext == L".cdr") return VectorFormat::CDR;
        if (ext == L".cmx") return VectorFormat::CMX;
        if (ext == L".vsd") return VectorFormat::VSD;
        if (ext == L".vsdx") return VectorFormat::VSDX;
        if (ext == L".ai") return VectorFormat::AI;
        if (ext == L".wmf") return VectorFormat::WMF;
        if (ext == L".emf") return VectorFormat::EMF;
        return VectorFormat::CDR;
    }

    static constexpr size_t FormatCount() { return static_cast<size_t>(VectorFormat::COUNT); }
    static constexpr size_t ElementCount() { return static_cast<size_t>(VectorElement::COUNT); }
};

}} // namespace DarkThumbs::Engine
