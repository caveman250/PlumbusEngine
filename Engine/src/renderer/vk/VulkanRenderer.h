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

		static constexpr size_t MAX_FRAMES_IN_FLIGHT = 3;

		class VulkanRenderer
		{
		public:
			static VulkanRenderer* Get();

			void Init(std::string appName);
			void Cleanup();

			void DrawFrame();
			void AwaitIdle();

			bool WindowShouldClose();

			VkPipelineShaderStageCreateInfo LoadShader(std::string fileName, VkShaderStageFlagBits stage, std::vector<DescriptorBinding>& outBindingInfo, int& numOutputs);

			InstanceRef GetInstance() { return m_Instance; }
			DeviceRef GetDevice() { return m_Device; }
			SwapChainRef GetSwapChain() { return m_SwapChain; }
			Window* GetWindow() { return m_Window; }
			const DescriptorPoolRef& GetDescriptorPool() { return m_DescriptorPool; }
			const PipelineCacheRef& GetPipelineCache() { return m_PipelineCache; }
			VkFormat GetDepthFormat();

			FrameBufferRef GetDeferredFramebuffer(uint32_t imageIndex) { return m_DeferredFrameBuffers[imageIndex]; }
			const CommandBufferRef& GetDeferredCommandBuffer(uint32_t imageIndex) { return m_DeferredCommandBuffers[imageIndex]; }
#if !PL_DIST
			FrameBufferRef GetDeferredOutputFramebuffer(uint32_t imageIndex) { return m_DeferredOutputFrameBuffers[imageIndex]; }
			const CommandBufferRef& GetDeferredOutputCommandBuffer(uint32_t imageIndex) { return m_DeferredOutputCommandBuffers[imageIndex]; }

			ImGUIImpl* GetImGui() { return m_ImGui; }
#endif

			std::vector<const char*> GetRequiredDeviceExtensions();
			std::vector<const char*> GetRequiredInstanceExtensions();
			std::vector<const char*> GetRequiredValidationLayers();

		private:
			void InitVulkan();
#if !PL_DIST
			void SetupDebugCallback();
#endif
			void GenerateFullscreenQuad();
			void CreateLightsUniformBuffers();
			void BuildPresentCommandBuffer(uint32_t imageIndex, int currFrame);
			void BuildDefferedCommandBuffer(uint32_t imageIndex);
#if !PL_DIST
			void SetupImGui();
			void BuildDeferredOutputCommandBuffer(uint32_t imageIndex);
#endif
			void RecreateSwapChain();
			void UpdateLightsUniformBuffer();

			void AquireSwapChainImage(uint32_t& imageIndex, int currFrame);
			void DrawDeferred(uint32_t imageIndex, int currFrame, VkSemaphore waitSemaphore);
#if !PL_DIST
			void DrawDeferredOutput(uint32_t imageIndex, int currFrame);
#endif
			void DrawOutput(uint32_t imageIndex, int currFrame);
			void Present(uint32_t& imageIndex, int currFrame);


			VkShaderModule CreateShaderModule(const std::vector<char>& code);

			VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

			Window* m_Window;
			vk::Mesh m_FullscreenQuad;

			InstanceRef m_Instance;
			VkDebugReportCallbackEXT m_Callback;
			DeviceRef m_Device;
			SwapChainRef m_SwapChain;
			
			DescriptorPoolRef m_DescriptorPool;
			PipelineCacheRef m_PipelineCache;

			MaterialRef m_DeferredOutputMaterial;
			std::vector<MaterialInstanceRef> m_DeferredOutputMaterialInstances;

			std::vector<VkSemaphore> m_DeferredSemaphores;
			std::vector<CommandBufferRef> m_DeferredCommandBuffers;
			std::vector<FrameBufferRef> m_DeferredFrameBuffers;
			
#if !PL_DIST
			std::vector<VkSemaphore> m_DeferredOutputSemaphores;
			std::vector<CommandBufferRef> m_DeferredOutputCommandBuffers;
			std::vector<FrameBufferRef> m_DeferredOutputFrameBuffers;
#endif

			std::vector<VkShaderModule> m_ShaderModules;

			plumbus::ImGUIImpl* m_ImGui = nullptr;

			//Lights
			struct PointLightBufferInfo
			{
				glm::vec4 m_Position;
				glm::vec4 m_Colour;
				float m_Radius;
				glm::vec3 dummyValue; //TODO why do i need this?
			};

			struct DirectionalLightBufferInfo
			{
				glm::vec4 m_Direction;
				glm::vec4 m_Colour;
				glm::mat4 m_Mvp;
			};

			static constexpr size_t MAX_POINT_LIGHTS = 6;
			static constexpr size_t MAX_DIRECTIONAL_LIGHTS = 1;
			struct UniformBufferLights
			{
				glm::vec4 m_ViewPos;
				PointLightBufferInfo m_PointLights[MAX_POINT_LIGHTS];
				DirectionalLightBufferInfo m_DirectionalLights[MAX_DIRECTIONAL_LIGHTS];
			};

			vk::Buffer m_LightsVulkanBuffer;
			UniformBufferLights m_LightsUniformBuffer;
		};
	}
}