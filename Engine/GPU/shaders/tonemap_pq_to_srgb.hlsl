// tonemap_pq_to_srgb.hlsl — PQ (SMPTE ST 2084) → sRGB GPU tone-mapping
// Copyright (c) 2026 ExplorerLens Project
//
// Maps HDR10 / BT.2100 PQ-encoded content to display-referred sRGB for
// thumbnails. Peak luminance and exposure are configurable via root constants.
//
// Dispatch: numthreads(8,8,1), Dispatch(ceil(W/8), ceil(H/8), 1)
//

Texture2D<float4>   t_hdr : register(t0);   // Input: PQ-encoded RGBA16F or RGBA32F
RWTexture2D<float4> u_sdr : register(u0);   // Output: sRGB RGBA8 or RGBA16F

cbuffer TonemapParams : register(b0) {
    float g_peakLuminance;   // Display peak luminance (nits), e.g. 203.0 for SDR
    float g_maxContentNits;  // Content mastering max luminance, e.g. 1000.0
    float g_gamma;           // Output gamma (2.2 for sRGB, 1.0 for linear)
    float g_exposure;        // Exposure multiplier (default 1.0)
};

// ---------------------------------------------------------------------------
// PQ transfer function (SMPTE ST 2084 EOTF)
// Input: encoded signal E' in [0,1]
// Output: linear luminance in [0, 10000] nits
// ---------------------------------------------------------------------------
float PqToLinear(float x)
{
    static const float m1 = 0.1593017578125f;   // 2610/4096 / 4
    static const float m2 = 78.84375f;          // 2523/4096 * 128
    static const float c1 = 0.8359375f;         // 3424/4096
    static const float c2 = 18.8515625f;        // 2413/4096 * 32
    static const float c3 = 18.6875f;           // 2392/4096 * 32

    float xpow = pow(max(x, 0.0f), 1.0f / m2);
    float num  = max(xpow - c1, 0.0f);
    float den  = c2 - c3 * xpow;
    return 10000.0f * pow(num / max(den, 1e-10f), 1.0f / m1);
}

// ---------------------------------------------------------------------------
// BT.2020 → BT.709 (sRGB) gamut conversion matrix
// ---------------------------------------------------------------------------
float3 Bt2020ToBt709(float3 c)
{
    // Derived from the standard 3x3 conversion matrix
    return float3(
         1.6604910f * c.x - 0.5876411f * c.y - 0.0728499f * c.z,
        -0.1245505f * c.x + 1.1328999f * c.y - 0.0083494f * c.z,
        -0.0181508f * c.x - 0.1005789f * c.y + 1.1187297f * c.z
    );
}

// ---------------------------------------------------------------------------
// Simple Reinhard luminance tone-map
// Maps scene luminance to [0,1] display after peak scaling
// ---------------------------------------------------------------------------
float3 TonemapReinhard(float3 linearRgb, float peakScene)
{
    float lum = dot(linearRgb, float3(0.2126f, 0.7152f, 0.0722f));
    float mapped = lum * (1.0f + lum / (peakScene * peakScene)) / (1.0f + lum);
    return linearRgb * (mapped / max(lum, 1e-6f));
}

// ---------------------------------------------------------------------------
// sRGB gamma (linearize → encode)
// ---------------------------------------------------------------------------
float LinearToSrgb(float x)
{
    return (x <= 0.0031308f) ? 12.92f * x : 1.055f * pow(x, 1.0f / 2.4f) - 0.055f;
}

float4 LinearToSrgbVec(float4 c) {
    return float4(LinearToSrgb(c.x), LinearToSrgb(c.y), LinearToSrgb(c.z), c.w);
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void CSMain(uint3 tid : SV_DispatchThreadID)
{
    uint2 dims;
    t_hdr.GetDimensions(dims.x, dims.y);
    if (tid.x >= dims.x || tid.y >= dims.y) return;

    float4 pq = t_hdr.Load(int3(tid.xy, 0));

    // Decode PQ → linear nits per channel
    float3 linear = float3(PqToLinear(pq.x), PqToLinear(pq.y), PqToLinear(pq.z));

    // BT.2020 → BT.709 gamut conversion
    linear = Bt2020ToBt709(max(linear, 0.0f));

    // Apply exposure
    linear *= g_exposure;

    // Normalize to [0, peakDisplay] range
    float normPeak = g_peakLuminance / 80.0f;  // 80 nit = SDR white
    linear /= 80.0f;   // Bring 80 nit → 1.0

    // Tone-map from maxContent-nit scene to range
    float scenePeak = g_maxContentNits / 80.0f;
    linear = TonemapReinhard(linear, scenePeak);

    // Clamp and apply output gamma
    float4 out4 = saturate(float4(linear, pq.w));
    if (g_gamma != 1.0f) {
        out4 = LinearToSrgbVec(out4);
    }

    u_sdr[tid.xy] = out4;
}
