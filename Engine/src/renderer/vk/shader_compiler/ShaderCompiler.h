#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>


#include "glslang/glslang/Public/ShaderLang.h"

namespace plumbus::vk::shaders
{
	class ShaderSettings;

	class ShaderCompiler
	{
	public:
		static std::string ApplyShaderSettings(std::string glslShader, ShaderSettings settings);
		static bool CompileShader(std::string glslShader, VkShaderStageFlagBits shaderStage, std::vector<unsigned int>& outSpirv);
	};
}
