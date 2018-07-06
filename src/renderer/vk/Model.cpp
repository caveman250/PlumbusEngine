#include "renderer/vk/Model.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"     
#include "assimp/postprocess.h"
#include "assimp/cimport.h"

#include <unordered_map>

#include "Application.h"
#include "ImageHelpers.h"
#include "Helpers.h"
#include "renderer/vk/VulkanRenderer.h"

namespace vk
{

	Model::Model()
	{
		m_ColourMap = new vk::Texture();
		m_NormalMap = new vk::Texture();
	}

	Model::~Model()
	{
		VulkanRenderer* renderer = static_cast<VulkanRenderer*>(Application::Get().GetRenderer());
		vkDestroyBuffer(renderer->GetVulkanDevice()->GetDevice(), m_UniformBuffer.m_Buffer, nullptr);
		vkFreeMemory(renderer->GetVulkanDevice()->GetDevice(), m_UniformBuffer.m_Memory, nullptr);
	}

	void Model::LoadModel(const std::string& filename)
	{
        VulkanRenderer* renderer = static_cast<VulkanRenderer*>(Application::Get().GetRenderer());

		const int flags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;
		VkDevice device = renderer->GetVulkanDevice()->GetDevice();

		Assimp::Importer Importer;
		const aiScene* pScene;

		// Load file
		pScene = Importer.ReadFile(filename.c_str(), flags);
		if (!pScene)
		{
			Log::Fatal(Importer.GetErrorString());
		}

		std::vector<ModelPart> parts;

		if (pScene)
		{
			parts.clear();
			parts.resize(pScene->mNumMeshes);

			glm::vec3 scale(1.0f);
			glm::vec2 uvscale(1.0f);
			glm::vec3 center(0.0f);

			std::vector<float> vertexBuffer;
			std::vector<uint32_t> indexBuffer;

			int vertexCount = 0;
			int indexCount = 0;

			// Load meshes
			for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
			{
				const aiMesh* paiMesh = pScene->mMeshes[i];

				parts[i] = {};
				parts[i].m_VertexBase = vertexCount;
				parts[i].m_IndexBase = indexCount;

				vertexCount += pScene->mMeshes[i]->mNumVertices;

				aiColor3D pColor(0.f, 0.f, 0.f);
				pScene->mMaterials[paiMesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, pColor);

				const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

				Dimension dim;

				VertexLayout layout = VertexLayout(
					{
						VERTEX_COMPONENT_POSITION,
						VERTEX_COMPONENT_UV,
						VERTEX_COMPONENT_COLOR,
						VERTEX_COMPONENT_NORMAL,
						VERTEX_COMPONENT_TANGENT,
					});

				for (unsigned int j = 0; j < paiMesh->mNumVertices; j++)
				{
					const aiVector3D* pPos = &(paiMesh->mVertices[j]);
					const aiVector3D* pNormal = &(paiMesh->mNormals[j]);
					const aiVector3D* pTexCoord = (paiMesh->HasTextureCoords(0)) ? &(paiMesh->mTextureCoords[0][j]) : &Zero3D;
					const aiVector3D* pTangent = (paiMesh->HasTangentsAndBitangents()) ? &(paiMesh->mTangents[j]) : &Zero3D;
					const aiVector3D* pBiTangent = (paiMesh->HasTangentsAndBitangents()) ? &(paiMesh->mBitangents[j]) : &Zero3D;

					for (auto& component : layout.components)
					{
						switch (component)
						{
						case VERTEX_COMPONENT_POSITION:
							vertexBuffer.push_back(pPos->x * scale.x + center.x);
							vertexBuffer.push_back(-pPos->y * scale.y + center.y);
							vertexBuffer.push_back(pPos->z * scale.z + center.z);
							break;
						case VERTEX_COMPONENT_NORMAL:
							vertexBuffer.push_back(pNormal->x);
							vertexBuffer.push_back(-pNormal->y);
							vertexBuffer.push_back(pNormal->z);
							break;
						case VERTEX_COMPONENT_UV:
							vertexBuffer.push_back(pTexCoord->x * uvscale.s);
							vertexBuffer.push_back(pTexCoord->y * uvscale.t);
							break;
						case VERTEX_COMPONENT_COLOR:
							vertexBuffer.push_back(pColor.r);
							vertexBuffer.push_back(pColor.g);
							vertexBuffer.push_back(pColor.b);
							break;
						case VERTEX_COMPONENT_TANGENT:
							vertexBuffer.push_back(pTangent->x);
							vertexBuffer.push_back(pTangent->y);
							vertexBuffer.push_back(pTangent->z);
							break;
						case VERTEX_COMPONENT_BITANGENT:
							vertexBuffer.push_back(pBiTangent->x);
							vertexBuffer.push_back(pBiTangent->y);
							vertexBuffer.push_back(pBiTangent->z);
							break;
							// Dummy components for padding
						case VERTEX_COMPONENT_DUMMY_FLOAT:
							vertexBuffer.push_back(0.0f);
							break;
						case VERTEX_COMPONENT_DUMMY_VEC4:
							vertexBuffer.push_back(0.0f);
							vertexBuffer.push_back(0.0f);
							vertexBuffer.push_back(0.0f);
							vertexBuffer.push_back(0.0f);
							break;
						};
					}

					dim.max.x = fmax(pPos->x, dim.max.x);
					dim.max.y = fmax(pPos->y, dim.max.y);
					dim.max.z = fmax(pPos->z, dim.max.z);

					dim.min.x = fmin(pPos->x, dim.min.x);
					dim.min.y = fmin(pPos->y, dim.min.y);
					dim.min.z = fmin(pPos->z, dim.min.z);
				}

				dim.size = dim.max - dim.min;

				parts[i].m_VertexCount = paiMesh->mNumVertices;

				uint32_t indexBase = static_cast<uint32_t>(indexBuffer.size());
				for (unsigned int j = 0; j < paiMesh->mNumFaces; j++)
				{
					const aiFace& Face = paiMesh->mFaces[j];
					if (Face.mNumIndices != 3)
						continue;
					indexBuffer.push_back(indexBase + Face.mIndices[0]);
					indexBuffer.push_back(indexBase + Face.mIndices[1]);
					indexBuffer.push_back(indexBase + Face.mIndices[2]);
					parts[i].m_IndexCount += 3;
					indexCount += 3;
				}
			}

			uint32_t vBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(float);
			uint32_t iBufferSize = static_cast<uint32_t>(indexBuffer.size()) * sizeof(uint32_t);

			m_IndexSize = (uint32_t)indexBuffer.size();

			Buffer vertexStaging, indexStaging;

			// Vertex buffer staging
			if (renderer->GetVulkanDevice()->CreateBuffer(
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				&vertexStaging,
				vBufferSize,
				vertexBuffer.data()) != VK_SUCCESS)
				Log::Fatal("failed to create vertex staging buffer");

			// Index buffer staging
			if (renderer->GetVulkanDevice()->CreateBuffer(
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				&indexStaging,
				iBufferSize,
				indexBuffer.data()) != VK_SUCCESS)
				Log::Fatal("failed to create index staging buffer");
			// Create device local target buffers
			// Vertex buffer
			if (renderer->GetVulkanDevice()->CreateBuffer(
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				&m_VertexBuffer,
				vBufferSize) != VK_SUCCESS)
				Log::Fatal("failed to create vertex buffer");

			// Index buffer
			if (renderer->GetVulkanDevice()->CreateBuffer(
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				&m_IndexBuffer,
				iBufferSize) != VK_SUCCESS)
				Log::Fatal("failed to create index buffer");

			// Copy from staging buffers
			VkCommandBuffer copyCmd = renderer->GetVulkanDevice()->CreateCommandBuffer();

			VkBufferCopy copyRegion{};

			copyRegion.size = m_VertexBuffer.m_Size;
			vkCmdCopyBuffer(copyCmd, vertexStaging.m_Buffer, m_VertexBuffer.m_Buffer, 1, &copyRegion);

			copyRegion.size = m_IndexBuffer.m_Size;
			vkCmdCopyBuffer(copyCmd, indexStaging.m_Buffer, m_IndexBuffer.m_Buffer, 1, &copyRegion);

            renderer->GetVulkanDevice()->FlushCommandBuffer(copyCmd, renderer->GetGraphicsQueue());

			// Destroy staging resources
			vkDestroyBuffer(renderer->GetVulkanDevice()->GetDevice(), vertexStaging.m_Buffer, nullptr);
			vkFreeMemory(renderer->GetVulkanDevice()->GetDevice(), vertexStaging.m_Memory, nullptr);
			vkDestroyBuffer(renderer->GetVulkanDevice()->GetDevice(), indexStaging.m_Buffer, nullptr);
			vkFreeMemory(renderer->GetVulkanDevice()->GetDevice(), indexStaging.m_Memory, nullptr);
		}
		else
		{
			Log::Fatal("Error loading model");
		}
	};

