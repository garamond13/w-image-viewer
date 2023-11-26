#include "vs_out.hlsli"
#include "helpers.hlsli"

Texture2D tex : register(t0);
SamplerState smp : register(s1);

cbuffer cb0 : register(b0)
{
    float theta; // x
}

float4 main(Vs_out vs_out) : SV_Target
{
    // Check is theta divisible by 360, if it is we dont need to rotate texcoord.
    if (is_not_zero(frac(theta / 360.0)))
        rotate_texcoord(vs_out.texcoord, theta);
        
    return tex.SampleLevel(smp, vs_out.texcoord, 0.0);
}
