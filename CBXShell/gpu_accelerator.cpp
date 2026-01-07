// gpu_accelerator.cpp - DirectX 11 GPU-Accelerated Thumbnail Generation
// DarkThumbs v5.2.0 - Implementation
// Copyright (c) 2025 DarkThumbs Project

#include "gpu_accelerator.h"
#include "StdAfx.h"
#include <algorithm>
#include <chrono>
#include <sstream>
#include <string>

namespace DarkThumbs {

// Constant buffer structure for compute shader
struct ResizeConstants {
  UINT sourceWidth;
  UINT sourceHeight;
  UINT targetWidth;
  UINT targetHeight;
  float texelSizeX;
  float texelSizeY;
  float scaleX;
  float scaleY;
};

//=============================================================================
// GPUAccelerator - Singleton Implementation
//=============================================================================

GPUAccelerator &GPUAccelerator::Instance() {
  static GPUAccelerator instance;
  return instance;
}

GPUAccelerator::GPUAccelerator()
    : m_initialized(false), m_gpuAvailable(false), m_deviceLost(false),
      m_queueRunning(false), m_poolFrameCounter(0) {
  m_stats.Reset();
  ZeroMemory(&m_deviceInfo, sizeof(m_deviceInfo));
}

GPUAccelerator::~GPUAccelerator() { Shutdown(); }

//=============================================================================
// Initialization
//=============================================================================

HRESULT GPUAccelerator::Initialize(bool allowWARP) {
  PROFILE_FUNCTION();

  if (m_initialized) {
    DT_LOG_INFO(LogCategory::GPU, "GPU Accelerator already initialized");
    return S_OK; // Already initialized
  }

  DT_LOG_INFO(LogCategory::GPU, "Initializing GPU Accelerator...");
  HRESULT hr = S_OK;

  // Create WIC factory first (needed for CPU fallback too)
  hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS(&m_wicFactory));

  if (FAILED(hr)) {
    DT_LOG_HRESULT(LogLevel::LVL_ERROR, LogCategory::GPU, "Create WIC factory",
                   hr);
    OutputDebugString(L"[GPU] Failed to create WIC factory\n");
    return hr;
  }

  DT_LOG_DEBUG(DarkThumbs::LogCategory::GPU,
               "WIC factory created successfully");

  // Try to create D3D11 device
  hr = CreateDevice(allowWARP);
  if (SUCCEEDED(hr)) {
    // Device created successfully
    hr = CreateResources();
    if (SUCCEEDED(hr)) {
      hr = CompileShaders();
      if (SUCCEEDED(hr)) {
        hr = DetectCapabilities();
        if (SUCCEEDED(hr)) {
          m_gpuAvailable = true;

          // Start async queue worker thread
          m_queueRunning = true;
          m_workerThread = std::thread([this]() { ProcessQueue(); });

          DT_LOG_INFO(LogCategory::GPU,
                      "GPU acceleration initialized successfully");
          std::ostringstream oss;
          oss << "Device: " << std::string(CW2A(m_deviceInfo.name.c_str()))
              << ", VRAM: "
              << (m_deviceInfo.dedicatedVideoMemory / (1024.0 * 1024.0))
              << " MB"
              << ", Feature Level: 0x" << std::hex << m_deviceInfo.featureLevel;
          DT_LOG_INFO(LogCategory::GPU, oss.str());

          OutputDebugStringW(
              L"[GPU] GPU acceleration initialized successfully\n");
          OutputDebugStringW(
              (L"[GPU] Device: " + m_deviceInfo.name + L"\n").c_str());

          wchar_t msg[256];
          swprintf_s(msg, L"[GPU] VRAM: %.1f MB, Feature Level: 0x%04X\n",
                     m_deviceInfo.dedicatedVideoMemory / (1024.0 * 1024.0),
                     m_deviceInfo.featureLevel);
          OutputDebugStringW(msg);
        } else {
          DT_LOG_HRESULT(LogLevel::LVL_ERROR, LogCategory::GPU,
                         "Detect capabilities", hr);
        }
      } else {
        DT_LOG_HRESULT(LogLevel::LVL_ERROR, LogCategory::GPU, "Compile shaders",
                       hr);
      }
    } else {
      DT_LOG_HRESULT(LogLevel::LVL_ERROR, LogCategory::GPU, "Create resources",
                     hr);
    }
  } else {
    DT_LOG_HRESULT(LogLevel::LVL_WARNING, LogCategory::GPU,
                   "Create D3D11 device", hr);
  }

  if (!m_gpuAvailable) {
    OutputDebugString(
        L"[GPU] GPU acceleration not available, using CPU fallback\n");
    // Continue anyway - we'll use CPU fallback
    hr = S_OK;
  }

  m_initialized = true;
  return hr;
}

void GPUAccelerator::Shutdown() {
  if (!m_initialized) {
    return;
  }

  // Stop async queue
  if (m_queueRunning) {
    m_queueRunning = false;
    m_queueCV.notify_all();
    if (m_workerThread.joinable()) {
      m_workerThread.join();
    }
  }

  // Clear queue
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    while (!m_queue.empty()) {
      m_queue.pop();
    }
  }

  // Clear texture pool
  {
    std::lock_guard<std::mutex> lock(m_poolMutex);
    ClearTexturePool();
  }

  // Release D3D resources
  m_samplerPoint.Reset();
  m_samplerLinear.Reset();
  m_constantBuffer.Reset();
  m_resizeCS.Reset();
  m_context.Reset();
  m_device.Reset();
  m_adapter.Reset();
  m_wicFactory.Reset();

  m_initialized = false;
  m_gpuAvailable = false;

  OutputDebugString(L"[GPU] GPU accelerator shutdown complete\n");
}

