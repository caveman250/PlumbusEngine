#include "renderer/vk/Model.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"     
#include "assimp/postprocess.h"
#include "assimp/cimport.h"

#include <unordered_map>

#include "Application.h"
#include "renderer/vk/ImageHelpers.h"
#include "Helpers.h"
#include "renderer/vk/VulkanRenderer.h"

namespace vk
{

	Model::Model()
	{
		m_ColourMap = new vk::Texture();
		m_NormalMap = new vk::Texture();
	}

	Model::~Model()
	{
		vk::VulkanRenderer* renderer = static_cast<vk::VulkanRenderer*>(Application::Get().GetRenderer());
		vkDestroyBuffer(renderer->GetVulkanDevice()->GetDevice(), m_UniformBuffer.m_Buffer, nullptr);
		vkFreeMemory(renderer->GetVulkanDevice()->GetDevice(), m_UniformBuffer.m_Memory, nullptr);
	}

	void Model::LoadModel(const std::string& fileName)
	{
        vk::VulkanRenderer* renderer = static_cast<vk::VulkanRenderer*>(Application::Get().GetRenderer());

		VkDevice device = renderer->GetVulkanDevice()->GetDevice();

		std::vector<VertexLayoutComponent> vertLayoutComponents;
        vertLayoutComponents.push_back(VERTEX_COMPONENT_POSITION);
        vertLayoutComponents.push_back(VERTEX_COMPONENT_UV);
        vertLayoutComponents.push_back(VERTEX_COMPONENT_COLOR);
        vertLayoutComponents.push_back(VERTEX_COMPONENT_NORMAL);
        vertLayoutComponents.push_back(VERTEX_COMPONENT_TANGENT);
        
        std::vector<float> vertexBuffer;
        std::vector<uint32_t> indexBuffer;
        
        LoadFromFile(fileName, vertLayoutComponents, vertexBuffer, indexBuffer);

		uint32_t vBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(float);
		uint32_t iBufferSize = static_cast<uint32_t>(indexBuffer.size()) * sizeof(uint32_t);

		m_IndexSize = (uint32_t)indexBuffer.size();

		Buffer vertexStaging, indexStaging;

		// Vertex buffer staging
		if (renderer->GetVulkanDevice()->CreateBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&vertexStaging,
			vBufferSize,
			vertexBuffer.data()) != VK_SUCCESS)
			Log::Fatal("failed to create vertex staging buffer");

		// Index buffer staging
		if (renderer->GetVulkanDevice()->CreateBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&indexStaging,
			iBufferSize,
			indexBuffer.data()) != VK_SUCCESS)
			Log::Fatal("failed to create index staging buffer");
		// Create device local target buffers
		// Vertex buffer
		if (renderer->GetVulkanDevice()->CreateBuffer(
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&m_VertexBuffer,
			vBufferSize) != VK_SUCCESS)
			Log::Fatal("failed to create vertex buffer");

		// Index buffer
		if (renderer->GetVulkanDevice()->CreateBuffer(
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&m_IndexBuffer,
			iBufferSize) != VK_SUCCESS)
			Log::Fatal("failed to create index buffer");

		// Copy from staging buffers
		VkCommandBuffer copyCmd = renderer->GetVulkanDevice()->CreateCommandBuffer();

		VkBufferCopy copyRegion{};

		copyRegion.size = m_VertexBuffer.m_Size;
		vkCmdCopyBuffer(copyCmd, vertexStaging.m_Buffer, m_VertexBuffer.m_Buffer, 1, &copyRegion);

		copyRegion.size = m_IndexBuffer.m_Size;
		vkCmdCopyBuffer(copyCmd, indexStaging.m_Buffer, m_IndexBuffer.m_Buffer, 1, &copyRegion);

        renderer->GetVulkanDevice()->FlushCommandBuffer(copyCmd, renderer->GetGraphicsQueue());

		// Destroy staging resources
		vkDestroyBuffer(renderer->GetVulkanDevice()->GetDevice(), vertexStaging.m_Buffer, nullptr);
		vkFreeMemory(renderer->GetVulkanDevice()->GetDevice(), vertexStaging.m_Memory, nullptr);
		vkDestroyBuffer(renderer->GetVulkanDevice()->GetDevice(), indexStaging.m_Buffer, nullptr);
		vkFreeMemory(renderer->GetVulkanDevice()->GetDevice(), indexStaging.m_Memory, nullptr);
	};

	void Model::Cleanup()
	{
		m_UniformBuffer.Cleanup();

		m_VertexBuffer.Cleanup();
		m_IndexBuffer.Cleanup();

		m_ColourMap->Cleanup();
		m_NormalMap->Cleanup();
	}

	void Model::Setup(base::Renderer* renderer)
	{
		vk::VulkanRenderer* vkRenderer = static_cast<vk::VulkanRenderer*>(renderer);

		CreateUniformBuffer(vkRenderer->GetVulkanDevice());
		CreateDescriptorSet(vkRenderer->GetDescriptorSetAllocateInfo());
	}

	void Model::UpdateUniformBuffer(ModelComponent::UniformBufferObject& ubo)
	{
		memcpy(m_UniformBuffer.m_Mapped, &ubo, sizeof(ubo));
	}

	void Model::CreateUniformBuffer(VulkanDevice* vulkanDevice)
	{
		CHECK_VK_RESULT(vulkanDevice->CreateBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&m_UniformBuffer,
			sizeof(ModelComponent::UniformBufferObject)));

		CHECK_VK_RESULT(m_UniformBuffer.Map());
	}

	void Model::CreateDescriptorSet(VkDescriptorSetAllocateInfo allocInfo)
	{
		VkDevice device = static_cast<vk::VulkanRenderer*>(Application::Get().GetRenderer())->GetVulkanDevice()->GetDevice();

		CHECK_VK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &m_DescriptorSet));

		// Binding 0: Vertex shader uniform buffer
		VkWriteDescriptorSet vertUniformModelWrite{};
		vertUniformModelWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vertUniformModelWrite.dstSet = m_DescriptorSet;
		vertUniformModelWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vertUniformModelWrite.dstBinding = 0;
		vertUniformModelWrite.pBufferInfo = &m_UniformBuffer.m_Descriptor;
		vertUniformModelWrite.descriptorCount = 1;

		// Binding 1: Color map
		VkWriteDescriptorSet colourWrite{};
		colourWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		colourWrite.dstSet = m_DescriptorSet;
		colourWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		colourWrite.dstBinding = 1;
		colourWrite.pImageInfo = &static_cast<vk::Texture*>(m_ColourMap)->m_Descriptor;
		colourWrite.descriptorCount = 1;

		// Binding 2: Normal map
		VkWriteDescriptorSet normalWrite{};
		normalWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		normalWrite.dstSet = m_DescriptorSet;
		normalWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		normalWrite.dstBinding = 2;
		normalWrite.pImageInfo = &static_cast<vk::Texture*>(m_NormalMap)->m_Descriptor;
		normalWrite.descriptorCount = 1;

		std::vector<VkWriteDescriptorSet> writeDescriptorSets =
		{
			vertUniformModelWrite,
			colourWrite,
			normalWrite
		};
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
	}

	void Model::SetupCommandBuffer(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout)
	{
		VkDeviceSize offsets[1] = { 0 };
		// Object
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &m_DescriptorSet, 0, NULL);
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &m_VertexBuffer.m_Buffer, offsets);
		vkCmdBindIndexBuffer(cmdBuffer, m_IndexBuffer.m_Buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmdBuffer, m_IndexSize, 1, 0, 0, 0);
	}

}

