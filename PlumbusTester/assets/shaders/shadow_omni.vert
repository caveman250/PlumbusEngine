layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec3 inTangent;

layout (location = 0) out vec4 outPos;
layout (location = 1) out vec3 outLightPos;

layout (binding = 0) uniform UBO
{
    mat4 projection;
    vec4 lightPos;
} ubo;

layout(push_constant) uniform PushConsts
{
    mat4 view;
    mat4 model;
} pushConsts;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = ubo.projection * pushConsts.view * pushConsts.model * vec4(inPos.x, inPos.y, inPos.z, 1.0);

    outPos = pushConsts.model * vec4(inPos.x, inPos.y, inPos.z, 1.0f);

    outLightPos = ubo.lightPos.xyz;
}