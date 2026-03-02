// ============================================================================
// LanczosGPUKernel.h — High-Quality Lanczos Resampling via D3D11 Compute
// Copyright (c) 2026 ExplorerLens Project
//
// PURPOSE
//   Implements Lanczos-3 (and optionally Lanczos-2) image resampling as a
//   DirectX 11 compute shader dispatched at runtime.  The HLSL source is
//   embedded as a string constant and compiled via D3DCompile (loaded
//   dynamically from d3dcompiler_47.dll — no link-time dependency).
//
// CLASSES
//   LanczosGPUKernel  — main entry point
//
// KEY API
//   Initialize(ID3D11Device* = nullptr)
//       Creates a D3D11 device (HARDWARE, then WARP fallback) if none is
//       supplied, compiles the HLSL compute shader, and creates the full
//       GPU pipeline (constant buffer, SRV, UAV, staging readback texture).
//
//   Resize(srcRGBA, srcW, srcH, dstRGBA, dstW, dstH)
//       Uploads the source RGBA8 image to a GPU texture, dispatches the
//       Lanczos-3 compute shader, reads back the result through a staging
//       texture, and writes it into dstRGBA.  If the GPU path is
//       unavailable the call transparently falls back to a software
//       separable Lanczos implementation (horizontal then vertical).
//
//   SetFilterRadius(taps)  — 2 for Lanczos-2 or 3 for Lanczos-3 (default)
//   GetStats()             — returns ResizeStats (path, time, pixels, etc.)
//
// INPUTS / OUTPUTS
//   Input:  const uint8_t* srcRGBA — tightly-packed RGBA8 (4 bytes/pixel)
//   Output: uint8_t* dstRGBA       — tightly-packed RGBA8
//
// DEPENDENCIES
//   Windows API only.  d3d11.dll and d3dcompiler_47.dll loaded at runtime
//   via LoadLibraryW/GetProcAddress — no .lib link requirement.
// ============================================================================
#pragma once

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <vector>
#include <string>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <atomic>

#ifndef EXPLORELENS_LANCZOS_PI
#define EXPLORELENS_LANCZOS_PI 3.14159265358979323846
#endif

namespace ExplorerLens {
namespace Engine {

// -----------------------------------------------------------------------
// ResizeStats — returned by GetStats()
// -----------------------------------------------------------------------
struct ResizeStats {
    bool     usedGPU = false;
    double   resizeTimeMicros = 0.0;
    uint64_t pixelsProcessed = 0;
    double   shaderCompileTimeMs = 0.0;
    uint64_t totalResizes = 0;
    uint64_t gpuResizes = 0;
    uint64_t cpuResizes = 0;
};

// -----------------------------------------------------------------------
// LanczosGPUKernel
// -----------------------------------------------------------------------
class LanczosGPUKernel {
public:
    LanczosGPUKernel() = default;
    ~LanczosGPUKernel() { Shutdown(); }

    LanczosGPUKernel(const LanczosGPUKernel&) = delete;
    LanczosGPUKernel& operator=(const LanczosGPUKernel&) = delete;

    // ---------------- public constants ----------------
    static constexpr uint32_t DEFAULT_TAPS = 3;

    // ================================================================
    // Initialize
    // ================================================================
    inline bool Initialize(ID3D11Device* device = nullptr) {
        if (m_ready) return true;

        // ---- dynamic-load d3d11.dll ----
        m_hD3D11 = ::LoadLibraryW(L"d3d11.dll");
        m_hCompiler = ::LoadLibraryW(L"d3dcompiler_47.dll");
        if (!m_hCompiler) m_hCompiler = ::LoadLibraryW(L"d3dcompiler_46.dll");

        if (device) {
            m_device = device;
            m_device->AddRef();
            m_device->GetImmediateContext(&m_ctx);
            m_ownsDevice = false;
        }
        else {
            if (!CreateDevice()) {
                // GPU unavailable — CPU-only mode is fine
                m_ready = true;
                return true;
            }
        }

        if (m_device && m_hCompiler) {
            CompileShader();
        }

        m_ready = true;
        return true;
    }

