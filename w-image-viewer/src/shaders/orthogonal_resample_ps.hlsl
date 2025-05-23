// Orhogonal resampling (separable).

#include "..\include\shader_config.h"
#include "include\kernel_functions.hlsli"

Texture2D tex : register(t0);
SamplerState smp : register(s1);

cbuffer cb0 : register(b0)
{
    int index;
    float support;
    float blur;
    
    // Free parameters.
    float p1;
    float p2;
    
    // Antiringing strenght.
    float ar;
    
    float scale;
    float radius;
    float2 dims;
    float2 pt;
    float2 axis;
}

// Expects abs(x).
float get_weight(float x)
{
    if (x <= support) {
        switch (index) {
            case WIV_KERNEL_FUNCTION_LANCZOS:
                return base(x, blur) * sinc(x, support);
            case WIV_KERNEL_FUNCTION_GINSENG:
                return base(x, blur) * jinc(x, support);
            case WIV_KERNEL_FUNCTION_HAMMING:
                return base(x, blur) * hamming(x, support);
            case WIV_KERNEL_FUNCTION_POW_COSINE:
                return base(x, blur) * power_of_cosine(x, support, p1);
            case WIV_KERNEL_FUNCTION_KAISER:
                return base(x, blur) * kaiser(x, support, p1);
            case WIV_KERNEL_FUNCTION_POW_GARAMOND:
                return base(x, blur) * power_of_garamond(x, support, p1, p2);
            case WIV_KERNEL_FUNCTION_POW_BLACKMAN:
                return base(x, blur) * power_of_blackman(x, support, p1, p2);
            case WIV_KERNEL_FUNCTION_GNW:
                return base(x, blur) * generalized_normal_window(x, p1, p2);
            case WIV_KERNEL_FUNCTION_SAID:
                return base(x, blur) * said(x, p1, p2);
            case WIV_KERNEL_FUNCTION_NEAREST:
                return nearest_neighbor(x);
            case WIV_KERNEL_FUNCTION_LINEAR:
                return linear_kernel(x);
            case WIV_KERNEL_FUNCTION_BICUBIC:
                return bicubic(x, p1);
            case WIV_KERNEL_FUNCTION_FSR:
                return fsr_kernel(x);
            case WIV_KERNEL_FUNCTION_BCSPLINE:
                return bc_spline(x, p1, p2);
            default: // Black image.
                return 0.0;
        }
    }

    // x > support
    else {
        return 0.0;
    }
}

// Samples one axis (x or y) at a time.
float4 main(float4 pos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    const float2 xy = texcoord * dims;
    const float2 base = xy * (1.0 - axis) + (floor(dot(xy, axis) - 0.5) + 0.5) * axis;
    const float f = dot(xy - base, axis);
    float4 csum = 0.0;
    float wsum = 0.0; // Weight sum.

    // Antiringing.
    float4 lo = 1e9;
    float4 hi = -1e9;
    
    for (float i = 1.0 - radius; i <= radius; ++i) {
        const float4 color = tex.SampleLevel(smp, (base + i * axis) * pt, 0.0);
        const float weight = get_weight(abs((i - f) * scale));
        csum += color * weight;
        wsum += weight;
        
        // Antiringing.
        if (ar > 0.0f && i >= 0.0 && i <= 1.0) {
            lo = min(lo, color);
            hi = max(hi, color);
        }
    }

    // Normalize weighted color sum.
    csum /= wsum;
    
    // Antiringing.
    if (ar > 0.0f) {
        return lerp(csum, clamp(csum, lo, hi), ar);
    }
    
    return csum;
}
