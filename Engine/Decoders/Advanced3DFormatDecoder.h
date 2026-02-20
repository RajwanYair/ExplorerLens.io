#pragma once
//==============================================================================
// Advanced3DFormatDecoder — Sprint 200
// Extended 3D format support: FBX (Autodesk), USD (Pixar/Apple), 3MF (3D print)
//
// Design:
//   - FBX binary header detection and node parsing
//   - USD/USDA/USDC/USDZ container handling
//   - 3MF (Open Packaging) mesh extraction
//   - Bounding box computation and auto-camera placement
//   - Wireframe + flat-shaded rendering modes
//==============================================================================

#include <cstdint>
#include <string>
#include <vector>
#include <array>

namespace DarkThumbs { namespace Engine {

/// 3D format type
enum class Advanced3DFormat : uint8_t {
    Unknown,
    FBX_Binary,      ///< Autodesk FBX binary format
    FBX_ASCII,       ///< Autodesk FBX ASCII format
    USDA,            ///< USD ASCII
    USDC,            ///< USD Crate (binary)
    USDZ,            ///< USD ZIP package
    ThreeMF,         ///< 3D Manufacturing Format (ZIP + XML)
    STEP,            ///< STEP/STP CAD format
    IGES             ///< IGES CAD format
};

/// 3D vertex
struct Vertex3D {
    float x = 0.0f, y = 0.0f, z = 0.0f;
};

/// 3D bounding box
struct BoundingBox3D {
    Vertex3D min{1e30f, 1e30f, 1e30f};
    Vertex3D max{-1e30f, -1e30f, -1e30f};

    void Expand(const Vertex3D& v) {
        if (v.x < min.x) min.x = v.x;
        if (v.y < min.y) min.y = v.y;
        if (v.z < min.z) min.z = v.z;
        if (v.x > max.x) max.x = v.x;
        if (v.y > max.y) max.y = v.y;
        if (v.z > max.z) max.z = v.z;
    }

    Vertex3D Center() const {
        return {(min.x + max.x) / 2.0f, (min.y + max.y) / 2.0f, (min.z + max.z) / 2.0f};
    }

    float Diagonal() const;
    bool IsValid() const { return min.x < max.x; }
};

/// Camera auto-placement parameters
struct AutoCamera {
    Vertex3D position;
    Vertex3D target;
    Vertex3D up{0.0f, 1.0f, 0.0f};
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    static AutoCamera FromBoundingBox(const BoundingBox3D& bbox);
};

/// 3D mesh info
struct MeshInfo3D {
    uint32_t vertexCount = 0;
    uint32_t triangleCount = 0;
    uint32_t materialCount = 0;
    Advanced3DFormat format = Advanced3DFormat::Unknown;
    BoundingBox3D bounds;
    std::wstring name;
    std::wstring units;
    bool hasNormals = false;
    bool hasUVs = false;
    bool hasColors = false;
};

/// 3D render mode
enum class Render3DMode : uint8_t {
    Wireframe,
    FlatShaded,
    SmoothShaded,
    Textured
};

//==============================================================================
// Advanced3DFormatDecoder
//==============================================================================
class Advanced3DFormatDecoder {
public:
    Advanced3DFormatDecoder();

    /// Detect format from file data
    static Advanced3DFormat DetectFormat(const uint8_t* data, size_t size);

    /// Parse mesh from file data
    bool Parse(const uint8_t* data, size_t size);

    /// Get mesh info
    const MeshInfo3D& GetMeshInfo() const { return m_meshInfo; }

    /// Get auto-camera for the mesh
    AutoCamera GetAutoCamera() const;

    /// Render thumbnail (BGRA output)
    std::vector<uint8_t> RenderThumbnail(uint32_t width, uint32_t height,
                                          Render3DMode mode = Render3DMode::FlatShaded) const;

    /// Get vertices for wireframe rendering
    const std::vector<Vertex3D>& GetVertices() const { return m_vertices; }
    const std::vector<std::array<uint32_t, 3>>& GetTriangles() const { return m_triangles; }

    /// Get supported extensions
    static const wchar_t* const* GetExtensions();
    static uint32_t GetExtensionCount();

    /// Get format name
    static const wchar_t* GetFormatName(Advanced3DFormat fmt);
    static const wchar_t* GetRenderModeName(Render3DMode mode);

private:
    MeshInfo3D m_meshInfo;
    std::vector<Vertex3D> m_vertices;
    std::vector<std::array<uint32_t, 3>> m_triangles;

    bool ParseFBXBinary(const uint8_t* data, size_t size);
    bool ParseUSDA(const uint8_t* data, size_t size);
    bool Parse3MF(const uint8_t* data, size_t size);
    void ComputeBounds();
};

}} // namespace DarkThumbs::Engine
