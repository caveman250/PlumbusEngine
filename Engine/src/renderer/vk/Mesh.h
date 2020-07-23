#pragma once
#include "plumbus.h"
#include "renderer/vk/Buffer.h"
#include "renderer/vk/Texture.h"
#include "glm/glm.hpp"
#include "components/ModelComponent.h"
#include "renderer/vk/Material.h"

namespace plumbus::vk
{
	class Scene;
	class Device;
	class Mesh
	{
	public:
		Mesh();
		~Mesh();

		static std::vector<Mesh*> LoadModel(const std::string& fileName, std::string defaultTexturePath, std::string defaultNormalPath);

		void PostLoad();
		void Cleanup();

		void Setup();
		void SetMaterial(MaterialRef material);

		void CreateUniformBuffer(Device* vulkanDevice);
		void SetupUniforms();
		void Render();

		void UpdateUniformBuffer(ModelComponent::UniformBufferObject& ubo);

		Buffer& GetVertexBuffer();
		Buffer& GetIndexBuffer();

		std::vector<float>& GetStagingVertexBuffer() { return m_StagingVertexBuffer; }
		std::vector<uint32_t>& GetStagingIndexBuffer() { return m_StagingIndexBuffer; }

		//todo there should really be a constructor for custom geometry, remove this once added.
		void SetIndexSize(uint32_t indexSize); 
		Texture* GetColourMap() { return m_ColourMap; }
		Texture* GetNormalMap() { return m_NormalMap; }
private:
        static std::vector<vk::Mesh*> LoadFromFile(const std::string& fileName,
                        std::vector<VertexLayoutComponent> vertLayoutComponents,
						std::string defaultDiffuseTexture,
						std::string defaultNormalTexture);

		uint32_t m_IndexSize;

		Texture* m_ColourMap;
		Texture* m_NormalMap;

		std::vector<float> m_StagingVertexBuffer;
		std::vector<uint32_t> m_StagingIndexBuffer;

		Buffer m_VulkanVertexBuffer;
		Buffer m_VulkanIndexBuffer;

		DescriptorSetRef m_DescriptorSet;

		Buffer m_UniformBuffer;

		VertexLayout m_VertexLayout;

		CommandBufferRef m_CommandBuffer;

		MaterialInstanceRef m_MaterialInstance;
	};

	struct ModelPart
	{
		uint32_t m_VertexBase;
		uint32_t m_VertexCount;
		uint32_t m_IndexBase;
		uint32_t m_IndexCount;
	};

	struct Dimension
	{
		glm::vec3 min = glm::vec3(FLT_MAX);
		glm::vec3 max = glm::vec3(-FLT_MAX);
		glm::vec3 size;
	};
}