    // ================================================================
    // Resize
    // ================================================================
    inline bool Resize(const uint8_t* srcRGBA, uint32_t srcW, uint32_t srcH,
        uint8_t* dstRGBA, uint32_t dstW, uint32_t dstH) {
        if (!srcRGBA || !dstRGBA || srcW == 0 || srcH == 0 || dstW == 0 || dstH == 0)
            return false;

        auto t0 = std::chrono::steady_clock::now();
        bool gpu = false;

        if (m_computeShader && m_device && m_ctx) {
            gpu = ResizeGPU(srcRGBA, srcW, srcH, dstRGBA, dstW, dstH);
        }
        if (!gpu) {
            ResizeCPU(srcRGBA, srcW, srcH, dstRGBA, dstW, dstH);
        }

        auto t1 = std::chrono::steady_clock::now();
        double us = std::chrono::duration<double, std::micro>(t1 - t0).count();

        m_stats.resizeTimeMicros = us;
        m_stats.pixelsProcessed = static_cast<uint64_t>(dstW) * dstH;
        m_stats.usedGPU = gpu;
        m_stats.totalResizes++;
        if (gpu) m_stats.gpuResizes++; else m_stats.cpuResizes++;
        return true;
    }

    // ================================================================
    // SetFilterRadius  (2 = Lanczos-2, 3 = Lanczos-3)
    // ================================================================
    inline void SetFilterRadius(uint32_t taps) {
        m_taps = (taps == 2) ? 2u : 3u;
    }

    // ================================================================
    // GetStats
    // ================================================================
    inline ResizeStats GetStats() const { return m_stats; }

private:
    // ---- state ----
    bool               m_ready = false;
    bool               m_ownsDevice = true;
    uint32_t           m_taps = DEFAULT_TAPS;
    HMODULE            m_hD3D11 = nullptr;
    HMODULE            m_hCompiler = nullptr;
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_ctx = nullptr;
    ID3D11ComputeShader* m_computeShader = nullptr;
    ID3D11Buffer* m_cbParams = nullptr;
    ResizeStats        m_stats{};

    // ---- constant buffer layout ----
    struct alignas(16) CBResize {
        uint32_t srcW;
        uint32_t srcH;
        uint32_t dstW;
        uint32_t dstH;
        float    scaleX;
        float    scaleY;
        uint32_t taps;
        uint32_t _pad;
    };

    // ================================================================
    // HLSL source (Lanczos-3 compute shader)
    // ================================================================
    static const char* HLSLSource() {
        return R"HLSL(
cbuffer CB : register(b0) {
    uint  srcW;
    uint  srcH;
    uint  dstW;
    uint  dstH;
    float scaleX;
    float scaleY;
    uint  taps;
    uint  _pad;
};

Texture2D<float4>   SrcTex : register(t0);
RWTexture2D<float4> DstTex : register(u0);

static const float PI = 3.14159265358979323846;

float Sinc(float x) {
    if (abs(x) < 1e-7) return 1.0;
    float px = PI * x;
    return sin(px) / px;
}

float Lanczos(float x, float a) {
    if (abs(x) < 1e-7) return 1.0;
    if (abs(x) >= a)   return 0.0;
    return Sinc(x) * Sinc(x / a);
}

[numthreads(16, 16, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID) {
    if (DTid.x >= dstW || DTid.y >= dstH) return;

    float cx = ((float)DTid.x + 0.5) * scaleX - 0.5;
    float cy = ((float)DTid.y + 0.5) * scaleY - 0.5;

    int   a  = (int)taps;
    float af = (float)a;

    float4 color   = float4(0, 0, 0, 0);
    float  wSum    = 0.0;

    int y0 = (int)floor(cy) - a + 1;
    int y1 = (int)floor(cy) + a;
    int x0 = (int)floor(cx) - a + 1;
    int x1 = (int)floor(cx) + a;

    for (int iy = y0; iy <= y1; iy++) {
        float wy = Lanczos(cy - (float)iy, af);
        int sy = clamp(iy, 0, (int)srcH - 1);
        for (int ix = x0; ix <= x1; ix++) {
            float wx = Lanczos(cx - (float)ix, af);
            int sx = clamp(ix, 0, (int)srcW - 1);
            float w = wx * wy;
            color += SrcTex.Load(int3(sx, sy, 0)) * w;
            wSum  += w;
        }
    }

    if (wSum > 0.0) color /= wSum;
    color = saturate(color);
    DstTex[DTid.xy] = color;
}
)HLSL";
    }

