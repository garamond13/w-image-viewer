// Used for both Gaussian blur and unsharp mask (separated).

Texture2D tex : register(t0);
Texture2D tex_original : register(t1);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
    int radius;
    float sigma;
    float2 pt;
    float amount;
};

// Normalized version is divided by sqrt(2 * pi * sigma * sigma).
float get_weight(float x)
{
    return exp(-x * x / (2.0 * sigma * sigma));
}

float4 unsharp_mask(float4 rgba, float2 texcoord)
{
    const float4 original = tex_original.SampleLevel(smp, texcoord, 0.0);
    return original + (original - rgba) * amount;
}

// Samples one axis (x or y) at a time.
float4 main(float4 pos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    float weight;
    float4 csum = tex.SampleLevel(smp, texcoord, 0.0);
    float wsum = 1.0; // Weight sum.
    for (int i = 1; i <= radius; ++i) {
        weight = get_weight(float(i));
        csum += (tex.SampleLevel(smp, texcoord + pt * float(-i), 0.0) + tex.SampleLevel(smp, texcoord + pt * float(i), 0.0)) * weight;
        wsum += 2.0 * weight;
    }

    // Normalize weighted color sum.
    csum /= wsum;

    // Should only called in the last pass of unsharp mask pass!
    if (amount > 0.0) {
        return unsharp_mask(csum, texcoord);
    }
    
    return csum;
}
