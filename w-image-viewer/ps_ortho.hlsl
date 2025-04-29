// Orhogonal resampling (separable).

#include "kernel_functions.hlsli"
#include "shader_config.h"

Texture2D tex : register(t0);
SamplerState smp : register(s1);

cbuffer cb0 : register(b0)
{
	int index; // x
	float radius; // y
	float blur; // z
	
	// Free parameters.
	float p1; // w
	float p2; // xx
	
	// Antiringing strenght.
	float ar; // yy
	
	float scale; // zz
	
	// Kernel bounds.
	float bound; // ww
	
	float2 dims; // xxx yyy
	
	// x or y axis, (1, 0) or (0, 1).
	float2 axis; // zzz www
	
	// Texel size.
	float2 pt; // xxxx yyyy
	
	bool use_ar; // zzzz
}

// Expects abs(x).
float get_weight(float x)
{
	if (x < radius) {
		[forcecase] switch (index) {
			case WIV_KERNEL_FUNCTION_LANCZOS:
				return base(x, blur) * sinc(x, radius);
			case WIV_KERNEL_FUNCTION_GINSENG:
				return base(x, blur) * jinc(x, radius);
			case WIV_KERNEL_FUNCTION_HAMMING:
				return base(x, blur) * hamming(x, radius);
			case WIV_KERNEL_FUNCTION_POW_COSINE:
				return base(x, blur) * power_of_cosine(x, radius, p1);
			case WIV_KERNEL_FUNCTION_KAISER:
				return base(x, blur) * kaiser(x, radius, p1);
			case WIV_KERNEL_FUNCTION_POW_GARAMOND:
				return base(x, blur) * power_of_garamond(x, radius, p1, p2);
			case WIV_KERNEL_FUNCTION_POW_BLACKMAN:
				return base(x, blur) * power_of_blackman(x, radius, p1, p2);
			case WIV_KERNEL_FUNCTION_GNW:
				return base(x, blur) * generalized_normal_window(x, p1, p2);
			case WIV_KERNEL_FUNCTION_SAID:
				return base(x, blur) * said(x, p1, p2);
			case WIV_KERNEL_FUNCTION_NEAREST:
				return nearest_neighbor(x);
			case WIV_KERNEL_FUNCTION_LINEAR:
				return linear_kernel(x);
			case WIV_KERNEL_FUNCTION_BICUBIC:
				return bicubic(x, p1);
			case WIV_KERNEL_FUNCTION_FSR:
				return modified_fsr_kernel(x, p1, p2);
			case WIV_KERNEL_FUNCTION_BCSPLINE:
				return bc_spline(x, p1, p2);
			default: // Black image.
				return 0.0;
		}
	}
	else // x >= radius
		return 0.0;
}

// Samples one axis (x or y) at a time.
float4 main(float4 pos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	const float fcoord = dot(frac(texcoord * dims - 0.5), axis);
	const float2 base = texcoord - fcoord * pt;
	float4 color;
	float4 csum = 0.0; // Weighted color sum.
	float weight;
	float wsum = 0.0; // Weight sum.

	// Antiringing.
	float4 lo = 1e9;
	float4 hi = -1e9;
	
	for (float i = 1.0 - bound; i <= bound; ++i) {
		color = tex.SampleLevel(smp, base + pt * i, 0.0);
		weight = get_weight(abs((i - fcoord) * scale));
		csum += color * weight;
		wsum += weight;
		
		// Antiringing.
		if (use_ar && i >= 0.0 && i <= 1.0) {
			lo = min(lo, color);
			hi = max(hi, color);
		}
		
	}
	csum /= wsum;
	
	// Antiringing.
	if (use_ar)
		return lerp(csum, clamp(csum, lo, hi), ar);
	
	return csum;
}
