// Color managment system (CMS).
// Tetrahedral interpolation.

#include "shader_config.h"

Texture2D tex : register(t0);
Texture3D lut : register(t2);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
	float lut_size; // x
}

float4 main(float4 pos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	const float4 color = tex.SampleLevel(smp, texcoord, 0.0); 
	const float3 index = saturate(color.rgb) * (lut_size - 1.0);
	
	// Get barycentric weights.
	// See https://doi.org/10.2312/egp.20211031
	//
	
	const float3 r = frac(index);
	const bool3 c = r.xyz >= r.yzx;
	const bool c_xy = c.x;
	const bool c_yz = c.y;
	const bool c_zx = c.z;
	const bool c_yx =!c.x;
	const bool c_zy =!c.y;
	const bool c_xz =!c.z;
	bool cond;
	float3 s = 0.0;
	float3 vert2 = 0.0;
	float3 vert3 = 1.0;

	#define order(x,y,z) \
	cond = c_ ## x ## y && c_ ## y ## z; \
	s = cond ? r.x ## y ## z : s; \
	vert2.x = cond ? 1.0 : vert2.x; \
	vert3.z = cond ? 0.0 : vert3.z;
	
	order(x, y, z)
	order(x, z, y)
	order(z, x, y)
	order(z, y, x)
	order(y, z, x)
	order(y, x, z)
	
	//
	
	const float3 base = floor(index) + 0.5;
	return float4(lut.SampleLevel(smp, base / lut_size, 0.0).rgb * (1.0 - s.x) + lut.SampleLevel(smp, (base + 1.0) / lut_size, 0.0).rgb * s.z + lut.SampleLevel(smp, (base + vert2) / lut_size, 0.0).rgb * (s.x - s.y) + lut.SampleLevel(smp, (base + vert3) / lut_size, 0.0).rgb * (s.y - s.z), color.a);
}
