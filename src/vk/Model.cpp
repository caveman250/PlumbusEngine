#include "vk/Model.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"     
#include "assimp/postprocess.h"
#include "assimp/cimport.h"

#include <unordered_map>

#include "Application.h"
#include "ImageHelpers.h"
#include "Helpers.h"

namespace vk
{
	Model::~Model()
	{
		Application& app = Application::Get();
		//vkDestroyImage(app.GetVulkanDevice()->GetDevice(), m_Image, nullptr);
		//vkFreeMemory(app.GetVulkanDevice()->GetDevice(), m_ImageMemory, nullptr);
		//
		//vkDestroySampler(app.GetVulkanDevice()->GetDevice(), m_TextureSampler, nullptr);
		//vkDestroyImageView(app.GetVulkanDevice()->GetDevice(), m_ImageView, nullptr);
	}

	void Model::LoadModel(const std::string& filename, vk::VertexLayout layout, VkQueue queue)
	{
		const int flags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;
		VkDevice device = Application::Get().GetVulkanDevice()->GetDevice();

		Assimp::Importer Importer;
		const aiScene* pScene;

		// Load file
		pScene = Importer.ReadFile(filename.c_str(), flags);
		if (!pScene)
		{
			std::string error = Importer.GetErrorString();
			Helpers::LogFatal(error);
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
						case vk::VERTEX_COMPONENT_POSITION:
							vertexBuffer.push_back(pPos->x * scale.x + center.x);
							vertexBuffer.push_back(-pPos->y * scale.y + center.y);
							vertexBuffer.push_back(pPos->z * scale.z + center.z);
							break;
						case vk::VERTEX_COMPONENT_NORMAL:
							vertexBuffer.push_back(pNormal->x);
							vertexBuffer.push_back(-pNormal->y);
							vertexBuffer.push_back(pNormal->z);
							break;
						case vk::VERTEX_COMPONENT_UV:
							vertexBuffer.push_back(pTexCoord->x * uvscale.s);
							vertexBuffer.push_back(pTexCoord->y * uvscale.t);
							break;
						case vk::VERTEX_COMPONENT_COLOR:
							vertexBuffer.push_back(pColor.r);
							vertexBuffer.push_back(pColor.g);
							vertexBuffer.push_back(pColor.b);
							break;
						case vk::VERTEX_COMPONENT_TANGENT:
							vertexBuffer.push_back(pTangent->x);
							vertexBuffer.push_back(pTangent->y);
							vertexBuffer.push_back(pTangent->z);
							break;
						case vk::VERTEX_COMPONENT_BITANGENT:
							vertexBuffer.push_back(pBiTangent->x);
							vertexBuffer.push_back(pBiTangent->y);
							vertexBuffer.push_back(pBiTangent->z);
							break;
							// Dummy components for padding
						case vk::VERTEX_COMPONENT_DUMMY_FLOAT:
							vertexBuffer.push_back(0.0f);
							break;
						case vk::VERTEX_COMPONENT_DUMMY_VEC4:
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

			vk::Buffer vertexStaging, indexStaging;

			// Vertex buffer staging
			if (Application::Get().GetVulkanDevice()->CreateBuffer(
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				&vertexStaging,
				vBufferSize,
				vertexBuffer.data()) != VK_SUCCESS)
				Helpers::LogFatal("failed to create vertex staging buffer");

			// Index buffer staging
			if (Application::Get().GetVulkanDevice()->CreateBuffer(
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				&indexStaging,
				iBufferSize,
				indexBuffer.data()) != VK_SUCCESS)
				Helpers::LogFatal("failed to create index staging buffer");
			// Create device local target buffers
			// Vertex buffer
			if (Application::Get().GetVulkanDevice()->CreateBuffer(
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				&m_VertexBuffer,
				vBufferSize) != VK_SUCCESS)
				Helpers::LogFatal("failed to create vertex buffer");

			// Index buffer
			if (Application::Get().GetVulkanDevice()->CreateBuffer(
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				&m_IndexBuffer,
				iBufferSize) != VK_SUCCESS)
				Helpers::LogFatal("failed to create index buffer");

			// Copy from staging buffers
			VkCommandBuffer copyCmd = Application::Get().GetVulkanDevice()->CreateCommandBuffer();

			VkBufferCopy copyRegion{};

			copyRegion.size = m_VertexBuffer.m_Size;
			vkCmdCopyBuffer(copyCmd, vertexStaging.m_Buffer, m_VertexBuffer.m_Buffer, 1, &copyRegion);

			copyRegion.size = m_IndexBuffer.m_Size;
			vkCmdCopyBuffer(copyCmd, indexStaging.m_Buffer, m_IndexBuffer.m_Buffer, 1, &copyRegion);

			Application::Get().GetVulkanDevice()->FlushCommandBuffer(copyCmd, queue);

			// Destroy staging resources
			vkDestroyBuffer(Application::Get().GetVulkanDevice()->GetDevice(), vertexStaging.m_Buffer, nullptr);
			vkFreeMemory(Application::Get().GetVulkanDevice()->GetDevice(), vertexStaging.m_Memory, nullptr);
			vkDestroyBuffer(Application::Get().GetVulkanDevice()->GetDevice(), indexStaging.m_Buffer, nullptr);
			vkFreeMemory(Application::Get().GetVulkanDevice()->GetDevice(), indexStaging.m_Memory, nullptr);
		}
		else
		{
			Helpers::LogFatal("Error loading model");
		}
	};

	void Model::Cleanup(VkDevice device)
	{
		m_VertexBuffer.Cleanup();
		m_IndexBuffer.Cleanup();

		m_ColourMap.Cleanup(device);
		m_NormalMap.Cleanup(device);
	}

}

