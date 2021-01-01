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
		struct PushConstant;

		namespace shaders {
			class ShaderSettings;
		}

		class Mesh;
		class Instance;

#define ENABLE_IMGUI !PL_PLATFORM_ANDROID && !PL_DIST

		struct ShaderReflectionObject
		{
			std::vector<StageInput> m_VertexStageInputs;
			std::vector<StageInput> m_FragmentStageInputs;
			int m_VertexStageOutputCount = 0;
			int m_FragmentStageOutputCount = 0;
			std::vector<DescriptorBinding> m_Bindings;
			std::vector<PushConstant> m_PushConstants;
		};

		class VulkanRenderer
		{
		public:
			static VulkanRenderer* Get();

			void Init(std::string appName);
			void Cleanup();

			void DrawFrame();
			void AwaitIdle();

			bool WindowShouldClose();

			VkPipelineShaderStageCreateInfo LoadShader(std::string fileName, VkShaderStageFlagBits stage,  shaders::ShaderSettings settings, ShaderReflectionObject& shaderReflection);

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

            //this could be expanded on greatly, but as a minor optimization, dont bind a material if it is already bound.
            void SetBoundMaterial(const MaterialInstance* materialInstance) { m_BoundMaterialInstance = materialInstance; }
            const MaterialInstance* GetBoundMaterialInstance() { return m_BoundMaterialInstance; }

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
            void GetNumLights(int& numPointLights, int& numDirLights);
            void UpdateOutputMaterial();

			VkShaderModule CreateShaderModule(const std::vector<unsigned int>& code);

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
			const MaterialInstance* m_BoundMaterialInstance = nullptr;

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

            glm::vec4 m_ViewPos;
            std::vector<PointLightBufferInfo> m_PointLights;
            std::vector<DirectionalLightBufferInfo> m_DirectionalLights;
			vk::Buffer m_ViewPosVulkanBuffer;
            vk::Buffer m_PointLightsVulkanBuffer;
            vk::Buffer m_DirLightsVulkanBuffer;

			int m_CachedDirShadowCount;
            int m_CachedOmniDirShadowCount;
		};
	}
}