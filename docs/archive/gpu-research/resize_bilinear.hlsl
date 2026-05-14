// resize_bilinear.hlsl — Bilinear Resize Compute Shader
// Copyright (c) 2026 ExplorerLens Project
//
// CS_5_0 compute shader: bilinear downscale/upscale for thumbnail generation.
// Dispatch: ceil(outW/8) x ceil(outH/8) x 1
//
// Register layout:
//   t0 — SRV  Texture2D<float4>  input  (BGRA / RGBA linear)
//   u0 — UAV  RWTexture2D<float4> output
//   b0 — CBV  ResizeConstants
//

Texture2D<float4>    g_Input  : register(t0);
RWTexture2D<float4>  g_Output : register(u0);

cbuffer ResizeConstants : register(b0)
{
    uint  g_SrcWidth;   // source texture width
    uint  g_SrcHeight;  // source texture height
    uint  g_DstWidth;   // output texture width
    uint  g_DstHeight;  // output texture height
    float g_Pad0;       // reserved
    float g_Pad1;       // reserved
    float g_Pad2;       // reserved
    float g_Pad3;       // reserved
};

SamplerState g_LinearClamp : register(s0);

[numthreads(8, 8, 1)]
void main(uint3 dispatch : SV_DispatchThreadID)
{
    uint dstX = dispatch.x;
    uint dstY = dispatch.y;

    if (dstX >= g_DstWidth || dstY >= g_DstHeight)
        return;

    // Map destination pixel centre to source UV [0, 1]
    float u = (float(dstX) + 0.5f) / float(g_DstWidth);
    float v = (float(dstY) + 0.5f) / float(g_DstHeight);

    g_Output[uint2(dstX, dstY)] = g_Input.SampleLevel(g_LinearClamp, float2(u, v), 0.0f);
}
