// bicubic_resize.hlsl — Mitchell-Netravali Bicubic Resize Compute Shader
// ExplorerLens Engine v15.0.0 "Zenith" — Sprint 361
// Copyright (c) 2026 ExplorerLens Project
//
// High-quality bicubic interpolation using Mitchell-Netravali filter.
// Complements the existing Lanczos3 shader (thumbnail_resize.hlsl) with a
// softer filter that avoids ringing artifacts on photographic content.
//
// Filter: Mitchell (B=1/3, C=1/3) — balanced sharpness vs ringing
// Alternative: Catmull-Rom (B=0, C=0.5) — sharper but more ringing
//
// Dispatch: ceil(dstWidth/16) x ceil(dstHeight/16) x 1

// ============================================================================
// Constant Buffer
// ============================================================================

cbuffer ResizeParams : register(b0) {
    uint   g_SrcWidth;       // Source image width
    uint   g_SrcHeight;      // Source image height
    uint   g_DstWidth;       // Destination (thumbnail) width
    uint   g_DstHeight;      // Destination (thumbnail) height
    float  g_B;              // Mitchell B parameter (default: 1/3)
    float  g_C;              // Mitchell C parameter (default: 1/3)
    uint   g_LinearSample;   // 1 = sample in linear space (gamma-correct)
    uint   g_Padding;
};

// ============================================================================
// Resources
// ============================================================================

Texture2D<float4>   g_Input  : register(t0);
RWTexture2D<float4> g_Output : register(u0);
SamplerState        g_Sampler : register(s0);

// ============================================================================
// Mitchell-Netravali Cubic Filter
// ============================================================================
//
// The general cubic filter is parameterized by B and C:
//   Mitchell:    B=1/3, C=1/3  — balanced (recommended default)
//   Catmull-Rom: B=0,   C=1/2  — sharper interpolation
//   B-Spline:    B=1,   C=0    — smoothest (heavy blur)
//   Cardinal:    B=0,   C=1    — sharpest (most ringing)
//
// Kernel is defined piecewise for |x| in [0,1) and [1,2):

float MitchellNetravali(float x, float B, float C) {
    float ax = abs(x);
    float ax2 = ax * ax;
    float ax3 = ax2 * ax;

    if (ax < 1.0) {
        return ((12.0 - 9.0 * B - 6.0 * C) * ax3 +
                (-18.0 + 12.0 * B + 6.0 * C) * ax2 +
                (6.0 - 2.0 * B)) / 6.0;
    } else if (ax < 2.0) {
        return ((-B - 6.0 * C) * ax3 +
                (6.0 * B + 30.0 * C) * ax2 +
                (-12.0 * B - 48.0 * C) * ax +
                (8.0 * B + 24.0 * C)) / 6.0;
    }
    return 0.0;
}

// ============================================================================
// Gamma Utilities (sRGB)
// ============================================================================

float3 SRGBToLinear(float3 c) {
    float3 lo = c / 12.92;
    float3 hi = pow(max((c + 0.055) / 1.055, 0.0), 2.4);
    return lerp(lo, hi, step(0.04045, c));
}

float3 LinearToSRGB(float3 c) {
    float3 lo = c * 12.92;
    float3 hi = 1.055 * pow(max(c, 1e-6), 1.0 / 2.4) - 0.055;
    return lerp(lo, hi, step(0.0031308, c));
}

// ============================================================================
// Main Compute Shader — 4x4 Bicubic Interpolation
// ============================================================================

[numthreads(16, 16, 1)]
void CSMain(uint3 dtid : SV_DispatchThreadID) {
    if (dtid.x >= g_DstWidth || dtid.y >= g_DstHeight)
        return;

    // Map destination pixel to source coordinates
    float scaleX = (float)g_SrcWidth  / (float)g_DstWidth;
    float scaleY = (float)g_SrcHeight / (float)g_DstHeight;

    // Center of the destination pixel in source space
    float srcX = ((float)dtid.x + 0.5) * scaleX - 0.5;
    float srcY = ((float)dtid.y + 0.5) * scaleY - 0.5;

    // Integer part (top-left of 4x4 patch)
    int ix = (int)floor(srcX);
    int iy = (int)floor(srcY);

    // Fractional part
    float fx = srcX - (float)ix;
    float fy = srcY - (float)iy;

    // Precompute 1D kernel weights
    float wx[4], wy[4];
    [unroll]
    for (int k = 0; k < 4; k++) {
        wx[k] = MitchellNetravali((float)(k - 1) - fx, g_B, g_C);
        wy[k] = MitchellNetravali((float)(k - 1) - fy, g_B, g_C);
    }

    // Normalize weights (ensure partition of unity)
    float sumX = wx[0] + wx[1] + wx[2] + wx[3];
    float sumY = wy[0] + wy[1] + wy[2] + wy[3];
    [unroll] for (int n = 0; n < 4; n++) { wx[n] /= sumX; wy[n] /= sumY; }

    // Accumulate 4x4 samples
    float4 result = float4(0.0, 0.0, 0.0, 0.0);

    [unroll]
    for (int j = 0; j < 4; j++) {
        [unroll]
        for (int i = 0; i < 4; i++) {
            // Clamp to image bounds
            int sx = clamp(ix + i - 1, 0, (int)g_SrcWidth  - 1);
            int sy = clamp(iy + j - 1, 0, (int)g_SrcHeight - 1);

            float4 sample = g_Input[int2(sx, sy)];

            // Optional: work in linear space for gamma-correct filtering
            if (g_LinearSample) {
                sample.rgb = SRGBToLinear(sample.rgb);
            }

            // Premultiplied alpha filtering
            float weight = wx[i] * wy[j];
            result.rgb += sample.rgb * sample.a * weight;
            result.a   += sample.a * weight;
        }
    }

    // Un-premultiply alpha
    if (result.a > 1e-6) {
        result.rgb /= result.a;
    }

    // Clamp result
    result = saturate(result);

    // Convert back to sRGB if we filtered in linear
    if (g_LinearSample) {
        result.rgb = LinearToSRGB(result.rgb);
    }

    g_Output[dtid.xy] = result;
}
