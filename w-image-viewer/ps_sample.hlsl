#include "vs_out.hlsli"

Texture2D tex : register(t0);
SamplerState smp : register(s1);

float4 main(Vs_out vs_out) : SV_Target
{
    return tex.SampleLevel(smp, vs_out.texcoord, 0.0);
}
