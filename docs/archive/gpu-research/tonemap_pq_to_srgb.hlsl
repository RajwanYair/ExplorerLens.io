// tonemap_pq_to_srgb.hlsl — ST.2084 PQ → sRGB Tonemap Compute Shader
// Copyright (c) 2026 ExplorerLens Project
//
// CS_5_0 compute shader: converts HDR PQ (SMPTE ST.2084) encoded image
// to display-referred sRGB for thumbnail preview.
//
// Input:  linear-light scRGB fp16 values (after PQ EOTF)
// Output: gamma-encoded sRGB [0,1] BGRA32
//
// Reference white: 203 cd/m² (SDR reference per BT.2408)
// Headroom: HDR10 10000 cd/m² → mapped to [0, 1] display range
//

Texture2D<float4>    g_Input  : register(t0);
RWTexture2D<float4>  g_Output : register(u0);

cbuffer TonemapConstants : register(b0)
{
    uint  g_Width;
    uint  g_Height;
    float g_ReferenceWhiteNits;  // typically 203.0
    float g_PeakLuminanceNits;   // typically 10000.0
    float g_ExposureBias;        // 0.0 = neutral
    float g_Pad0;
    float g_Pad1;
    float g_Pad2;
};

// ST.2084 PQ inverse EOTF: linear light → PQ encoded
float3 LinearToPQ(float3 lin)
{
    const float m1 = 2610.0f / 16384.0f;
    const float m2 = 2523.0f / 4096.0f * 128.0f;
    const float c1 = 3424.0f / 4096.0f;
    const float c2 = 2413.0f / 4096.0f * 32.0f;
    const float c3 = 2392.0f / 4096.0f * 32.0f;

    float3 y = pow(max(lin / 10000.0f, 0.0f), m1);
    return pow((c1 + c2 * y) / (1.0f + c3 * y), m2);
}

// ST.2084 EOTF: PQ encoded → linear light (nits)
float3 PQToLinear(float3 pq)
{
    const float m1 = 2610.0f / 16384.0f;
    const float m2 = 2523.0f / 4096.0f * 128.0f;
    const float c1 = 3424.0f / 4096.0f;
    const float c2 = 2413.0f / 4096.0f * 32.0f;
    const float c3 = 2392.0f / 4096.0f * 32.0f;

    float3 e = pow(max(pq, 0.0f), 1.0f / m2);
    return 10000.0f * pow(max(e - c1, 0.0f) / (c2 - c3 * e), 1.0f / m1);
}

// Neutral "knee" tonemap: compress HDR headroom into SDR range
float3 ToneMapKnee(float3 linear, float referenceWhite, float peakNits)
{
    // Normalise to [0, 1] where 1 = referenceWhite nits
    float3 x = linear / referenceWhite;

    // Reinhard extended to peak
    float peak = peakNits / referenceWhite;
    return x * (1.0f + x / (peak * peak)) / (1.0f + x);
}

// sRGB gamma encode (IEC 61966-2-1)
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

    // Input is linear-light scRGB multiply by PeakLuminance to get nits
    float3 linearNits = sampleVal.rgb * g_PeakLuminanceNits * pow(2.0f, g_ExposureBias);

    float3 tonemapped = ToneMapKnee(linearNits, g_ReferenceWhiteNits, g_PeakLuminanceNits);
    float3 srgb       = LinearToSRGB(clamp(tonemapped, 0.0f, 1.0f));

    g_Output[uint2(x, y)] = float4(srgb, sampleVal.a);
}
