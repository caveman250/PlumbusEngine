#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <array>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

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

	struct UniformBufferObject 
	{
		glm::mat4 m_Model;
		glm::mat4 m_View;
		glm::mat4 m_Proj;
	};

public:
	struct Vertex 
	{
		glm::vec3 m_Pos;
		glm::vec3 m_Colour;
		glm::vec2 m_TexCoord;

		static VkVertexInputBindingDescription GetBindingDescription() 
		{
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() 
		{
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, m_Pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, m_Colour);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, m_TexCoord);

			return attributeDescriptions;
		}

		bool operator==(const Vertex& other) const 
		{
			return m_Pos == other.m_Pos 
				&& m_Colour == other.m_Colour 
				&& m_TexCoord == other.m_TexCoord;
		}
	};

public:
	void Run();

private:
	void InitWindow();
	void InitVulkan();
	void CreateInstance();
	void SetupDebugCallback();
	void PickPhysicalDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	void CreateLogicalDevice();
	void CreateSurface();

	void CreateSwapChain();
	void RecreateSwapChain();
	void CleanupSwapChain();

	void CreateImageViews();
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void CreateDescriptorSetLayout();
	void CreateGraphicsPipeline();
	void CreateRenderPass();
	void CreateFrameBuffers();
	void CreateCommandPool();

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void CreateUniformBuffer();
	void CreateDescriptorPool();
	void CreateDescriptorSet();
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

	void CreateCommandBuffers();
	void CreateSemaphores();

	void CreateDepthResources();

	void CreateTextureImage();
	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void CreateTextureImageView();
	void CreateTextureSampler();

	void MainLoop();
	void DrawFrame();
	void UpdateUniformBuffer();

	void Cleanup();

	static void OnWindowResized(GLFWwindow* window, int width, int height);

	VkFormat FindDepthFormat();
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	bool HasStencilComponent(VkFormat format);

	std::vector<const char*> GetRequiredExtensions();
	bool CheckValidationLayerSupport();

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	void LoadModel();

	GLFWwindow* m_Window;
	VkInstance m_Instance;
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
	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_GraphicsPipeline;
	std::vector<VkFramebuffer> m_SwapChainFramebuffers;
	VkCommandPool m_CommandPool;
	std::vector<VkCommandBuffer> m_CommandBuffers;

	VkSemaphore m_ImageAvailableSemaphore;
	VkSemaphore m_RenderFinishedSemaphore;


	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexBufferMemory;
	VkBuffer m_IndexBuffer;
	VkDeviceMemory m_IndexBufferMemory;
	VkBuffer m_UniformBuffer;
	VkDeviceMemory m_UniformBufferMemory;

	VkImage m_TextureImage;
	VkDeviceMemory m_TextureImageMemory;
	VkImageView m_TextureImageView;
	VkSampler m_TextureSampler;

	VkImage m_DepthImage;
	VkDeviceMemory m_DepthImageMemory;
	VkImageView m_DepthImageView;

	VkDescriptorPool m_DescriptorPool;
	VkDescriptorSet m_DescriptorSet;

	//Temp
	VkShaderModule m_VertShaderModule;
	VkShaderModule m_FragShaderModule;

};