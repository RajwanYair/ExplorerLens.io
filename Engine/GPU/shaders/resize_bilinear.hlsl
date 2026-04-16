// resize_bilinear.hlsl — GPU bilinear image resize compute shader
// Copyright (c) 2026 ExplorerLens Project
//
// Dispatch: numthreads(8,8,1), Dispatch(ceil(dstW/8), ceil(dstH/8), 1)
// Input:  t_src  — source texture (any format, SRV)
// Output: u_dst  — destination texture (UAV, RGBA8_UNORM or RGBA16_FLOAT)
//

Texture2D<float4>   t_src : register(t0);
RWTexture2D<float4> u_dst : register(u0);

// Push constants (root constants at b0)
cbuffer ResizeParams : register(b0) {
    uint  g_srcW;   // source width  (pixels)
    uint  g_srcH;   // source height (pixels)
    uint  g_dstW;   // destination width
    uint  g_dstH;   // destination height
    float g_scaleX; // srcW / dstW
    float g_scaleY; // srcH / dstH
    uint  g_pad0;
    uint  g_pad1;
};

// Bilinear sample from source (flipped Y for Windows conventions)
float4 SampleBilinear(float2 srcPos)
{
    float x  = srcPos.x - 0.5f;
    float y  = srcPos.y - 0.5f;
    int   x0 = (int)x;
    int   y0 = (int)y;
    int   x1 = x0 + 1;
    int   y1 = y0 + 1;

    // Clamp to [0, dim-1]
    x0 = clamp(x0, 0, (int)g_srcW - 1);
    y0 = clamp(y0, 0, (int)g_srcH - 1);
    x1 = clamp(x1, 0, (int)g_srcW - 1);
    y1 = clamp(y1, 0, (int)g_srcH - 1);

    float fx = x - (float)x0;
    float fy = y - (float)y0;

    float4 c00 = t_src.Load(int3(x0, y0, 0));
    float4 c10 = t_src.Load(int3(x1, y0, 0));
    float4 c01 = t_src.Load(int3(x0, y1, 0));
    float4 c11 = t_src.Load(int3(x1, y1, 0));

    return lerp(lerp(c00, c10, fx), lerp(c01, c11, fx), fy);
}

[numthreads(8, 8, 1)]
void CSMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x >= g_dstW || tid.y >= g_dstH) return;

    // Map destination pixel to source pixel center
    float srcX = ((float)tid.x + 0.5f) * g_scaleX;
    float srcY = ((float)tid.y + 0.5f) * g_scaleY;

    u_dst[tid.xy] = SampleBilinear(float2(srcX, srcY));
}
