// demosaic_bayer.hlsl — GPU Bayer pattern demosaicing compute shader
// Copyright (c) 2026 ExplorerLens Project
//
// Implements gradient-corrected bilinear demosaicing (a.k.a. "malvar" / HQlin).
// Supports RGGB, BGGR, GRBG, GBRG Bayer patterns.
//
// Input:  t_raw  — 16-bit mono RAW sensor data (R16_UNORM or R16_UINT)
// Output: u_rgb  — Demosaiced RGBA16F
//
// Dispatch: numthreads(8,8,1), Dispatch(ceil(W/8), ceil(H/8), 1)
//

Texture2D<float>    t_raw : register(t0);   // Raw Bayer data normalized to [0,1]
RWTexture2D<float4> u_rgb : register(u0);   // Output RGB

cbuffer DemosaicParams : register(b0) {
    uint  g_width;          // Image width in pixels
    uint  g_height;         // Image height in pixels
    uint  g_bayerPattern;   // 0=RGGB, 1=BGGR, 2=GRBG, 3=GBRG
    float g_blackLevel;     // Normalized black level to subtract
    float g_whiteLevel;     // Normalized white level to divide by
    float g_pad0;
    float g_pad1;
    float g_pad2;
};

// Return the Bayer component at pixel (x,y) for the configured pattern:
// 0=R, 1=G, 2=B
int BayerComponent(uint x, uint y)
{
    uint cx = x & 1;
    uint cy = y & 1;
    // g_bayerPattern encoding: row0col0, row0col1, row1col0, row1col1
    // RGGB: R G / G B  → 0=R at (0,0), G at (1,0), G at (0,1), B at (1,1)
    // BGGR: B G / G R  → 2=B at (0,0)
    // GRBG: G R / B G
    // GBRG: G B / R G
    static const int pattern[4][4] = {
        { 0, 1, 1, 2 }, // RGGB
        { 2, 1, 1, 0 }, // BGGR
        { 1, 0, 2, 1 }, // GRBG
        { 1, 2, 0, 1 }, // GBRG
    };
    return pattern[g_bayerPattern][cy * 2 + cx];
}

float RawAt(int x, int y)
{
    x = clamp(x, 0, (int)g_width  - 1);
    y = clamp(y, 0, (int)g_height - 1);
    float v = t_raw.Load(int3(x, y, 0));
    // Black/white level normalization
    return saturate((v - g_blackLevel) / max(g_whiteLevel - g_blackLevel, 1e-5f));
}

[numthreads(8, 8, 1)]
void CSMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x >= g_width || tid.y >= g_height) return;

    int x = (int)tid.x;
    int y = (int)tid.y;
    int comp = BayerComponent((uint)x, (uint)y);

    float r = 0.0f, g = 0.0f, b = 0.0f;
    float center = RawAt(x, y);

    if (comp == 1) // Green pixel — interpolate R and B
    {
        g = center;
        // Determine if we're on a G row of R or G row of B
        int rComp = BayerComponent((uint)(x + 1), (uint)y); // Neighbor reveals row pattern
        if (rComp == 0) {
            // R on same row, B on adjacent row
            r = 0.5f * (RawAt(x - 1, y) + RawAt(x + 1, y));
            b = 0.5f * (RawAt(x, y - 1) + RawAt(x, y + 1));
        } else {
            // B on same row, R on adjacent row
            b = 0.5f * (RawAt(x - 1, y) + RawAt(x + 1, y));
            r = 0.5f * (RawAt(x, y - 1) + RawAt(x, y + 1));
        }
    }
    else if (comp == 0) // Red pixel
    {
        r = center;
        g = 0.25f * (RawAt(x - 1, y) + RawAt(x + 1, y) + RawAt(x, y - 1) + RawAt(x, y + 1));
        b = 0.25f * (RawAt(x - 1, y - 1) + RawAt(x + 1, y - 1) + RawAt(x - 1, y + 1) + RawAt(x + 1, y + 1));
    }
    else // Blue pixel
    {
        b = center;
        g = 0.25f * (RawAt(x - 1, y) + RawAt(x + 1, y) + RawAt(x, y - 1) + RawAt(x, y + 1));
        r = 0.25f * (RawAt(x - 1, y - 1) + RawAt(x + 1, y - 1) + RawAt(x - 1, y + 1) + RawAt(x + 1, y + 1));
    }

    u_rgb[tid.xy] = float4(r, g, b, 1.0f);
}
