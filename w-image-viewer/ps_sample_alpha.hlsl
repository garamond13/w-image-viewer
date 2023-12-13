#include "vs_out.hlsli"
#include "helpers.hlsli"

Texture2D tex : register(t0);
SamplerState smp : register(s1);

cbuffer cb0 : register(b0)
{
    float2 size; // x y
    float theta; // z
    float3 tile1; // xx yy zz
    float3 tile2; // xxx yyy zzz
}

float4 main(Vs_out vs_out) : SV_Target
{
    // Check is theta divisible by 360, if it is we dont need to rotate texcoord.
    if (is_not_zero(frac(theta / 360.0)))
        rotate_texcoord(vs_out.texcoord, theta);
        
    const float4 color = saturate(tex.SampleLevel(smp, vs_out.texcoord, 0.0));
    return float4(color.rgb + (fmod(floor(vs_out.texcoord.x * size.x) + floor(vs_out.texcoord.y * size.y), 2.0) ? tile2 : tile1) * (1.0 - color.a), 1.0);
}
