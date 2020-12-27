layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in uint inColor;

layout (push_constant) uniform PushConstants {
	vec2 scale;
	vec2 translate;
} pushConstants;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;

out gl_PerVertex 
{
	vec4 gl_Position;   
};

vec4 UnPackRGBA(uint color)
{   
	//katesmellz
	float s = 1.0f/255.0f;
	float r = (color & 0xff) * s;
	float g = ((color >> 8) & 0xff) * s;
	float b = ((color >> 16) & 0xff) * s;
	float a = ((color >> 24) & 0xff) * s;
	return vec4(r, g, b, a);
}

void main() 
{
	outUV = inUV;
	outColor = UnPackRGBA(inColor);
	gl_Position = vec4(inPos * pushConstants.scale + pushConstants.translate, 0.0, 1.0);
}