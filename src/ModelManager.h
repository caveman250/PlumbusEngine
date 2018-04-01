#pragma once
#include <vector>
#include "ModelLoader.h"

struct Vertex;
class Model;
class ModelManager
{
public:
	void Cleanup();

	std::vector<Vertex> GetModelVertices();
	std::vector<uint32_t> GetModelIndices();
	Model& AddModel(Model model);
	void RemoveModel(Model& model);
	std::vector<Model>& GetModels() { return m_Models; }

	void LoadTextures();

	ModelLoader& GetModelLoader() { return m_ModelLoader; }

private:
	std::vector<Model> m_Models;

	ModelLoader m_ModelLoader;
};