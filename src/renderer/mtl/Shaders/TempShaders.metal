#include <metal_stdlib>
using namespace metal;

struct Vertex
{
    float4 position [[position]];
//    float2 uv;
//    float3 colour;
//    float3 normal;
//    float3 tangent;
};

struct Uniforms
{
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};


vertex Vertex basic_vertex(device Vertex *vertices [[buffer(0)]],
                             constant Uniforms *uniforms [[buffer(1)]],
                             uint vid [[vertex_id]])
{
    Vertex vertexOut;
    vertexOut.position = uniforms->proj * uniforms->view * uniforms->model * vertices[vid].position;
    
    //vertexOut.colour = float3(1,0,0);
    
    return vertexOut;
}

fragment half4 basic_fragment(Vertex vertexIn [[stage_in]])
{
    return half4(1,1,1,1);
}

