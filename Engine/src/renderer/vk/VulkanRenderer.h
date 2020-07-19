#pragma once

#include "vulkan/vulkan.h"
#include "renderer/base/Renderer.h"
#include "renderer/vk/Mesh.h"
#include "renderer/vk/Device.h"
#include "renderer/vk/FrameBuffer.h"
#include "renderer/vk/Window.h"
#include "renderer/vk/SwapChain.h"

namespace plumbus
{
	class ImGUIImpl;

	namespace vk
	{
		class Mesh;
		class Instance;

		class VulkanRenderer : public base::Renderer
		{
		private:
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

			struct PointLightBufferInfo
			{
				glm::vec4 m_Position;
				glm::vec3 m_Colour;
				float m_Radius;
			};

			struct DirectionalLightBufferInfo
			{
				glm::vec3 m_Colour;
				glm::vec3 m_Direction;
			};

			static constexpr size_t MAX_POINT_LIGHTS = 6;
			static constexpr size_t MAX_DIRECTIONAL_LIGHTS = 1;
			struct UniformBufferLights
			{
				glm::vec4 m_ViewPos;
				PointLightBufferInfo m_PointLights[MAX_POINT_LIGHTS];
				DirectionalLightBufferInfo m_DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
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
			static VulkanRenderer* Get();

			virtual void Init() override;
			virtual void Cleanup() override;

			virtual void DrawFrame() override;
			virtual bool WindowShouldClose() override;

			virtual void AwaitIdle() override;

			virtual void OnModelAddedToScene() override;
			virtual void OnModelRemovedFromScene() override;

			std::shared_ptr<vk::Instance> GetInstance() { return m_Instance; }
			std::shared_ptr<vk::Device> GetDevice() { return m_Device; }
			SwapChainRef GetSwapChain() { return m_SwapChain; }

			vk::Window* GetVulkanWindow() { return static_cast<vk::Window*>(m_Window); }

			VkDescriptorPool& GetDescriptorPool() { return m_DescriptorPool; }
			VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_DescriptorSetLayout; }
			GLFWwindow* GetWindow() { return static_cast<vk::Window*>(m_Window)->GetWindow(); }
			VkPipelineShaderStageCreateInfo LoadShader(std::string fileName, VkShaderStageFlagBits stage);

			std::vector<const char*> GetRequiredDeviceExtensions();
			std::vector<const char*> GetRequiredInstanceExtensions();
			std::vector<const char*> GetRequiredValidationLayers();

			VkDescriptorSetAllocateInfo GetDescriptorSetAllocateInfo();

			FrameBufferRef GetOffscreenFramebuffer() { return m_OffscreenFrameBuffer; }
			const CommandBufferRef& GetOffscreenCommandBuffer() { return m_OffScreenCmdBuffer; }
			VkPipelineCache& GetPipelineCache() { return m_PipelineCache; }

			//used for imgui
			vk::Texture m_OutputTexture;

			VkFormat FindDepthFormat();

			ImGUIImpl* GetImGui() { return m_ImGui; }

		private:
			void InitVulkan();
			void SetupDebugCallback();
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
			void RecreateSwapChain();
			void UpdateUniformBuffersScreen();
			void UpdateLightsUniformBuffer();
			void SetupImGui();

			static void OnWindowResized(GLFWwindow* window, int width, int height);

			VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

			VkShaderModule CreateShaderModule(const std::vector<char>& code);

			InstanceRef m_Instance;
			VkDebugReportCallbackEXT m_Callback;
			DeviceRef m_Device;
			SwapChainRef m_SwapChain;
			
			VkDescriptorPool m_DescriptorPool;
			VkDescriptorSetLayout m_DescriptorSetLayout;
			VkDescriptorSet m_OutputDescriptorSet;
			PipelineLayouts m_PipelineLayouts;
			Pipelines m_Pipelines;

			VkSemaphore m_OffscreenSemaphore;
			VkSemaphore m_OutputSemaphore;

			FrameBufferRef m_OffscreenFrameBuffer;
			FrameBufferRef m_OutputFrameBuffer;
			UniformBuffers m_UniformBuffers;
			UniformBufferVert m_VertUBO;
			UniformBufferLights m_LightsUBO;

			VkPipelineCache m_PipelineCache;
			CommandBufferRef m_OffScreenCmdBuffer;
			CommandBufferRef m_OutputCmdBuffer;

			vk::Mesh m_ScreenQuad;
			std::vector<VkShaderModule> m_ShaderModules;

			plumbus::ImGUIImpl* m_ImGui = nullptr;
		};
	}
}