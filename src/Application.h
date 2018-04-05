#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include "Scene.h"
#include "Vertex.h"
#include "Model.h"

class Scene;
class Model;
class Application
{
private:
	struct QueueFamilyIndices 
	{
		int m_GraphicsFamily = -1;
		int m_PresentFamily = -1;

		bool Valid() 
		{
			return m_GraphicsFamily >= 0 && m_PresentFamily >= 0;
		}
	};

	struct SwapChainSupportDetails 
	{
		VkSurfaceCapabilitiesKHR m_Capabilities;
		std::vector<VkSurfaceFormatKHR> m_Formats;
		std::vector<VkPresentModeKHR> m_PresentModes;
	};

public:
	Application();

	static void CreateInstance() { m_Instance = new Application(); }
	static Application& Get() { return *m_Instance; }
	void Run();

	VkDevice& GetDevice() { return m_Device; }
	VkDescriptorPool& GetDescriptorPool() { return m_DescriptorPool; }
	VkExtent2D& GetSwapChainExtent() { return m_SwapChainExtent; }
	VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_DescriptorSetLayout; }
	GLFWwindow* GetWindow() { return m_Window; }
	double GetDeltaTime() { return m_DeltaTime; }

	VkResult CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data = nullptr);
	VkResult CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, vk::Buffer *buffer, VkDeviceSize size, void *data = nullptr);
	VkCommandBuffer CreateCommandBuffer();
	void FlushCommandBuffer(VkCommandBuffer commandBuffer);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
	void InitWindow();
	void InitScene();
	void InitVulkan();
	void CreateVulkanInstance();
	void SetupDebugCallback();
	void PickPhysicalDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	void CreateLogicalDevice();
	void CreateSurface();
	void CreateDescriptorSetLayout();
	void CreateSwapChain();
	void RecreateSwapChain();
	void CleanupSwapChain();
	void CreateImageViews();
	void CreateGraphicsPipeline();
	void CreateRenderPass();
	void CreateFrameBuffers();
	void CreateCommandPool();

private:
	void CreateDescriptorPool();
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	void CreateCommandBuffers();
	void CreateSemaphores();

	void MainLoop();
	void UpdateScene();
	void DrawFrame();
	void Cleanup();
	static void OnWindowResized(GLFWwindow* window, int width, int height);

	VkFormat FindDepthFormat();
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	void CreateDepthResources();

	std::vector<const char*> GetRequiredExtensions();
	bool CheckValidationLayerSupport();
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	GLFWwindow* m_Window;
	VkInstance m_VulkanInstance;
	VkDebugReportCallbackEXT m_Callback;
	VkDevice m_Device;
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkQueue m_GraphicsQueue;
	VkSurfaceKHR m_Surface;
	VkQueue m_PresentQueue;
	VkSwapchainKHR m_SwapChain;
	std::vector<VkImage> m_SwapChainImages;
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent;
	std::vector<VkImageView> m_SwapChainImageViews;
	VkRenderPass m_RenderPass;
	VkDescriptorPool m_DescriptorPool;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_GraphicsPipeline;
	std::vector<VkFramebuffer> m_SwapChainFramebuffers;
	VkCommandPool m_CommandPool;
	std::vector<VkCommandBuffer> m_CommandBuffers;
	VkSemaphore m_ImageAvailableSemaphore;
	VkSemaphore m_RenderFinishedSemaphore;

	VkImage m_DepthImage;
	VkDeviceMemory m_DepthImageMemory;
	VkImageView m_DepthImageView;

	//Temp
	VkShaderModule m_VertShaderModule;
	VkShaderModule m_FragShaderModule;

	static Application* m_Instance;
	std::vector<Model> m_Models;
	Scene* m_Scene;
	double m_DeltaTime;
	double m_lastUpdateTime;

};