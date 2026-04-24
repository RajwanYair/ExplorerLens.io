// ============================================================================
// TinyGltfModelDecoderStub.h -- S267 / ROADMAP v6.0 L13 3D model thumbnails
//
// Evaluation contract for a unified 3D-model decoder pipeline (glTF/GLB/OBJ/
// STL/PLY).  Parses geometry with tinygltf / tinyobjloader / tinystl, then
// rasterises on the shared D3D11 preview renderer.  Replaces ModelDecoder
// for the roadmap Phase 3 unification work.
// ============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ExplorerLens::Engine {

enum class TinyGltfInputFormat : uint8_t
{
    AUTO_DETECT  = 0,
    GLTF_JSON    = 1,   // *.gltf + external buffers
    GLTF_BINARY  = 2,   // *.glb
    OBJ          = 3,
    STL_ASCII    = 4,
    STL_BINARY   = 5,
    PLY_ASCII    = 6,
    PLY_BINARY   = 7,
};

enum class TinyGltfStatus : uint8_t
{
    OK                       = 0,
    FORMAT_UNRECOGNISED      = 1,
    JSON_PARSE_ERROR         = 2,
    BUFFER_MISSING           = 3,
    EXTERNAL_URI_BLOCKED     = 4,  // security: no http(s), no absolute FS paths
    TRIANGLE_BUDGET_EXCEEDED = 5,
    RENDER_INIT_FAILED       = 6,
    BUDGET_EXCEEDED          = 7,
};

struct TinyGltfCameraPose
{
    float yawDeg     = 35.0f;   // "hero" angle
    float pitchDeg   = -15.0f;
    float fovDeg     = 45.0f;
    float zoomFactor = 1.0f;
};

struct TinyGltfDecodeOptions
{
    TinyGltfInputFormat input        = TinyGltfInputFormat::AUTO_DETECT;
    TinyGltfCameraPose  pose         = {};
    uint32_t            targetWidth  = 256;
    uint32_t            targetHeight = 256;
    uint32_t            msaaSamples  = 4;
    bool                pbrLighting  = true;
    bool                wireframeOverlay = false;
    uint32_t            budgetMs     = 400;
};

struct TinyGltfProbeResult
{
    TinyGltfStatus      status         = TinyGltfStatus::OK;
    TinyGltfInputFormat detectedFormat = TinyGltfInputFormat::AUTO_DETECT;
    uint64_t            triangleCount  = 0;
    uint64_t            vertexCount    = 0;
    uint32_t            materialCount  = 0;
    uint32_t            textureCount   = 0;
    bool                hasPbrMaterials = false;
    bool                hasAnimations   = false;
    bool                hasSkinning     = false;
};

inline constexpr uint64_t kTinyGltfMaxTriangles      = 20ull * 1000ull * 1000ull; // 20 M tris
inline constexpr uint64_t kTinyGltfMaxSourceBytes    = 512ull * 1024ull * 1024ull; // 512 MiB
inline constexpr uint32_t kTinyGltfDefaultBudgetMs   = 400;
inline constexpr uint32_t kTinyGltfHardBudgetMs      = 2000;

static_assert(std::is_trivially_copyable_v<TinyGltfDecodeOptions>,
              "TinyGltfDecodeOptions must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TinyGltfProbeResult>,
              "TinyGltfProbeResult must be trivially copyable");

} // namespace ExplorerLens::Engine
