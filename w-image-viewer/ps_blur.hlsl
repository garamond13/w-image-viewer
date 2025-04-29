// Used for both Gaussian blur and unsharp mask (separated).

Texture2D tex : register(t0);
Texture2D tex_original : register(t1);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
	int radius; // x
	float sigma; // y
	
	// Only used by unsharp mask.
	float amount; // z
	
	// Texel size, (1 / width, 0) or (0, 1 / height).
	float2 pt; // xx yy
};

// Normalized version is divided by sqrt(2 * pi * sigma * sigma).
#define get_weight(x) (exp(-(x) * (x) / (2.0 * sigma * sigma)))

// Samples one axis (x or y) at a time.
float4 main(float4 pos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	float weight;
	float4 csum = tex.SampleLevel(smp, texcoord, 0.0); // Weighted color sum.
	float wsum = 1.0; // Weight sum.
	for (int i = 1; i <= radius; ++i) {
		weight = get_weight(float(i));
		csum += (tex.SampleLevel(smp, texcoord + pt * float(-i), 0.0) + tex.SampleLevel(smp, texcoord + pt * float(i), 0.0)) * weight;
		wsum += 2.0 * weight;
	}
	
	// Unsharp mask.
	// Last pass!
	if (amount > 0.0) {
		const float4 original = tex_original.SampleLevel(smp, texcoord, 0.0);
		return original + (original - csum / wsum) * amount;
	}
	
	return csum / wsum;
}