//=============================================================================
// Device Creation
//=============================================================================

HRESULT GPUAccelerator::CreateDevice(bool allowWARP) {
  HRESULT hr = S_OK;

  // Feature levels to try (D3D11.0 is minimum)
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
  hr = D3D11CreateDevice(nullptr, // Default adapter
                         D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
                         featureLevels, ARRAYSIZE(featureLevels),
                         D3D11_SDK_VERSION, &m_device, &featureLevel,
                         &m_context);

  if (FAILED(hr) && allowWARP) {
    // Fall back to WARP (software) device
    OutputDebugString(L"[GPU] Hardware device failed, trying WARP...\n");
    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr,
                           createDeviceFlags, featureLevels,
                           ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
                           &m_device, &featureLevel, &m_context);

    if (SUCCEEDED(hr)) {
      m_deviceInfo.isHardwareDevice = false;
      m_deviceInfo.name = L"Microsoft Basic Render Driver (WARP)";
    }
  } else if (SUCCEEDED(hr)) {
    m_deviceInfo.isHardwareDevice = true;

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
            m_deviceInfo.name = desc.Description;
            m_deviceInfo.dedicatedVideoMemory = desc.DedicatedVideoMemory;
            m_deviceInfo.dedicatedSystemMemory = desc.DedicatedSystemMemory;
            m_deviceInfo.sharedSystemMemory = desc.SharedSystemMemory;
            m_deviceInfo.vendorId = desc.VendorId;

            // Log vendor information
            const wchar_t *vendorName = L"Unknown";
            if (desc.VendorId == 0x8086)
              vendorName = L"Intel";
            else if (desc.VendorId == 0x10DE)
              vendorName = L"NVIDIA";
            else if (desc.VendorId == 0x1002)
              vendorName = L"AMD";
            else if (desc.VendorId == 0x1414)
              vendorName = L"Microsoft (WARP)";

            wchar_t msg[256];
            swprintf_s(msg, L"[GPU] Vendor: %s (0x%04X)\n", vendorName,
                       desc.VendorId);
            OutputDebugStringW(msg);

            // Intel-specific detection and optimization flags
            if (desc.VendorId == 0x8086) {
              std::wstring gpuName = desc.Description;

              // Detect high-performance Intel GPUs
              if (gpuName.find(L"Iris") != std::wstring::npos ||
                  gpuName.find(L"Arc") != std::wstring::npos ||
                  gpuName.find(L"Xe") != std::wstring::npos) {
                OutputDebugStringW(L"[GPU] Intel high-performance GPU detected "
                                   L"(Iris/Xe/Arc)\n");
              } else if (gpuName.find(L"HD Graphics") != std::wstring::npos) {
                OutputDebugStringW(L"[GPU] Intel HD Graphics detected\n");
              } else if (gpuName.find(L"UHD Graphics") != std::wstring::npos) {
                OutputDebugStringW(L"[GPU] Intel UHD Graphics detected\n");
              }

              // Intel integrated GPUs use shared memory effectively
              if (desc.SharedSystemMemory > (2ULL * 1024 * 1024 * 1024)) {
                wchar_t memMsg[256];
                swprintf_s(memMsg,
                           L"[GPU] Intel GPU with %.1f GB shared memory - "
                           L"optimizing for integrated graphics\n",
                           desc.SharedSystemMemory /
                               (1024.0 * 1024.0 * 1024.0));
                OutputDebugStringW(memMsg);
              }
            }
          }
        }
      }
    }
  }

  if (SUCCEEDED(hr)) {
    m_deviceInfo.featureLevel = featureLevel;

    // Log feature level
    const wchar_t *featureLevelName = L"Unknown";
    if (featureLevel == D3D_FEATURE_LEVEL_11_1)
      featureLevelName = L"11.1";
    else if (featureLevel == D3D_FEATURE_LEVEL_11_0)
      featureLevelName = L"11.0";
    else if (featureLevel == D3D_FEATURE_LEVEL_10_1)
      featureLevelName = L"10.1";
    else if (featureLevel == D3D_FEATURE_LEVEL_10_0)
      featureLevelName = L"10.0";

    wchar_t msg[128];
    swprintf_s(msg, L"[GPU] Feature Level: DirectX %s\n", featureLevelName);
    OutputDebugStringW(msg);
  }

  return hr;
}

HRESULT GPUAccelerator::CreateResources() {
  HRESULT hr = S_OK;

  // Create constant buffer for resize shader
  D3D11_BUFFER_DESC cbDesc = {};
  cbDesc.ByteWidth = (sizeof(ResizeConstants) + 15) & ~15; // Align to 16 bytes
  cbDesc.Usage = D3D11_USAGE_DYNAMIC;
  cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  hr = m_device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer);
  if (FAILED(hr)) {
    OutputDebugString(L"[GPU] Failed to create constant buffer\n");
    return hr;
  }

  // Create linear sampler for high-quality resizing
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
    OutputDebugString(L"[GPU] Failed to create linear sampler\n");
    return hr;
  }

  // Create point sampler for pixel-perfect operations
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
  hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerPoint);
  if (FAILED(hr)) {
    OutputDebugString(L"[GPU] Failed to create point sampler\n");
    return hr;
  }

  return S_OK;
}

