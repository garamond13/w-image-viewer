#include "shader_config.h"

Texture2D tex : register(t0);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
	int index; // x
	
	// Example: 2.2.
	float gamma_value; // y
}

float3 gamma_to_linear(float3 rgb)
{
	return pow(rgb, gamma_value);
}

float3 srgb_to_linear(float3 x)
{
	return x < 0.04045 ? x / 12.92 : pow((x + 0.055) / 1.055, 2.4);
}

float4 main(float4 pos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	const float4 color = saturate(tex.SampleLevel(smp, texcoord, 0.0));
	
	switch (index) {
		case WIV_CMS_TRC_GAMMA:
			return float4(gamma_to_linear(color.rgb), color.a);
		case WIV_CMS_TRC_SRGB:
			return float4(srgb_to_linear(color.rgb), color.a);
		
		// Red image.
		default:
			return float4(1.0, 0.0, 0.0, 1.0);
	}
}
