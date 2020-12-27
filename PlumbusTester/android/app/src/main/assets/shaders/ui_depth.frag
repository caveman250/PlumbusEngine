layout (binding = 0) uniform sampler2D imageSampler;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main() 
{
    float r = texture(imageSampler, inUV).r;
	outColor = vec4(r, r, r, 1.0f);
}