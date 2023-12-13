// Cylindrical resampling, Jinc based (not separable).

#include "vs_out.hlsli"
#include "shader_config.h"

#define USE_JINC_BASE
#include "kernel_functions.hlsli"

Texture2D tex : register(t0);
SamplerState smp : register(s1);

cbuffer cb0 : register(b0)
{
    int index; // x
    float radius; // y
    float blur; // z
    
    // Free parameters.
    float p1; // w
    float p2; // xx
    
    // Antiringing strenght.
    float ar; // yy
    
    float2 dims; // zz, ww
    float scale; // xxx
}

// Expects abs(x).
float get_weight(float x)
{
    if (x < radius) {
        [forcecase] switch (index) {
            case WIV_KERNEL_FUNCTION_LANCZOS:
                return base(x, blur) * jinc(x, radius); // EWA Lanczos.
            case WIV_KERNEL_FUNCTION_GINSENG:
                return base(x, blur) * sinc(x, radius); // EWA Ginseng.
            case WIV_KERNEL_FUNCTION_HAMMING:
                return base(x, blur) * hamming(x, radius);
            case WIV_KERNEL_FUNCTION_POW_COSINE:
                return base(x, blur) * power_of_cosine(x, radius, p1);
            case WIV_KERNEL_FUNCTION_KAISER:
                return base(x, blur) * kaiser(x, radius, p1);
            case WIV_KERNEL_FUNCTION_POW_GARAMOND:
                return base(x, blur) * power_of_garamond(x, radius, p1, p2);
            case WIV_KERNEL_FUNCTION_POW_BLACKMAN:
                return base(x, blur) * power_of_blackman(x, radius, p1, p2);
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
                return modified_fsr_kernel(x, p1, p2);
            case WIV_KERNEL_FUNCTION_BCSPLINE:
                return bc_spline(x, p1, p2);
            default: // Black image.
                return 0.0;
        }
    }
    else // x >= radius
        return 0.0;
}

float4 main(Vs_out vs_out) : SV_TARGET
{
    const float2 fcoord = frac(vs_out.texcoord * dims - 0.5);
    const float2 pt = 1.0 / dims; // Texel size.
    const float2 base = vs_out.texcoord - fcoord * pt;
    float4 color;
    float4 csum = 0.0; // Weighted color sum.
    float weight;
    float wsum = 0.0; // Weight sum.

    // Antiringing.
    //
    
    float4 lo = 1e9;
    float4 hi = -1e9;
    
    // Antiringing shouldnt be used when downsampling!
    const bool use_ar = ar > 0.0 && is_equal(scale, 1.0);
    
    //

    // Get required radius.
    const float r = ceil(radius * scale);    
    
    [loop] for (float j = 1.0 - r; j <= r; ++j) {
        [loop] for (float i = 1.0 - r; i <= r; ++i) {
            color = tex.SampleLevel(smp, base + pt * float2(i, j), 0.0);
            weight = get_weight(length(float2(i, j) - fcoord) / scale);
            csum += color * weight;
            wsum += weight;

            // Antiringing.
            if (use_ar && j >= 0.0 && j <= 1.0 && i >= 0.0 && i <= 1.0) {
                lo = min(lo, color);
                hi = max(hi, color);
            }
        }
    }
    csum /= wsum;

    // Antiringing.
    if (use_ar)
        return lerp(csum, clamp(csum, lo, hi), ar);

    return csum;
}