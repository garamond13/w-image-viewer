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

#define gamma(rgb) (pow((rgb), rcp_gamma))

#define srgb(rgb) ((rgb) < 0.0031308 ? 12.92 * (rgb) : 1.055 * pow((rgb), 1.0 / 2.4) - 0.055)

float4 main(Vs_out vs_out) : SV_Target
{
    float4 color = saturate(tex.SampleLevel(smp, vs_out.texcoord, 0.0));
    
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
