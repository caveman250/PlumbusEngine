#include "Instance.h"
#include "Helpers.h"

namespace plumbus::vk
{
	InstanceRef Instance::CreateInstance(const std::string& appName, uint32_t appVersion, const std::vector<const char*> enabledLayers, const std::vector<const char*> enabledExtensions)
	{
		InstanceRef instance = std::make_shared<Instance>();

		if (!CheckLayerSupport(enabledLayers))
		{
			Log::Fatal("validation layers requested, but not available!");
		}

		//App Info
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = appName.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(appVersion, 0, 0);
		appInfo.pEngineName = "PlumbusEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(appVersion, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;
		//Create Info

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		Log::Info("required extensions:");

		for (const auto& extension : enabledExtensions)
		{
			Log::Info("\t %s", extension);
		}

		createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
		createInfo.ppEnabledExtensionNames = enabledExtensions.data();

		createInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
		createInfo.ppEnabledLayerNames = enabledLayers.data();

		CHECK_VK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance->m_VulkanInstance));

		return instance;
	}

	bool Instance::CheckLayerSupport(const std::vector<const char*>& enabledLayers)
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		Log::Info("Available Validation Layers:");
		for (VkLayerProperties& props : availableLayers)
		{
			Log::Info("\t%s", props.layerName);
		}

		for (const char* layerName : enabledLayers)
		{
			Log::Info("Looking for validation layer: %s", layerName);
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					Log::Info("Found.");
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				Log::Info("%s not found.", layerName);
				return false;
			}
		}

		return true;
	}

	void Instance::Destroy()
	{
		vkDestroyInstance(m_VulkanInstance, nullptr);
	}

}