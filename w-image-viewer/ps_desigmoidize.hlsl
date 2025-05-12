// Expects sigmoidized input.

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

float3 desigmoidize(float3 rgb)
{
	// offset = 1 / (1 + exp(contrast * midpoint))
	// scale = 1 / (1 + exp(contrast * (midpoint - 1))) - offset
	return (1.0 / (1.0 + exp(contrast * (midpoint - rgb))) - offset) / scale;
}

float4 main(float4 pos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	const float4 color = tex.SampleLevel(smp, texcoord, 0.0);
	return float4(desigmoidize(color.rgb), color.a);
}
