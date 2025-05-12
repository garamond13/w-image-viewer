// Expects linearized input.

Texture2D tex : register(t0);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
	float contrast;
	float midpoint;
	float offset;
	float scale;
}

// Based on https://github.com/ImageMagick/ImageMagick/blob/main/MagickCore/enhance.c

float3 sigmoidize(float3 rgb)
{
	// offset = 1 / (1 + exp(contrast * midpoint))
	// scale = 1 / (1 + exp(contrast * (midpoint - 1))) - offset
	return midpoint - log(1.0 / (scale * rgb + offset) - 1.0) / contrast;
}

float4 main(float4 pos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	const float4 color = saturate(tex.SampleLevel(smp, texcoord, 0.0));
	return float4(sigmoidize(color.rgb), color.a);
}
