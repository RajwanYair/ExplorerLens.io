//==============================================================================
// Model Decoder - 3D Model Thumbnail Provider
// Sprint 12: Basic 3D model format support (OBJ, FBX, STL, GLTF)
// Sprint 182: Enhanced model decoder (PLY, DAE, flat-shading lighting)
// Copyright (c) 2026 - DarkThumbs Project
//==============================================================================

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <string>

namespace DarkThumbs {
namespace Engine {

    /// <summary>
    /// Decoder for 3D model formats
    /// Supported formats: OBJ, FBX, STL (ASCII/Binary), GLTF/GLB, PLY, DAE, 3DS
    /// Renders flat-shaded preview with directional lighting using GDI
    /// GPU acceleration via Direct3D 11 available for complex models
    /// </summary>
    class ModelDecoder : public IThumbnailDecoder
    {
    public:
        ModelDecoder();
        ~ModelDecoder() override;

        // IThumbnailDecoder interface
        bool CanDecode(const wchar_t* filePath) override;
        HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
        const wchar_t** GetSupportedExtensions() const override;
        DecoderInfo GetInfo() const override;
        const wchar_t* GetName() const override;
        uint32_t GetExtensionCount() const override;
        bool SupportsGPU() const override;
        bool IsArchiveDecoder() const override;

    private:
        // Mesh data structures
        struct Vertex {
            DirectX::XMFLOAT3 position;
            DirectX::XMFLOAT3 normal;
            DirectX::XMFLOAT2 texcoord;
        };

        struct MeshData {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            DirectX::XMFLOAT3 boundsMin;
            DirectX::XMFLOAT3 boundsMax;
        };

        // Format-specific loaders
        bool LoadOBJ(const wchar_t* filePath, MeshData& mesh);
        bool LoadSTL(const wchar_t* filePath, MeshData& mesh);
        bool LoadSTLBinary(const uint8_t* data, size_t size, MeshData& mesh);
        bool LoadSTLAscii(const char* text, MeshData& mesh);
        bool LoadGLTF(const wchar_t* filePath, MeshData& mesh);
        bool LoadPLY(const wchar_t* filePath, MeshData& mesh);
        bool LoadPLYAscii(const char* data, size_t headerEnd, const std::vector<std::string>& vertexProps,
                          uint32_t vertexCount, uint32_t faceCount, MeshData& mesh);
        bool LoadPLYBinary(const uint8_t* data, size_t headerEnd, const std::vector<std::string>& vertexProps,
                           uint32_t vertexCount, uint32_t faceCount, bool littleEndian, MeshData& mesh);
        bool LoadDAE(const wchar_t* filePath, MeshData& mesh);
        
        // Rendering
        HBITMAP RenderMeshPreview(const MeshData& mesh, uint32_t width, uint32_t height);
        void CalculateBounds(MeshData& mesh);
        void ComputeFaceNormals(MeshData& mesh);
        DirectX::XMMATRIX CalculateViewMatrix(const MeshData& mesh);

        // Lighting
        struct LightParams {
            DirectX::XMFLOAT3 direction;   // Normalized light direction
            float ambient;                   // Ambient intensity [0,1]
            float diffuse;                   // Diffuse intensity [0,1]
            COLORREF baseColor;              // Base mesh color
            COLORREF backgroundColor;        // Background color
        };
        LightParams GetDefaultLighting() const;
        COLORREF ComputeLitColor(const DirectX::XMFLOAT3& normal, const LightParams& light) const;

        // Utility
        std::unique_ptr<uint8_t[]> ReadFileData(const wchar_t* filePath, size_t& outSize);

        // D3D11 resources (lazy-initialized)
        ID3D11Device* m_device;
        ID3D11DeviceContext* m_context;
        bool m_d3dInitialized;
        bool InitializeD3D();
        void CleanupD3D();

        // Supported extensions (8 formats + nullptr terminator)
        static wchar_t* s_extensions[9];
    };

} // namespace Engine
} // namespace DarkThumbs
