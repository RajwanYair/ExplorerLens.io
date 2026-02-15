//==============================================================================
// Model Decoder - 3D Model Thumbnail Provider
// Sprint 12: Basic 3D model format support (OBJ, FBX, STL, GLTF)
// Copyright (c) 2026 - DarkThumbs Project
//==============================================================================

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <memory>

namespace DarkThumbs {
namespace Engine {

    /// <summary>
    /// Decoder for 3D model formats
    /// Supported formats: OBJ, FBX, STL (ASCII/Binary), GLTF/GLB
    /// Renders simple wireframe/solid preview using Direct3D 11
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
        
        // Rendering
        HBITMAP RenderMeshPreview(const MeshData& mesh, uint32_t width, uint32_t height);
        void CalculateBounds(MeshData& mesh);
        DirectX::XMMATRIX CalculateViewMatrix(const MeshData& mesh);

        // Utility
        std::unique_ptr<uint8_t[]> ReadFileData(const wchar_t* filePath, size_t& outSize);

        // D3D11 resources (lazy-initialized)
        ID3D11Device* m_device;
        ID3D11DeviceContext* m_context;
        bool m_d3dInitialized;
        bool InitializeD3D();
        void CleanupD3D();

        // Supported extensions
        static wchar_t* s_extensions[5];
    };

} // namespace Engine
} // namespace DarkThumbs
