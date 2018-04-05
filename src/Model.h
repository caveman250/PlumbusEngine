#pragma once
#include <string>
#include <vector>
#include "Vertex.h"
#include "vk/Buffer.h"

class Scene;
class Model
{
public:
	Model() {}
	~Model();
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;

	vk::Buffer m_VertexBuffer;
	vk::Buffer m_IndexBuffer;

	void CreateTextureSampler();

	VkImage m_Image;
	VkDeviceMemory m_ImageMemory;
	VkImageView m_ImageView;
	VkSampler m_TextureSampler;

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
