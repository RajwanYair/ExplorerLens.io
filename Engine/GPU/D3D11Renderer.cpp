// D3D11Renderer.cpp - DirectX 11 GPU Renderer Implementation

#include <windows.h>
#include "D3D11Renderer.h"
#include <d3dcompiler.h>
#include <chrono>
#include <algorithm>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;

namespace DarkThumbs {
namespace Engine {

D3D11Renderer::D3D11Renderer()
    : m_initialized(false)
    , m_gpuAvailable(false)
    , m_deviceLost(false)
    , m_videoMemoryMB(0)
    , m_vendorId(0)
    , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
    , m_isHardware(false)
    , m_totalOperations(0)
    , m_gpuOperations(0)
    , m_totalGpuTimeMs(0.0) {
}

D3D11Renderer::~D3D11Renderer() {
    Shutdown();
}

//==============================================================================
// Initialization
//==============================================================================

HRESULT D3D11Renderer::Initialize() {
    if (m_initialized) {
        return S_OK; // Already initialized
    }

    HRESULT hr = CreateDevice(true); // Allow WARP fallback
    if (SUCCEEDED(hr)) {
        hr = CreateResources();
        if (SUCCEEDED(hr)) {
            hr = CompileShaders();
            if (SUCCEEDED(hr)) {
                m_gpuAvailable = true;
            }
        }
    }

    m_initialized = true;
    return hr;
}

void D3D11Renderer::Shutdown() {
    if (!m_initialized) {
        return;
    }

    // Release D3D resources
    m_samplerLinear.Reset();
    m_constantBuffer.Reset();
    m_resizeCS.Reset();
    m_context.Reset();
    m_device.Reset();
    m_adapter.Reset();

    m_initialized = false;
    m_gpuAvailable = false;
}

HRESULT D3D11Renderer::CreateDevice(bool allowWARP) {
    HRESULT hr = S_OK;

    // Feature levels to try (D3D11.0 minimum)
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Try hardware device first
    D3D_FEATURE_LEVEL featureLevel;
    hr = D3D11CreateDevice(
        nullptr,                    // Default adapter
        D3D_DRIVER_TYPE_HARDWARE,   // Hardware device
        nullptr,                    // No software module
        createDeviceFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &m_device,
        &featureLevel,
        &m_context
    );

    if (FAILED(hr) && allowWARP) {
        // Fall back to WARP (software) device
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            createDeviceFlags,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &m_device,
            &featureLevel,
            &m_context
        );

        if (SUCCEEDED(hr)) {
            m_isHardware = false;
            m_deviceName = L"Microsoft Basic Render Driver (WARP)";
        }
    } else if (SUCCEEDED(hr)) {
        m_isHardware = true;

        // Get adapter info
        ComPtr<IDXGIDevice> dxgiDevice;
        hr = m_device.As(&dxgiDevice);
        if (SUCCEEDED(hr)) {
            ComPtr<IDXGIAdapter> adapter;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr)) {
                hr = adapter.As(&m_adapter);
                if (SUCCEEDED(hr)) {
                    DXGI_ADAPTER_DESC1 desc;
                    hr = m_adapter->GetDesc1(&desc);
                    if (SUCCEEDED(hr)) {
                        m_deviceName = desc.Description;
                        m_videoMemoryMB = static_cast<uint32_t>(desc.DedicatedVideoMemory / (1024 * 1024));
                        m_vendorId = desc.VendorId;
                    }
                }
            }
        }
    }

    if (SUCCEEDED(hr)) {
        m_featureLevel = featureLevel;
    }

    return hr;
}

HRESULT D3D11Renderer::CreateResources() {
    HRESULT hr = S_OK;

    // Create constant buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(ResizeConstants);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = m_device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer);
    if (FAILED(hr)) {
        return hr;
    }

    // Create linear sampler
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerLinear);
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

