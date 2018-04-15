#pragma once
#include <vector>
#include <fstream>
#include <assert.h>
#include <iostream>
#include "vulkan/vulkan.h"

#ifndef CHECK_VK_RESULT
#define CHECK_VK_RESULT(f)																											\
{																																	\
	VkResult res = (f);																												\
	if (res != VK_SUCCESS)																											\
	{																																\
		std::cout << "Fatal : VkResult is \"" << ErrorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl;	\
		assert(res == VK_SUCCESS);																									\
	}																																\
}
#endif

std::string ErrorString(VkResult errorCode);

class Helpers
{
public:
	static std::vector<char> ReadFile(const std::string& filename);

	static void LogInfo(std::string msg);

	static void LogFatal(std::string msg);
};