	void Model::Cleanup()
	{
		m_UniformBuffer.Cleanup();

		m_VertexBuffer.Cleanup();
		m_IndexBuffer.Cleanup();

		m_ColourMap->Cleanup();
		m_NormalMap->Cleanup();
	}

	void Model::Setup(base::Renderer* renderer)
	{
		VulkanRenderer* vkRenderer = static_cast<VulkanRenderer*>(renderer);

		CreateUniformBuffer(vkRenderer->GetVulkanDevice());
		CreateDescriptorSet(vkRenderer->GetDescriptorSetAllocateInfo());
	}

	void Model::UpdateUniformBuffer(ModelComponent::UniformBufferObject& ubo)
	{
		memcpy(m_UniformBuffer.m_Mapped, &ubo, sizeof(ubo));
	}

	void Model::CreateUniformBuffer(VulkanDevice* vulkanDevice)
	{
		CHECK_VK_RESULT(vulkanDevice->CreateBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&m_UniformBuffer,
			sizeof(ModelComponent::UniformBufferObject)));

		CHECK_VK_RESULT(m_UniformBuffer.Map());
	}

	void Model::CreateDescriptorSet(VkDescriptorSetAllocateInfo allocInfo)
	{
		VkDevice device = static_cast<VulkanRenderer*>(Application::Get().GetRenderer())->GetVulkanDevice()->GetDevice();

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

	void Model::SetupCommandBuffer(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout)
	{
		VkDeviceSize offsets[1] = { 0 };
		// Object
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &m_DescriptorSet, 0, NULL);
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &m_VertexBuffer.m_Buffer, offsets);
		vkCmdBindIndexBuffer(cmdBuffer, m_IndexBuffer.m_Buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmdBuffer, m_IndexSize, 1, 0, 0, 0);
	}

}

