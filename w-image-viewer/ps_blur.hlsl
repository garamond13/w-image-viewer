// Used for both Gaussian blur and unsharp mask (separated).

#include "vs_out.hlsli"

Texture2D tex : register(t0);
Texture2D tex_original : register(t1);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
    float radius; // x
    float sigma; // y
    
    // Only used by unsharp mask.
    float amount; // z
    
    // x or y axis, (1, 0) or (0, 1).
    float2 axis; // xx yy
    
    // Texel size, (1 / width, 0) or (0, 1 / height).
    float2 pt; // zz ww
};

// Normalized version is divided by sqrt(2 * pi * sigma * sigma).
#define get_weight(x) (exp(-(x) * (x) / (2.0 * sigma * sigma)))

// Samples one axis (x or y) at a time.
float4 main(Vs_out vs_out) : SV_Target
{
    float weight;
    float4 csum = tex.SampleLevel(smp, vs_out.texcoord, 0.0); // Weighted color sum.
    float wsum = 1.0; // Weight sum.
    [loop] for (float i = 1.0; i <= radius; ++i) {
        weight = get_weight(i);
        csum += (tex.SampleLevel(smp, vs_out.texcoord + pt * -i, 0.0) + tex.SampleLevel(smp, vs_out.texcoord + pt * i, 0.0)) * weight;
        wsum += 2.0 * weight;
    }
    
    // Unsharp mask.
    // Last pass!
    if (amount > 0.0) {
        const float4 original = tex_original.SampleLevel(smp, vs_out.texcoord, 0.0);
        return original + (original - csum / wsum) * amount;
    }
    
    return csum / wsum;
}