    // ================================================================
    // CreateDevice — D3D11CreateDevice (HARDWARE → WARP)
    // ================================================================
    inline bool CreateDevice() {
        if (!m_hD3D11) return false;
        using PFN_D3D11Create = HRESULT(WINAPI*)(
            IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT,
            const D3D_FEATURE_LEVEL*, UINT, UINT,
            ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);

        auto pfn = reinterpret_cast<PFN_D3D11Create>(
            ::GetProcAddress(m_hD3D11, "D3D11CreateDevice"));
        if (!pfn) return false;

        D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
        D3D_FEATURE_LEVEL got{};
        HRESULT hr = pfn(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            levels, 1, D3D11_SDK_VERSION,
            &m_device, &got, &m_ctx);
        if (FAILED(hr)) {
            hr = pfn(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
                levels, 1, D3D11_SDK_VERSION,
                &m_device, &got, &m_ctx);
        }
        if (FAILED(hr)) { m_device = nullptr; m_ctx = nullptr; return false; }
        m_ownsDevice = true;
        return true;
    }

    // ================================================================
    // CompileShader — D3DCompile from d3dcompiler_47.dll
    // ================================================================
    inline void CompileShader() {
        using PFN_D3DCompile = HRESULT(WINAPI*)(
            LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*,
            ID3DInclude*, LPCSTR, LPCSTR, UINT, UINT,
            ID3DBlob**, ID3DBlob**);

        auto pfnCompile = reinterpret_cast<PFN_D3DCompile>(
            ::GetProcAddress(m_hCompiler, "D3DCompile"));
        if (!pfnCompile) return;

        auto t0 = std::chrono::steady_clock::now();

        const char* src = HLSLSource();
        ID3DBlob* blob = nullptr;
        ID3DBlob* err = nullptr;
        HRESULT hr = pfnCompile(src, strlen(src), "LanczosCS", nullptr,
            nullptr, "CSMain", "cs_5_0", 0, 0,
            &blob, &err);
        if (err) err->Release();
        if (FAILED(hr) || !blob) return;

        auto t1 = std::chrono::steady_clock::now();
        m_stats.shaderCompileTimeMs =
            std::chrono::duration<double, std::milli>(t1 - t0).count();

        m_device->CreateComputeShader(blob->GetBufferPointer(),
            blob->GetBufferSize(), nullptr,
            &m_computeShader);
        blob->Release();

        // constant buffer
        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = sizeof(CBResize);
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        m_device->CreateBuffer(&bd, nullptr, &m_cbParams);
    }

