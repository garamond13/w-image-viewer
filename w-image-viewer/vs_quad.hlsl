#include "vs_out.hlsli"

//fullscreen triangle
Vs_out main(uint id : SV_VertexID)
{
    Vs_out vs_out;
    vs_out.texcoord = float2((id << 1) & 2, id & 2);
    vs_out.position = float4(vs_out.texcoord * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    return vs_out;
}
