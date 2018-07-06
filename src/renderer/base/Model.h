#pragma once
#include <string>
#include "renderer/base/Renderer.h"

namespace base
{
	class Model
	{
	public:
		Model() {}
		~Model() {}

		virtual void LoadModel(const std::string& filename) = 0;
		virtual void Cleanup() = 0;
		virtual void UpdateUniformBuffer(ModelComponent::UniformBufferObject& ubo) = 0;
		virtual void Setup(Renderer* renderer) = 0;

		base::Texture* m_ColourMap;
		base::Texture* m_NormalMap;
	};
}