HRESULT GPUAccelerator::CompileShaders() {
  HRESULT hr = S_OK;

  // HLSL shader source embedded (compile at runtime)
  const char *shaderSource = R"(
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
        SamplerState samplerLinear : register(s0);
        
        static const float PI = 3.14159265359;
        static const float LANCZOS_SIZE = 3.0;
        static const float EPSILON = 0.0001;
        
        float sinc(float x) {
            x = abs(x);
            if (x < EPSILON) return 1.0;
            float pix = PI * x;
            return sin(pix) / pix;
        }
        
        float lanczos3(float x) {
            x = abs(x);
            if (x >= LANCZOS_SIZE) return 0.0;
            return sinc(x) * sinc(x / LANCZOS_SIZE);
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

  // Compile shader at runtime
  ComPtr<ID3DBlob> shaderBlob;
  ComPtr<ID3DBlob> errorBlob;

  UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
  compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
  compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

  hr = D3DCompile(shaderSource, strlen(shaderSource), "ThumbnailResize",
                  nullptr, nullptr, "CSResizeLanczos3", "cs_5_0", compileFlags,
                  0, &shaderBlob, &errorBlob);

  if (FAILED(hr)) {
    if (errorBlob) {
      OutputDebugStringA("[GPU] Shader compilation error: ");
      OutputDebugStringA(static_cast<char *>(errorBlob->GetBufferPointer()));
      OutputDebugStringA("\n");
    }
    OutputDebugString(
        L"[GPU] Failed to compile compute shader, will use CPU fallback\n");
    return hr;
  }

  // Create compute shader
  hr = m_device->CreateComputeShader(shaderBlob->GetBufferPointer(),
                                     shaderBlob->GetBufferSize(), nullptr,
                                     &m_resizeCS);

  if (FAILED(hr)) {
    OutputDebugString(L"[GPU] Failed to create compute shader\n");
    return hr;
  }

  OutputDebugString(L"[GPU] Lanczos3 compute shader compiled successfully\n");
  return S_OK;
}

HRESULT GPUAccelerator::DetectCapabilities() {
  m_deviceInfo.capabilities = GPUCapability::BasicRendering;

  // Check for compute shader support (D3D11.0+)
  if (m_deviceInfo.featureLevel >= D3D_FEATURE_LEVEL_11_0) {
    m_deviceInfo.capabilities =
        m_deviceInfo.capabilities | GPUCapability::ComputeShaders;
  }

  // Check for D3D11.1 features
  if (m_deviceInfo.featureLevel >= D3D_FEATURE_LEVEL_11_1) {
    m_deviceInfo.capabilities =
        m_deviceInfo.capabilities | GPUCapability::FastSemantics;
  }

  // Check for dedicated GPU
  if (m_deviceInfo.isHardwareDevice &&
      m_deviceInfo.dedicatedVideoMemory > 256 * 1024 * 1024) {
    m_deviceInfo.capabilities =
        m_deviceInfo.capabilities | GPUCapability::HighPerformance;
  }

  return S_OK;
}

bool GPUAccelerator::HasComputeShaders() const {
  return HasCapability(m_deviceInfo.capabilities,
                       GPUCapability::ComputeShaders);
}

bool GPUAccelerator::IsHighPerformance() const {
  return HasCapability(m_deviceInfo.capabilities,
                       GPUCapability::HighPerformance);
}

//=============================================================================
// Thumbnail Generation (Synchronous)
//=============================================================================

HRESULT GPUAccelerator::CreateThumbnail(IWICBitmapSource *pSource,
                                        UINT targetWidth, UINT targetHeight,
                                        IWICBitmap **ppThumbnail) {
  PROFILE_SCOPE("GPU_CreateThumbnail");

  if (!pSource || !ppThumbnail) {
    DT_LOG_ERROR(LogCategory::GPU, "CreateThumbnail: Invalid parameters");
    return E_POINTER;
  }

  *ppThumbnail = nullptr;

  // Use CPU fallback if GPU not available
  if (!m_gpuAvailable || m_deviceLost) {
    DT_LOG_DEBUG(DarkThumbs::LogCategory::GPU,
                 "Using CPU fallback (GPU unavailable or device lost)");
    return CreateThumbnailCPU(pSource, targetWidth, targetHeight, ppThumbnail);
  }

  auto startTime = std::chrono::high_resolution_clock::now();

  HRESULT hr = S_OK;
  ComPtr<ID3D11Texture2D> sourceTexture;
  ComPtr<ID3D11ShaderResourceView> sourceSRV;
  ComPtr<ID3D11Texture2D> outputTexture;

  // Create texture from WIC source
  hr = CreateTextureFromWIC(pSource, &sourceTexture, &sourceSRV);
  if (FAILED(hr)) {
    DT_LOG_HRESULT(LogLevel::LVL_WARNING, LogCategory::GPU,
                   "CreateTextureFromWIC", hr);
    OutputDebugString(
        L"[GPU] Failed to create texture from WIC, using CPU fallback\n");
    return CreateThumbnailCPU(pSource, targetWidth, targetHeight, ppThumbnail);
  }

  // Get source dimensions
  D3D11_TEXTURE2D_DESC srcDesc;
  sourceTexture->GetDesc(&srcDesc);

  std::string resizeInfo =
      std::string("Resizing texture: ") + std::to_string(srcDesc.Width) + "x" +
      std::to_string(srcDesc.Height) + " -> " + std::to_string(targetWidth) +
      "x" + std::to_string(targetHeight);
  DT_LOG_DEBUG(DarkThumbs::LogCategory::GPU, resizeInfo);

  // Resize texture using GPU
  hr = ResizeTexture(sourceSRV.Get(), srcDesc.Width, srcDesc.Height,
                     targetWidth, targetHeight, &outputTexture);
  if (FAILED(hr)) {
    DT_LOG_HRESULT(LogLevel::LVL_WARNING, LogCategory::GPU, "ResizeTexture",
                   hr);
    OutputDebugString(L"[GPU] Failed to resize texture, using CPU fallback\n");
    return CreateThumbnailCPU(pSource, targetWidth, targetHeight, ppThumbnail);
  }

  // Convert back to WIC bitmap
  hr = TextureToWICBitmap(outputTexture.Get(), ppThumbnail);

  // Release textures back to pool
  ReleaseTexture(sourceTexture.Get());
  ReleaseTexture(outputTexture.Get());

  if (FAILED(hr)) {
    DT_LOG_HRESULT(LogLevel::LVL_WARNING, LogCategory::GPU,
                   "TextureToWICBitmap", hr);
    OutputDebugString(
        L"[GPU] Failed to convert to WIC bitmap, using CPU fallback\n");
    return CreateThumbnailCPU(pSource, targetWidth, targetHeight, ppThumbnail);
  }

  // Update statistics and periodic pool cleanup
  auto endTime = std::chrono::high_resolution_clock::now();
  double elapsedMs =
      std::chrono::duration<double, std::milli>(endTime - startTime).count();

  std::string perfInfo = std::string("GPU thumbnail generated in ") +
                         std::to_string(elapsedMs) + " ms";
  DT_LOG_DEBUG(DarkThumbs::LogCategory::PERFORMANCE, perfInfo);

  {
    std::lock_guard<std::mutex> lock(m_statsMutex);
    m_stats.totalThumbnails++;
    m_stats.gpuThumbnails++;
    m_stats.avgGpuTimeMs =
        (m_stats.avgGpuTimeMs * (m_stats.gpuThumbnails - 1) + elapsedMs) /
        m_stats.gpuThumbnails;
  }

  // Periodic texture pool cleanup
  {
    std::lock_guard<std::mutex> lock(m_poolMutex);
    m_poolFrameCounter++;

    if (m_poolFrameCounter >= POOL_CLEANUP_INTERVAL) {
      m_poolFrameCounter = 0;

      auto now = std::chrono::steady_clock::now();
      size_t removedCount = 0;

      auto it = m_texturePool.begin();
      while (it != m_texturePool.end()) {
        if (!it->inUse) {
          auto age = std::chrono::duration_cast<std::chrono::milliseconds>(
                         now - it->lastUsed)
                         .count();

          if (age > POOL_TEXTURE_LIFETIME_MS) {
            it = m_texturePool.erase(it);
            removedCount++;
            continue;
          }
        }
        ++it;
      }

      if (removedCount > 0) {
        wchar_t msg[128];
        swprintf_s(msg, 128,
                   L"[GPU] Texture pool: Cleaned up %zu old textures (pool "
                   L"size: %zu)\n",
                   removedCount, m_texturePool.size());
        OutputDebugString(msg);
      }
    }
  }

  return S_OK;
}

HRESULT GPUAccelerator::CreateThumbnailFromFile(const std::wstring &filePath,
                                                UINT targetWidth,
                                                UINT targetHeight,
                                                IWICBitmap **ppThumbnail) {
  if (!m_wicFactory) {
    return E_NOT_VALID_STATE;
  }

  ComPtr<IWICBitmapDecoder> decoder;
  HRESULT hr = m_wicFactory->CreateDecoderFromFilename(
      filePath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand,
      &decoder);

  if (FAILED(hr)) {
    return hr;
  }

  ComPtr<IWICBitmapFrameDecode> frame;
  hr = decoder->GetFrame(0, &frame);
  if (FAILED(hr)) {
    return hr;
  }

  return CreateThumbnail(frame.Get(), targetWidth, targetHeight, ppThumbnail);
}

//=============================================================================
// CPU Fallback Implementation
//=============================================================================

HRESULT GPUAccelerator::CreateThumbnailCPU(IWICBitmapSource *pSource,
                                           UINT targetWidth, UINT targetHeight,
                                           IWICBitmap **ppThumbnail) {
  if (!m_wicFactory || !pSource || !ppThumbnail) {
    return E_POINTER;
  }

  auto startTime = std::chrono::high_resolution_clock::now();

  HRESULT hr = S_OK;
  ComPtr<IWICBitmapScaler> scaler;

  hr = m_wicFactory->CreateBitmapScaler(&scaler);
  if (FAILED(hr)) {
    return hr;
  }

  // Use high-quality Fant interpolation
  hr = scaler->Initialize(pSource, targetWidth, targetHeight,
                          WICBitmapInterpolationModeFant);
  if (FAILED(hr)) {
    return hr;
  }

  // Create output bitmap
  hr = m_wicFactory->CreateBitmapFromSource(
      scaler.Get(), WICBitmapCacheOnDemand, ppThumbnail);

  // Update statistics
  auto endTime = std::chrono::high_resolution_clock::now();
  double elapsedMs =
      std::chrono::duration<double, std::milli>(endTime - startTime).count();

  {
    std::lock_guard<std::mutex> lock(m_statsMutex);
    m_stats.totalThumbnails++;
    m_stats.cpuFallbacks++;
    m_stats.avgCpuTimeMs =
        (m_stats.avgCpuTimeMs * (m_stats.cpuFallbacks - 1) + elapsedMs) /
        m_stats.cpuFallbacks;
  }

  return hr;
}

//=============================================================================
// GPU Rendering Helpers - Full Implementation
//=============================================================================

HRESULT GPUAccelerator::CreateTextureFromWIC(IWICBitmapSource *pSource,
                                             ID3D11Texture2D **ppTexture,
                                             ID3D11ShaderResourceView **ppSRV) {
  if (!pSource || !ppTexture) {
    return E_POINTER;
  }

  HRESULT hr = S_OK;

  // Get source dimensions
  UINT width, height;
  hr = pSource->GetSize(&width, &height);
  if (FAILED(hr)) {
    return hr;
  }

  // Get source pixel format
  WICPixelFormatGUID pixelFormat;
  hr = pSource->GetPixelFormat(&pixelFormat);
  if (FAILED(hr)) {
    return hr;
  }

  // Convert to BGRA8 if needed (most common format, fully supported)
  ComPtr<IWICBitmapSource> convertedSource;
  if (!IsEqualGUID(pixelFormat, GUID_WICPixelFormat32bppBGRA)) {
    ComPtr<IWICFormatConverter> converter;
    hr = m_wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
      return hr;
    }

    hr = converter->Initialize(pSource, GUID_WICPixelFormat32bppBGRA,
                               WICBitmapDitherTypeNone, nullptr, 0.0,
                               WICBitmapPaletteTypeMedianCut);
    if (FAILED(hr)) {
      return hr;
    }

    convertedSource = converter;
  } else {
    convertedSource = pSource;
  }

  // Allocate pixel buffer
  UINT stride = width * 4; // 4 bytes per pixel (BGRA)
  UINT bufferSize = stride * height;
  std::vector<BYTE> pixels(bufferSize);

  // Copy pixels from WIC
  hr = convertedSource->CopyPixels(nullptr, stride, bufferSize, pixels.data());
  if (FAILED(hr)) {
    return hr;
  }

  // Acquire texture from pool (or create new one)
  ComPtr<ID3D11Texture2D> texture;
  hr = AcquireTexture(width, height, DXGI_FORMAT_B8G8R8A8_UNORM,
                      D3D11_BIND_SHADER_RESOURCE, &texture, ppSRV, nullptr);

  if (FAILED(hr)) {
    OutputDebugString(L"[GPU] Failed to acquire source texture from pool\n");
    return hr;
  }

  // Upload pixel data to texture
  m_context->UpdateSubresource(texture.Get(), 0, nullptr, pixels.data(), stride,
                               0);

  *ppTexture = texture.Detach();
  return S_OK;
}

