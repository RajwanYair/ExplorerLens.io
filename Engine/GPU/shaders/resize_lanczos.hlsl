// resize_lanczos.hlsl — GPU Lanczos-3 image resize compute shader
// Copyright (c) 2026 ExplorerLens Project
//
// Lanczos (a=3) provides much sharper results than bilinear, at ~3x cost.
// Used in quality mode (HIGH/BEST) when GPU is available.
//
// Dispatch: numthreads(8,8,1), Dispatch(ceil(dstW/8), ceil(dstH/8), 1)
//

Texture2D<float4>   t_src : register(t0);
RWTexture2D<float4> u_dst : register(u0);

cbuffer ResizeParams : register(b0) {
    uint  g_srcW;
    uint  g_srcH;
    uint  g_dstW;
    uint  g_dstH;
    float g_scaleX;
    float g_scaleY;
    uint  g_pad0;
    uint  g_pad1;
};

static const float PI = 3.14159265358979323846f;
static const int   LANCZOS_A = 3; // Lanczos window radius

float Lanczos(float x)
{
    if (abs(x) < 1e-6f) return 1.0f;
    if (abs(x) >= (float)LANCZOS_A) return 0.0f;
    float px = PI * x;
    return (float)LANCZOS_A * sin(px) * sin(px / (float)LANCZOS_A) / (px * px);
}

float4 SampleLanczos(float2 srcPos)
{
    int xBase = (int)floor(srcPos.x);
    int yBase = (int)floor(srcPos.y);

    float4 colorSum  = float4(0, 0, 0, 0);
    float  weightSum = 0.0f;

    for (int dy = -LANCZOS_A + 1; dy <= LANCZOS_A; ++dy)
    {
        float wy = Lanczos(srcPos.y - (float)(yBase + dy));
        for (int dx = -LANCZOS_A + 1; dx <= LANCZOS_A; ++dx)
        {
            float wx = Lanczos(srcPos.x - (float)(xBase + dx));
            float w  = wx * wy;

            int sx = clamp(xBase + dx, 0, (int)g_srcW - 1);
            int sy = clamp(yBase + dy, 0, (int)g_srcH - 1);

            colorSum  += t_src.Load(int3(sx, sy, 0)) * w;
            weightSum += w;
        }
    }

    if (weightSum > 1e-6f) colorSum /= weightSum;
    return saturate(colorSum);
}

[numthreads(8, 8, 1)]
void CSMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x >= g_dstW || tid.y >= g_dstH) return;

    float srcX = ((float)tid.x + 0.5f) * g_scaleX;
    float srcY = ((float)tid.y + 0.5f) * g_scaleY;

    u_dst[tid.xy] = SampleLanczos(float2(srcX, srcY));
}
