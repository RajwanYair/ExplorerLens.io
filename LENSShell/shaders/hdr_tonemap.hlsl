// hdr_tonemap.hlsl — HDR to SDR Tone Mapping Compute Shader
// ExplorerLens Engine v15.0.0 "Zenith" — Sprint 360
// Copyright (c) 2026 ExplorerLens Project
//
// Converts HDR (EXR/HDR/AVIF) pixel data to SDR for thumbnail display.
// Supports multiple tone mapping operators selectable via constant buffer.
//
// Dispatch: ceil(width/16) x ceil(height/16) x 1

// ============================================================================
// Constant Buffer
// ============================================================================

cbuffer TonemapParams : register(b0) {
    uint   g_Width;         // Image width in pixels
    uint   g_Height;        // Image height in pixels
    uint   g_Operator;      // 0=Reinhard, 1=ACES, 2=Uncharted2, 3=AgX
    float  g_Exposure;      // Exposure adjustment (default: 1.0)
    float  g_Gamma;         // Output gamma (default: 2.2 for sRGB)
    float  g_WhitePoint;    // White point for Reinhard extended (default: 4.0)
    float  g_Saturation;    // Post-tonemap saturation (default: 1.0)
    uint   g_Padding;
};

// ============================================================================
// Resources
// ============================================================================

Texture2D<float4>   g_Input  : register(t0);
RWTexture2D<float4> g_Output : register(u0);

// ============================================================================
// Tone Mapping Operators
// ============================================================================

// Reinhard with extended luminance mapping
float3 TonemapReinhard(float3 color, float whitePoint) {
    float lum = dot(color, float3(0.2126, 0.7152, 0.0722));
    float wp2 = whitePoint * whitePoint;
    float mapped = (lum * (1.0 + lum / wp2)) / (1.0 + lum);
    return color * (mapped / max(lum, 1e-6));
}

// ACES Filmic (approximation by Stephen Hill)
float3 TonemapACES(float3 x) {
    // ACES input transform
    float3x3 inputMat = float3x3(
        0.59719, 0.07600, 0.02840,
        0.35458, 0.90834, 0.13383,
        0.04823, 0.01566, 0.83777
    );
    // ACES output transform
    float3x3 outputMat = float3x3(
         1.60475, -0.10208, -0.00327,
        -0.53108,  1.10813, -0.07276,
        -0.07367, -0.00605,  1.07602
    );
    
    float3 v = mul(inputMat, x);
    float3 a = v * (v + 0.0245786) - 0.000090537;
    float3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    v = a / b;
    return mul(outputMat, v);
}

// Uncharted 2 filmic curve (John Hable)
float3 Uncharted2Tonemap(float3 x) {
    float A = 0.15;  // Shoulder strength
    float B = 0.50;  // Linear strength
    float C = 0.10;  // Linear angle
    float D = 0.20;  // Toe strength
    float E = 0.02;  // Toe numerator
    float F = 0.30;  // Toe denominator
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 TonemapUncharted2(float3 color, float whitePoint) {
    float3 curr = Uncharted2Tonemap(color * 2.0);
    float3 whiteScale = 1.0 / Uncharted2Tonemap(float3(whitePoint, whitePoint, whitePoint));
    return curr * whiteScale;
}

// AgX base contrast (inspired by Blender's AgX)
float3 TonemapAgX(float3 color) {
    // AgX log encoding
    float3 logColor = max(color, 1e-10);
    logColor = log2(logColor);
    logColor = (logColor - (-12.47393)) / (4.026069 - (-12.47393));
    logColor = saturate(logColor);
    
    // Polynomial approximation of AgX curve
    float3 x = logColor;
    float3 x2 = x * x;
    float3 x4 = x2 * x2;
    return 15.5 * x4 * x2 - 40.14 * x4 * x + 31.96 * x4 - 6.868 * x2 * x + 0.4298 * x2 + 0.1191 * x - 0.00232;
}

// ============================================================================
// Color Space Utilities
// ============================================================================

// Linear to sRGB gamma
float3 LinearToSRGB(float3 lin) {
    float3 lo = lin * 12.92;
    float3 hi = 1.055 * pow(max(lin, 1e-6), 1.0 / 2.4) - 0.055;
    return lerp(lo, hi, step(0.0031308, lin));
}

// Saturation adjustment (post-tonemap)
float3 AdjustSaturation(float3 color, float saturation) {
    float lum = dot(color, float3(0.2126, 0.7152, 0.0722));
    return lerp(float3(lum, lum, lum), color, saturation);
}

// ============================================================================
// Main Compute Shader
// ============================================================================

[numthreads(16, 16, 1)]
void CSMain(uint3 dtid : SV_DispatchThreadID) {
    if (dtid.x >= g_Width || dtid.y >= g_Height)
        return;

    float4 hdrColor = g_Input[dtid.xy];
    float3 rgb = hdrColor.rgb;

    // Apply exposure
    rgb *= g_Exposure;

    // Apply selected tone mapping operator
    float3 mapped;
    switch (g_Operator) {
        case 0:  mapped = TonemapReinhard(rgb, g_WhitePoint);       break;
        case 1:  mapped = TonemapACES(rgb);                         break;
        case 2:  mapped = TonemapUncharted2(rgb, g_WhitePoint);     break;
        case 3:  mapped = TonemapAgX(rgb);                          break;
        default: mapped = TonemapACES(rgb);                         break;
    }

    // Clamp to [0,1]
    mapped = saturate(mapped);

    // Saturation adjustment
    mapped = AdjustSaturation(mapped, g_Saturation);

    // Convert to sRGB gamma
    mapped = LinearToSRGB(mapped);

    g_Output[dtid.xy] = float4(mapped, hdrColor.a);
}
