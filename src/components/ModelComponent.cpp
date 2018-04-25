#include "ModelComponent.h"
#include "Application.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "TranslationComponent.h"
#include "Helpers.h"
#include "GameObject.h"
#include <cstring>

ModelComponent::ModelComponent(std::string modelPath, std::string texturePath, std::string normalPath)
	: Component()
{
	m_ModelPath = modelPath;
	m_TexturePath = texturePath;
	m_NormalPath = normalPath;
}

ModelComponent::~ModelComponent()
{
	Application& app = Application::Get();
	vkDestroyBuffer(app.GetVulkanDevice()->GetDevice(), m_UniformBuffer.m_Buffer, nullptr);
	vkFreeMemory(app.GetVulkanDevice()->GetDevice(), m_UniformBuffer.m_Memory, nullptr);
}

vk::Model* ModelComponent::GetModel()
{
	return m_Model;
}

void ModelComponent::LoadModel(VkQueue queue, vk::VertexLayout layout)
{
	m_Model = new vk::Model();
	m_Model->LoadModel(m_ModelPath, layout, queue);
	m_Model->m_ColourMap.LoadTexture(m_TexturePath, queue, VK_FORMAT_BC3_UNORM_BLOCK);
	m_Model->m_NormalMap.LoadTexture(m_NormalPath, queue, VK_FORMAT_BC3_UNORM_BLOCK);
}

void ModelComponent::OnUpdate(Scene* scene)
{
	UpdateUniformBuffer(scene);
}

void ModelComponent::Cleanup(VkDevice device)
{
	m_UniformBuffer.Cleanup();
	m_Model->Cleanup(device);
}

void ModelComponent::UpdateUniformBuffer(Scene* scene)
{
	m_UniformBufferObject.m_Proj = scene->GetCamera()->GetProjectionMatrix();
	m_UniformBufferObject.m_View = scene->GetCamera()->GetViewMatrix();

	::TranslationComponent* transComp = GetOwner()->GetComponent<::TranslationComponent>();
	glm::mat4 model;
	model = glm::translate(model, transComp->GetTranslation());
	model = glm::rotate(model, transComp->GetRotation().x, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, transComp->GetRotation().y, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, transComp->GetRotation().z, glm::vec3(0.0f, 0.0f, 1.0f));

	model = glm::scale(model, transComp->GetScale());

	m_UniformBufferObject.m_Model = model;

	memcpy(m_UniformBuffer.m_Mapped, &m_UniformBufferObject, sizeof(m_UniformBufferObject));
}

void ModelComponent::CreateUniformBuffer(vk::VulkanDevice* vulkanDevice)
{
	CHECK_VK_RESULT(vulkanDevice->CreateBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&m_UniformBuffer,
		sizeof(m_UniformBufferObject)));

	CHECK_VK_RESULT(m_UniformBuffer.Map());
}

void ModelComponent::CreateDescriptorSet(VkDescriptorSetAllocateInfo allocInfo)
{
	VkDevice device = Application::Get().GetVulkanDevice()->GetDevice();

	CHECK_VK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &m_Model->m_DescriptorSet));

	// Binding 0: Vertex shader uniform buffer
	VkWriteDescriptorSet vertUniformModelWrite{};
	vertUniformModelWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	vertUniformModelWrite.dstSet = m_Model->m_DescriptorSet;
	vertUniformModelWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vertUniformModelWrite.dstBinding = 0;
	vertUniformModelWrite.pBufferInfo = &m_UniformBuffer.m_Descriptor;
	vertUniformModelWrite.descriptorCount = 1;

	// Binding 1: Color map
	VkWriteDescriptorSet colourWrite{};
	colourWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	colourWrite.dstSet = m_Model->m_DescriptorSet;
	colourWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	colourWrite.dstBinding = 1;
	colourWrite.pImageInfo = &m_Model->m_ColourMap.m_Descriptor;
	colourWrite.descriptorCount = 1;

	// Binding 2: Normal map
	VkWriteDescriptorSet normalWrite{};
	normalWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	normalWrite.dstSet = m_Model->m_DescriptorSet;
	normalWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	normalWrite.dstBinding = 2;
	normalWrite.pImageInfo = &m_Model->m_NormalMap.m_Descriptor;
	normalWrite.descriptorCount = 1;

	std::vector<VkWriteDescriptorSet> writeDescriptorSets =
	{
		vertUniformModelWrite,
		colourWrite,
		normalWrite
	};
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
}

void ModelComponent::SetupCommandBuffer(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout)
{
	VkDeviceSize offsets[1] = { 0 };
	// Object
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &m_Model->m_DescriptorSet, 0, NULL);
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &m_Model->m_VertexBuffer.m_Buffer, offsets);
	vkCmdBindIndexBuffer(cmdBuffer, m_Model->m_IndexBuffer.m_Buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(cmdBuffer, m_Model->m_IndexSize, 1, 0, 0, 0);
}
