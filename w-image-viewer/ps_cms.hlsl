// Color managment system (CMS).
// Tetrahedral interpolation.

#include "vs_out.hlsli"
#include "shader_config.h"

Texture2D tex : register(t0);
Texture3D lut : register(t2);
SamplerState smp : register(s0);

#define LUT_SIZE (float)WIV_CMS_LUT_SIZE

float4 main(Vs_out vs_out) : SV_Target
{
    float4 color = tex.SampleLevel(smp, vs_out.texcoord, 0.0); 
    float3 index = saturate(color.rgb) * (LUT_SIZE - 1.0);
    
    // Get barycentric weights.
    // See https://doi.org/10.2312/egp.20211031
    //
    
    float3 r = frac(index);
    float3 vert2 = 0.0;
    float3 vert3 = 1.0;
    bool3 c = r.xyz >= r.yzx;
    bool c_xy = c.x;
    bool c_yz = c.y;
    bool c_zx = c.z;
    bool c_yx =!c.x;
    bool c_zy =!c.y;
    bool c_xz =!c.z;
    bool cond;
    float3 s = 0.0;
            
#define ORDER(x,y,z) \
    cond = c_ ## x ## y && c_ ## y ## z; \
    s = cond ? r.x ## y ## z : s; \
    vert2.x = cond ? 1.0 : vert2.x; \
    vert3.z = cond ? 0.0 : vert3.z;
    
    ORDER(x, y, z)
    ORDER(x, z, y)
    ORDER(z, x, y)
    ORDER(z, y, x)
    ORDER(y, z, x)
    ORDER(y, x, z)
    
    //
    
    float3 base = floor(index) + 0.5;
    return float4(lut.SampleLevel(smp, base / LUT_SIZE, 0.0).rgb * (1.0 - s.x) + lut.SampleLevel(smp, (base + 1.0) / LUT_SIZE, 0.0).rgb * s.z + lut.SampleLevel(smp, (base + vert2) / LUT_SIZE, 0.0).rgb * (s.x - s.y) + lut.SampleLevel(smp, (base + vert3) / LUT_SIZE, 0.0).rgb * (s.y - s.z), color.a);
}
