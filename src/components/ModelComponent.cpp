#include "ModelComponent.h"
#include "ModelManager.h"
#include "Application.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "TranslationComponent.h"

ModelComponent::ModelComponent(std::string modelPath, std::string texturePath)
	: Component()
{
	m_ModelPath = modelPath;
	m_TexturePath = texturePath;
}

ModelComponent::~ModelComponent()
{
	Application& app = Application::Get();
	vkDestroyBuffer(app.GetDevice(), m_UniformBuffer, nullptr);
	vkFreeMemory(app.GetDevice(), m_UniformBufferMemory, nullptr);
}

Model* ModelComponent::GetModel()
{
	return m_Model;
}

void ModelComponent::LoadModel()
{
	m_Model = &ModelManager::Get().AddModel(ModelManager::Get().GetModelLoader().LoadModel(m_ModelPath, m_TexturePath));
}

void ModelComponent::OnUpdate(Scene* scene)
{
	UpdateUniformBuffer(scene);
}

void ModelComponent::UpdateUniformBuffer(Scene* scene)
{
	Application& app = Application::Get();

	UniformBufferObject ubo = {};

	TranslationComponent* transComp = GetOwner()->GetComponent<TranslationComponent>();
	if (!transComp)
		throw std::runtime_error("ModelComponent can not be used without a valid TranslationComponent");

	glm::vec3 pos = transComp->GetTranslation();
	glm::vec3 rot = transComp->GetRotation();

	glm::mat4 model = glm::mat4();

	model = glm::translate(model, pos);
	model = glm::scale(model, glm::vec3(1.f, 1.f, 1.f));
	model = glm::rotate(model, rot.x, glm::vec3(1.f, 0.f, 0.f));
	model = glm::rotate(model, rot.y, glm::vec3(0.f, 1.f, 0.f));
	model = glm::rotate(model, rot.z, glm::vec3(0.f, 0.f, 1.f));

	ubo.m_Model = glm::mat4(model);
	ubo.m_View = scene->GetCamera()->GetViewMatrix();
	ubo.m_Proj = scene->GetCamera()->GetProjectionMatrix();
	//y and z are flipped from opengl
	ubo.m_Proj[1][1] *= -1;

	void* data;
	vkMapMemory(app.GetDevice(), m_UniformBufferMemory, 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(app.GetDevice(), m_UniformBufferMemory);
}

void ModelComponent::CreateUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	Application::Get().CreateBuffer(bufferSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		m_UniformBuffer,
		m_UniformBufferMemory);
}

void ModelComponent::CreateDescriptorSet()
{
	Application& app = Application::Get();

	VkDescriptorSetLayout layouts[] = { app.GetDescriptorSetLayout() };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = app.GetDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(app.GetDevice(), &allocInfo, &m_DescriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor set!");
	}

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = m_UniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(UniformBufferObject);

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = m_Model->m_ImageView;
	imageInfo.sampler = m_Model->m_TextureSampler;

	std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = m_DescriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = m_DescriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(app.GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}