HRESULT GPUAccelerator::ResizeTexture(ID3D11ShaderResourceView *pSourceSRV,
                                      UINT sourceWidth, UINT sourceHeight,
                                      UINT targetWidth, UINT targetHeight,
                                      ID3D11Texture2D **ppOutput) {
  if (!pSourceSRV || !ppOutput) {
    return E_POINTER;
  }

  HRESULT hr = S_OK;

  // Phase 2: Use compute shader if available
  if (m_resizeCS) {
    return ResizeTextureComputeShader(pSourceSRV, sourceWidth, sourceHeight,
                                      targetWidth, targetHeight, ppOutput);
  }

  // Fallback: Use WIC scaler (Phase 1 behavior)
  OutputDebugString(
      L"[GPU] Compute shader not available, using WIC fallback\n");

  // Create output texture
  D3D11_TEXTURE2D_DESC texDesc = {};
  texDesc.Width = targetWidth;
  texDesc.Height = targetHeight;
  texDesc.MipLevels = 1;
  texDesc.ArraySize = 1;
  texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  texDesc.SampleDesc.Count = 1;
  texDesc.SampleDesc.Quality = 0;
  texDesc.Usage = D3D11_USAGE_DEFAULT;
  texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  texDesc.CPUAccessFlags = 0;

  ComPtr<ID3D11Texture2D> outputTexture;
  hr = m_device->CreateTexture2D(&texDesc, nullptr, &outputTexture);
  if (FAILED(hr)) {
    return hr;
  }

  // Get source texture from SRV
  ComPtr<ID3D11Resource> sourceResource;
  pSourceSRV->GetResource(&sourceResource);

  ComPtr<ID3D11Texture2D> sourceTexture;
  hr = sourceResource.As(&sourceTexture);
  if (FAILED(hr)) {
    return hr;
  }

  // Create staging texture for readback
  D3D11_TEXTURE2D_DESC stagingDesc = texDesc;
  stagingDesc.Width = sourceWidth;
  stagingDesc.Height = sourceHeight;
  stagingDesc.Usage = D3D11_USAGE_STAGING;
  stagingDesc.BindFlags = 0;
  stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

  ComPtr<ID3D11Texture2D> stagingTexture;
  hr = m_device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
  if (FAILED(hr)) {
    return hr;
  }

  // Copy to staging
  m_context->CopyResource(stagingTexture.Get(), sourceTexture.Get());

  // Map and read pixels
  D3D11_MAPPED_SUBRESOURCE mapped;
  hr = m_context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped);
  if (FAILED(hr)) {
    return hr;
  }

  // Create WIC bitmap from mapped data
  ComPtr<IWICBitmap> sourceBitmap;
  hr = m_wicFactory->CreateBitmapFromMemory(
      sourceWidth, sourceHeight, GUID_WICPixelFormat32bppBGRA, mapped.RowPitch,
      mapped.RowPitch * sourceHeight, static_cast<BYTE *>(mapped.pData),
      &sourceBitmap);

  m_context->Unmap(stagingTexture.Get(), 0);

  if (FAILED(hr)) {
    return hr;
  }

  // Use WIC scaler for resizing
  ComPtr<IWICBitmapScaler> scaler;
  hr = m_wicFactory->CreateBitmapScaler(&scaler);
  if (FAILED(hr)) {
    return hr;
  }

  hr = scaler->Initialize(sourceBitmap.Get(), targetWidth, targetHeight,
                          WICBitmapInterpolationModeFant);
  if (FAILED(hr)) {
    return hr;
  }

  // Create output texture from scaled bitmap
  UINT scaledStride = targetWidth * 4;
  UINT scaledBufferSize = scaledStride * targetHeight;
  std::vector<BYTE> scaledPixels(scaledBufferSize);

  hr = scaler->CopyPixels(nullptr, scaledStride, scaledBufferSize,
                          scaledPixels.data());
  if (FAILED(hr)) {
    return hr;
  }

  // Update output texture
  m_context->UpdateSubresource(outputTexture.Get(), 0, nullptr,
                               scaledPixels.data(), scaledStride, 0);

  *ppOutput = outputTexture.Detach();
  return S_OK;
}

