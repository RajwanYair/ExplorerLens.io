//==============================================================================
// ExplorerLens Engine — STEP/IGES CAD Decoder
// ISO 10303 STEP and IGES CAD file thumbnail generation.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// CAD format type
enum class CADFormat : uint8_t {
    STEP_AP203,  // STEP AP203 — config-controlled design
    STEP_AP214,  // STEP AP214 — automotive design
    STEP_AP242,  // STEP AP242 — model-based 3D
    IGES,        // Initial Graphics Exchange Specification
    COUNT
};

/// CAD entity type
enum class CADEntity : uint8_t {
    Point,
    Line,
    Arc,
    Circle,
    Surface,
    Solid,
    Assembly,
    COUNT
};

/// CAD rendering mode
enum class CADRenderMode : uint8_t {
    Wireframe,
    HiddenLine,
    Shaded,
    ShadedEdges,
    Xray,
    COUNT
};

/// CAD file metadata
struct CADFileInfo
{
    CADFormat format = CADFormat::STEP_AP203;
    uint32_t entityCount = 0;
    uint32_t assemblyCount = 0;
    uint32_t partCount = 0;
    uint32_t surfaceCount = 0;
    std::wstring author;
    std::wstring organization;
    std::wstring schema;  // e.g. "AUTOMOTIVE_DESIGN"
};

/// CAD preview config
struct CADPreviewConfig
{
    CADRenderMode renderMode = CADRenderMode::Shaded;
    uint32_t thumbnailSize = 256;
    float cameraAzimuth = 45.0f;
    float cameraElevation = 30.0f;
    bool showAxes = true;
    bool showBoundingBox = false;
    uint32_t bgColor = 0xFFF0F0F0;  // Light gray
};

/// STEP/IGES CAD decoder
class CADFormatDecoder
{
  public:
    static const wchar_t* FormatName(CADFormat f)
    {
        switch (f) {
            case CADFormat::STEP_AP203:
                return L"STEP AP203";
            case CADFormat::STEP_AP214:
                return L"STEP AP214";
            case CADFormat::STEP_AP242:
                return L"STEP AP242";
            case CADFormat::IGES:
                return L"IGES";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* EntityName(CADEntity e)
    {
        switch (e) {
            case CADEntity::Point:
                return L"Point";
            case CADEntity::Line:
                return L"Line";
            case CADEntity::Arc:
                return L"Arc";
            case CADEntity::Circle:
                return L"Circle";
            case CADEntity::Surface:
                return L"Surface";
            case CADEntity::Solid:
                return L"Solid";
            case CADEntity::Assembly:
                return L"Assembly";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* RenderModeName(CADRenderMode m)
    {
        switch (m) {
            case CADRenderMode::Wireframe:
                return L"Wireframe";
            case CADRenderMode::HiddenLine:
                return L"Hidden Line";
            case CADRenderMode::Shaded:
                return L"Shaded";
            case CADRenderMode::ShadedEdges:
                return L"Shaded + Edges";
            case CADRenderMode::Xray:
                return L"X-Ray";
            default:
                return L"Unknown";
        }
    }

    /// STEP magic: "ISO-10303-21;" in first line
    static bool CheckSTEPMagic(const uint8_t* data, size_t size)
    {
        if (size < 13)
            return false;
        const char magic[] = "ISO-10303-21;";
        for (int i = 0; i < 13; ++i)
            if (data[i] != static_cast<uint8_t>(magic[i]))
                return false;
        return true;
    }

    static constexpr size_t FormatCount()
    {
        return static_cast<size_t>(CADFormat::COUNT);
    }
    static constexpr size_t EntityCount()
    {
        return static_cast<size_t>(CADEntity::COUNT);
    }
    static constexpr size_t RenderModeCount()
    {
        return static_cast<size_t>(CADRenderMode::COUNT);
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
