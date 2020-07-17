#include "plumbus.h"

#include "renderer/vk/Mesh.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"     
#include "assimp/postprocess.h"
#include "assimp/cimport.h"

#include "BaseApplication.h"
#include "renderer/vk/ImageHelpers.h"
#include "Helpers.h"
#include "renderer/vk/VulkanRenderer.h"

namespace plumbus::vk
{
	Mesh::Mesh()
		: base::Mesh()
	{
		m_VertexLayout = VertexLayout(
		{
			VERTEX_COMPONENT_POSITION,
			VERTEX_COMPONENT_UV,
			VERTEX_COMPONENT_COLOR,
			VERTEX_COMPONENT_NORMAL,
			VERTEX_COMPONENT_TANGENT,
		});

		m_ColourMap = new vk::Texture();
		m_NormalMap = new vk::Texture();
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::PostLoad()
	{
		vk::VulkanRenderer* renderer = VulkanRenderer::Get();
		VkDevice device = renderer->GetDevice()->GetVulkanDevice();

		uint32_t vBufferSize = static_cast<uint32_t>(m_VertexBuffer.size()) * sizeof(float);
		uint32_t iBufferSize = static_cast<uint32_t>(m_IndexBuffer.size()) * sizeof(uint32_t);

		m_IndexSize = (uint32_t)m_IndexBuffer.size();

		Buffer vertexStaging, indexStaging;

		// Vertex buffer staging
		if (renderer->GetDevice()->CreateBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&vertexStaging,
			vBufferSize,
			m_VertexBuffer.data()) != VK_SUCCESS)
			Log::Fatal("failed to create vertex staging buffer");

		// Index buffer staging
		if (renderer->GetDevice()->CreateBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&indexStaging,
			iBufferSize,
			m_IndexBuffer.data()) != VK_SUCCESS)
			Log::Fatal("failed to create index staging buffer");
		// Create device local target buffers
		// Vertex buffer
		if (renderer->GetDevice()->CreateBuffer(
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&m_VulkanVertexBuffer,
			vBufferSize) != VK_SUCCESS)
			Log::Fatal("failed to create vertex buffer");

		// Index buffer
		if (renderer->GetDevice()->CreateBuffer(
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&m_VulkanIndexBuffer,
			iBufferSize) != VK_SUCCESS)
			Log::Fatal("failed to create index buffer");

		// Copy from staging buffers
		VkCommandBuffer copyCmd = renderer->GetDevice()->CreateCommandBuffer();

		VkBufferCopy copyRegion{};

		copyRegion.size = m_VulkanVertexBuffer.m_Size;
		vkCmdCopyBuffer(copyCmd, vertexStaging.m_Buffer, m_VulkanVertexBuffer.m_Buffer, 1, &copyRegion);

		copyRegion.size = m_VulkanIndexBuffer.m_Size;
		vkCmdCopyBuffer(copyCmd, indexStaging.m_Buffer, m_VulkanIndexBuffer.m_Buffer, 1, &copyRegion);

		renderer->GetDevice()->FlushCommandBuffer(copyCmd, renderer->GetGraphicsQueue());

		// Destroy staging resources
		vkDestroyBuffer(renderer->GetDevice()->GetVulkanDevice(), vertexStaging.m_Buffer, nullptr);
		vkFreeMemory(renderer->GetDevice()->GetVulkanDevice(), vertexStaging.m_Memory, nullptr);
		vkDestroyBuffer(renderer->GetDevice()->GetVulkanDevice(), indexStaging.m_Buffer, nullptr);
		vkFreeMemory(renderer->GetDevice()->GetVulkanDevice(), indexStaging.m_Memory, nullptr);
	}

	void Mesh::Cleanup()
	{
		vk::VulkanRenderer* renderer = VulkanRenderer::Get();
		VkDevice device = renderer->GetDevice()->GetVulkanDevice();

		m_UniformBuffer.Cleanup();
		m_VulkanVertexBuffer.Cleanup();
		m_VulkanIndexBuffer.Cleanup();

		m_ColourMap->Cleanup();
		m_NormalMap->Cleanup();
	}

	void Mesh::Setup(base::Renderer* renderer)
	{
		if (PLUMBUS_VERIFY(m_Material != nullptr))
		{
			vk::VulkanRenderer* vkRenderer = static_cast<vk::VulkanRenderer*>(renderer);
			m_Material->Setup(&m_VertexLayout);
			CreateUniformBuffer(vkRenderer->GetDevice());
			CreateDescriptorSet(vkRenderer->GetDescriptorSetAllocateInfo());
		}
	}

	void Mesh::UpdateUniformBuffer(ModelComponent::UniformBufferObject& ubo)
	{
		memcpy(m_UniformBuffer.m_Mapped, &ubo, sizeof(ubo));
	}

	void Mesh::CreateUniformBuffer(Device* vulkanDevice)
	{
		CHECK_VK_RESULT(vulkanDevice->CreateBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&m_UniformBuffer,
			sizeof(ModelComponent::UniformBufferObject)));

		CHECK_VK_RESULT(m_UniformBuffer.Map());
	}

	void Mesh::CreateDescriptorSet(VkDescriptorSetAllocateInfo allocInfo)
	{
		VkDevice device = VulkanRenderer::Get()->GetDevice()->GetVulkanDevice();

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

	void Mesh::SetupCommandBuffer(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout)
	{
		vk::Material* vkMaterial = static_cast<vk::Material*>(m_Material.get());
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkMaterial->GetPipeline());

		VkDeviceSize offsets[1] = { 0 };
		// Object
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkMaterial->GetPipelineLayout(), 0, 1, &m_DescriptorSet, 0, NULL);
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &m_VulkanVertexBuffer.m_Buffer, offsets);
		vkCmdBindIndexBuffer(cmdBuffer, m_VulkanIndexBuffer.m_Buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmdBuffer, m_IndexSize, 1, 0, 0, 0);
	}

	Buffer& Mesh::GetVertexBuffer()
	{
		return m_VulkanVertexBuffer;
	}

	Buffer& Mesh::GetIndexBuffer()
	{
		return m_VulkanIndexBuffer;
	}

	void Mesh::SetIndexSize(uint32_t indexSize)
	{
		m_IndexSize = indexSize;
	}

	void Mesh::SetMaterial(MaterialRef material)
	{
		PLUMBUS_ASSERT(dynamic_cast<vk::Material*>(material.get()) != nullptr);
		m_Material = material;
	}

}