HRESULT GPUAccelerator::ResizeTextureComputeShader(
    ID3D11ShaderResourceView *pSourceSRV, UINT sourceWidth, UINT sourceHeight,
    UINT targetWidth, UINT targetHeight, ID3D11Texture2D **ppOutput) {
  HRESULT hr = S_OK;

  // Acquire output texture from pool with UAV binding
  ComPtr<ID3D11Texture2D> outputTexture;
  ComPtr<ID3D11UnorderedAccessView> outputUAV;

  hr = AcquireTexture(targetWidth, targetHeight, DXGI_FORMAT_B8G8R8A8_UNORM,
                      D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
                      &outputTexture, nullptr, &outputUAV);

  if (FAILED(hr)) {
    OutputDebugString(L"[GPU] Failed to acquire output texture from pool\n");
    return hr;
  }

  // Update constant buffer
  ResizeConstants constants;
  constants.sourceWidth = sourceWidth;
  constants.sourceHeight = sourceHeight;
  constants.targetWidth = targetWidth;
  constants.targetHeight = targetHeight;
  constants.texelSizeX = 1.0f / sourceWidth;
  constants.texelSizeY = 1.0f / sourceHeight;
  constants.scaleX = static_cast<float>(targetWidth) / sourceWidth;
  constants.scaleY = static_cast<float>(targetHeight) / sourceHeight;

  D3D11_MAPPED_SUBRESOURCE mapped;
  hr = m_context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
                      &mapped);
  if (SUCCEEDED(hr)) {
    memcpy(mapped.pData, &constants, sizeof(ResizeConstants));
    m_context->Unmap(m_constantBuffer.Get(), 0);
  }

  // Set compute shader and resources
  m_context->CSSetShader(m_resizeCS.Get(), nullptr, 0);
  m_context->CSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
  m_context->CSSetShaderResources(0, 1, &pSourceSRV);
  m_context->CSSetUnorderedAccessViews(0, 1, outputUAV.GetAddressOf(), nullptr);
  m_context->CSSetSamplers(0, 1, m_samplerLinear.GetAddressOf());

  // Dispatch compute shader (8x8 thread groups)
  UINT dispatchX = (targetWidth + 7) / 8;
  UINT dispatchY = (targetHeight + 7) / 8;
  m_context->Dispatch(dispatchX, dispatchY, 1);

  // Unbind resources
  ID3D11ShaderResourceView *nullSRV = nullptr;
  ID3D11UnorderedAccessView *nullUAV = nullptr;
  m_context->CSSetShaderResources(0, 1, &nullSRV);
  m_context->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);

  *ppOutput = outputTexture.Detach();
  return S_OK;
}

