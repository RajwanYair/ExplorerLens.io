// resize_lanczos.hlsl — Lanczos-3 Resize Compute Shader
// Copyright (c) 2026 ExplorerLens Project
//
// CS_5_0 separable Lanczos-3 resampling kernel for high-quality thumbnail generation.
// Two-pass: horizontal then vertical (call dispatch twice with different g_Pass values).
//
// Pass 0: horizontal  — reads g_Input,  writes g_Intermediate
// Pass 1: vertical    — reads g_Intermed, writes g_Output
//
// Dispatch: ceil(outW/8) x ceil(outH/8) x 1 (both passes)
//

Texture2D<float4>    g_Input      : register(t0);
Texture2D<float4>    g_Intermed   : register(t1);
RWTexture2D<float4>  g_Intermediate : register(u0);
RWTexture2D<float4>  g_Output     : register(u1);

cbuffer LanczosConstants : register(b0)
{
    uint  g_SrcWidth;
    uint  g_SrcHeight;
    uint  g_DstWidth;
    uint  g_DstHeight;
    uint  g_Pass;       // 0 = horizontal, 1 = vertical
    float g_InvSrcW;    // 1.0 / SrcWidth  (texel size)
    float g_InvSrcH;    // 1.0 / SrcHeight
    float g_Pad;
};

static const float PI = 3.14159265358979f;
static const int   RADIUS = 3;

float Sinc(float x)
{
    if (abs(x) < 1e-6f) return 1.0f;
    float px = PI * x;
    return sin(px) / px;
}

float Lanczos3(float x)
{
    float ax = abs(x);
    if (ax >= float(RADIUS)) return 0.0f;
    return Sinc(ax) * Sinc(ax / float(RADIUS));
}

[numthreads(8, 8, 1)]
void main(uint3 dispatch : SV_DispatchThreadID)
{
    uint dstX = dispatch.x;
    uint dstY = dispatch.y;

    if (dstX >= g_DstWidth || dstY >= g_DstHeight)
        return;

    if (g_Pass == 0u)
    {
        // Horizontal pass: map dstX → src X, dstY unchanged
        float scale  = float(g_SrcWidth) / float(g_DstWidth);
        float srcX   = (float(dstX) + 0.5f) * scale - 0.5f;

        float4 acc    = float4(0, 0, 0, 0);
        float  wSum   = 0.0f;
        int    start  = int(floor(srcX)) - RADIUS + 1;

        [unroll(12)]
        for (int i = 0; i < 2 * RADIUS; ++i)
        {
            int sx = start + i;
            sx = clamp(sx, 0, int(g_SrcWidth) - 1);
            float w = Lanczos3(float(start + i) - srcX);
            acc  += g_Input.Load(int3(sx, int(dstY), 0)) * w;
            wSum += w;
        }

        g_Intermediate[uint2(dstX, dstY)] = (wSum > 1e-6f) ? (acc / wSum) : float4(0, 0, 0, 1);
    }
    else
    {
        // Vertical pass: map dstY → src Y, dstX unchanged
        float scale  = float(g_SrcHeight) / float(g_DstHeight);
        float srcY   = (float(dstY) + 0.5f) * scale - 0.5f;

        float4 acc    = float4(0, 0, 0, 0);
        float  wSum   = 0.0f;
        int    start  = int(floor(srcY)) - RADIUS + 1;

        [unroll(12)]
        for (int i = 0; i < 2 * RADIUS; ++i)
        {
            int sy = start + i;
            sy = clamp(sy, 0, int(g_DstHeight) - 1);
            float w = Lanczos3(float(start + i) - srcY);
            acc  += g_Intermed.Load(int3(int(dstX), sy, 0)) * w;
            wSum += w;
        }

        g_Output[uint2(dstX, dstY)] = (wSum > 1e-6f) ? (acc / wSum) : float4(0, 0, 0, 1);
    }
}