HRESULT D3D11Renderer::CompileShaders() {
    // Lanczos3 compute shader (inline HLSL)
    const char* shaderSource = R"(
        cbuffer ResizeConstants : register(b0) {
            uint sourceWidth;
            uint sourceHeight;
            uint targetWidth;
            uint targetHeight;
            float texelSizeX;
            float texelSizeY;
            float scaleX;
            float scaleY;
        };
        
        Texture2D<float4> sourceTexture : register(t0);
        RWTexture2D<float4> targetTexture : register(u0);
        
        #define PI 3.14159265359
        #define EPSILON 0.0001
        
        float sinc(float x) {
            x *= PI;
            return (abs(x) < EPSILON) ? 1.0 : sin(x) / x;
        }
        
        float lanczos3(float x) {
            if (abs(x) < EPSILON) return 1.0;
            if (abs(x) >= 3.0) return 0.0;
            return sinc(x) * sinc(x / 3.0);
        }
        
        float3 srgbToLinear(float3 srgb) {
            float3 linear;
            [unroll]
            for (int i = 0; i < 3; i++) {
                linear[i] = (srgb[i] <= 0.04045) ? srgb[i] / 12.92 : pow((srgb[i] + 0.055) / 1.055, 2.4);
            }
            return linear;
        }
        
        float3 linearToSrgb(float3 linear) {
            float3 srgb;
            [unroll]
            for (int i = 0; i < 3; i++) {
                srgb[i] = (linear[i] <= 0.0031308) ? 12.92 * linear[i] : 1.055 * pow(linear[i], 1.0 / 2.4) - 0.055;
            }
            return srgb;
        }
        
        [numthreads(8, 8, 1)]
        void CSResizeLanczos3(uint3 DTid : SV_DispatchThreadID) {
            if (DTid.x >= targetWidth || DTid.y >= targetHeight) return;
            
            float srcX = (DTid.x + 0.5) / scaleX;
            float srcY = (DTid.y + 0.5) / scaleY;
            
            int kernelRadius = 3;
            float4 sum = float4(0.0, 0.0, 0.0, 0.0);
            float weightSum = 0.0;
            
            [unroll]
            for (int dy = -kernelRadius; dy < kernelRadius; dy++) {
                [unroll]
                for (int dx = -kernelRadius; dx < kernelRadius; dx++) {
                    int sx = int(srcX) + dx;
                    int sy = int(srcY) + dy;
                    sx = clamp(sx, 0, int(sourceWidth) - 1);
                    sy = clamp(sy, 0, int(sourceHeight) - 1);
                    
                    float distX = srcX - (sx + 0.5);
                    float distY = srcY - (sy + 0.5);
                    float weight = lanczos3(distX) * lanczos3(distY);
                    
                    float4 pixel = sourceTexture.Load(int3(sx, sy, 0));
                    pixel.rgb = srgbToLinear(pixel.rgb);
                    sum += pixel * weight;
                    weightSum += weight;
                }
            }
            
            if (weightSum > EPSILON) sum /= weightSum;
            sum.rgb = linearToSrgb(sum.rgb);
            sum = saturate(sum);
            targetTexture[DTid.xy] = sum;
        }
    )";

    // Compile shader
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

    HRESULT hr = D3DCompile(
        shaderSource,
        strlen(shaderSource),
        "ThumbnailResize",
        nullptr,
        nullptr,
        "CSResizeLanczos3",
        "cs_5_0",
        compileFlags,
        0,
        &shaderBlob,
        &errorBlob
    );

    if (FAILED(hr)) {
        return hr;
    }

    // Create compute shader
    hr = m_device->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr,
        &m_resizeCS
    );

    return hr;
}

//==============================================================================
// Rendering Pipeline
//==============================================================================

