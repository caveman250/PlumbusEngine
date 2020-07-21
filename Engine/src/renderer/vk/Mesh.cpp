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
#include "renderer/vk/CommandBuffer.h"
#include "DescriptorSet.h"

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

		renderer->GetDevice()->FlushCommandBuffer(copyCmd, renderer->GetDevice()->GetGraphicsQueue());

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
			CreateUniformBuffer(vkRenderer->GetDevice().get());
			CreateDescriptorSet();

			m_CommandBuffer = vkRenderer->GetOffscreenCommandBuffer();
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

	void Mesh::CreateDescriptorSet()
	{
		vk::Texture* vkColourMap = static_cast<vk::Texture*>(m_ColourMap);
		vk::Texture* vkNormalMap = static_cast<vk::Texture*>(m_NormalMap);

		m_DescriptorSet = DescriptorSet::CreateDescriptorSet(VulkanRenderer::Get()->GeDescriptorPool(), static_cast<vk::Material*>(m_Material.get())->GetLayout());
		m_DescriptorSet->AddBuffer(&m_UniformBuffer, DescriptorSet::BindingUsage::VertexShader);
		m_DescriptorSet->AddTexture(vkColourMap->m_TextureSampler, vkColourMap->m_ImageView, DescriptorSet::BindingUsage::FragmentShader);
		m_DescriptorSet->AddTexture(vkNormalMap->m_TextureSampler, vkNormalMap->m_ImageView, DescriptorSet::BindingUsage::FragmentShader);
		m_DescriptorSet->Build();
	}

	void Mesh::Render()
	{
		Material* material = GetVkMaterial();

		m_CommandBuffer->BindPipeline(material->GetPipeline());
		m_CommandBuffer->BindDescriptorSet(material->GetPipelineLayout(), m_DescriptorSet);
		m_CommandBuffer->BindVertexBuffer(m_VulkanVertexBuffer);
		m_CommandBuffer->BindIndexBuffer(m_VulkanIndexBuffer);
		m_CommandBuffer->RecordDraw(m_IndexSize);
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
		PL_ASSERT(dynamic_cast<vk::Material*>(material.get()) != nullptr);
		m_Material = material;
	}

}

