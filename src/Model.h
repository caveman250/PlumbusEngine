#pragma once
#include <string>
#include <vector>
#include "Vertex.h"

class Scene;
class Model
{
public:
	Model() {}
	~Model();
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;

	struct UniformBufferObject
	{
		glm::mat4 m_Model;
		glm::mat4 m_View;
		glm::mat4 m_Proj;
	};

	void CreateDescriptorSet();
	void CreateUniformBuffer();
	void CreateTextureSampler();
	void UpdateUniformBuffer(Scene* scene);

	VkImage m_Image;
	VkDeviceMemory m_ImageMemory;
	VkImageView m_ImageView;
	VkSampler m_TextureSampler;

	VkBuffer m_UniformBuffer;
	VkDeviceMemory m_UniformBufferMemory;

	VkDescriptorSet m_DescriptorSet;

	std::string m_Texture;

	bool m_Reverse = false;

	bool operator==(const Model& other) const
	{
		return m_Vertices == other.m_Vertices
			&& m_Indices == other.m_Indices
			&& m_Image == other.m_Image
			&& m_ImageMemory == other.m_ImageMemory;
	}

private:

};
