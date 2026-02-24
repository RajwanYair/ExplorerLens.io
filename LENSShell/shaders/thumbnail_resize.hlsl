// thumbnail_resize.hlsl - GPU Compute Shader for High-Quality Thumbnail Resizing
// ExplorerLens.io v14.0.0 - DirectX 11 Compute Shader
// Copyright (c) 2026 ExplorerLens.io Project
//
// High-quality image resizing using Lanczos3 resampling
// Optimized for thumbnail generation (256x256 typical output)
// Features:
// - Lanczos3 windowed sinc filter (6x6 kernel)
// - Gamma-correct resizing
// - Premultiplied alpha handling
// - Optimized for GPU execution

// Constant buffer
cbuffer ResizeConstants : register(b0)
{
    uint sourceWidth;
    uint sourceHeight;
    uint targetWidth;
    uint targetHeight;
    float texelSizeX;       // 1.0 / sourceWidth
    float texelSizeY;       // 1.0 / sourceHeight
    float scaleX;           // targetWidth / sourceWidth
    float scaleY;           // targetHeight / sourceHeight
};

// Input/Output textures
Texture2D<float4> sourceTexture : register(t0);
RWTexture2D<float4> targetTexture : register(u0);
SamplerState samplerLinear : register(s0);

// Constants for Lanczos3 filter
static const float PI = 3.14159265359;
static const float LANCZOS_SIZE = 3.0; // Lanczos3 = 3-lobe filter
static const float EPSILON = 0.0001;

//=============================================================================
// Lanczos Filter Functions
//=============================================================================

// Sinc function: sin(x) / x
float sinc(float x)
{
    x = abs(x);
    if (x < EPSILON)
        return 1.0;
    
    float pix = PI * x;
    return sin(pix) / pix;
}

// Lanczos kernel: sinc(x) * sinc(x / a) for |x| < a
float lanczos3(float x)
{
    x = abs(x);
    
    if (x >= LANCZOS_SIZE)
        return 0.0;
    
    return sinc(x) * sinc(x / LANCZOS_SIZE);
}

//=============================================================================
// Color Space Conversion (Gamma Correction)
//=============================================================================

// sRGB to linear
float3 srgbToLinear(float3 srgb)
{
    float3 linear;
    [unroll]
    for (int i = 0; i < 3; i++)
    {
        linear[i] = (srgb[i] <= 0.04045)
            ? srgb[i] / 12.92
            : pow((srgb[i] + 0.055) / 1.055, 2.4);
    }
    return linear;
}

// Linear to sRGB
float3 linearToSrgb(float3 linear)
{
    float3 srgb;
    [unroll]
    for (int i = 0; i < 3; i++)
    {
        srgb[i] = (linear[i] <= 0.0031308)
            ? 12.92 * linear[i]
            : 1.055 * pow(linear[i], 1.0 / 2.4) - 0.055;
    }
    return srgb;
}

//=============================================================================
// Lanczos3 Resampling
//=============================================================================

[numthreads(8, 8, 1)]
void CSResizeLanczos3(uint3 DTid : SV_DispatchThreadID)
{
    // Skip if outside target bounds
    if (DTid.x >= targetWidth || DTid.y >= targetHeight)
        return;
    
    // Calculate source position (center of target pixel)
    float srcX = (DTid.x + 0.5) / scaleX;
    float srcY = (DTid.y + 0.5) / scaleY;
    
    // Lanczos3 uses a 6x6 kernel (3 lobes on each side)
    int kernelRadius = 3;
    
    float4 sum = float4(0.0, 0.0, 0.0, 0.0);
    float weightSum = 0.0;
    
    // Sample 6x6 neighborhood
    [unroll]
    for (int dy = -kernelRadius; dy < kernelRadius; dy++)
    {
        [unroll]
        for (int dx = -kernelRadius; dx < kernelRadius; dx++)
        {
            // Source pixel position
            int sx = int(srcX) + dx;
            int sy = int(srcY) + dy;
            
            // Clamp to source bounds
            sx = clamp(sx, 0, int(sourceWidth) - 1);
            sy = clamp(sy, 0, int(sourceHeight) - 1);
            
            // Calculate Lanczos weight
            float distX = srcX - (sx + 0.5);
            float distY = srcY - (sy + 0.5);
            float weight = lanczos3(distX) * lanczos3(distY);
            
            // Sample source pixel
            float4 pixel = sourceTexture.Load(int3(sx, sy, 0));
            
            // Gamma-correct conversion to linear space
            pixel.rgb = srgbToLinear(pixel.rgb);
            
            // Handle premultiplied alpha
            // (Uncomment if source has premultiplied alpha)
            // if (pixel.a > EPSILON)
            //     pixel.rgb /= pixel.a;
            
            // Accumulate weighted sample
            sum += pixel * weight;
            weightSum += weight;
        }
    }
    
    // Normalize by total weight
    if (weightSum > EPSILON)
    {
        sum /= weightSum;
    }
    
    // Restore premultiplied alpha if needed
    // (Uncomment if output needs premultiplied alpha)
    // sum.rgb *= sum.a;
    
    // Convert back to sRGB
    sum.rgb = linearToSrgb(sum.rgb);
    
    // Clamp to valid range
    sum = saturate(sum);
    
    // Write result
    targetTexture[DTid.xy] = sum;
}

