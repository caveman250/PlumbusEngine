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

#define ENABLE_IMGUI !PL_PLATFORM_ANDROID && !PL_DIST

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

			FrameBufferRef GetDeferredFramebuffer() { return m_DeferredFrameBuffer; }
			const CommandBufferRef& GetDeferredCommandBuffer() { return m_DeferredCommandBuffer; }
#if ENABLE_IMGUI
			FrameBufferRef GetDeferredOutputFramebuffer() { return m_DeferredOutputFrameBuffer; }
			const CommandBufferRef& GetDeferredOutputCommandBuffer() { return m_DeferredOutputCommandBuffer; }

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
			void BuildPresentCommandBuffer(uint32_t imageIndex);
			void BuildDefferedCommandBuffer();
#if ENABLE_IMGUI
			void SetupImGui();
			void BuildDeferredOutputCommandBuffer();
#endif
			void RecreateSwapChain();
			void UpdateLightsUniformBuffer();

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
			MaterialInstanceRef m_DeferredOutputMaterialInstance;

			VkSemaphore m_DeferredSemaphore;
			CommandBufferRef m_DeferredCommandBuffer;
			FrameBufferRef m_DeferredFrameBuffer;
			
#if !PL_DIST
			VkSemaphore m_DeferredOutputSemaphore;
			CommandBufferRef m_DeferredOutputCommandBuffer;
			FrameBufferRef m_DeferredOutputFrameBuffer;
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