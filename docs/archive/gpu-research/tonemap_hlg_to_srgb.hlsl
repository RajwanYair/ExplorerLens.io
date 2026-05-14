// tonemap_hlg_to_srgb.hlsl — HLG → sRGB Tonemap Compute Shader
// Copyright (c) 2026 ExplorerLens Project
//
// CS_5_0 compute shader: converts HLG (Hybrid Log-Gamma, ARIB STD-B67) HDR
// encoded image to display-referred sRGB for thumbnail preview.
//
// Steps: HLG OETF inverse → scene-referred linear → HLG OOTF → display sRGB
// Reference display: 1000 cd/m² peak with BT.2020 primaries (standard for HLG)
//

Texture2D<float4>    g_Input  : register(t0);
RWTexture2D<float4>  g_Output : register(u0);

cbuffer HLGConstants : register(b0)
{
    uint  g_Width;
    uint  g_Height;
    float g_DisplayPeakNits;   // typically 1000.0
    float g_SystemGamma;       // OOTF system gamma, typically 1.2 (BT.2100)
    float g_Pad0;
    float g_Pad1;
    float g_Pad2;
    float g_Pad3;
};

// HLG inverse OETF (scene-referred linear from HLG encoded)
float HLGInverseOETF(float e)
{
    const float a = 0.17883277f;
    const float b = 0.28466892f;
    const float c = 0.55991073f;

    if (e <= 0.5f)
        return (e * e) / 3.0f;
    else
        return (exp((e - c) / a) + b) / 12.0f;
}

float3 HLGInverseOETF3(float3 e)
{
    return float3(HLGInverseOETF(e.x), HLGInverseOETF(e.y), HLGInverseOETF(e.z));
}

// HLG OOTF: scene-referred → display-referred linear
// Uses relative luminance for the non-linear system-gamma operation
float3 HLGOOTF(float3 sceneLinear, float systemGamma, float peakNits)
{
    // Compute relative luminance (BT.2020 coefficients)
    float Yscene = dot(sceneLinear, float3(0.2627f, 0.6780f, 0.0593f));
    float Ys     = max(Yscene, 1e-10f);

    // OOTF: F_D = alpha * Y_S^(gamma-1) * F_S
    float alpha  = peakNits / 12.0f;
    float factor = alpha * pow(Ys, systemGamma - 1.0f);

    return sceneLinear * factor;
}

// sRGB gamma encode
float3 LinearToSRGB(float3 lin)
{
    lin = max(lin, 0.0f);
    float3 lo = lin * 12.92f;
    float3 hi = 1.055f * pow(lin, 1.0f / 2.4f) - 0.055f;
    return select(lin <= 0.0031308f, lo, hi);
}

[numthreads(8, 8, 1)]
void main(uint3 dispatch : SV_DispatchThreadID)
{
    uint x = dispatch.x;
    uint y = dispatch.y;

    if (x >= g_Width || y >= g_Height)
        return;

    float4 sampleVal = g_Input.Load(int3(int(x), int(y), 0));

    // Inverse OETF: HLG-encoded → scene-referred linear
    float3 sceneLinear = HLGInverseOETF3(sampleVal.rgb);

    // OOTF: scene-referred → display-referred linear (normalised to [0,1])
    float3 displayLinear = HLGOOTF(sceneLinear, g_SystemGamma, g_DisplayPeakNits);
    displayLinear /= g_DisplayPeakNits;

    // sRGB gamma encode
    float3 srgb = LinearToSRGB(clamp(displayLinear, 0.0f, 1.0f));

    g_Output[uint2(x, y)] = float4(srgb, sampleVal.a);
}
