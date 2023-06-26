
#include "vs_out.hlsli"
#include "kernel_functions.hlsli"
#include "helpers.hlsli"
#include "shader_config.h"

Texture2D tex : register(t0);
SamplerState smp : register(s1);

cbuffer cb0 : register(b0)
{
    int index; //x
    float radius; //y
    float blur; //z
    float p1; //w
    float p2; //xx
    float ar; //yy
    float scale; //zz
    float2 dims; //xxx yyy
    float2 axis; //zzz www
}

//get texel size
static float2 pt = 1.0 / dims * axis;

//antiringing shouldnt be used when downsampling
static bool use_ar = ar > 0.0 && is_equal(scale, 1.0);

float get_weight(float x)
{
    if (x < radius) {
        [forcecase] switch (index) {
        case WIV_KERNEL_FUNCTION_LANCZOS:
            return base(x, blur) * sinc(x, radius);
        case WIV_KERNEL_FUNCTION_GINSENG:
            return base(x, blur) * jinc(x, radius);
        case WIV_KERNEL_FUNCTION_HAMMING:
            return base(x, blur) * hamming(x, radius);
        case WIV_KERNEL_FUNCTION_POW_COSINE:
            return base(x, blur) * power_of_cosine(x, radius, p1);
        case WIV_KERNEL_FUNCTION_GARAMOND_KAISER:
            return base(x, blur) * garamond_kaiser(x, radius, p1, p2);
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
        default: //black image
            return 0.0;
        }
    }
    else //x >= radius
        return 0.0;
}

float4 main(Vs_out vs_out) : SV_Target
{   
    const float fcoord = dot(frac(vs_out.texcoord * dims - 0.5), axis);
    const float2 base = vs_out.texcoord - fcoord * pt;
    float4 color;
    float4 csum = 0.0; //weighted color sum
    float weight;
    float wsum = 0.0; //weight sum
     
    //antiringing
    float4 low = 1e9;
    float4 high = -1e9;
    
    //get required radius
    const float r = ceil(radius * scale);
    
    [loop] for (float i = 1.0 - r; i <= r; ++i) {
        color = tex.SampleLevel(smp, base + pt * i, 0.0);
        weight = get_weight(abs((i - fcoord) / scale));
        csum += color * weight;
        wsum += weight;
        
        //antiringing
        if (use_ar && i >= 0.0 && i <= 1.0) {
            low = min(low, color);
            high = max(high, color);
        }
    }
    //normalize color values
    csum /= wsum;
    
    //antiringing
    if (use_ar)
        return lerp(csum, clamp(csum, low, high), ar);
    
    return csum;
}
