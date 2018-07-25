#include <metal_stdlib>
using namespace metal;

struct Light
{
    float3 direction;
    float3 ambientColour;
    float3 diffuseColour;
    float3 specularColour;
};

constant Light light =
{
    .direction = { 0.13, 0.72, 0.68 },
    .ambientColour = { 0.8, 0.8, 0.8 },
    .diffuseColour = { 1, 1, 1 },
    .specularColour = { 0.2, 0.2, 0.2 }
};

struct Vertex
{
    float4 position [[position]];
    float2 uv;
//    float3 colour;
    float3 normal;
    float3 tangent;
};

struct ProjectedVertex
{
    float4 position [[position]];
    float2 uv;
    //    float3 colour;
    float3 normal;
    float3 tangent;
    float3 eyePosition;
};

struct Uniforms
{
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};

constant float3 kSpecularColor= { 1, 1, 1 };
constant float kSpecularPower = 80;


vertex ProjectedVertex basic_vertex(device Vertex *vertices [[buffer(0)]],
                             constant Uniforms *uniforms [[buffer(1)]],
                             uint vid [[vertex_id]])
{
    ProjectedVertex vertexOut;
    vertexOut.position = uniforms->proj * uniforms->view * uniforms->model * vertices[vid].position;
    vertexOut.uv = vertices[vid].uv;
    vertexOut.normal = vertices[vid].normal;
    vertexOut.tangent = vertices[vid].tangent;
    vertexOut.eyePosition = -vertexOut.position.xyz;
    
    //vertexOut.colour = float3(1,0,0);
    
    return vertexOut;
}

fragment float4 basic_fragment(ProjectedVertex vert [[stage_in]],
                                 constant Uniforms &uniforms [[buffer(0)]],
                                 texture2d<float> diffuseTexture [[texture(0)]],
                                 sampler diffuseSampler [[sampler(0)]],
                                 texture2d<float> normalTexture [[texture(1)]],
                                 sampler normalSampler [[sampler(1)]])
{
    float3 diffuseColour = diffuseTexture.sample(diffuseSampler, vert.uv).rgb;
    float3 ambientColour = light.ambientColour * diffuseColour;
    
    float3 N = normalize(vert.normal);
    float3 T = normalize(vert.tangent);
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);
    
    float3 normal = TBN * normalize(normalTexture.sample(normalSampler, vert.uv).rgb * 2.0 - float3(1.0));
    
    float diffuseIntensity = saturate(dot(normal, light.direction));
    float3 diffuseTerm = light.diffuseColour * diffuseColour * diffuseIntensity;
    
    float3 specularTerm(0);
    if (diffuseIntensity > 0)
    {
        float3 eyeDirection = normalize(vert.eyePosition);
        float3 halfway = normalize(light.direction + eyeDirection);
        float specularFactor = pow(saturate(dot(normal, halfway)), kSpecularPower);
        specularTerm = light.specularColour * kSpecularColor * specularFactor;
    }
    
    return float4(ambientColour + diffuseTerm + specularTerm, 1);
}

//fragment float4 fragment_texture(ProjectedVertex vert [[stage_in]],
//                                 constant Uniforms &uniforms [[buffer(0)]],
//                                 texture2d<float> diffuseTexture [[texture(0)]],
//                                 sampler samplr [[sampler(0)]])
//{
//    float3 diffuseColor = diffuseTexture.sample(samplr, vert.texCoords).rgb;
//
//    float3 ambientTerm = light.ambientColor * diffuseColor;
//
//
//
//    float3 normal = normalize(vert.normal);
//    float diffuseIntensity = saturate(dot(normal, light.direction));
//    float3 diffuseTerm = light.diffuseColor * diffuseColor * diffuseIntensity;
//
//    float3 specularTerm(0);
//    if (diffuseIntensity > 0)
//    {
//        float3 eyeDirection = normalize(vert.eyePosition);
//        float3 halfway = normalize(light.direction + eyeDirection);
//        float specularFactor = pow(saturate(dot(normal, halfway)), kSpecularPower);
//        specularTerm = light.specularColor * kSpecularColor * specularFactor;
//    }
//
//    return float4(ambientTerm + diffuseTerm + specularTerm, 1);
//}