HRESULT GPUAccelerator::TextureToWICBitmap(ID3D11Texture2D *pTexture,
                                           IWICBitmap **ppBitmap) {
  if (!pTexture || !ppBitmap) {
    return E_POINTER;
  }

  HRESULT hr = S_OK;

  // Get texture description
  D3D11_TEXTURE2D_DESC desc;
  pTexture->GetDesc(&desc);

  // Create staging texture for readback
  D3D11_TEXTURE2D_DESC stagingDesc = desc;
  stagingDesc.Usage = D3D11_USAGE_STAGING;
  stagingDesc.BindFlags = 0;
  stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

  ComPtr<ID3D11Texture2D> stagingTexture;
  hr = m_device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
  if (FAILED(hr)) {
    return hr;
  }

  // Copy to staging
  m_context->CopyResource(stagingTexture.Get(), pTexture);

  // Map and read
  D3D11_MAPPED_SUBRESOURCE mapped;
  hr = m_context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped);
  if (FAILED(hr)) {
    return hr;
  }

  // Create WIC bitmap from mapped data
  hr = m_wicFactory->CreateBitmapFromMemory(
      desc.Width, desc.Height, GUID_WICPixelFormat32bppBGRA, mapped.RowPitch,
      mapped.RowPitch * desc.Height, static_cast<BYTE *>(mapped.pData),
      ppBitmap);

  m_context->Unmap(stagingTexture.Get(), 0);

  return hr;
}

