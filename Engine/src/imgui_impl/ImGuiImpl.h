#pragma once
#include "vulkan/vulkan.h"
#include "renderer/vk/Device.h"
#include "renderer/vk/Buffer.h"
#include "glm/glm.hpp"
#include "imgui/imgui.h"
#include "renderer/vk/ImageHelpers.h"

namespace plumbus
{
	class ImGUIImpl
	{
	public:
		struct PushConstBlock
		{
			glm::vec2 scale;
			glm::vec2 translate;
		};

		PushConstBlock m_PushConstBlock;

		ImGUIImpl(vk::Device* device);

		~ImGUIImpl();

		void Init(float width, float height);
		void InitGLFWCallbacks();
		void InitResources(VkRenderPass renderPass, VkQueue copyQueue);
		void NewFrame();
		void UpdateBuffers();
		void DrawFrame(VkCommandBuffer commandBuffer);

		VkDescriptorSet AddTexture(VkSampler sampler, VkImageView image_view);

		static void OnMouseScolled(GLFWwindow* window, double xoffset, double yoffset);
		static void OnKeyDown(GLFWwindow*, int key, int, int action, int mods);
		static void OnChar(GLFWwindow*, unsigned int c);
	private:
		VkSampler m_Sampler;
		vk::Buffer m_VertexBuffer;
		vk::Buffer m_IndexBuffer;
		int32_t m_VertexCount = 0;
		int32_t m_IndexCount = 0;
		VkDeviceMemory m_FontMemory = VK_NULL_HANDLE;
		VkImage m_FontImage = VK_NULL_HANDLE;
		VkImageView m_FontView = VK_NULL_HANDLE;
		VkPipelineCache m_PipelineCache;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_Pipeline;
		VkDescriptorPool m_DescriptorPool;
		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkDescriptorSet m_DescriptorSet;
		vk::Device* m_Device;

		VkDescriptorSet m_GameViewTextureDescSet = VK_NULL_HANDLE;

		GameObject* m_SelectedObject = nullptr;
	};
}