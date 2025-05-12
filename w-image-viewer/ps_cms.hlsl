// Color managment system (CMS).
// Tetrahedral interpolation.

Texture2D tex : register(t0);
Texture3D lut : register(t2);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
	float lut_size;
	bool dither;
	float random_number;
}

// Tri dither
// Source https://github.com/crosire/reshade-shaders/blob/slim/Shaders/TriDither.fxh
//

#define remap(v,a,b) (((v) - (a)) / ((b) - (a)))

float rand21(float2 uv)
{
	const float2 noise = frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453);
	return (noise.x + noise.y) * 0.5;
}

float rand11(float x)
{
	return frac(x * 0.024390243);
}

float permute(float x)
{
	return ((34.0 * x + 1.0) * x) % 289.0;
}

float3 tri_dither(float3 color, float2 uv, int bits)
{
	const float bitstep = exp2(bits) - 1.0;
	const float lsb = 1.0 / bitstep;
	const float lobit = 0.5 / bitstep;
	const float hibit = (bitstep - 0.5) / bitstep;
	const float3 m = float3(uv, rand21(uv + random_number)) + 1.0;
	float h = permute(permute(permute(m.x) + m.y) + m.z);
	float3 noise1;
	float3 noise2;
	noise1.x = rand11(h);
	h = permute(h);
	noise2.x = rand11(h);
	h = permute(h);
	noise1.y = rand11(h);
	h = permute(h);
	noise2.y = rand11(h);
	h = permute(h);
	noise1.z = rand11(h);
	h = permute(h);
	noise2.z = rand11(h);
	const float3 lo = saturate(remap(color.xyz, 0.0, lobit));
	const float3 hi = saturate(remap(color.xyz, 1.0, hibit));
	const float3 uni = noise1 - 0.5;
	const float3 tri = noise1 - noise2;
	return lerp(uni, tri, min(lo, hi)) * lsb;
}

//

// See https://doi.org/10.2312/egp.20211031
void get_barycentric_weights(float3 r, out float4 bary, out float3 vert2, out float3 vert3)
{
	vert2 = 0.0;
	vert3 = 1.0;
	const bool3 c = r.xyz >= r.yzx;
	const bool c_xy = c.x;
	const bool c_yz = c.y;
	const bool c_zx = c.z;
	const bool c_yx =!c.x;
	const bool c_zy =!c.y;
	const bool c_xz =!c.z;
	bool cond;
	float3 s = 0.0;
	
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

	bary = float4(1.0 - s.x, s.z, s.x - s.y, s.y - s.z);
}

float4 main(float4 pos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	float4 color = tex.SampleLevel(smp, texcoord, 0.0); 
	const float3 index = saturate(color.rgb) * (lut_size - 1.0);
	float4 bary;
	float3 vert2;
	float3 vert3;
	get_barycentric_weights(frac(index), bary, vert2, vert3);
	const float3 base = floor(index) + 0.5;
	color.rgb = lut.SampleLevel(smp, base / lut_size, 0.0).rgb * bary.x + lut.SampleLevel(smp, (base + 1.0) / lut_size, 0.0).rgb * bary.y + lut.SampleLevel(smp, (base + vert2) / lut_size, 0.0).rgb * bary.z + lut.SampleLevel(smp, (base + vert3) / lut_size, 0.0).rgb * bary.w;
	if (dither) {
		color.rgb += tri_dither(color.rgb, texcoord, 8);
	}
	return color;
}
