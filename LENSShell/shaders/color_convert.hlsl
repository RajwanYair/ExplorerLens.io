// color_convert.hlsl — Wide Gamut Color Space Conversion Compute Shader
// ExplorerLens Engine v15.0.0 "Zenith" — Sprint 360
// Copyright (c) 2026 ExplorerLens Project
//
// Converts images from wide-gamut color spaces (Display P3, Rec. 2020, Adobe RGB)
// to sRGB for thumbnail display on standard monitors.
//
// Dispatch: ceil(width/16) x ceil(height/16) x 1

// ============================================================================
// Constant Buffer
// ============================================================================

cbuffer ColorConvertParams : register(b0) {
    uint   g_Width;          // Image width
    uint   g_Height;         // Image height
    uint   g_SourceSpace;    // 0=sRGB(noop), 1=DisplayP3, 2=Rec2020, 3=AdobeRGB, 4=ProPhotoRGB
    uint   g_RenderIntent;   // 0=Perceptual, 1=Relative, 2=Absolute, 3=Saturation
    float  g_Adaptation;     // Chromatic adaptation strength (0.0-1.0, default: 1.0)
    uint   g_GamutClamp;     // 0=clip, 1=compress (soft gamut mapping)
    float  g_Padding0;
    float  g_Padding1;
};

// ============================================================================
// Resources
// ============================================================================

Texture2D<float4>   g_Input  : register(t0);
RWTexture2D<float4> g_Output : register(u0);

// ============================================================================
// Color Space Conversion Matrices (source linear → sRGB linear)
// Pre-computed: M_sRGB_from_XYZ * M_XYZ_from_Source
// ============================================================================

// Display P3 → sRGB (D65 white point, no chromatic adaptation needed)
static const float3x3 MAT_P3_TO_SRGB = float3x3(
     1.2249401, -0.0420569, -0.0196376,
    -0.2249402,  1.0420571, -0.0786361,
     0.0000001, -0.0000001,  1.0982735
);

// Rec. 2020 → sRGB (D65 white point)
static const float3x3 MAT_REC2020_TO_SRGB = float3x3(
     1.6604910, -0.1245505, -0.0181508,
    -0.5876411,  1.1328999, -0.1005789,
    -0.0728499, -0.0083494,  1.1187297
);

// Adobe RGB (1998) → sRGB (D65 white point)
static const float3x3 MAT_ADOBERGB_TO_SRGB = float3x3(
     1.3982832, -0.0622680,  0.0163553,
    -0.3982831,  1.0622681, -0.0363553,
    -0.0000001, -0.0000001,  1.0200000
);

// ProPhoto RGB → sRGB (requires D50→D65 Bradford adaptation)
static const float3x3 MAT_PROPHOTO_TO_SRGB = float3x3(
     2.0340752, -0.2287242, -0.0085574,
    -0.7279681,  1.2317562, -0.1485164,
    -0.3061071, -0.0030320,  1.1570738
);

// ============================================================================
// Transfer Functions
// ============================================================================

// sRGB EOTF (encoded → linear)
float3 sRGBToLinear(float3 c) {
    float3 lo = c / 12.92;
    float3 hi = pow(max((c + 0.055) / 1.055, 0.0), 2.4);
    return lerp(lo, hi, step(0.04045, c));
}

// sRGB OETF (linear → encoded)
float3 LinearToSRGB(float3 c) {
    float3 lo = c * 12.92;
    float3 hi = 1.055 * pow(max(c, 1e-6), 1.0 / 2.4) - 0.055;
    return lerp(lo, hi, step(0.0031308, c));
}

// Display P3 uses sRGB transfer function (same as above)

// Rec. 2020 EOTF (10/12-bit BT.1886)
float3 Rec2020ToLinear(float3 c) {
    float alpha = 1.09929682680944;
    float beta  = 0.018053968510807;
    float3 lo = c / 4.5;
    float3 hi = pow(max((c + alpha - 1.0) / alpha, 0.0), 1.0 / 0.45);
    return lerp(lo, hi, step(beta * 4.5, c));
}

