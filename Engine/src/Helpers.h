#pragma once
#include "plumbus.h"
#include "vulkan/vulkan.h"

#ifndef CHECK_VK_RESULT
#define CHECK_VK_RESULT(f)																											\
{																																	\
	VkResult res = (f);																												\
	PL_ASSERT(res == VK_SUCCESS, "VkResult is \"%s\" in %s at line %d\nExpr: %s", ErrorString(res).c_str(), __FILE__ , __LINE__, #f);\
	if(res != VK_SUCCESS) { plumbus::Log::Error( "VkResult is \"%s\" in %s at line %d\nExpr: %s", ErrorString(res).c_str(), __FILE__ , __LINE__, #f); }\
}																																	
#endif

std::string ErrorString(VkResult errorCode);
std::string DeviceTypeString(VkPhysicalDeviceType deviceType);

class Helpers
{
public:
	static std::vector<char> ReadFile(const std::string& filename);
};
