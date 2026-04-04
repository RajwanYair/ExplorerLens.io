//==============================================================================
// ExplorerLens Engine — 3D Model Renderer V2
// Enhanced rendering for glTF 2.0, USDZ, FBX, and STL with PBR lighting,
// turntable camera, and LOD selection for high-quality 3D thumbnails.
//==============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class Model3DFormat : uint8_t {
    GLTF2 = 0,
    USDZ,
    FBX,
    OBJ,
    STL,
    PLY,
    COUNT
};

enum class Model3DLightingMode : uint8_t {
    Studio = 0,    // Softbox studio lighting
    Outdoor,       // HDRI sky lighting
    Flat,          // Flat shading (low-res preview)
    Wireframe,     // Wireframe overlay
    PBR = Studio,  // compat alias
    COUNT = Wireframe + 1
};

enum class Model3DCameraPreset : uint8_t {
    FrontIso = 0,      // Front isometric
    TopDown,           // Top orthographic
    TurntableAuto,     // Auto-selected turntable angle
    BestAngle,         // Content-aware best viewpoint
    Front = FrontIso,  // compat alias
    COUNT = BestAngle + 1
};

struct Model3DRenderConfig
{
    Model3DLightingMode lightingMode = Model3DLightingMode::Studio;
    Model3DCameraPreset cameraPreset = Model3DCameraPreset::BestAngle;
    uint32_t renderWidth = 256;
    uint32_t renderHeight = 256;
    bool enablePBR = true;
    bool enableShadows = false;
    uint32_t msaaSamples = 4;
};

struct Model3DMetadata
{
    Model3DFormat format = Model3DFormat::GLTF2;
    uint32_t triangleCount = 0;
    uint32_t materialCount = 0;
    uint32_t meshCount = 0;
    bool hasAnimations = false;
    bool hasPBRMaterials = false;
};

class Model3DRendererV2
{
  public:
    static const wchar_t* FormatName(Model3DFormat f)
    {
        switch (f) {
            case Model3DFormat::GLTF2:
                return L"glTF 2.0";
            case Model3DFormat::USDZ:
                return L"USDZ";
            case Model3DFormat::FBX:
                return L"FBX";
            case Model3DFormat::OBJ:
                return L"OBJ";
            case Model3DFormat::STL:
                return L"STL";
            case Model3DFormat::PLY:
                return L"PLY";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* LightingName(Model3DLightingMode l)
    {
        switch (l) {
            case Model3DLightingMode::Studio:
                return L"Studio";
            case Model3DLightingMode::Outdoor:
                return L"Outdoor HDRI";
            case Model3DLightingMode::Flat:
                return L"Flat";
            case Model3DLightingMode::Wireframe:
                return L"Wireframe";
            default:
                return L"Unknown";
        }
    }

    static const wchar_t* CameraPresetName(Model3DCameraPreset c)
    {
        switch (c) {
            case Model3DCameraPreset::FrontIso:
                return L"Front Isometric";
            case Model3DCameraPreset::TopDown:
                return L"Top Down";
            case Model3DCameraPreset::TurntableAuto:
                return L"Turntable Auto";
            case Model3DCameraPreset::BestAngle:
                return L"Best Angle";
            default:
                return L"Unknown";
        }
    }

    static constexpr size_t FormatCount()
    {
        return static_cast<size_t>(Model3DFormat::COUNT);
    }
    static constexpr size_t LightingCount()
    {
        return static_cast<size_t>(Model3DLightingMode::COUNT);
    }
    static constexpr size_t CameraPresetCount()
    {
        return static_cast<size_t>(Model3DCameraPreset::COUNT);
    }

    // Compatibility aliases (tests)
    static const wchar_t* LightingModeName(Model3DLightingMode l)
    {
        return LightingName(l);
    }
    static constexpr size_t LightingModeCount()
    {
        return LightingCount();
    }

    static Model3DRenderConfig DefaultConfig()
    {
        return Model3DRenderConfig{};
    }
};

}  // namespace Engine
}  // namespace ExplorerLens
