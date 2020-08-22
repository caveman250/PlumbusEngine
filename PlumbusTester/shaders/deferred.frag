#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform sampler2D samplerposition;
layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D samplerAlbedo;
layout (binding = 4) uniform sampler2D samplerShadows;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragcolor;

struct PointLight {
	vec4 position;
	vec4 color;
	float radius;
};

struct DirectionalLight {
	vec4 direction;
	vec4 color;
	mat4 mvp;
};

const int MAX_POINT_LIGHTS = 6;
const int MAX_DIRECTIONAL_LIGHTS = 1;

layout (binding = 3) uniform UBO 
{
	vec4 viewPos;
	PointLight pointLights[MAX_POINT_LIGHTS];
	DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
} ubo;

float shadowProj(vec4 P, vec2 offset)
{
	float shadow = 1.0;
	vec4 shadowCoord = P / P.w;
	shadowCoord.st = shadowCoord.st * 0.5 + 0.5;
	
	float dist = texture(samplerShadows, shadowCoord.st + offset).r;
	if (shadowCoord.w > 0.0 && dist < shadowCoord.z) 
	{
		shadow = 0.25;
	}

	return shadow;
}

vec3 shadow(vec3 fragcolor, vec3 fragpos) 
{
	for(int i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i)
	{
		vec4 shadowClip	= ubo.directionalLights[i].mvp * vec4(fragpos, 1.0);
		float shadowFactor = shadowProj(shadowClip, vec2(0.0));
		fragcolor *= shadowFactor;
	}
	return fragcolor;
}

void main() 
{
	// Get G-Buffer values
	vec3 fragPos = texture(samplerposition, inUV).rgb;
	vec3 normal = texture(samplerNormal, inUV).rgb;
	vec4 albedo = texture(samplerAlbedo, inUV);
	
	#define ambient 0.3
	
	// Ambient part
	vec3 fragcolor  = albedo.rgb * ambient;
	
	for (int i = 0; i < MAX_POINT_LIGHTS; ++i)
	{
		// Vector to light
		vec3 L = ubo.pointLights[i].position.xyz - fragPos;
		// Distance from light to fragment position
		float dist = length(L);
	
		// Viewer to fragment
		vec3 V = ubo.viewPos.xyz - fragPos;
		V = normalize(V);

		// Light to fragment
		L = normalize(L);

		// Attenuation
		float atten = ubo.pointLights[i].radius / (pow(dist, 2.0) + 1.0);

		// Diffuse part
		vec3 N = normalize(normal);
		float NdotL = max(0.0, dot(N, L));
		vec3 diff = ubo.pointLights[i].color.xyz * albedo.rgb * NdotL * atten;

		// Specular part
		// Specular map values are stored in alpha of albedo mrt
		vec3 R = reflect(-L, N);
		float NdotR = max(0.0, dot(R, V));
		vec3 spec = ubo.pointLights[i].color.xyz * albedo.a * pow(NdotR, 16.0) * atten;

		fragcolor += diff + spec;	
	}
	
	for (int i = 0; i < MAX_DIRECTIONAL_LIGHTS; ++i)
	{
		vec3 lightDir = ubo.directionalLights[i].direction.xyz;
		lightDir.y = -lightDir.y;
		vec3 L = normalize(lightDir);

		// Diffuse part
		vec3 N = normalize(normal);
		float NdotL = max(0.0, dot(N, L));
		vec3 diff = ubo.directionalLights[i].color.xyz * albedo.rgb * NdotL;

		// Specular part
		// Specular map values are stored in alpha of albedo mrt
		vec3 R = reflect(-L, N);
		vec3 V = ubo.viewPos.xyz - fragPos;
		V = normalize(V);
		float NdotR = max(0.0, dot(R, V));
		vec3 spec = ubo.directionalLights[i].color.xyz * albedo.a * pow(NdotR, 16.0);

		fragcolor += diff + spec;
	}	

	fragcolor = shadow(fragcolor, fragPos);

	// vec4 shadowClip	= ubo.directionalLights[0].mvp * vec4(fragPos, 1.0);
	// vec4 shadowCoord = shadowClip / shadowClip.w;
	// shadowCoord.st = shadowCoord.st * 0.5 + 0.5;
	// float dist = texture(samplerShadows, shadowCoord.st).r;

	// vec3 fragcolor = vec3(dist, dist, dist);
   
  	outFragcolor = vec4(fragcolor, 1.0);	
}