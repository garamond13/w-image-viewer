#ifndef __VS_OUT_HLSLI__
#define __VS_OUT_HLSLI__

struct Vs_out
{
    float4 position : SV_POSITION; //xyzw
    float2 texcoord : TEXCOORD; //uv
};

#endif // __VS_OUT_HLSLI__