//=============================================================================
// Async Queue
//=============================================================================

void GPUAccelerator::QueueThumbnail(
    const std::wstring &filePath, UINT targetWidth, UINT targetHeight,
    int priority, std::function<void(IWICBitmap *)> callback) {
  ThumbnailRequest request;
  request.sourcePath = filePath;
  request.targetWidth = targetWidth;
  request.targetHeight = targetHeight;
  request.priority = priority;
  request.callback = callback;

  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_queue.push(request);
  }

  m_queueCV.notify_one();
}

void GPUAccelerator::CancelPendingRequests() {
  std::lock_guard<std::mutex> lock(m_queueMutex);
  while (!m_queue.empty()) {
    m_queue.pop();
  }
}

size_t GPUAccelerator::GetQueueSize() const {
  std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(m_queueMutex));
  return m_queue.size();
}

void GPUAccelerator::ProcessQueue() {
  while (m_queueRunning) {
    ThumbnailRequest request;

    {
      std::unique_lock<std::mutex> lock(m_queueMutex);
      m_queueCV.wait(lock,
                     [this] { return !m_queue.empty() || !m_queueRunning; });

      if (!m_queueRunning) {
        break;
      }

      if (m_queue.empty()) {
        continue;
      }

      request = m_queue.top();
      m_queue.pop();
    }

    // Process request
    ComPtr<IWICBitmap> thumbnail;
    HRESULT hr =
        CreateThumbnailFromFile(request.sourcePath, request.targetWidth,
                                request.targetHeight, &thumbnail);

    if (request.callback) {
      request.callback(SUCCEEDED(hr) ? thumbnail.Get() : nullptr);
    }
  }
}

//=============================================================================
// Device Status
//=============================================================================

bool GPUAccelerator::CheckDeviceStatus() {
  if (!m_device) {
    return false;
  }

  HRESULT hr = m_device->GetDeviceRemovedReason();
  if (FAILED(hr)) {
    m_deviceLost = true;
    OutputDebugString(L"[GPU] Device lost detected\n");
    return false;
  }

  m_deviceLost = false;
  return true;
}

HRESULT GPUAccelerator::RecoverDevice() {
  if (!m_deviceLost) {
    return S_OK;
  }

  OutputDebugString(L"[GPU] Attempting device recovery...\n");

  // Release old device
  m_context.Reset();
  m_device.Reset();

  // Try to recreate
  HRESULT hr = CreateDevice(true);
  if (SUCCEEDED(hr)) {
    hr = CreateResources();
    if (SUCCEEDED(hr)) {
      hr = CompileShaders();
      if (SUCCEEDED(hr)) {
        m_deviceLost = false;
        m_gpuAvailable = true;
        OutputDebugString(L"[GPU] Device recovery successful\n");
      }
    }
  }

  return hr;
}

//=============================================================================
// Texture Pool Management
//=============================================================================

