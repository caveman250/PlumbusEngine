#include "ModelManager.h"
#include "Model.h"
#include "ImageHelpers.h"

ModelManager* ModelManager::m_Instance = nullptr;

void ModelManager::Cleanup()
{
	m_Models.clear();
}

std::vector<Vertex> ModelManager::GetModelVertices()
{
	std::vector<Vertex> vertices;
	for (Model& model : m_Models)
	{
		for (Vertex& vert : model.m_Vertices)
			vertices.push_back(vert);
	}

	return vertices;
}

std::vector<uint32_t> ModelManager::GetModelIndices()
{
	std::vector<uint32_t> indices;
	for (Model& model : m_Models)
	{
		for (uint32_t& indice : model.m_Indices)
			indices.push_back(indice);
	}

	return indices;
}

Model& ModelManager::AddModel(Model& model)
{
	m_Models.push_back(model);
	return m_Models.back();
}

void ModelManager::RemoveModel(Model& model)
{
	auto it = std::find(m_Models.begin(), m_Models.end(), model);
	m_Models.erase(it);
}

void ModelManager::LoadTextures()
{
	for (Model& model : m_Models)
	{
		m_ModelLoader.LoadTexture(model.m_Texture, model.m_Image, model.m_ImageMemory);
		model.m_ImageView = ImageHelpers::CreateImageView(model.m_Image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

