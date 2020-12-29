#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform sampler2D samplerposition;
layout (binding = 1) uniform sampler2D samplerNormal;
layout (binding = 2) uniform sampler2D samplerAlbedo;
#if NUM_SHADOWS
layout (binding = 3) uniform sampler2D samplerShadows[NUM_SHADOWS];
#endif

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

layout (binding = 4) uniform ViewPos { vec4 value; } viewPos;
#if NUM_POINT_LIGHTS
layout (binding = 5) uniform PointLights { PointLight lights[NUM_POINT_LIGHTS]; } pointLights;
#endif
#if NUM_DIR_LIGHTS
layout (binding = 6) uniform DirectionalLights { DirectionalLight lights[NUM_DIR_LIGHTS]; } dirLights;
#endif

#if NUM_SHADOWS && NUM_DIR_LIGHTS
float shadowProj(vec4 P, vec2 offset, int index, float NdotL)
{
	float shadow = 1.0;
	vec4 shadowCoord = P / P.w;
	shadowCoord.st = shadowCoord.st * 0.5 + 0.5;

	float bias = 0.005 * tan(acos(NdotL)); // cosTheta is dot( n,l ), clamped between 0 and 1
	bias = clamp(bias, 0, 0.01);

	float dist = texture(samplerShadows[index], shadowCoord.st + offset).r;
	if (shadowCoord.w > 0.0 && dist < shadowCoord.z - bias)
	{
		shadow = 0;
	}

	return shadow;
}

float shadow(vec3 fragpos, int index, float NdotL)
{
	vec3 fragPosYUp = vec3(fragpos.x, -fragpos.z, fragpos.y);
	vec4 shadowClip	= dirLights.lights[index].mvp * vec4(fragPosYUp, 1.0);
	float shadowFactor = shadowProj(shadowClip, vec2(0.0), index, NdotL);
	return shadowFactor;
}
#endif

void main()
{
	// Get G-Buffer values
	vec3 fragPos = texture(samplerposition, inUV).rgb;
	vec3 normal = texture(samplerNormal, inUV).rgb;
	vec4 albedo = texture(samplerAlbedo, inUV);

	#define ambient 0.3

	// Ambient part
	vec3 fragcolor  = albedo.rgb * ambient;

#if NUM_POINT_LIGHTS
	for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
	{
		vec3 worldPos =  pointLights.lights[i].position.xyz;
		float temp = worldPos.z;
		worldPos.z = -worldPos.y;
		worldPos.y = temp;

		// Vector to light
		vec3 L = worldPos.xyz - fragPos;
		// Distance from light to fragment position
		float dist = length(L);

		// Viewer to fragment
		vec3 V = viewPos.value.xyz - fragPos;
		V = normalize(V);

		// Light to fragment
		L = normalize(L);

		// Attenuation
		float atten = pointLights.lights[i].radius / (pow(dist, 2.0) + 1.0);

		// Diffuse part
		vec3 N = normalize(normal);
		float NdotL = clamp(dot(N, L), 0.0, 1.0);
		vec3 diff = pointLights.lights[i].color.xyz * albedo.rgb * NdotL * atten;

		// Specular part
		// Specular map values are stored in alpha of albedo mrt
		vec3 R = reflect(-L, N);
		float NdotR = max(0.0, dot(R, V));
		vec3 spec = pointLights.lights[i].color.xyz * albedo.a * pow(NdotR, 16.0) * atten;

		fragcolor += diff + spec;
	}
#endif

#if NUM_DIR_LIGHTS
	for (int i = 0; i < NUM_DIR_LIGHTS; ++i)
	{
		vec3 lightDir = dirLights.lights[i].direction.xyz;
		lightDir.y = -lightDir.y;
		vec3 L = normalize(lightDir);

		// Diffuse part
		vec3 N = normalize(normal);
		float NdotL = clamp(dot(N, L), 0.0, 1.0);
		vec3 diff = dirLights.lights[i].color.xyz * albedo.rgb * NdotL;

		// Specular part
		// Specular map values are stored in alpha of albedo mrt
		vec3 R = reflect(-L, N);
		vec3 V = viewPos.value.xyz - fragPos;
		V = normalize(V);
		float NdotR = max(0.0, dot(R, V));
		vec3 spec = dirLights.lights[i].color.xyz * albedo.a * pow(NdotR, 16.0);

#if NUM_SHADOWS
		float shadow = shadow(fragPos, i, NdotL);
		fragcolor += (diff + spec) * shadow;
#else
		fragcolor += (diff + spec);
#endif
	}
#endif

	outFragcolor = vec4(fragcolor, 1.0);
}