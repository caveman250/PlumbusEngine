#include "plumbus.h"
#include "Device.h"
#include "Helpers.h"
#include "Buffer.h"
#include "VulkanRenderer.h"
#include "Instance.h"

namespace plumbus::vk
{
	std::shared_ptr<plumbus::vk::Device> Device::CreateDevice()
	{
		return std::make_shared<Device>();
	}


	Device::Device()
	{
		m_Surface = VulkanRenderer::Get()->GetWindow()->GetSurface();
		PickPhysicalDevice();
		CreateLogicalDevice(VulkanRenderer::Get()->GetRequiredDeviceExtensions(), VulkanRenderer::Get()->GetRequiredValidationLayers(), true);
		vkGetDeviceQueue(m_Device, GetQueueFamilyIndices().m_GraphicsFamily, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, GetQueueFamilyIndices().m_PresentFamily, 0, &m_PresentQueue);
	}

	Device::~Device()
	{
		if (m_Device)
		{
			vkDestroyDevice(m_Device, nullptr);
		}
	}

	uint32_t Device::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i))
				&& (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		Log::Fatal("failed to find suitable memory type!");
		return -1;
	}

	Device::QueueFamilyIndices Device::FindQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		//get the count
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		//now we know the size, fill our properties array
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);

			if (queueFamily.queueCount > 0 && presentSupport)
			{
				Log::Info("present family queue index: %i" , i);
				indices.m_PresentFamily = i;
			}

			//pretty basic, if the family has more than one queue (read: thread),
			//and supports graphics (literally, any graphics at all, vulkan supports devices that cant draw anything to the screen)
			//then select it as our queue family by storing the index
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				Log::Info("graphics family queue index: %i" , i);
				indices.m_GraphicsFamily = i;
			}

			//currently just returns valid if non zero, 
			//more complex family validation here if ever required.
			if (indices.Valid())
			{
				break;
			}

			++i;
		}

		//note that m_Indices could be expanded to return multiple valid families
		return indices;
	}

	void Device::CreateLogicalDevice(std::vector<const char*> deviceExtensions, const std::vector<const char*> validationLayers, bool enableValidationLayers)
	{
		Log::Info("Create logical device.");
		//find a valid queue family(s?)
		m_Indices = FindQueueFamilies(m_PhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> uniqueQueueFamilies = { m_Indices.m_GraphicsFamily, m_Indices.m_PresentFamily };

		float queuePriority = 1.0f;
		for (int queueFamily : uniqueQueueFamilies)
		{
			//we have a "queue" but now we need to create a structure to communicate info about it to vulkan.
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;

			//Vulkan lets you assign priorities to queues to influence work sheduling
			//eg: work should always be sent to queue1 (score 1.0) over queue2 (score 0.5) when possible
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}


		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.textureCompressionBC = VK_TRUE;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		//add device specific extensions here if needed
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers)
		{
			Log::Info("\tAdding validation layers to logical device");
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		CHECK_VK_RESULT(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device));

		m_CommandPool = CreateCommandPool();

		//i now have a logical device and a graphics queue  with vulkan wrappers (m_Device) (m_GraphicsQueue) that let me control them
	}

	VkResult Device::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, vk::Buffer *buffer, VkDeviceSize size, void *data /*= nullptr*/)
	{
		buffer->m_Device = m_Device;

		// Create the buffer handle
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.usage = usageFlags;
		bufferCreateInfo.size = size;
		if (vkCreateBuffer(m_Device, &bufferCreateInfo, nullptr, &buffer->m_Buffer) != VK_SUCCESS)
			Log::Fatal("Failed to create buffer");

		// Create the memory backing up the buffer handle
		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkGetBufferMemoryRequirements(m_Device, buffer->m_Buffer, &memReqs);
		memAllocInfo.allocationSize = memReqs.size;
		// Find a memory type index that fits the properties of the buffer
		memAllocInfo.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
		if (vkAllocateMemory(m_Device, &memAllocInfo, nullptr, &buffer->m_Memory) != VK_SUCCESS)
			Log::Fatal("Failed to allocate buffer memory");

		buffer->m_Alignment = memReqs.alignment;
		buffer->m_Size = size;
		buffer->m_UsageFlags = usageFlags;
		buffer->m_MemoryPropertyFlags = memoryPropertyFlags;

		// If a pointer to the buffer data has been passed, map the buffer and copy over the data
		if (data != nullptr)
		{
			if (buffer->Map() != VK_SUCCESS)
				Log::Fatal("failed to map buffer");

			memcpy(buffer->m_Mapped, data, size);
			buffer->Unmap();
		}

		// Initialize a default descriptor that covers the whole buffer size
		buffer->SetupDescriptor();

		// Attach the memory to the buffer object
		return buffer->Bind();
	}

	VkResult Device::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data /*= nullptr*/)
	{
		// Create the buffer handle
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.usage = usageFlags;
		bufferCreateInfo.size = size;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (vkCreateBuffer(m_Device, &bufferCreateInfo, nullptr, buffer) != VK_SUCCESS)
			Log::Fatal("Failed to create buffer");

		// Create the memory backing up the buffer handle
		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vkGetBufferMemoryRequirements(m_Device, *buffer, &memReqs);
		memAllocInfo.allocationSize = memReqs.size;
		// Find a memory type index that fits the properties of the buffer
		memAllocInfo.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
		if (vkAllocateMemory(m_Device, &memAllocInfo, nullptr, memory) != VK_SUCCESS)
			Log::Fatal("Failed to allocate buffer memory");

		// If a pointer to the buffer data has been passed, map the buffer and copy over the data
		if (data != nullptr)
		{
			void *mapped;
			if (vkMapMemory(m_Device, *memory, 0, size, 0, &mapped) != VK_SUCCESS)
				Log::Fatal("Failed to map buffer data");
			memcpy(mapped, data, size);
			// If host coherency hasn't been requested, do a manual flush to make writes visible
			if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			{
				VkMappedMemoryRange mappedMemoryRange{};
				mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mappedMemoryRange.memory = *memory;
				mappedMemoryRange.offset = 0;
				mappedMemoryRange.size = size;
				vkFlushMappedMemoryRanges(m_Device, 1, &mappedMemoryRange);
			}
			vkUnmapMemory(m_Device, *memory);
		}

		// Attach the memory to the buffer object
		if (vkBindBufferMemory(m_Device, *buffer, *memory, 0) != VK_SUCCESS)
			Log::Fatal("failed to bind buffer memory");

		return VK_SUCCESS;
	}

	VkCommandPool Device::CreateCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_PhysicalDevice);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.m_GraphicsFamily;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

		VkCommandPool commandPool;

		if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		{
			Log::Fatal("failed to create command pool!");
		}

		return commandPool;
	}

	VkCommandBuffer Device::CreateCommandBuffer(bool begin)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

		if (begin)
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);
		}

		return commandBuffer;
	}

	void Device::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);

		vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
	}

	void Device::PickPhysicalDevice()
	{
		VulkanRenderer* renderer = VulkanRenderer::Get();
		InstanceRef instance = renderer->GetInstance();

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance->GetVulkanInstance(), &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			Log::Fatal("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance->GetVulkanInstance(), &deviceCount, devices.data());

		Log::Info("Found %i device(s)", devices.size());
		for (const auto& device : devices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			Log::Info(deviceProperties.deviceName);
		}

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device))
			{
				m_PhysicalDevice = device;
				break;
			}
		}

		if (m_PhysicalDevice == VK_NULL_HANDLE)
		{
			Log::Fatal("failed to find a suitable GPU!");
		}
	}

	bool Device::IsDeviceSuitable(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			Log::Info("Device: %s Invalid! type is not a discrete gpu.", deviceProperties.deviceName);
			return false;
		}

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		// if (!deviceFeatures.geometryShader)
		// {
		// 	Log::Info("Device: %s Invalid! no geometry shader support.", deviceProperties.deviceName);
		// 	return false;
		// }

		if (!CheckDeviceExtensionSupport(device))
		{
			Log::Info("Device: %s Invalid! missing required extension support", deviceProperties.deviceName);
			return false;
		}

		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		if (swapChainSupport.m_Formats.empty() || swapChainSupport.m_PresentModes.empty())
		{
			Log::Info("Device: %s Invalid! missing swap chain support.", deviceProperties.deviceName);
			return false;
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
		if (!supportedFeatures.samplerAnisotropy)
		{
			Log::Info("Device: %s Invalid! missing missing Anisotropy sampler support.", deviceProperties.deviceName);
			return false;
		}

		return true;
	}

	bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());


		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		Log::Info("Device: %s: Available extensions:", deviceProperties.deviceName);
		for (VkExtensionProperties prop : availableExtensions)
		{
			Log::Info("\t %s", prop.extensionName);
		}

		std::vector<const char*> reqDeviceExtensions = VulkanRenderer::Get()->GetRequiredDeviceExtensions();
		std::set<std::string> requiredExtensions(reqDeviceExtensions.begin(), reqDeviceExtensions.end());
		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	Device::SwapChainSupportDetails Device::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		VulkanRenderer* renderer = VulkanRenderer::Get();

		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.m_Capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.m_Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.m_Formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.m_PresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.m_PresentModes.data());
		}

		return details;
	}
}