// Adobe RGB gamma 2.2
float3 AdobeRGBToLinear(float3 c) {
    return pow(max(c, 0.0), 2.19921875);  // Exact Adobe RGB gamma
}

// ProPhoto RGB gamma 1.8
float3 ProPhotoToLinear(float3 c) {
    float3 lo = c / 16.0;
    float3 hi = pow(max(c, 0.0), 1.8);
    return lerp(lo, hi, step(0.001953125, c));
}

// ============================================================================
// Gamut Mapping
// ============================================================================

// Soft gamut compression — maps out-of-gamut colors back in without hard clipping
float3 GamutCompress(float3 rgb) {
    float lum = dot(rgb, float3(0.2126, 0.7152, 0.0722));
    float3 diff = rgb - lum;
    
    // Knee compression for negative and >1 excursions
    float threshold = 0.2;
    float limit = 1.2;
    
    float3 compressed;
    [unroll]
    for (int i = 0; i < 3; i++) {
        float val = rgb[i];
        if (val < 0.0) {
            // Compress negatives toward 0
            float t = -val / limit;
            compressed[i] = -limit * t / (1.0 + t);
            compressed[i] = max(compressed[i], 0.0);
        } else if (val > 1.0) {
            // Compress > 1.0 toward 1.0
            float excess = val - 1.0;
            float t = excess / (limit - 1.0);
            compressed[i] = 1.0 + (limit - 1.0) * t / (1.0 + t);
            compressed[i] = min(compressed[i], 1.0);
        } else {
            compressed[i] = val;
        }
    }
    
    return compressed;
}

// ============================================================================
// Main Compute Shader
// ============================================================================

[numthreads(16, 16, 1)]
void CSMain(uint3 dtid : SV_DispatchThreadID) {
    if (dtid.x >= g_Width || dtid.y >= g_Height)
        return;

    float4 pixel = g_Input[dtid.xy];
    float3 rgb = pixel.rgb;

    // Early out for sRGB (no conversion needed)
    if (g_SourceSpace == 0) {
        g_Output[dtid.xy] = pixel;
        return;
    }

    // Step 1: Decode source transfer function → linear
    float3 linear_rgb;
    switch (g_SourceSpace) {
        case 1:  linear_rgb = sRGBToLinear(rgb);        break;  // Display P3 uses sRGB TF
        case 2:  linear_rgb = Rec2020ToLinear(rgb);      break;
        case 3:  linear_rgb = AdobeRGBToLinear(rgb);     break;
        case 4:  linear_rgb = ProPhotoToLinear(rgb);     break;
        default: linear_rgb = sRGBToLinear(rgb);         break;
    }

    // Step 2: Apply color space conversion matrix (source linear → sRGB linear)
    float3 srgb_linear;
    switch (g_SourceSpace) {
        case 1:  srgb_linear = mul(MAT_P3_TO_SRGB, linear_rgb);        break;
        case 2:  srgb_linear = mul(MAT_REC2020_TO_SRGB, linear_rgb);   break;
        case 3:  srgb_linear = mul(MAT_ADOBERGB_TO_SRGB, linear_rgb);  break;
        case 4:  srgb_linear = mul(MAT_PROPHOTO_TO_SRGB, linear_rgb);  break;
        default: srgb_linear = linear_rgb;                               break;
    }

    // Step 3: Gamut mapping
    if (g_GamutClamp == 1) {
        srgb_linear = GamutCompress(srgb_linear);
    } else {
        srgb_linear = saturate(srgb_linear);
    }

    // Step 4: Encode sRGB transfer function (linear → encoded)
    float3 output_rgb = LinearToSRGB(srgb_linear);

    g_Output[dtid.xy] = float4(output_rgb, pixel.a);
}
