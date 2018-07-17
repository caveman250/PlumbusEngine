#include <metal_stdlib>
using namespace metal;

vertex float4 basic_vertex(const device packed_float3* vertexArray [[buffer(0)]],
                       unsigned int vID[[vertex_id]])
{
    return float4(vertexArray[vID], 1.0);
}

fragment half4 basic_fragment()
{
    return half4(1.0, 0.0, 0.0, 1.0);
}

