#include "vs_out.hlsli"
#include "shader_config.h"

Texture2D tex : register(t0);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
    int index; // x
    
    // Example: 2.2.
    float gamma_value; // y
}

#define gamma(rgb) (pow((rgb), gamma_value))

#define srgb(rgb) ((rgb) < 0.04045 ? (rgb) / 12.92 : pow(((rgb) + 0.055) / 1.055, 2.4))

float4 main(Vs_out vs_out) : SV_Target
{
    const float4 color = saturate(tex.SampleLevel(smp, vs_out.texcoord, 0.0));
    
    [forcecase] switch (index) {
        case WIV_CMS_TRC_GAMMA:
            return float4(gamma(color.rgb), color.a);
        case WIV_CMS_TRC_SRGB:
            return float4(srgb(color.rgb), color.a);
        
        // Red image.
        default:
            return float4(1.0, 0.0, 0.0, 1.0);
    }
}
