// tonemap_hlg_to_srgb.hlsl — HLG (Hybrid Log-Gamma) → sRGB GPU tone-mapping
// Copyright (c) 2026 ExplorerLens Project
//
// Implements the HLG EOTF (ITU-R BT.2100) and converts to display-referred sRGB.
// Suitable for HDR video stills, HEVC/HEIF HDR content.
//
// Dispatch: numthreads(8,8,1), Dispatch(ceil(W/8), ceil(H/8), 1)
//

Texture2D<float4>   t_hdr : register(t0);   // HLG scene-referred RGBA16F
RWTexture2D<float4> u_sdr : register(u0);   // sRGB output

cbuffer HlgTonemapParams : register(b0) {
    float g_displayPeakNits;   // Target display peak (default: 203 nit SDR)
    float g_systemGamma;       // BT.2100 system gamma parameter γ (default 1.2)
    float g_exposure;
    float g_pad;
};

// ---------------------------------------------------------------------------
// HLG OETF inverse (scene → display luminance, normalized)
// Input E' ∈ [0,1], output ∈ [0,1] linear scene luminance
// ---------------------------------------------------------------------------
float HlgInv(float x)
{
    static const float a = 0.17883277f;
    static const float b = 0.28466892f;
    static const float c = 0.55991073f;

    if (x <= 0.5f) {
        return (x * x) / 3.0f;
    } else {
        return (exp((x - c) / a) + b) / 12.0f;
    }
}

// HLG EOTF: scene → display luminance [0,1]
// Applied per component, then BT.2100 system gamma correction applied to luma
float3 HlgEotf(float3 E_prime, float peakNits, float sysGamma)
{
    // 1. Apply HLG OETF inverse per component
    float3 scene = float3(HlgInv(E_prime.x), HlgInv(E_prime.y), HlgInv(E_prime.z));

    // 2. BT.2100 reference white is 1000 nit; normalize to display peak
    scene *= (peakNits / 1000.0f);

    // 3. System gamma adjustment (typically 1.0–1.2)
    float luma = dot(scene, float3(0.2627f, 0.6780f, 0.0593f));
    float lgamma = pow(max(luma, 1e-6f), sysGamma - 1.0f);
    return scene * lgamma;
}

// BT.2020 → BT.709 gamut
float3 Bt2020ToBt709(float3 c)
{
    return float3(
         1.6604910f * c.x - 0.5876411f * c.y - 0.0728499f * c.z,
        -0.1245505f * c.x + 1.1328999f * c.y - 0.0083494f * c.z,
        -0.0181508f * c.x - 0.1005789f * c.y + 1.1187297f * c.z
    );
}

float LinearToSrgb(float x) {
    return (x <= 0.0031308f) ? 12.92f * x : 1.055f * pow(x, 1.0f / 2.4f) - 0.055f;
}

[numthreads(8, 8, 1)]
void CSMain(uint3 tid : SV_DispatchThreadID)
{
    uint2 dims;
    t_hdr.GetDimensions(dims.x, dims.y);
    if (tid.x >= dims.x || tid.y >= dims.y) return;

    float4 hlgIn = t_hdr.Load(int3(tid.xy, 0));

    // HLG EOTF: encoded → linear display luminance
    float3 linear = HlgEotf(hlgIn.rgb * g_exposure, g_displayPeakNits, g_systemGamma);

    // Gamut conversion BT.2020 → BT.709
    linear = Bt2020ToBt709(max(linear, 0.0f));

    // sRGB gamma encode
    float4 srgb = saturate(float4(
        LinearToSrgb(linear.x),
        LinearToSrgb(linear.y),
        LinearToSrgb(linear.z),
        hlgIn.w
    ));

    u_sdr[tid.xy] = srgb;
}