HRESULT D3D11Renderer::RenderThumbnail(const uint8_t* imageData, uint32_t imageWidth, uint32_t imageHeight,
                                       uint32_t thumbWidth, uint32_t thumbHeight, HBITMAP* outBitmap) {
    if (!m_gpuAvailable || !imageData || !outBitmap) {
        return E_INVALIDARG;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    // Create texture from raw data
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = imageWidth;
    desc.Height = imageHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = imageData;
    initData.SysMemPitch = imageWidth * 4;

    ComPtr<ID3D11Texture2D> sourceTexture;
    HRESULT hr = m_device->CreateTexture2D(&desc, &initData, &sourceTexture);
    if (FAILED(hr)) {
        return hr;
    }

    // Create SRV
    ComPtr<ID3D11ShaderResourceView> sourceSRV;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr = m_device->CreateShaderResourceView(sourceTexture.Get(), &srvDesc, &sourceSRV);
    if (FAILED(hr)) {
        return hr;
    }

    // Scale texture
    ComPtr<ID3D11Texture2D> targetTexture;
    hr = ScaleTexture(sourceSRV.Get(), imageWidth, imageHeight, thumbWidth, thumbHeight, targetTexture);
    if (FAILED(hr)) {
        return hr;
    }

    // Convert to HBITMAP
    hr = TextureToHBITMAP(targetTexture.Get(), thumbWidth, thumbHeight, outBitmap);
    if (FAILED(hr)) {
        return hr;
    }

    // Update statistics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    
    std::lock_guard<std::mutex> lock(m_statsMutex);
    m_totalOperations++;
    m_gpuOperations++;
    m_totalGpuTimeMs += duration / 1000.0;
    
    return S_OK;
}

HRESULT D3D11Renderer::ScaleBitmap(HBITMAP hSource, uint32_t targetWidth, uint32_t targetHeight,
                                   HBITMAP* phResult) {
    if (!m_gpuAvailable || !hSource || !phResult) {
        return E_INVALIDARG;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    // Create texture from HBITMAP
    ComPtr<ID3D11Texture2D> sourceTexture;
    ComPtr<ID3D11ShaderResourceView> sourceSRV;
    uint32_t sourceWidth, sourceHeight;
    
    HRESULT hr = CreateTextureFromHBITMAP(hSource, sourceTexture, sourceSRV, sourceWidth, sourceHeight);
    if (FAILED(hr)) {
        return hr;
    }

    // Scale texture using compute shader
    ComPtr<ID3D11Texture2D> targetTexture;
    hr = ScaleTexture(sourceSRV.Get(), sourceWidth, sourceHeight, targetWidth, targetHeight, targetTexture);
    if (FAILED(hr)) {
        return hr;
    }

    // Convert texture back to HBITMAP
    hr = TextureToHBITMAP(targetTexture.Get(), targetWidth, targetHeight, phResult);
    if (FAILED(hr)) {
        m_totalOperations++;
        return hr;
    }

    // Update statistics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    
    std::lock_guard<std::mutex> lock(m_statsMutex);
    m_totalOperations++;
    m_gpuOperations++;
    m_totalGpuTimeMs += duration / 1000.0;
    
    return S_OK;
}

HRESULT D3D11Renderer::CreateTextureFromHBITMAP(HBITMAP hBitmap,
                                                 ComPtr<ID3D11Texture2D>& pTexture,
                                                 ComPtr<ID3D11ShaderResourceView>& pSRV,
                                                 uint32_t& width, uint32_t& height) {
    BITMAP bm;
    if (!GetObject(hBitmap, sizeof(BITMAP), &bm)) {
        return E_FAIL;
    }

    width = bm.bmWidth;
    height = bm.bmHeight;

    // Create staging texture
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    // Get bitmap bits
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -(LONG)height; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    // Calculate pitch and allocate pixel buffer
    const uint32_t pitch = width * 4;
    const size_t bufferSize = pitch * height;
    BYTE* pixelData = new BYTE[bufferSize];
    
    // Get bitmap bits
    HDC hdc = GetDC(nullptr);
    GetDIBits(hdc, hBitmap, 0, height, NULL, &bmi, DIB_RGB_COLORS);
    const int result = GetDIBits(hdc, hBitmap, 0, height, pixelData, &bmi, DIB_RGB_COLORS);
    ReleaseDC(nullptr, hdc);

    if (!result) {
        delete[] pixelData;
        return E_FAIL;
    }

    // Create texture from pixel data
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixelData;
    initData.SysMemPitch = pitch;

    HRESULT hr = m_device->CreateTexture2D(&desc, &initData, &pTexture);
    delete[] pixelData; // Clean up temporary buffer
    
    if (FAILED(hr)) {
        return hr;
    }

    // Create SRV
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr = m_device->CreateShaderResourceView(pTexture.Get(), &srvDesc, &pSRV);
    return hr;
}

HRESULT D3D11Renderer::ScaleTexture(ID3D11ShaderResourceView* pSourceSRV,
                                    uint32_t sourceWidth, uint32_t sourceHeight,
                                    uint32_t targetWidth, uint32_t targetHeight,
                                    ComPtr<ID3D11Texture2D>& pOutput) {
    // Create output texture
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = targetWidth;
    desc.Height = targetHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;

    HRESULT hr = m_device->CreateTexture2D(&desc, nullptr, &pOutput);
    if (FAILED(hr)) {
        return hr;
    }

    // Create UAV for output
    ComPtr<ID3D11UnorderedAccessView> outputUAV;
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = desc.Format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;

    hr = m_device->CreateUnorderedAccessView(pOutput.Get(), &uavDesc, &outputUAV);
    if (FAILED(hr)) {
        return hr;
    }

    // Update constant buffer
    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = m_context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr)) {
        return hr;
    }

    ResizeConstants* constants = static_cast<ResizeConstants*>(mapped.pData);
    constants->sourceWidth = sourceWidth;
    constants->sourceHeight = sourceHeight;
    constants->targetWidth = targetWidth;
    constants->targetHeight = targetHeight;
    constants->texelSizeX = 1.0f / sourceWidth;
    constants->texelSizeY = 1.0f / sourceHeight;
    constants->scaleX = static_cast<float>(targetWidth) / sourceWidth;
    constants->scaleY = static_cast<float>(targetHeight) / sourceHeight;

    m_context->Unmap(m_constantBuffer.Get(), 0);

    // Set compute shader and resources
    m_context->CSSetShader(m_resizeCS.Get(), nullptr, 0);
    m_context->CSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    m_context->CSSetShaderResources(0, 1, &pSourceSRV);
    m_context->CSSetUnorderedAccessViews(0, 1, outputUAV.GetAddressOf(), nullptr);

    // Dispatch compute shader (8x8 thread groups)
    UINT groupsX = (targetWidth + 7) / 8;
    UINT groupsY = (targetHeight + 7) / 8;
    m_context->Dispatch(groupsX, groupsY, 1);

    // Unbind resources
    ID3D11ShaderResourceView* nullSRV = nullptr;
    ID3D11UnorderedAccessView* nullUAV = nullptr;
    m_context->CSSetShaderResources(0, 1, &nullSRV);
    m_context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);

    return S_OK;
}

