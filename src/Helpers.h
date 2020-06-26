#pragma once
#include "plumbus.h"
#include "vulkan/vulkan.h"

#ifndef CHECK_VK_RESULT
#define CHECK_VK_RESULT(f)																											\
{																																	\
	VkResult res = (f);																												\
	if (res != VK_SUCCESS)																											\
	{																																\
		Log::Fatal("VkResult is \"%s\" in %s at line %d", ErrorString(res).c_str(), __FILE__ , __LINE__ );							\
	}																																\
}																																	
#endif

std::string ErrorString(VkResult errorCode);

class Helpers
{
public:
	static std::vector<char> ReadFile(const std::string& filename);
};
