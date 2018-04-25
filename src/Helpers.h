#pragma once
#include <vector>
#include <fstream>
#include <assert.h>
#include <iostream>
#include "vulkan/vulkan.h"
#include "imgui_impl/Log.h"

#ifndef CHECK_VK_RESULT
#define CHECK_VK_RESULT(f)																											\
{																																	\
	VkResult res = (f);																												\
	if (res != VK_SUCCESS)																											\
	{																																\
		Log::Info("Fatal : VkResult is \"" , ErrorString(res) , "\" in " , __FILE__ , " at line " , __LINE__ );						\
		assert(res == VK_SUCCESS);																									\
	}																																\
}
#endif

std::string ErrorString(VkResult errorCode);

class Helpers
{
public:
	static std::vector<char> ReadFile(const std::string& filename);
};