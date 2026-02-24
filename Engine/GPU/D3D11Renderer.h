// D3D11Renderer.h - DirectX 11 GPU Renderer for ExplorerLens Engine
// Engine v1.0.0 - GPU-Accelerated Thumbnail Scaling
// Copyright (c) 2026 ExplorerLens Project
//
// Features:
// - Hardware-accelerated thumbnail scaling with Lanczos3 compute shader
// - Automatic fallback to CPU when GPU unavailable
// - Device loss recovery
// - Performance statistics tracking
// - Thread-safe operation

#pragma once

#include "../Core/IGPURenderer.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <mutex>
#include <string>

namespace ExplorerLens {
namespace Engine {

class D3D11Renderer : public IGPURenderer {
public:
    D3D11Renderer();
    ~D3D11Renderer() override;

    // IGPURenderer interface
    HRESULT Initialize() override;
    void Shutdown() override;
    bool IsAvailable() const override { return m_gpuAvailable; }
    
    HRESULT RenderThumbnail(const uint8_t* imageData, uint32_t imageWidth, uint32_t imageHeight,
                           uint32_t thumbWidth, uint32_t thumbHeight, HBITMAP* outBitmap) override;
    
    HRESULT GetGPUInfo(wchar_t* outName, uint32_t nameSize, uint32_t* outMemoryMB) const override;
    const wchar_t* GetRendererType() const override { return L"DirectX 11"; }
    
    // Additional methods for advanced usage
    HRESULT ScaleBitmap(HBITMAP hSource, uint32_t targetWidth, uint32_t targetHeight,
                       HBITMAP* phResult);
    
    // Batch processing support
    struct BatchRenderRequest {
        const uint8_t* imageData;
        uint32_t imageWidth;
        uint32_t imageHeight;
        uint32_t thumbWidth;
        uint32_t thumbHeight;
        HBITMAP* outBitmap;
        HRESULT result;
    };
    
    HRESULT RenderThumbnailBatch(BatchRenderRequest* requests, uint32_t requestCount);
    
    // Device status
    bool CheckDeviceStatus();
    HRESULT RecoverDevice();

private:
    // Initialization helpers
    HRESULT CreateDevice(bool allowWARP);
    HRESULT CreateResources();
    HRESULT CompileShaders();
    
    // Rendering pipeline
    HRESULT CreateTextureFromHBITMAP(HBITMAP hBitmap, 
                                     Microsoft::WRL::ComPtr<ID3D11Texture2D>& pTexture,
                                     Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& pSRV,
                                     uint32_t& width, uint32_t& height);
    
    HRESULT ScaleTexture(ID3D11ShaderResourceView* pSourceSRV,
                        uint32_t sourceWidth, uint32_t sourceHeight,
                        uint32_t targetWidth, uint32_t targetHeight,
                        Microsoft::WRL::ComPtr<ID3D11Texture2D>& pOutput);
    
    HRESULT TextureToHBITMAP(ID3D11Texture2D* pTexture,
                            uint32_t width, uint32_t height,
                            HBITMAP* phBitmap);

    // Device and context
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter;
    
    // Compute shader resources
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_resizeCS;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerLinear;
    
    // Device information
    std::wstring m_deviceName;
    uint32_t m_videoMemoryMB;
    uint32_t m_vendorId;
    D3D_FEATURE_LEVEL m_featureLevel;
    bool m_isHardware;
    
    // State flags
    bool m_initialized;
    bool m_gpuAvailable;
    bool m_deviceLost;
    
    // Statistics tracking (for internal use)
    mutable std::mutex m_statsMutex;
    uint64_t m_totalOperations;
    uint64_t m_gpuOperations;
    double m_totalGpuTimeMs;
    
    // Constant buffer structure for compute shader
    struct ResizeConstants {
        uint32_t sourceWidth;
        uint32_t sourceHeight;
        uint32_t targetWidth;
        uint32_t targetHeight;
        float texelSizeX;
        float texelSizeY;
        float scaleX;
        float scaleY;
    };
};

} // namespace Engine
} // namespace ExplorerLens

// Factory function
extern "C" {
    __declspec(dllexport) ExplorerLens::Engine::IGPURenderer* CreateD3D11Renderer();
}

