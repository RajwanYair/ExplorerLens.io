// ============================================================================
// HDRToneMapKernel.h — HDR-to-SDR Tone Mapping via GPU Compute Shader
// ExplorerLens Engine v15.0.0  (Sprint 565)
// Copyright (c) 2026 ExplorerLens Project
//
// PURPOSE
//   Converts high-dynamic-range floating-point images (EXR, HDR, AVIF-HDR)
//   into standard 8-bit SDR thumbnails suitable for Windows Explorer.
//   Four tone-mapping operators are provided — Reinhard (extended with
//   white point), ACES filmic (Stephen Hill's RRT+ODT fit), Hable /
//   Uncharted 2, and simple Exposure — each implemented both as an
//   embedded HLSL compute shader and as a pure-CPU fallback.
//
// CLASSES
//   HDRToneMapKernel — main entry point
//
// KEY API
//   Initialize()
//       Creates a D3D11 device (dynamic load), compiles all 4 shader
//       variants via D3DCompile.
//
//   ToneMap(srcHDR, width, height, channels, dstSDR, op)
//       Uploads float HDR data to a GPU texture, dispatches the selected
//       tone-map compute shader, gamma-corrects, reads back 8-bit RGBA.
//       Falls back to software if the GPU path is unavailable.
//
//   SetExposure(float ev)   — exposure compensation in EV stops
//   SetGamma(float gamma)   — output gamma curve (default 2.2)
//   GetStats()              — operator used, GPU/CPU, time, pixels
//
// INPUTS / OUTPUTS
//   Input:  const float* srcHDR — linear float RGB (3 or 4 channels)
//   Output: uint8_t* dstSDR     — tightly-packed RGBA8
//
// DEPENDENCIES
//   Windows API only.  d3d11.dll and d3dcompiler_47.dll loaded at runtime.
// ============================================================================
#pragma once

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <string>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

// -----------------------------------------------------------------------
// ToneMapKernelOp — selectable tone-mapping curve
// -----------------------------------------------------------------------
enum class ToneMapKernelOp : uint8_t {
    Reinhard  = 0,
    ACES      = 1,
    Hable     = 2,
    Exposure  = 3
};

inline const char* ToneMapKernelOpName(ToneMapKernelOp op) {
    switch (op) {
        case ToneMapKernelOp::Reinhard: return "Reinhard";
        case ToneMapKernelOp::ACES:     return "ACES";
        case ToneMapKernelOp::Hable:    return "Hable";
        case ToneMapKernelOp::Exposure: return "Exposure";
    }
    return "Unknown";
}

// -----------------------------------------------------------------------
// ToneMapStats
// -----------------------------------------------------------------------
struct ToneMapStats {
    ToneMapKernelOp operatorUsed   = ToneMapKernelOp::ACES;
    bool            usedGPU        = false;
    double          processingTimeUs = 0.0;
    uint64_t        pixelsProcessed = 0;
    uint64_t        totalCalls      = 0;
    uint64_t        gpuCalls        = 0;
    uint64_t        cpuCalls        = 0;
};

// -----------------------------------------------------------------------
// HDRToneMapKernel
// -----------------------------------------------------------------------
class HDRToneMapKernel {
public:
    HDRToneMapKernel()  = default;
    ~HDRToneMapKernel() { Shutdown(); }

    HDRToneMapKernel(const HDRToneMapKernel&)            = delete;
    HDRToneMapKernel& operator=(const HDRToneMapKernel&) = delete;

    // ================================================================
    // Initialize — create D3D device, compile all 4 shader variants
    // ================================================================
    inline bool Initialize() {
        if (m_ready) return true;

        m_hD3D11    = ::LoadLibraryW(L"d3d11.dll");
        m_hCompiler = ::LoadLibraryW(L"d3dcompiler_47.dll");
        if (!m_hCompiler) m_hCompiler = ::LoadLibraryW(L"d3dcompiler_46.dll");

        CreateDevice();

        if (m_device && m_hCompiler) {
            CompileVariant("TONEMAP_REINHARD",  0);
            CompileVariant("TONEMAP_ACES",      1);
            CompileVariant("TONEMAP_HABLE",     2);
            CompileVariant("TONEMAP_EXPOSURE",  3);
        }
        m_ready = true;
        return true;
    }

