// Expects linearized input.

#include "vs_out.hlsli"

Texture2D tex : register(t0);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
    // Contrast.
    float c; // x 
    
    // Midpoint.
    float m; // y 
}

// Based on https://github.com/ImageMagick/ImageMagick/blob/main/MagickCore/enhance.c
#define sigmoidize(rgb) (m - log(1.0 / ((1.0 / (1.0 + exp(c * (m - 1.0))) - 1.0 / (1.0 + exp(c * m))) * (rgb) + 1.0 / (1.0 + exp(c * m))) - 1.0) / c)

float4 main(Vs_out vs_out) : SV_Target
{
    const float4 color = saturate(tex.SampleLevel(smp, vs_out.texcoord, 0.0));
    return float4(sigmoidize(color.rgb), color.a);
}
