#pragma once

#include "vulkan/vulkan.h"
#include "renderer/base/Renderer.h"
#include "renderer/vk/Mesh.h"
#include "renderer/vk/Device.h"
#include "renderer/vk/FrameBuffer.h"
#include "renderer/vk/Window.h"

namespace plumbus
{
	class ImGUIImpl;

	namespace vk
	{
		class Mesh;

		class VulkanRenderer : public base::Renderer
		{
		private:
			struct SwapChainSupportDetails
			{
				VkSurfaceCapabilitiesKHR m_Capabilities;
				std::vector<VkSurfaceFormatKHR> m_Formats;
				std::vector<VkPresentModeKHR> m_PresentModes;
			};

			struct UniformBuffers
			{
				vk::Buffer m_VertFullScreen;
				vk::Buffer m_FragLights;
			};

			struct UniformBufferVert
			{
				glm::mat4 m_Projection;
				glm::mat4 m_Model;
				glm::mat4 m_View;
				glm::vec4 m_InstancePos[3];
			};

			struct LightBufferInfo
			{
				glm::vec4 m_Position;
				glm::vec3 m_Colour;
				float m_Radius;
			};

			static const size_t MAX_LIGHTS = 50;
			struct UniformBufferLights
			{
				LightBufferInfo m_Lights[MAX_LIGHTS];
				glm::vec4 m_ViewPos;
			};

			struct PipelineLayouts
			{
				VkPipelineLayout m_Deferred;
				VkPipelineLayout m_Offscreen;
			};

			struct Pipelines
			{
				VkPipeline m_Deferred;
				VkPipeline m_Offscreen;
				//VkPipeline m_Debug;
				VkPipeline m_Output;
			};

		public:

			virtual void Init() override;
			virtual void Cleanup() override;

			virtual void DrawFrame() override;
			virtual bool WindowShouldClose() override;

			virtual void AwaitIdle() override;

			virtual void OnModelAddedToScene() override;
			virtual void OnModelRemovedFromScene() override;

			vk::VulkanDevice* GetVulkanDevice() { return m_VulkanDevice; }
			VkDescriptorPool& GetDescriptorPool() { return m_DescriptorPool; }
			VkExtent2D& GetSwapChainExtent() { return m_SwapChainExtent; }
			VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_DescriptorSetLayout; }
			GLFWwindow* GetWindow() { return static_cast<vk::Window*>(m_Window)->GetWindow(); }
			VkPipelineShaderStageCreateInfo LoadShader(std::string fileName, VkShaderStageFlagBits stage);
			VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }

			VkDescriptorSetAllocateInfo GetDescriptorSetAllocateInfo();

			vk::FrameBuffer* GetOffscreenFramebuffer() { return m_OffscreenFrameBuffer; }
			VkPipelineCache& GetPipelineCache() { return m_PipelineCache; }

			//used for imgui
			vk::Texture m_OutputTexture;

			VkFormat FindDepthFormat();

			ImGUIImpl* GetImGui() { return m_ImGui; }

		private:
			void InitVulkan();
			void CreateVulkanInstance();
			void SetupDebugCallback();
			void PickPhysicalDevice();
			bool IsDeviceSuitable(VkPhysicalDevice device);
			bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
			void CreatePipelineCache();
			void GenerateQuads();
			
			void CreateUniformBuffers();
			void InitLightsVBO();
			void CreateDescriptorSetLayout();
			void CreatePipelines();
			void CreateDescriptorPool();
			void CreateDescriptorSet();
			void BuildImguiCommandBuffer(int index);
			void BuildDefferedCommandBuffer();
			void BuildOutputFrameBuffer();
			void CreateSwapChain();
			void RecreateSwapChain();
			void CleanupSwapChain();
			void CreateImageViews();
			void CreateRenderPass();
			void UpdateUniformBuffersScreen();
			void UpdateLightsUniformBuffer();
			void CreateDepthResources();
			void CreateFrameBuffers();
			void CreateCommandBuffers();
			void CreateSemaphores();
			void SetupImGui();

			static void OnWindowResized(GLFWwindow* window, int width, int height);

			std::vector<const char*> GetRequiredExtensions();
			bool CheckValidationLayerSupport();

			VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

			SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
			VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
			VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
			VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

			VkShaderModule CreateShaderModule(const std::vector<char>& code);

			VkInstance m_VulkanInstance;
			VkDebugReportCallbackEXT m_Callback;
			vk::VulkanDevice* m_VulkanDevice;
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
			VkDescriptorSet m_OutputDescriptorSet;
			PipelineLayouts m_PipelineLayouts;
			Pipelines m_Pipelines;
			std::vector<VkFramebuffer> m_Framebuffers;
			std::vector<VkCommandBuffer> m_CommandBuffers;
			VkSemaphore m_ImageAvailableSemaphore;
			VkSemaphore m_RenderFinishedSemaphore;
			VkSemaphore m_OffscreenSemaphore;
			VkSemaphore m_OutputSemaphore;

			vk::FrameBuffer* m_OffscreenFrameBuffer;
			vk::FrameBuffer* m_OutputFrameBuffer;
			UniformBuffers m_UniformBuffers;
			UniformBufferVert m_VertUBO;
			UniformBufferLights m_LightsUBO;
			VkPipelineCache m_PipelineCache;
			VkCommandBuffer m_OffScreenCmdBuffer = VK_NULL_HANDLE;
			VkCommandBuffer m_OutputCmdBuffer = VK_NULL_HANDLE;
			VkImage m_DepthImage;
			VkDeviceMemory m_DepthImageMemory;
			VkImageView m_DepthImageView;
			vk::Mesh m_ScreenQuad;
			std::vector<VkShaderModule> m_ShaderModules;

			plumbus::ImGUIImpl* m_ImGui = nullptr;
		};
	}
}