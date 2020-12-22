#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec3 inTangent;

layout (binding = 0) uniform UBO 
{
	mat4 model;
	mat4 view;
	mat4 projection;
} ubo;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	vec4 tmpPos = inPos;

	gl_Position = ubo.projection * ubo.view * ubo.model * tmpPos;	
}