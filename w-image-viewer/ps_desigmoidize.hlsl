// Expects sigmoidized input.

#include "vs_out.hlsli"

Texture2D tex : register(t0);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
    float c; // Contrast.
    float m; // Midpoint.
}

// Based on https://github.com/ImageMagick/ImageMagick/blob/main/MagickCore/enhance.c
#define desigmoidize(rgba) (1.0 / (1.0 + exp(c * (m - (rgba)))) - 1.0 / (1.0 + exp(c * m))) / ( 1.0 / (1.0 + exp(c * (m - 1.0))) - 1.0 / (1.0 + exp(c * m)))

float4 main(Vs_out vs_out) : SV_Target
{
    float4 color = tex.SampleLevel(smp, vs_out.texcoord, 0.0);
    return float4(desigmoidize(color.rgb), color.a);
}
