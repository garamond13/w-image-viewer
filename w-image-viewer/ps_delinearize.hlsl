#include "vs_out.hlsli"
#include "shader_config.h"

Texture2D tex : register(t0);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
    int index; // x
    
    // Example: 1.0 / 2.2.
    float rcp_gamma; // y
}

float3 linear_to_gamma(float3 rgb)
{
    return pow(rgb, rcp_gamma);
}

float linear_to_srgb(float x)
{
    return x < 0.0031308 ? 12.92 * x : 1.055 * pow(x, 1.0 / 2.4) - 0.055;
}

float3 linear_to_srgb(float3 rgb)
{
    return float3(linear_to_srgb(rgb.r), linear_to_srgb(rgb.g), linear_to_srgb(rgb.b));
}

float4 main(Vs_out vs_out) : SV_Target
{
    const float4 color = saturate(tex.SampleLevel(smp, vs_out.texcoord, 0.0));
    
    [forcecase] switch (index) {
        case WIV_CMS_TRC_GAMMA:
            return float4(linear_to_gamma(color.rgb), color.a);
        case WIV_CMS_TRC_SRGB:
            return float4(linear_to_srgb(color.rgb), color.a);
    
        // Red image.
        default:
            return float4(1.0, 0.0, 0.0, 1.0);
    }
}
