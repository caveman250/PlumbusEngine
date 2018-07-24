#include <metal_stdlib>
using namespace metal;

struct Vertex
{
    float4 position [[position]];
    float2 uv;
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
    vertexOut.uv = vertices[vid].uv;
    
    //vertexOut.colour = float3(1,0,0);
    
    return vertexOut;
}

fragment float4 basic_fragment(Vertex vert [[stage_in]],
                                 constant Uniforms &uniforms [[buffer(0)]],
                                 texture2d<float> diffuseTexture [[texture(0)]],
                                 sampler samplr [[sampler(0)]])
{
    float3 diffuseColor = diffuseTexture.sample(samplr, vert.uv).rgb;

    return float4(diffuseColor, 1);
}

