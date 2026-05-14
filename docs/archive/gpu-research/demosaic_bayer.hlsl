// demosaic_bayer.hlsl — Bayer Pattern Demosaic Compute Shader
// Copyright (c) 2026 ExplorerLens Project
//
// CS_5_0 compute shader: bilinear Bayer CFA demosaic for RAW thumbnail preview.
//
// Input:  R8_UINT or R16_UINT single-channel Bayer mosaic texture
//         Values normalised to [0,1] float before input (use SRV with float format)
// Output: BGRA float4 [0,1] decoded colour
//
// CFA pattern selected via g_PatternType:
//   0 = RGGB (most common)
//   1 = BGGR
//   2 = GRBG
//   3 = GBRG
//
// Algorithm: bilinear interpolation (MHC variant, sufficient for thumbnails)
//

Texture2D<float>     g_Input  : register(t0);
RWTexture2D<float4>  g_Output : register(u0);

cbuffer DemosaicConstants : register(b0)
{
    uint  g_Width;
    uint  g_Height;
    uint  g_PatternType;  // 0=RGGB, 1=BGGR, 2=GRBG, 3=GBRG
    float g_BlackLevel;   // normalised black-level subtract (typically 0.0625)
    float g_WhiteLevel;   // normalised white-level scale  (typically 1.0)
    float g_Pad0;
    float g_Pad1;
    float g_Pad2;
};

// Read a clamped texel from the single-channel Bayer texture
float Fetch(int px, int py)
{
    px = clamp(px, 0, int(g_Width)  - 1);
    py = clamp(py, 0, int(g_Height) - 1);
    return g_Input.Load(int3(px, py, 0));
}

// Determine Bayer channel at (px, py) for the given pattern.
// Returns: 0=R, 1=Gr, 2=Gb, 3=B
uint BayerChannel(uint px, uint py)
{
    uint p = (py & 1u) * 2u + (px & 1u);
    //              RGGB   BGGR   GRBG   GBRG
    // p=0 (TL)  :  R      B      Gr     Gb
    // p=1 (TR)  :  Gr     Gb     R      B
    // p=2 (BL)  :  Gb     Gr     B      R
    // p=3 (BR)  :  B      R      Gb     Gr

    static const uint LUT[4][4] = {
        { 0, 1, 2, 3 },  // RGGB
        { 3, 2, 1, 0 },  // BGGR
        { 1, 0, 3, 2 },  // GRBG
        { 2, 3, 0, 1 },  // GBRG
    };

    return LUT[g_PatternType & 3u][p];
}

[numthreads(8, 8, 1)]
void main(uint3 dispatch : SV_DispatchThreadID)
{
    uint x = dispatch.x;
    uint y = dispatch.y;

    if (x >= g_Width || y >= g_Height)
        return;

    uint ch = BayerChannel(x, y);
    int  ix = int(x);
    int  iy = int(y);

    float R, G, B;

    if (ch == 0u)  // R pixel
    {
        R = Fetch(ix, iy);
        G = (Fetch(ix - 1, iy) + Fetch(ix + 1, iy) + Fetch(ix, iy - 1) + Fetch(ix, iy + 1)) * 0.25f;
        B = (Fetch(ix - 1, iy - 1) + Fetch(ix + 1, iy - 1) + Fetch(ix - 1, iy + 1) + Fetch(ix + 1, iy + 1)) * 0.25f;
    }
    else if (ch == 3u)  // B pixel
    {
        B = Fetch(ix, iy);
        G = (Fetch(ix - 1, iy) + Fetch(ix + 1, iy) + Fetch(ix, iy - 1) + Fetch(ix, iy + 1)) * 0.25f;
        R = (Fetch(ix - 1, iy - 1) + Fetch(ix + 1, iy - 1) + Fetch(ix - 1, iy + 1) + Fetch(ix + 1, iy + 1)) * 0.25f;
    }
    else if (ch == 1u)  // Gr pixel (green in R row)
    {
        G = Fetch(ix, iy);
        R = (Fetch(ix - 1, iy) + Fetch(ix + 1, iy)) * 0.5f;
        B = (Fetch(ix, iy - 1) + Fetch(ix, iy + 1)) * 0.5f;
    }
    else  // Gb pixel (green in B row)
    {
        G = Fetch(ix, iy);
        B = (Fetch(ix - 1, iy) + Fetch(ix + 1, iy)) * 0.5f;
        R = (Fetch(ix, iy - 1) + Fetch(ix, iy + 1)) * 0.5f;
    }

    // Apply black/white level correction
    float scale = 1.0f / max(g_WhiteLevel - g_BlackLevel, 1e-6f);
    R = saturate((R - g_BlackLevel) * scale);
    G = saturate((G - g_BlackLevel) * scale);
    B = saturate((B - g_BlackLevel) * scale);

    // Write BGRA (DirectX native channel order is BGRA for BGRA8 UAV targets)
    g_Output[uint2(x, y)] = float4(B, G, R, 1.0f);
}
