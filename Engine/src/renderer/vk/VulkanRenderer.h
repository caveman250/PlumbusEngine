#pragma once

#include "vulkan/vulkan.h"
#include "renderer/vk/Mesh.h"
#include "renderer/vk/Device.h"
#include "renderer/vk/FrameBuffer.h"
#include "renderer/vk/Window.h"
#include "renderer/vk/SwapChain.h"
#include "DescriptorSetLayout.h"

namespace plumbus
{
	class ImGUIImpl;

	namespace vk
	{
		class Mesh;
		class Instance;

		class VulkanRenderer
		{
		public:
			static VulkanRenderer* Get();

			virtual void Init();
			virtual void Cleanup();

			virtual void DrawFrame();
			virtual bool WindowShouldClose();

			virtual void AwaitIdle();

			virtual void OnModelAddedToScene();
			virtual void OnModelRemovedFromScene();

			InstanceRef GetInstance() { return m_Instance; }
			DeviceRef GetDevice() { return m_Device; }
			SwapChainRef GetSwapChain() { return m_SwapChain; }

			Window* GetWindow() { return m_Window; }

			VkPipelineShaderStageCreateInfo LoadShader(std::string fileName, VkShaderStageFlagBits stage, std::vector<DescriptorBinding>& outBindingInfo, int& numOutputs);

			std::vector<const char*> GetRequiredDeviceExtensions();
			std::vector<const char*> GetRequiredInstanceExtensions();
			std::vector<const char*> GetRequiredValidationLayers();

			FrameBufferRef GetOffscreenFramebuffer() { return m_OffscreenFrameBuffer; }
			const CommandBufferRef& GetOffscreenCommandBuffer() { return m_OffScreenCmdBuffer; }
			FrameBufferRef GetOutputFramebuffer() { return m_OutputFrameBuffer; }
			const CommandBufferRef& GetOutputCommandBuffer() { return m_OutputCmdBuffer; }

			//TODO: this shouldnt be a big global thing, create pools where relavnt.
			const DescriptorPoolRef& GetDescriptorPool() { return m_DescriptorPool; }

			const PipelineCacheRef& GetPipelineCache() { return m_PipelineCache; }

			VkFormat FindDepthFormat();

			ImGUIImpl* GetImGui() { return m_ImGui; }

		private:
			void InitVulkan();
			void SetupDebugCallback();
			void GenerateQuads();
			
			void CreateUniformBuffers();
			void InitLightsVBO();
			void BuildImguiCommandBuffer(int index);
			void BuildDefferedCommandBuffer();
			void BuildOutputCommandBuffer();
			void RecreateSwapChain();
			void UpdateLightsUniformBuffer();
			void SetupImGui();

			static void OnWindowResized(GLFWwindow* window, int width, int height);

			VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

			VkShaderModule CreateShaderModule(const std::vector<char>& code);

			Window* m_Window;

			InstanceRef m_Instance;
			VkDebugReportCallbackEXT m_Callback;
			DeviceRef m_Device;
			SwapChainRef m_SwapChain;
			
			DescriptorPoolRef m_DescriptorPool;

			PipelineLayoutRef m_DeferredPipelineLayout;
			PipelineLayoutRef m_OffscreenPipelineLayout;
			PipelineRef m_DeferredPipeline;
			PipelineRef m_OffscreenPipeline;

			MaterialRef m_OutputMaterial;
			MaterialInstanceRef m_OutputMaterialInstance;
			PipelineRef m_OutputPipeline;

			VkSemaphore m_OffscreenSemaphore;
			VkSemaphore m_OutputSemaphore;

			FrameBufferRef m_OffscreenFrameBuffer;
			FrameBufferRef m_OutputFrameBuffer;
			vk::Buffer m_FragLights;
			
			PipelineCacheRef m_PipelineCache;
			CommandBufferRef m_OffScreenCmdBuffer;
			CommandBufferRef m_OutputCmdBuffer;

			vk::Mesh m_ScreenQuad;
			std::vector<VkShaderModule> m_ShaderModules;

			plumbus::ImGUIImpl* m_ImGui = nullptr;

			//Lights
			struct PointLightBufferInfo
			{
				glm::vec4 m_Position;
				glm::vec3 m_Colour;
				float m_Radius;
			};

			struct DirectionalLightBufferInfo
			{
				glm::vec4 m_Colour;
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

			UniformBufferLights m_LightsUBO;
		};
	}
}