    // ================================================================
    // ResizeGPU — upload → dispatch → readback
    // ================================================================
    inline bool ResizeGPU(const uint8_t* src, uint32_t srcW, uint32_t srcH,
        uint8_t* dst, uint32_t dstW, uint32_t dstH) {
        if (!m_computeShader || !m_cbParams) return false;

        // -- source texture + SRV --
        D3D11_TEXTURE2D_DESC td{};
        td.Width = srcW;
        td.Height = srcH;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_IMMUTABLE;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = src;
        init.SysMemPitch = srcW * 4;

        ID3D11Texture2D* srcTex = nullptr;
        if (FAILED(m_device->CreateTexture2D(&td, &init, &srcTex))) return false;

        ID3D11ShaderResourceView* srv = nullptr;
        m_device->CreateShaderResourceView(srcTex, nullptr, &srv);

        // -- output texture + UAV --
        D3D11_TEXTURE2D_DESC otd{};
        otd.Width = dstW;
        otd.Height = dstH;
        otd.MipLevels = 1;
        otd.ArraySize = 1;
        otd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        otd.SampleDesc.Count = 1;
        otd.Usage = D3D11_USAGE_DEFAULT;
        otd.BindFlags = D3D11_BIND_UNORDERED_ACCESS;

        ID3D11Texture2D* outTex = nullptr;
        if (FAILED(m_device->CreateTexture2D(&otd, nullptr, &outTex))) {
            srv->Release(); srcTex->Release(); return false;
        }

        ID3D11UnorderedAccessView* uav = nullptr;
        m_device->CreateUnorderedAccessView(outTex, nullptr, &uav);

        // -- staging texture for readback --
        D3D11_TEXTURE2D_DESC std{};
        std.Width = dstW;
        std.Height = dstH;
        std.MipLevels = 1;
        std.ArraySize = 1;
        std.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        std.SampleDesc.Count = 1;
        std.Usage = D3D11_USAGE_STAGING;
        std.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

        ID3D11Texture2D* stageTex = nullptr;
        if (FAILED(m_device->CreateTexture2D(&std, nullptr, &stageTex))) {
            uav->Release(); outTex->Release(); srv->Release(); srcTex->Release();
            return false;
        }

        // -- update constant buffer --
        D3D11_MAPPED_SUBRESOURCE mapped{};
        m_ctx->Map(m_cbParams, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        auto* cb = static_cast<CBResize*>(mapped.pData);
        cb->srcW = srcW;
        cb->srcH = srcH;
        cb->dstW = dstW;
        cb->dstH = dstH;
        cb->scaleX = static_cast<float>(srcW) / static_cast<float>(dstW);
        cb->scaleY = static_cast<float>(srcH) / static_cast<float>(dstH);
        cb->taps = m_taps;
        cb->_pad = 0;
        m_ctx->Unmap(m_cbParams, 0);

        // -- dispatch --
        m_ctx->CSSetShader(m_computeShader, nullptr, 0);
        m_ctx->CSSetConstantBuffers(0, 1, &m_cbParams);
        m_ctx->CSSetShaderResources(0, 1, &srv);
        m_ctx->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

        uint32_t gx = (dstW + 15) / 16;
        uint32_t gy = (dstH + 15) / 16;
        m_ctx->Dispatch(gx, gy, 1);

        // -- readback --
        m_ctx->CopyResource(stageTex, outTex);

        D3D11_MAPPED_SUBRESOURCE rm{};
        HRESULT hr = m_ctx->Map(stageTex, 0, D3D11_MAP_READ, 0, &rm);
        if (SUCCEEDED(hr)) {
            const uint8_t* pSrc = static_cast<const uint8_t*>(rm.pData);
            for (uint32_t y = 0; y < dstH; ++y) {
                memcpy(dst + y * dstW * 4, pSrc + y * rm.RowPitch, dstW * 4);
            }
            m_ctx->Unmap(stageTex, 0);
        }

        // -- cleanup --
        ID3D11ShaderResourceView* nullSRV = nullptr;
        ID3D11UnorderedAccessView* nullUAV = nullptr;
        m_ctx->CSSetShaderResources(0, 1, &nullSRV);
        m_ctx->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);

        stageTex->Release();
        uav->Release();
        outTex->Release();
        srv->Release();
        srcTex->Release();

        return SUCCEEDED(hr);
    }

