#include "plumbus.h"
#include "Helpers.h"

#if PL_PLATFORM_WINDOWS
#include "platform/windows/Platform.h"
#else
#include "platform/android/Platform.h"
#endif

std::string ErrorString(VkResult errorCode)
{
	switch (errorCode)
	{
#define STR(r) case VK_ ##r: return #r
		STR(NOT_READY);
		STR(TIMEOUT);
		STR(EVENT_SET);
		STR(EVENT_RESET);
		STR(INCOMPLETE);
		STR(ERROR_OUT_OF_HOST_MEMORY);
		STR(ERROR_OUT_OF_DEVICE_MEMORY);
		STR(ERROR_INITIALIZATION_FAILED);
		STR(ERROR_DEVICE_LOST);
		STR(ERROR_MEMORY_MAP_FAILED);
		STR(ERROR_LAYER_NOT_PRESENT);
		STR(ERROR_EXTENSION_NOT_PRESENT);
		STR(ERROR_FEATURE_NOT_PRESENT);
		STR(ERROR_INCOMPATIBLE_DRIVER);
		STR(ERROR_TOO_MANY_OBJECTS);
		STR(ERROR_FORMAT_NOT_SUPPORTED);
		STR(ERROR_SURFACE_LOST_KHR);
		STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
		STR(SUBOPTIMAL_KHR);
		STR(ERROR_OUT_OF_DATE_KHR);
		STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
		STR(ERROR_VALIDATION_FAILED_EXT);
		STR(ERROR_INVALID_SHADER_NV);
		STR(ERROR_OUT_OF_POOL_MEMORY);
#undef STR
	default:
		return "UNKNOWN_ERROR";
	}
}

std::string DeviceTypeString(VkPhysicalDeviceType deviceType)
{
	switch (deviceType)
	{
#define STR(r) case r: return #r
		STR(VK_PHYSICAL_DEVICE_TYPE_OTHER);
		STR(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
		STR(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
		STR(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
		STR(VK_PHYSICAL_DEVICE_TYPE_CPU);
#undef STR
		default:
			return "UNKNOWN";
	}
}

#if PL_PLATFORM_ANDROID
#include "android_native_app_glue.h"
extern android_app* Android_application;
#endif

std::vector<char> Helpers::ReadBinaryFile(const std::string& filename)
{
#if PL_PLATFORM_ANDROID

	AAssetManager * mgr = Android_application->activity->assetManager;
	if(AAsset* asset = AAssetManager_open(mgr, (plumbus::Platform::GetAssetsPath() + filename).c_str(), AASSET_MODE_BUFFER))
	{
		std::vector<char> buffer;
		//holds size of searched file
		off64_t length = AAsset_getLength64(asset);
		buffer.resize(length);

		//read data chunk
		if(PL_VERIFY(AAsset_read(asset, buffer.data(), length) > 0))
		{
			return buffer;
		}
	}

	return std::vector<char>();
#else
	std::ifstream file(plumbus::Platform::GetAssetsPath() + filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		plumbus::Log::Error("Helpers::ReadFile: failed to open file %s!", filename.c_str());
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
#endif
}

std::string Helpers::ReadTextFile(const std::string& filename)
{
#if PL_PLATFORM_ANDROID

	AAssetManager * mgr = Android_application->activity->assetManager;
	if(AAsset* asset = AAssetManager_open(mgr, (plumbus::Platform::GetAssetsPath() + filename).c_str(), AASSET_MODE_BUFFER))
	{
		std::vector<char> buffer;
		//holds size of searched file
		off64_t length = AAsset_getLength64(asset);
		buffer.resize(length);

		//read data chunk
		if(PL_VERIFY(AAsset_read(asset, buffer.data(), length) > 0))
		{
			return std::string((const char*)buffer.data());
		}
	}

	return std::string();
#else
	std::ifstream file(plumbus::Platform::GetAssetsPath() + filename);

	if (!file.is_open())
	{
		plumbus::Log::Error("Helpers::ReadFile: failed to open file %s!", filename.c_str());
	}

	return std::string((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
#endif
}

std::string Helpers::FormatStr(const char* fmt, ...)
{
	char buffer[256];

	va_list args;
	va_start(args, fmt);
	const auto formatVal = std::vsnprintf(buffer, sizeof buffer, fmt, args);
	va_end(args);

	if (formatVal < 0)
	{
		return {};
	}

	const size_t len = formatVal;
	if (len < sizeof buffer)
	{
		return { buffer, len };
	}
	
	std::string str(len, '\0');
	va_start(args, fmt);
	std::vsnprintf(str.data(), len + 1, fmt, args);
	va_end(args);

	return str;
}