HRESULT GPUAccelerator::AcquireTexture(UINT width, UINT height,
                                       DXGI_FORMAT format, UINT bindFlags,
                                       ID3D11Texture2D **ppTexture,
                                       ID3D11ShaderResourceView **ppSRV,
                                       ID3D11UnorderedAccessView **ppUAV) {
  if (!ppTexture) {
    return E_POINTER;
  }

  *ppTexture = nullptr;
  if (ppSRV)
    *ppSRV = nullptr;
  if (ppUAV)
    *ppUAV = nullptr;

  HRESULT hr = S_OK;

  // Try to find a matching texture in the pool
  {
    std::lock_guard<std::mutex> lock(m_poolMutex);

    for (auto &entry : m_texturePool) {
      if (!entry.inUse && entry.width == width && entry.height == height &&
          entry.format == format && entry.bindFlags == bindFlags) {
        // Found a matching texture, reuse it
        entry.inUse = true;
        entry.lastUsed = std::chrono::steady_clock::now();

        *ppTexture = entry.texture.Get();
        (*ppTexture)->AddRef();

        if (ppSRV && entry.srv) {
          *ppSRV = entry.srv.Get();
          (*ppSRV)->AddRef();
        }

        if (ppUAV && entry.uav) {
          *ppUAV = entry.uav.Get();
          (*ppUAV)->AddRef();
        }

        OutputDebugString(L"[GPU] Texture pool: Reused texture\n");
        return S_OK;
      }
    }
  }

  // No matching texture found, create a new one
  D3D11_TEXTURE2D_DESC texDesc = {};
  texDesc.Width = width;
  texDesc.Height = height;
  texDesc.MipLevels = 1;
  texDesc.ArraySize = 1;
  texDesc.Format = format;
  texDesc.SampleDesc.Count = 1;
  texDesc.SampleDesc.Quality = 0;
  texDesc.Usage = D3D11_USAGE_DEFAULT;
  texDesc.BindFlags = bindFlags;
  texDesc.CPUAccessFlags = 0;

  ComPtr<ID3D11Texture2D> newTexture;
  hr = m_device->CreateTexture2D(&texDesc, nullptr, &newTexture);
  if (FAILED(hr)) {
    OutputDebugString(L"[GPU] Failed to create texture for pool\n");
    return hr;
  }

  // Create SRV if requested
  ComPtr<ID3D11ShaderResourceView> newSRV;
  if (ppSRV && (bindFlags & D3D11_BIND_SHADER_RESOURCE)) {
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr =
        m_device->CreateShaderResourceView(newTexture.Get(), &srvDesc, &newSRV);
    if (FAILED(hr)) {
      OutputDebugString(L"[GPU] Failed to create SRV for pooled texture\n");
      return hr;
    }
  }

  // Create UAV if requested
  ComPtr<ID3D11UnorderedAccessView> newUAV;
  if (ppUAV && (bindFlags & D3D11_BIND_UNORDERED_ACCESS)) {
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;

    hr = m_device->CreateUnorderedAccessView(newTexture.Get(), &uavDesc,
                                             &newUAV);
    if (FAILED(hr)) {
      OutputDebugString(L"[GPU] Failed to create UAV for pooled texture\n");
      return hr;
    }
  }

  // Add to pool if not full
  {
    std::lock_guard<std::mutex> lock(m_poolMutex);

    if (m_texturePool.size() < MAX_POOL_SIZE) {
      TexturePoolEntry entry;
      entry.texture = newTexture;
      entry.srv = newSRV;
      entry.uav = newUAV;
      entry.width = width;
      entry.height = height;
      entry.format = format;
      entry.bindFlags = bindFlags;
      entry.inUse = true;
      entry.lastUsed = std::chrono::steady_clock::now();

      m_texturePool.push_back(entry);

      wchar_t msg[128];
      swprintf_s(
          msg, 128,
          L"[GPU] Texture pool: Added new texture (pool size: %zu/%zu)\n",
          m_texturePool.size(), MAX_POOL_SIZE);
      OutputDebugString(msg);
    } else {
      OutputDebugString(L"[GPU] Texture pool full, texture not pooled\n");
    }
  }

  *ppTexture = newTexture.Detach();
  if (ppSRV && newSRV) {
    *ppSRV = newSRV.Detach();
  }
  if (ppUAV && newUAV) {
    *ppUAV = newUAV.Detach();
  }

  return S_OK;
}

void GPUAccelerator::ReleaseTexture(ID3D11Texture2D *pTexture) {
  if (!pTexture) {
    return;
  }

  std::lock_guard<std::mutex> lock(m_poolMutex);

  // Find the texture in the pool and mark as not in use
  for (auto &entry : m_texturePool) {
    if (entry.texture.Get() == pTexture) {
      entry.inUse = false;
      entry.lastUsed = std::chrono::steady_clock::now();
      OutputDebugString(L"[GPU] Texture pool: Released texture\n");
      return;
    }
  }

  // Texture not in pool (happens when pool was full during creation)
  // Let it be destroyed by caller's Release()
}

void GPUAccelerator::ClearTexturePool() {
  // Called with m_poolMutex already locked from Shutdown()
  // Full clear during shutdown
  m_texturePool.clear();
  OutputDebugString(L"[GPU] Texture pool: Cleared all textures\n");
}

//=============================================================================
// ThumbnailGenerator Helper Class
//=============================================================================

HRESULT ThumbnailGenerator::Generate(IWICBitmapSource *pSource,
                                     UINT targetWidth, UINT targetHeight,
                                     IWICBitmap **ppThumbnail) {
  return GPUAccelerator::Instance().CreateThumbnail(pSource, targetWidth,
                                                    targetHeight, ppThumbnail);
}

HRESULT ThumbnailGenerator::GenerateFromFile(const std::wstring &filePath,
                                             UINT targetWidth,
                                             UINT targetHeight,
                                             IWICBitmap **ppThumbnail) {
  return GPUAccelerator::Instance().CreateThumbnailFromFile(
      filePath, targetWidth, targetHeight, ppThumbnail);
}

//=============================================================================
// Utility Functions
//=============================================================================

namespace GPUUtil {

bool IsDirectX11Available() {
  ComPtr<ID3D11Device> testDevice;
  D3D_FEATURE_LEVEL featureLevel;

  HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                 nullptr, 0, D3D11_SDK_VERSION, &testDevice,
                                 &featureLevel, nullptr);

  return SUCCEEDED(hr);
}

void GetOptimalSize(UINT srcWidth, UINT srcHeight, UINT maxSize, UINT &outWidth,
                    UINT &outHeight) {
  if (srcWidth <= maxSize && srcHeight <= maxSize) {
    outWidth = srcWidth;
    outHeight = srcHeight;
    return;
  }

  float aspectRatio =
      static_cast<float>(srcWidth) / static_cast<float>(srcHeight);

  if (srcWidth > srcHeight) {
    outWidth = maxSize;
    outHeight = static_cast<UINT>(maxSize / aspectRatio);
  } else {
    outHeight = maxSize;
    outWidth = static_cast<UINT>(maxSize * aspectRatio);
  }

  // Ensure dimensions are at least 1
  outWidth = max(1, outWidth);
  outHeight = max(1, outHeight);
}

UINT CalculateMipLevels(UINT width, UINT height) {
  UINT levels = 1;
  UINT size = max(width, height);

  while (size > 1) {
    size >>= 1;
    levels++;
  }

  return levels;
}

} // namespace GPUUtil

} // namespace DarkThumbs
