#include "helpers.hlsli"

Texture2D tex : register(t0);
SamplerState smp : register(s1);

cbuffer cb0 : register(b0)
{
	float2 size; // x y
	float theta; // z
	bool rotate; // w
	float3 tile1; // xx yy zz
	float3 tile2; // xxx yyy zzz
}

float4 main(float4 pos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	if (rotate)
		rotate_texcoord(texcoord, theta);       
	const float4 color = saturate(tex.SampleLevel(smp, texcoord, 0.0));
	return float4(color.rgb + (fmod(floor(texcoord.x * size.x) + floor(texcoord.y * size.y), 2.0) ? tile2 : tile1) * (1.0 - color.a), 1.0);
}
