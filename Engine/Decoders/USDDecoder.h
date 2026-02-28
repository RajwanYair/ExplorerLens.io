//==============================================================================
// ExplorerLens Engine — USD/USDZ Support
// Pixar Universal Scene Description decoder for 3D scene thumbnails.
//==============================================================================
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

/// USD stage element type
enum class USDElementType : uint8_t {
 Mesh, // Geometry mesh
 Camera, // Camera definition
 Light, // Light source
 Material, // PBR material
 Xform, // Transform hierarchy
 Scope, // Scoping group
 Shader, // Shader network
 COUNT
};

/// USD file variant
enum class USDVariant : uint8_t {
 USDA, // ASCII text format
 USDC, // Binary crate format
 USDZ, // Zip-packaged (AR-ready)
 COUNT
};

/// USD scene metadata
struct USDSceneInfo {
 USDVariant variant = USDVariant::USDA;
 uint32_t meshCount = 0;
 uint32_t materialCount = 0;
 uint32_t cameraCount = 0;
 uint32_t lightCount = 0;
 uint64_t totalVertices = 0;
 uint64_t totalFaces = 0;
 double metersPerUnit = 0.01; // Default centimeters
 std::wstring defaultPrim;
 bool hasAnimations = false;
 bool hasTextures = false;
};

/// USD decoder configuration
struct USDDecoderConfig {
 uint32_t maxMeshVertices = 1000000;
 uint32_t thumbnailSize = 256;
 bool enablePBR = true;
 bool enableTextures = true;
 bool wireframeMode = false;
 double cameraDistance = 2.0;
};

/// USD/USDZ decoder
class USDDecoder {
public:
 static const wchar_t* ElementName(USDElementType e) {
 switch (e) {
 case USDElementType::Mesh: return L"Mesh";
 case USDElementType::Camera: return L"Camera";
 case USDElementType::Light: return L"Light";
 case USDElementType::Material: return L"Material";
 case USDElementType::Xform: return L"Xform";
 case USDElementType::Scope: return L"Scope";
 case USDElementType::Shader: return L"Shader";
 default: return L"Unknown";
 }
 }

 static const wchar_t* VariantName(USDVariant v) {
 switch (v) {
 case USDVariant::USDA: return L"USD ASCII";
 case USDVariant::USDC: return L"USD Crate";
 case USDVariant::USDZ: return L"USDZ Package";
 default: return L"Unknown";
 }
 }

 /// Detect USD variant from extension
 static USDVariant DetectVariant(const std::wstring& ext) {
 if (ext == L".usda" || ext == L".usd") return USDVariant::USDA;
 if (ext == L".usdc") return USDVariant::USDC;
 if (ext == L".usdz") return USDVariant::USDZ;
 return USDVariant::USDA;
 }

 /// USDZ magic bytes check (ZIP PK header)
 static bool CheckUSDZMagic(const uint8_t* data, size_t size) {
 if (size < 4) return false;
 return data[0] == 0x50 && data[1] == 0x4B && data[2] == 0x03 && data[3] == 0x04;
 }

 static constexpr size_t ElementCount() { return static_cast<size_t>(USDElementType::COUNT); }
 static constexpr size_t VariantCount() { return static_cast<size_t>(USDVariant::COUNT); }
};

}} // namespace ExplorerLens::Engine