    // ================================================================
    // ToneMap — main entry
    // ================================================================
    inline bool ToneMap(const float* srcHDR, uint32_t width, uint32_t height,
                        uint32_t channels, uint8_t* dstSDR,
                        ToneMapKernelOp op = ToneMapKernelOp::ACES) {
        if (!srcHDR || !dstSDR || width == 0 || height == 0 || channels < 3)
            return false;

        auto t0 = std::chrono::steady_clock::now();
        bool gpu = false;
        uint32_t idx = static_cast<uint32_t>(op);

        if (m_device && m_ctx && idx < 4 && m_shaders[idx]) {
            gpu = ToneMapGPU(srcHDR, width, height, channels, dstSDR, idx);
        }
        if (!gpu) {
            ToneMapCPU(srcHDR, width, height, channels, dstSDR, op);
        }

        auto t1 = std::chrono::steady_clock::now();
        m_stats.processingTimeUs = std::chrono::duration<double, std::micro>(t1 - t0).count();
        m_stats.pixelsProcessed  = static_cast<uint64_t>(width) * height;
        m_stats.operatorUsed     = op;
        m_stats.usedGPU          = gpu;
        m_stats.totalCalls++;
        if (gpu) m_stats.gpuCalls++; else m_stats.cpuCalls++;
        return true;
    }

    inline void SetExposure(float ev) { m_exposureEV = ev; }
    inline void SetGamma(float gamma) { m_gamma = (gamma > 0.1f) ? gamma : 2.2f; }
    inline ToneMapStats GetStats() const { return m_stats; }

private:
    // ---- state ----
    bool               m_ready       = false;
    float              m_exposureEV  = 0.0f;
    float              m_gamma       = 2.2f;
    HMODULE            m_hD3D11      = nullptr;
    HMODULE            m_hCompiler   = nullptr;
    ID3D11Device*      m_device      = nullptr;
    ID3D11DeviceContext* m_ctx       = nullptr;
    ID3D11ComputeShader* m_shaders[4] = {};
    ID3D11Buffer*      m_cbToneMap   = nullptr;
    ToneMapStats       m_stats{};

    struct alignas(16) CBToneMap {
        uint32_t width;
        uint32_t height;
        float    exposure;
        float    gamma;
        float    whitePoint;
        uint32_t opIndex;
        uint32_t channels;
        uint32_t _pad;
    };

    // ================================================================
    // HLSL — all 4 operators in one source, macro-switched
    // ================================================================
    static const char* HLSLSource() {
        return R"HLSL(
cbuffer CB : register(b0) {
    uint  imgWidth;
    uint  imgHeight;
    float exposure;
    float gamma;
    float whitePoint;
    uint  opIndex;
    uint  channels;
    uint  _pad;
};

Texture2D<float4>   SrcTex : register(t0);
RWTexture2D<float4> DstTex : register(u0);

float3 ApplyExposure(float3 c, float e) {
    return c * pow(2.0, e);
}

float3 ReinhardToneMap(float3 c) {
    // Extended Reinhard with white point
    float lum = dot(c, float3(0.2126, 0.7152, 0.0722));
    float numer = lum * (1.0 + lum / (whitePoint * whitePoint));
    float mapped = numer / (1.0 + lum);
    float scale = (lum > 1e-6) ? (mapped / lum) : 1.0;
    return c * scale;
}

float3 ACESToneMap(float3 x) {
    // Stephen Hill's RRT + ODT approximation
    float3 a = x * (x * 2.51 + 0.03);
    float3 b = x * (x * 2.43 + 0.59) + 0.14;
    return a / b;
}

float3 HableToneMap(float3 x) {
    // Hable / Uncharted 2 formula
    float A = 0.15; float B = 0.50; float C = 0.10;
    float D = 0.20; float E = 0.02; float F = 0.30;
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F)) - E/F;
}

float3 GammaCorrect(float3 c, float g) {
    float ig = 1.0 / g;
    return float3(pow(saturate(c.x), ig),
                  pow(saturate(c.y), ig),
                  pow(saturate(c.z), ig));
}