    // ================================================================
    // ResizeCPU — separable Lanczos (horizontal → vertical)
    // ================================================================
    inline void ResizeCPU(const uint8_t* src, uint32_t srcW, uint32_t srcH,
        uint8_t* dst, uint32_t dstW, uint32_t dstH) {
        // Horizontal pass → intermediate buffer
        std::vector<float> tmp(static_cast<size_t>(dstW) * srcH * 4, 0.0f);

        const float sx = static_cast<float>(srcW) / static_cast<float>(dstW);
        const float sy = static_cast<float>(srcH) / static_cast<float>(dstH);
        const int   a = static_cast<int>(m_taps);
        const float af = static_cast<float>(a);

        // -- horizontal --
        for (uint32_t y = 0; y < srcH; ++y) {
            for (uint32_t x = 0; x < dstW; ++x) {
                float cx = (static_cast<float>(x) + 0.5f) * sx - 0.5f;
                int   x0 = static_cast<int>(std::floor(cx)) - a + 1;
                int   x1 = static_cast<int>(std::floor(cx)) + a;

                float r = 0, g = 0, b = 0, aa = 0, wSum = 0;
                for (int ix = x0; ix <= x1; ++ix) {
                    float w = LanczosWeight(cx - static_cast<float>(ix), af);
                    int   sx2 = (std::max)(0, (std::min)(static_cast<int>(srcW) - 1, ix));
                    const uint8_t* p = src + (static_cast<size_t>(y) * srcW + sx2) * 4;
                    r += w * p[0];
                    g += w * p[1];
                    b += w * p[2];
                    aa += w * p[3];
                    wSum += w;
                }
                if (wSum > 0.0f) { r /= wSum; g /= wSum; b /= wSum; aa /= wSum; }
                size_t idx = (static_cast<size_t>(y) * dstW + x) * 4;
                tmp[idx + 0] = r;
                tmp[idx + 1] = g;
                tmp[idx + 2] = b;
                tmp[idx + 3] = aa;
            }
        }

        // -- vertical --
        for (uint32_t y = 0; y < dstH; ++y) {
            float cy = (static_cast<float>(y) + 0.5f) * sy - 0.5f;
            int   y0 = static_cast<int>(std::floor(cy)) - a + 1;
            int   y1 = static_cast<int>(std::floor(cy)) + a;

            for (uint32_t x = 0; x < dstW; ++x) {
                float r = 0, g = 0, b2 = 0, aa = 0, wSum = 0;
                for (int iy = y0; iy <= y1; ++iy) {
                    float w = LanczosWeight(cy - static_cast<float>(iy), af);
                    int   sy2 = (std::max)(0, (std::min)(static_cast<int>(srcH) - 1, iy));
                    size_t idx = (static_cast<size_t>(sy2) * dstW + x) * 4;
                    r += w * tmp[idx + 0];
                    g += w * tmp[idx + 1];
                    b2 += w * tmp[idx + 2];
                    aa += w * tmp[idx + 3];
                    wSum += w;
                }
                if (wSum > 0.0f) { r /= wSum; g /= wSum; b2 /= wSum; aa /= wSum; }

                auto clamp8 = [](float v) -> uint8_t {
                    int i = static_cast<int>(v + 0.5f);
                    return static_cast<uint8_t>((std::max)(0, (std::min)(255, i)));
                    };
                size_t oi = (static_cast<size_t>(y) * dstW + x) * 4;
                dst[oi + 0] = clamp8(r);
                dst[oi + 1] = clamp8(g);
                dst[oi + 2] = clamp8(b2);
                dst[oi + 3] = clamp8(aa);
            }
        }
    }

    // ================================================================
    // LanczosWeight — sinc(x) * sinc(x/a) with windowing
    // ================================================================
    static inline float LanczosWeight(float x, float a) {
        if (std::fabs(x) < 1e-7f) return 1.0f;
        if (std::fabs(x) >= a)    return 0.0f;
        float px = static_cast<float>(EXPLORELENS_LANCZOS_PI) * x;
        float pxa = px / a;
        return (std::sin(px) / px) * (std::sin(pxa) / pxa);
    }

    // ================================================================
    // Shutdown
    // ================================================================
    inline void Shutdown() {
        if (m_cbParams) { m_cbParams->Release();       m_cbParams = nullptr; }
        if (m_computeShader) { m_computeShader->Release();  m_computeShader = nullptr; }
        if (m_ctx) { m_ctx->Release();            m_ctx = nullptr; }
        if (m_device && m_ownsDevice) { m_device->Release(); }
        m_device = nullptr;
        if (m_hCompiler) { ::FreeLibrary(m_hCompiler);  m_hCompiler = nullptr; }
        if (m_hD3D11) { ::FreeLibrary(m_hD3D11);     m_hD3D11 = nullptr; }
        m_ready = false;
    }
};

} // namespace Engine
} // namespace ExplorerLens