HRESULT D3D11Renderer::TextureToHBITMAP(ID3D11Texture2D* pTexture,
                                        uint32_t width, uint32_t height,
                                        HBITMAP* phBitmap) {
    // Create staging texture for readback
    D3D11_TEXTURE2D_DESC desc;
    pTexture->GetDesc(&desc);
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    ComPtr<ID3D11Texture2D> stagingTexture;
    HRESULT hr = m_device->CreateTexture2D(&desc, nullptr, &stagingTexture);
    if (FAILED(hr)) {
        return hr;
    }

    // Copy to staging
    m_context->CopyResource(stagingTexture.Get(), pTexture);

    // Map staging texture
    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = m_context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) {
        return hr;
    }

    // Create HBITMAP
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -(LONG)height; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HDC hdc = GetDC(nullptr);
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    ReleaseDC(nullptr, hdc);

    if (!hBitmap) {
        m_context->Unmap(stagingTexture.Get(), 0);
        return E_FAIL;
    }

    // Copy pixels
    uint32_t pitch = width * 4;
    for (uint32_t y = 0; y < height; y++) {
        memcpy(
            static_cast<BYTE*>(pBits) + y * pitch,
            static_cast<BYTE*>(mapped.pData) + y * mapped.RowPitch,
            pitch
        );
    }

    m_context->Unmap(stagingTexture.Get(), 0);

    *phBitmap = hBitmap;
    return S_OK;
}

//==============================================================================
// Info and Stats
//==============================================================================

HRESULT D3D11Renderer::GetGPUInfo(wchar_t* outName, uint32_t nameSize, uint32_t* outMemoryMB) const {
    if (outName && nameSize > 0) {
        wcsncpy_s(outName, nameSize, m_deviceName.c_str(), _TRUNCATE);
    }
    
    if (outMemoryMB) {
        *outMemoryMB = m_videoMemoryMB;
    }
    
    return S_OK;
}

bool D3D11Renderer::CheckDeviceStatus() {
    if (!m_device) {
        return false;
    }

    HRESULT hr = m_device->GetDeviceRemovedReason();
    if (hr != S_OK) {
        m_deviceLost = true;
        return false;
    }

    return true;
}

HRESULT D3D11Renderer::RecoverDevice() {
    if (!m_deviceLost) {
        return S_OK;
    }

    Shutdown();
    HRESULT hr = Initialize();
    if (SUCCEEDED(hr)) {
        m_deviceLost = false;
    }

    return hr;
}

//==============================================================================
// Batch Processing (Sprint 20)
//==============================================================================

HRESULT D3D11Renderer::RenderThumbnailBatch(BatchRenderRequest* requests, uint32_t requestCount) {
    if (!requests || requestCount == 0) {
        return E_INVALIDARG;
    }
    
    if (!m_initialized || !m_gpuAvailable) {
        // Fallback: process sequentially without GPU
        for (uint32_t i = 0; i < requestCount; ++i) {
            requests[i].result = E_FAIL; // Would call CPU fallback in real implementation
        }
        return S_FALSE;
    }
    
    HRESULT overallResult = S_OK;
    uint32_t successCount = 0;
    
    // Process batch with GPU (can be parallelized further with command lists)
    for (uint32_t i = 0; i < requestCount; ++i) {
        auto& req = requests[i];
        
        HRESULT hr = RenderThumbnail(
            req.imageData, req.imageWidth, req.imageHeight,
            req.thumbWidth, req.thumbHeight, req.outBitmap
        );
        
        req.result = hr;
        
        if (SUCCEEDED(hr)) {
            successCount++;
        } else {
            overallResult = S_FALSE; // Partial success
        }
    }
    
    // Update statistics
    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_totalOperations += requestCount;
        m_gpuOperations += successCount;
    }
    
    return (successCount == requestCount) ? S_OK : overallResult;
}

} // namespace Engine
} // namespace DarkThumbs

//==============================================================================
// Factory Function
//==============================================================================

DarkThumbs::Engine::IGPURenderer* CreateD3D11Renderer() {
    return new DarkThumbs::Engine::D3D11Renderer();
}
