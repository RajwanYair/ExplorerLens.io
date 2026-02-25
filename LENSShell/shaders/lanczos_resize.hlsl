// Lanczos3 Resize Compute Shader
//
// High-quality image downscaling using Lanczos3 kernel.
// Produces sharper thumbnails than bilinear filtering, especially
// for images with fine detail (text, line art, architectural photos).
//
// Dispatch: ceil(outputWidth/16) x ceil(outputHeight/16) x 1

// Input/output textures
Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);
SamplerState LinearSampler : register(s0);

// Constants
cbuffer ResizeConstants : register(b0) {
    uint InputWidth;
    uint InputHeight;
    uint OutputWidth;
    uint OutputHeight;
    float ScaleX;       // InputWidth / OutputWidth
    float ScaleY;       // InputHeight / OutputHeight
    float FilterRadius; // Lanczos kernel radius (default: 3.0)
    float Sharpness;    // Sharpness boost (0.0 = none, 1.0 = max)
};

// Lanczos kernel: sinc(x) * sinc(x/a) for |x| < a, 0 otherwise
// where sinc(x) = sin(pi*x) / (pi*x)
static const float PI = 3.14159265358979323846f;

float Sinc(float x) {
    if (abs(x) < 1e-6f)
        return 1.0f;
    float px = PI * x;
    return sin(px) / px;
}

float LanczosWeight(float x, float radius) {
    if (abs(x) >= radius)
        return 0.0f;
    return Sinc(x) * Sinc(x / radius);
}

[numthreads(16, 16, 1)]
void CSMain(uint3 dispatchID : SV_DispatchThreadID) {
    if (dispatchID.x >= OutputWidth || dispatchID.y >= OutputHeight)
        return;

    // Map output pixel to input space
    float srcX = ((float)dispatchID.x + 0.5f) * ScaleX - 0.5f;
    float srcY = ((float)dispatchID.y + 0.5f) * ScaleY - 0.5f;

    float radius = FilterRadius;  // Typically 3.0 for Lanczos3

    // Compute filter support window
    int x0 = (int)floor(srcX - radius * max(1.0f, ScaleX));
    int x1 = (int)ceil(srcX + radius * max(1.0f, ScaleX));
    int y0 = (int)floor(srcY - radius * max(1.0f, ScaleY));
    int y1 = (int)ceil(srcY + radius * max(1.0f, ScaleY));

    // Clamp to input bounds
    x0 = max(x0, 0);
    x1 = min(x1, (int)InputWidth - 1);
    y0 = max(y0, 0);
    y1 = min(y1, (int)InputHeight - 1);

    // Accumulate weighted samples
    float4 colorSum = float4(0, 0, 0, 0);
    float weightSum = 0.0f;

    // Pre-compute scale factor for kernel coordinates
    float invScaleX = min(1.0f, 1.0f / ScaleX);
    float invScaleY = min(1.0f, 1.0f / ScaleY);

    for (int iy = y0; iy <= y1; iy++) {
        float dy = ((float)iy - srcY) * invScaleY;
        float wy = LanczosWeight(dy, radius);
        if (wy == 0.0f) continue;

        for (int ix = x0; ix <= x1; ix++) {
            float dx = ((float)ix - srcX) * invScaleX;
            float wx = LanczosWeight(dx, radius);
            if (wx == 0.0f) continue;

            float weight = wx * wy;
            float4 sample = InputTexture.Load(int3(ix, iy, 0));
            colorSum += sample * weight;
            weightSum += weight;
        }
    }

    // Normalize
    if (weightSum > 0.0f) {
        colorSum /= weightSum;
    }

    // Optional sharpness boost via unsharp mask approximation
    if (Sharpness > 0.0f) {
        float2 uv = float2((float)dispatchID.x / (float)OutputWidth,
                           (float)dispatchID.y / (float)OutputHeight);
        float4 blurred = InputTexture.SampleLevel(LinearSampler, uv, 0);
        colorSum += (colorSum - blurred) * Sharpness * 0.5f;
    }

    // Clamp to valid range
    colorSum = saturate(colorSum);

    OutputTexture[dispatchID.xy] = colorSum;
}
