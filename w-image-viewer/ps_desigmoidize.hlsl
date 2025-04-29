// Expects sigmoidized input.

Texture2D tex : register(t0);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
	// Contrast.
	float c; // x
	
	// Midpoint.
	float m; // y
	
	float offset; // z
	float scale; // w
}

// Based on https://github.com/ImageMagick/ImageMagick/blob/main/MagickCore/enhance.c
//desigmoidize(x) = 1 / (1 + exp(c * (m - x))) - 1 / (1 + exp(c * m))) / (1 / (1 + exp(c * (m - 1))) - 1 / (1 + exp(c * m))

// offset = 1 / (1 + exp(c * m))
// scale = 1 / (1 + exp(c * (m - 1))) - offset

#define desigmoidize(rgb) ((1.0 / (1.0 + exp(c * (m - (rgb)))) - offset) / scale)

float4 main(float4 pos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	const float4 color = tex.SampleLevel(smp, texcoord, 0.0);
	return float4(desigmoidize(color.rgb), color.a);
}
