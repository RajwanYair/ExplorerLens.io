// colorspace_yuv_to_rgb.hlsl — YUV / YCbCr → RGB GPU conversion shaders
// Copyright (c) 2026 ExplorerLens Project
//
// Covers BT.601 (SDTV), BT.709 (HDTV), BT.2020 (UHD) matrix variants.
// Used by video/HEIF/AVIF decoders that deliver YCbCr planes.
//
// Dispatch: numthreads(8,8,1), Dispatch(ceil(W/8), ceil(H/8), 1)
//

Texture2D<float>    t_y  : register(t0);   // Luma plane (R8_UNORM or R16_UNORM)
Texture2D<float2>   t_uv : register(t1);   // Interleaved CbCr plane (R8G8_UNORM)
RWTexture2D<float4> u_rgb : register(u0);

cbuffer ColorspaceParams : register(b0) {
    uint  g_width;
    uint  g_height;
    uint  g_matrix;     // 0=BT.601, 1=BT.709, 2=BT.2020
    uint  g_range;      // 0=limited (16-235), 1=full (0-255)
    float g_pad0;
    float g_pad1;
    float g_pad2;
    float g_pad3;
};

float3 YuvToRgb_BT601(float y, float u, float v) {
    return float3(
        y + 1.13983f * v,
        y - 0.39465f * u - 0.58060f * v,
        y + 2.03211f * u
    );
}

float3 YuvToRgb_BT709(float y, float u, float v) {
    return float3(
        y + 1.28033f * v,
        y - 0.21482f * u - 0.38059f * v,
        y + 2.12798f * u
    );
}

float3 YuvToRgb_BT2020(float y, float u, float v) {
    return float3(
        y + 1.47460f * v,
        y - 0.16455f * u - 0.57135f * v,
        y + 1.88140f * u
    );
}

[numthreads(8, 8, 1)]
void CSMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x >= g_width || tid.y >= g_height) return;

    float yVal = t_y.Load(int3(tid.xy, 0));
    // UV plane is 4:2:0 subsampled — sample the UV for this 2x2 block
    float2 uvVal = t_uv.Load(int3(tid.x / 2, tid.y / 2, 0));

    float u, v;

    if (g_range == 0) {
        // Limited range: Y in [16/255, 235/255], UV in [16/255, 240/255]
        yVal = (yVal - 16.0f / 255.0f) * (255.0f / 219.0f);
        u    = (uvVal.x - 128.0f / 255.0f) * (255.0f / 224.0f);
        v    = (uvVal.y - 128.0f / 255.0f) * (255.0f / 224.0f);
    } else {
        // Full range
        u = uvVal.x - 0.5f;
        v = uvVal.y - 0.5f;
    }

    float3 rgb;
    if      (g_matrix == 0) rgb = YuvToRgb_BT601  (yVal, u, v);
    else if (g_matrix == 1) rgb = YuvToRgb_BT709  (yVal, u, v);
    else                    rgb = YuvToRgb_BT2020  (yVal, u, v);

    u_rgb[tid.xy] = float4(saturate(rgb), 1.0f);
}