//=============================================================================
// Bilinear Resampling (Faster, Lower Quality)
//=============================================================================

[numthreads(8, 8, 1)]
void CSResizeBilinear(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= targetWidth || DTid.y >= targetHeight)
        return;
    
    // Calculate normalized UV coordinates
    float u = (DTid.x + 0.5) / float(targetWidth);
    float v = (DTid.y + 0.5) / float(targetHeight);
    
    // Sample with bilinear filtering (hardware-accelerated)
    float4 color = sourceTexture.SampleLevel(samplerLinear, float2(u, v), 0);
    
    targetTexture[DTid.xy] = color;
}

//=============================================================================
// Catmull-Rom Bicubic Resampling (Medium Quality, Good Performance)
//=============================================================================

float CatmullRom(float x)
{
    x = abs(x);
    
    if (x >= 2.0)
        return 0.0;
    
    if (x <= 1.0)
        return (1.5 * x - 2.5) * x * x + 1.0;
    else
        return ((-0.5 * x + 2.5) * x - 4.0) * x + 2.0;
}

[numthreads(8, 8, 1)]
void CSResizeCatmullRom(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= targetWidth || DTid.y >= targetHeight)
        return;
    
    // Calculate source position
    float srcX = (DTid.x + 0.5) / scaleX;
    float srcY = (DTid.y + 0.5) / scaleY;
    
    // Catmull-Rom uses 4x4 kernel
    int kernelRadius = 2;
    
    float4 sum = float4(0.0, 0.0, 0.0, 0.0);
    float weightSum = 0.0;
    
    [unroll]
    for (int dy = -kernelRadius; dy < kernelRadius; dy++)
    {
        [unroll]
        for (int dx = -kernelRadius; dx < kernelRadius; dx++)
        {
            int sx = int(srcX) + dx;
            int sy = int(srcY) + dy;
            
            sx = clamp(sx, 0, int(sourceWidth) - 1);
            sy = clamp(sy, 0, int(sourceHeight) - 1);
            
            float distX = srcX - (sx + 0.5);
            float distY = srcY - (sy + 0.5);
            float weight = CatmullRom(distX) * CatmullRom(distY);
            
            float4 pixel = sourceTexture.Load(int3(sx, sy, 0));
            
            // Gamma-correct resizing
            pixel.rgb = srgbToLinear(pixel.rgb);
            
            sum += pixel * weight;
            weightSum += weight;
        }
    }
    
    if (weightSum > EPSILON)
    {
        sum /= weightSum;
    }
    
    sum.rgb = linearToSrgb(sum.rgb);
    sum = saturate(sum);
    
    targetTexture[DTid.xy] = sum;
}

//=============================================================================
// Box Filter (Downsampling Only - Very Fast)
//=============================================================================

[numthreads(8, 8, 1)]
void CSResizeBox(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= targetWidth || DTid.y >= targetHeight)
        return;
    
    // Calculate source region covered by this target pixel
    float srcLeft = DTid.x / scaleX;
    float srcTop = DTid.y / scaleY;
    float srcRight = (DTid.x + 1) / scaleX;
    float srcBottom = (DTid.y + 1) / scaleY;
    
    int left = int(floor(srcLeft));
    int top = int(floor(srcTop));
    int right = int(ceil(srcRight));
    int bottom = int(ceil(srcBottom));
    
    // Clamp to source bounds
    left = clamp(left, 0, int(sourceWidth) - 1);
    top = clamp(top, 0, int(sourceHeight) - 1);
    right = clamp(right, 0, int(sourceWidth));
    bottom = clamp(bottom, 0, int(sourceHeight));
    
    float4 sum = float4(0.0, 0.0, 0.0, 0.0);
    int count = 0;
    
    // Average all pixels in the box
    for (int y = top; y < bottom; y++)
    {
        for (int x = left; x < right; x++)
        {
            float4 pixel = sourceTexture.Load(int3(x, y, 0));
            pixel.rgb = srgbToLinear(pixel.rgb);
            sum += pixel;
            count++;
        }
    }
    
    if (count > 0)
    {
        sum /= float(count);
    }
    
    sum.rgb = linearToSrgb(sum.rgb);
    sum = saturate(sum);
    
    targetTexture[DTid.xy] = sum;
}
