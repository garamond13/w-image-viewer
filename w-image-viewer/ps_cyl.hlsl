// Cylindrical resampling, Jinc based (not separable).

#include "shader_config.h"

#define USE_JINC_BASE
#include "kernel_functions.hlsli"

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
	
	float2 dims; // zz ww
	float scale; // xxx
	
	// Kernel bounds.
	float bound; // yyy
	
	// Texel size.
	float2 pt; // zzz www
	
	bool use_ar; // xxxx
}

// Expects abs(x).
float get_weight(float x)
{
	if (x < radius) {
		switch (index) {
			case WIV_KERNEL_FUNCTION_LANCZOS:
				return base(x, blur) * jinc(x, radius); // EWA Lanczos.
			case WIV_KERNEL_FUNCTION_GINSENG:
				return base(x, blur) * sinc(x, radius); // EWA Ginseng.
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

	// x >= radius
	else {
		return 0.0;
	}
}

float4 main(float4 pos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	const float2 fcoord = frac(texcoord * dims - 0.5);
	const float2 base = texcoord - fcoord * pt;
	float4 csum = 0.0;
	float wsum = 0.0; // Weight sum.

	// Antiringing.
	float4 lo = 1e9;
	float4 hi = -1e9;
	
	for (float j = 1.0 - bound; j <= bound; ++j) {
		for (float i = 1.0 - bound; i <= bound; ++i) {
			float4 color = tex.SampleLevel(smp, base + pt * float2(i, j), 0.0);
			float weight = get_weight(length(float2(i, j) - fcoord) * scale);
			csum += color * weight;
			wsum += weight;

			// Antiringing.
			if (use_ar && j >= 0.0 && j <= 1.0 && i >= 0.0 && i <= 1.0) {
				lo = min(lo, color);
				hi = max(hi, color);
			}
		}
	}

	// Normalize weighted color sum.
	csum /= wsum;

	// Antiringing.
	if (use_ar) {
		return lerp(csum, clamp(csum, lo, hi), ar);
	}

	return csum;
}