[numthreads(16, 16, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID) {
    if (DTid.x >= imgWidth || DTid.y >= imgHeight) return;

    float4 hdr = SrcTex.Load(int3(DTid.xy, 0));
    float3 c = ApplyExposure(hdr.xyz, exposure);

#ifdef TONEMAP_REINHARD
    c = ReinhardToneMap(c);
#elif defined(TONEMAP_ACES)
    c = ACESToneMap(c);
#elif defined(TONEMAP_HABLE)
    float3 W = float3(11.2, 11.2, 11.2);
    float3 num = HableToneMap(c * 2.0);
    float3 den = HableToneMap(W);
    c = num / den;
#elif defined(TONEMAP_EXPOSURE)
    c = saturate(c);
#endif

    c = GammaCorrect(c, gamma);
    DstTex[DTid.xy] = float4(c, hdr.a);
}
)HLSL";
    }

    // ================================================================
    // CreateDevice
    // ================================================================
    inline void CreateDevice() {
        if (!m_hD3D11) return;
        using PFN = HRESULT(WINAPI*)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE,
                                     UINT, const D3D_FEATURE_LEVEL*, UINT, UINT,
                                     ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);
        auto pfn = reinterpret_cast<PFN>(::GetProcAddress(m_hD3D11, "D3D11CreateDevice"));
        if (!pfn) return;

        D3D_FEATURE_LEVEL fl[] = { D3D_FEATURE_LEVEL_11_0 };
        D3D_FEATURE_LEVEL got{};
        HRESULT hr = pfn(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                         fl, 1, D3D11_SDK_VERSION, &m_device, &got, &m_ctx);
        if (FAILED(hr)) {
            pfn(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0,
                fl, 1, D3D11_SDK_VERSION, &m_device, &got, &m_ctx);
        }
        if (m_device) {
            D3D11_BUFFER_DESC bd{};
            bd.ByteWidth     = sizeof(CBToneMap);
            bd.Usage          = D3D11_USAGE_DYNAMIC;
            bd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            m_device->CreateBuffer(&bd, nullptr, &m_cbToneMap);
        }
    }

    // ================================================================
    // CompileVariant — compile one tone-map shader variant
    // ================================================================
    inline void CompileVariant(const char* define, uint32_t idx) {
        using PFN_Compile = HRESULT(WINAPI*)(LPCVOID, SIZE_T, LPCSTR,
                                             const D3D_SHADER_MACRO*, ID3DInclude*,
                                             LPCSTR, LPCSTR, UINT, UINT,
                                             ID3DBlob**, ID3DBlob**);
        auto pfn = reinterpret_cast<PFN_Compile>(
            ::GetProcAddress(m_hCompiler, "D3DCompile"));
        if (!pfn) return;

        D3D_SHADER_MACRO macros[2] = {};
        macros[0].Name       = define;
        macros[0].Definition = "1";
        macros[1].Name       = nullptr;
        macros[1].Definition = nullptr;

        const char* src = HLSLSource();
        ID3DBlob* blob = nullptr;
        ID3DBlob* err  = nullptr;
        HRESULT hr = pfn(src, strlen(src), "ToneMapCS", macros,
                         nullptr, "CSMain", "cs_5_0", 0, 0, &blob, &err);
        if (err) err->Release();
        if (SUCCEEDED(hr) && blob) {
            m_device->CreateComputeShader(blob->GetBufferPointer(),
                                          blob->GetBufferSize(), nullptr,
                                          &m_shaders[idx]);
            blob->Release();
        }
    }

    // ================================================================
    // ToneMapGPU — upload float tex → dispatch → readback RGBA8
    // ================================================================
    inline bool ToneMapGPU(const float* srcHDR, uint32_t w, uint32_t h,
                           uint32_t ch, uint8_t* dstSDR, uint32_t opIdx) {
        // -- HDR source as R32G32B32A32_FLOAT texture --
        // Convert to 4-channel if needed
        std::vector<float> rgba;
        const float* texData = srcHDR;
        if (ch == 3) {
            rgba.resize(static_cast<size_t>(w) * h * 4);
            for (size_t i = 0; i < static_cast<size_t>(w) * h; ++i) {
                rgba[i * 4 + 0] = srcHDR[i * 3 + 0];
                rgba[i * 4 + 1] = srcHDR[i * 3 + 1];
                rgba[i * 4 + 2] = srcHDR[i * 3 + 2];
                rgba[i * 4 + 3] = 1.0f;
            }
            texData = rgba.data();
        }

        D3D11_TEXTURE2D_DESC td{};
        td.Width = w; td.Height = h; td.MipLevels = 1; td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        td.SampleDesc.Count = 1; td.Usage = D3D11_USAGE_IMMUTABLE;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA init{}; init.pSysMem = texData; init.SysMemPitch = w * 16;
        ID3D11Texture2D* srcTex = nullptr;
        if (FAILED(m_device->CreateTexture2D(&td, &init, &srcTex))) return false;

        ID3D11ShaderResourceView* srv = nullptr;
        m_device->CreateShaderResourceView(srcTex, nullptr, &srv);

        // -- output RGBA8 texture + UAV --
        D3D11_TEXTURE2D_DESC otd{};
        otd.Width = w; otd.Height = h; otd.MipLevels = 1; otd.ArraySize = 1;
        otd.Format = DXGI_FORMAT_R8G8B8A8_UNORM; otd.SampleDesc.Count = 1;
        otd.Usage = D3D11_USAGE_DEFAULT; otd.BindFlags = D3D11_BIND_UNORDERED_ACCESS;

        ID3D11Texture2D* outTex = nullptr;
        if (FAILED(m_device->CreateTexture2D(&otd, nullptr, &outTex))) {
            srv->Release(); srcTex->Release(); return false;
        }
        ID3D11UnorderedAccessView* uav = nullptr;
        m_device->CreateUnorderedAccessView(outTex, nullptr, &uav);

        // -- staging for readback --
        D3D11_TEXTURE2D_DESC stg{};
        stg.Width = w; stg.Height = h; stg.MipLevels = 1; stg.ArraySize = 1;
        stg.Format = DXGI_FORMAT_R8G8B8A8_UNORM; stg.SampleDesc.Count = 1;
        stg.Usage = D3D11_USAGE_STAGING; stg.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

        ID3D11Texture2D* stageTex = nullptr;
        if (FAILED(m_device->CreateTexture2D(&stg, nullptr, &stageTex))) {
            uav->Release(); outTex->Release(); srv->Release(); srcTex->Release();
            return false;
        }

        // -- update constant buffer --
        D3D11_MAPPED_SUBRESOURCE mapped{};
        m_ctx->Map(m_cbToneMap, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        auto* cb = static_cast<CBToneMap*>(mapped.pData);
        cb->width      = w;
        cb->height     = h;
        cb->exposure   = m_exposureEV;
        cb->gamma      = m_gamma;
        cb->whitePoint = 4.0f;
        cb->opIndex    = opIdx;
        cb->channels   = ch;
        cb->_pad       = 0;
        m_ctx->Unmap(m_cbToneMap, 0);

        // -- dispatch --
        m_ctx->CSSetShader(m_shaders[opIdx], nullptr, 0);
        m_ctx->CSSetConstantBuffers(0, 1, &m_cbToneMap);
        m_ctx->CSSetShaderResources(0, 1, &srv);
        m_ctx->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
        m_ctx->Dispatch((w + 15) / 16, (h + 15) / 16, 1);

        // -- readback --
        m_ctx->CopyResource(stageTex, outTex);
        D3D11_MAPPED_SUBRESOURCE rm{};
        HRESULT hr = m_ctx->Map(stageTex, 0, D3D11_MAP_READ, 0, &rm);
        if (SUCCEEDED(hr)) {
            const uint8_t* pSrc = static_cast<const uint8_t*>(rm.pData);
            for (uint32_t y = 0; y < h; ++y)
                memcpy(dstSDR + y * w * 4, pSrc + y * rm.RowPitch, w * 4);
            m_ctx->Unmap(stageTex, 0);
        }

        // -- cleanup --
        ID3D11ShaderResourceView* nullSRV = nullptr;
        ID3D11UnorderedAccessView* nullUAV = nullptr;
        m_ctx->CSSetShaderResources(0, 1, &nullSRV);
        m_ctx->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
        stageTex->Release(); uav->Release(); outTex->Release();
        srv->Release(); srcTex->Release();
        return SUCCEEDED(hr);
    }

    // ================================================================
    // ToneMapCPU — all 4 operators in software
    // ================================================================
    inline void ToneMapCPU(const float* srcHDR, uint32_t w, uint32_t h,
                           uint32_t ch, uint8_t* dstSDR, ToneMapKernelOp op) {
        const float exposureMul = std::pow(2.0f, m_exposureEV);
        const float invGamma    = 1.0f / m_gamma;
        const float whitePoint  = 4.0f;
        const uint64_t npix     = static_cast<uint64_t>(w) * h;

        for (uint64_t i = 0; i < npix; ++i) {
            float r = srcHDR[i * ch + 0] * exposureMul;
            float g = srcHDR[i * ch + 1] * exposureMul;
            float b = srcHDR[i * ch + 2] * exposureMul;
            float a = (ch >= 4) ? srcHDR[i * ch + 3] : 1.0f;

            switch (op) {
            case ToneMapKernelOp::Reinhard: {
                float lum  = 0.2126f * r + 0.7152f * g + 0.0722f * b;
                float num  = lum * (1.0f + lum / (whitePoint * whitePoint));
                float den  = 1.0f + lum;
                float mapped = (den > 1e-7f) ? (num / den) : 0.0f;
                float scale  = (lum > 1e-7f) ? (mapped / lum) : 1.0f;
                r *= scale; g *= scale; b *= scale;
            } break;

            case ToneMapKernelOp::ACES: {
                // Stephen Hill's fit:  (x*(2.51x+0.03)) / (x*(2.43x+0.59)+0.14)
                auto aces = [](float x) -> float {
                    float v = (x * (2.51f * x + 0.03f)) /
                              (x * (2.43f * x + 0.59f) + 0.14f);
                    return (std::max)(0.0f, (std::min)(1.0f, v));
                };
                r = aces(r); g = aces(g); b = aces(b);
            } break;

            case ToneMapKernelOp::Hable: {
                // Hable / Uncharted 2
                auto hable = [](float x) -> float {
                    const float A = 0.15f, B = 0.50f, C = 0.10f;
                    const float D = 0.20f, E = 0.02f, F = 0.30f;
                    return ((x * (A * x + C * B) + D * E) /
                            (x * (A * x + B) + D * F)) - E / F;
                };
                const float W = 11.2f;
                float denom = hable(W);
                r = hable(r * 2.0f) / denom;
                g = hable(g * 2.0f) / denom;
                b = hable(b * 2.0f) / denom;
            } break;

            case ToneMapKernelOp::Exposure:
            default:
                // Simple clamp (exposure already applied)
                r = (std::min)(1.0f, (std::max)(0.0f, r));
                g = (std::min)(1.0f, (std::max)(0.0f, g));
                b = (std::min)(1.0f, (std::max)(0.0f, b));
                break;
            }

            // Gamma
            r = std::pow((std::max)(0.0f, (std::min)(1.0f, r)), invGamma);
            g = std::pow((std::max)(0.0f, (std::min)(1.0f, g)), invGamma);
            b = std::pow((std::max)(0.0f, (std::min)(1.0f, b)), invGamma);

            size_t oi = static_cast<size_t>(i) * 4;
            dstSDR[oi + 0] = static_cast<uint8_t>(r * 255.0f + 0.5f);
            dstSDR[oi + 1] = static_cast<uint8_t>(g * 255.0f + 0.5f);
            dstSDR[oi + 2] = static_cast<uint8_t>(b * 255.0f + 0.5f);
            dstSDR[oi + 3] = static_cast<uint8_t>((std::min)(1.0f, (std::max)(0.0f, a)) * 255.0f + 0.5f);
        }
    }

    // ================================================================
    // Shutdown
    // ================================================================
    inline void Shutdown() {
        if (m_cbToneMap) { m_cbToneMap->Release(); m_cbToneMap = nullptr; }
        for (auto& s : m_shaders) { if (s) { s->Release(); s = nullptr; } }
        if (m_ctx)    { m_ctx->Release();    m_ctx = nullptr; }
        if (m_device) { m_device->Release(); m_device = nullptr; }
        if (m_hCompiler) { ::FreeLibrary(m_hCompiler); m_hCompiler = nullptr; }
        if (m_hD3D11)    { ::FreeLibrary(m_hD3D11);    m_hD3D11 = nullptr; }
        m_ready = false;
    }
};

} // namespace Engine
} // namespace ExplorerLens
