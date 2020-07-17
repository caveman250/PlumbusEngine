#pragma once
#include "plumbus.h"
#include "vulkan/vulkan.h"

namespace plumbus::vk
{
	class Instance
	{
	public:
		static std::shared_ptr<Instance> CreateInstance(const std::string& appName, uint32_t appVersion, const std::vector<const char*> enabledLayers, const std::vector<const char*> enabledExtensions);

		VkInstance GetInstance() { return m_VulkanInstance; }
		void Destroy();

	private:
		static bool CheckLayerSupport(const std::vector<const char*>& enabledLayers);

		VkInstance m_VulkanInstance;
	};
}