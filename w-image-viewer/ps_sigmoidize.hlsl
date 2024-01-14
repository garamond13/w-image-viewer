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
    
    float offset;
    float scale;
}

// Based on https://github.com/ImageMagick/ImageMagick/blob/main/MagickCore/enhance.c
//sigmoidize(x) = (m - log(1 / ((1 / (1 + exp(c * (m - 1))) - 1 / (1 + exp(c * m))) * x + 1 / (1 + exp(c * m))) - 1) / c)

// offset = 1 / (1 + exp(c * m))
// scale = 1 / (1 + exp(c * (m - 1))) - offset

#define sigmoidize(rgb) (m - log(1.0 / (scale * (rgb) + offset) - 1.0) / c)

float4 main(Vs_out vs_out) : SV_Target
{
    const float4 color = saturate(tex.SampleLevel(smp, vs_out.texcoord, 0.0));
    return float4(sigmoidize(color.rgb), color.a);
}
