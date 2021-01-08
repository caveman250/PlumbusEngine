#include "plumbus.h"

#include "renderer/vk/Mesh.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"     
#include "assimp/postprocess.h"
#include "assimp/cimport.h"

#include "BaseApplication.h"
#include "renderer/vk/ImageHelpers.h"
#include "Helpers.h"
#include "renderer/vk/VulkanRenderer.h"
#include "renderer/vk/CommandBuffer.h"
#include "DescriptorSet.h"
#include "PipelineLayout.h"
#include "MaterialInstance.h"
#if PL_PLATFORM_ANDROID
#include "platform/android/Platform.h"
#else
#include "platform/Platform.h"
#endif

namespace plumbus::vk
{
	Mesh::Mesh()
	{
		m_ColourMap = new vk::Texture();
		m_NormalMap = new vk::Texture();
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::PostLoad()
	{
		vk::VulkanRenderer* renderer = VulkanRenderer::Get();
		VkDevice device = renderer->GetDevice()->GetVulkanDevice();

		uint32_t vBufferSize = static_cast<uint32_t>(m_StagingVertexBuffer.size()) * sizeof(float);
		uint32_t iBufferSize = static_cast<uint32_t>(m_StagingIndexBuffer.size()) * sizeof(uint32_t);

		m_IndexSize = (uint32_t)m_StagingIndexBuffer.size();

		Buffer vertexStaging, indexStaging;

		// Vertex buffer staging
		if (renderer->GetDevice()->CreateBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&vertexStaging,
			vBufferSize,
			m_StagingVertexBuffer.data()) != VK_SUCCESS)
			Log::Fatal("failed to create vertex staging buffer");

		// Index buffer staging
		if (renderer->GetDevice()->CreateBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&indexStaging,
			iBufferSize,
			m_StagingIndexBuffer.data()) != VK_SUCCESS)
			Log::Fatal("failed to create index staging buffer");
		// Create device local target buffers
		// Vertex buffer
		if (renderer->GetDevice()->CreateBuffer(
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&m_VulkanVertexBuffer,
			vBufferSize) != VK_SUCCESS)
			Log::Fatal("failed to create vertex buffer");

		// Index buffer
		if (renderer->GetDevice()->CreateBuffer(
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&m_VulkanIndexBuffer,
			iBufferSize) != VK_SUCCESS)
			Log::Fatal("failed to create index buffer");

		// Copy from staging buffers
		VkCommandBuffer copyCmd = renderer->GetDevice()->CreateCommandBuffer();

		VkBufferCopy copyRegion{};

		copyRegion.size = m_VulkanVertexBuffer.m_Size;
		vkCmdCopyBuffer(copyCmd, vertexStaging.m_Buffer, m_VulkanVertexBuffer.m_Buffer, 1, &copyRegion);

		copyRegion.size = m_VulkanIndexBuffer.m_Size;
		vkCmdCopyBuffer(copyCmd, indexStaging.m_Buffer, m_VulkanIndexBuffer.m_Buffer, 1, &copyRegion);

		renderer->GetDevice()->FlushCommandBuffer(copyCmd, renderer->GetDevice()->GetGraphicsQueue());

		// Destroy staging resources
		vkDestroyBuffer(renderer->GetDevice()->GetVulkanDevice(), vertexStaging.m_Buffer, nullptr);
		vkFreeMemory(renderer->GetDevice()->GetVulkanDevice(), vertexStaging.m_Memory, nullptr);
		vkDestroyBuffer(renderer->GetDevice()->GetVulkanDevice(), indexStaging.m_Buffer, nullptr);
		vkFreeMemory(renderer->GetDevice()->GetVulkanDevice(), indexStaging.m_Memory, nullptr);
	}

	void Mesh::Cleanup()
	{
		vk::VulkanRenderer* renderer = VulkanRenderer::Get();

		m_UniformBuffer.Cleanup();
		m_VulkanVertexBuffer.Cleanup();
		m_VulkanIndexBuffer.Cleanup();

		m_ColourMap->Cleanup();
		m_NormalMap->Cleanup();
	}

	void Mesh::Setup()
	{
		if (PL_VERIFY(m_MaterialInstance))
		{
			vk::VulkanRenderer* vkRenderer = VulkanRenderer::Get();
			CreateUniformBuffer(vkRenderer->GetDevice().get());
			SetupUniforms();
		}
	}

	void Mesh::UpdateUniformBuffer(components::ModelComponent::UniformBufferObject& ubo)
	{
		memcpy(m_UniformBuffer.m_Mapped, &ubo, sizeof(ubo));
	}

	void Mesh::CreateUniformBuffer(Device* vulkanDevice)
	{
		CHECK_VK_RESULT(vulkanDevice->CreateBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&m_UniformBuffer,
			sizeof(components::ModelComponent::UniformBufferObject)));

		CHECK_VK_RESULT(m_UniformBuffer.Map());
	}

	void Mesh::SetupUniforms()
	{
		vk::Texture* vkColourMap = static_cast<vk::Texture*>(m_ColourMap);
		vk::Texture* vkNormalMap = static_cast<vk::Texture*>(m_NormalMap);

		m_MaterialInstance->SetBufferUniform("UBO", &m_UniformBuffer);
		m_MaterialInstance->SetTextureUniform("samplerColor", {{vkColourMap->m_TextureSampler, vkColourMap->m_ImageView}}, false);
		m_MaterialInstance->SetTextureUniform("samplerNormalMap", {{vkNormalMap->m_TextureSampler, vkNormalMap->m_ImageView}}, false);
	}

	void Mesh::Render(CommandBufferRef commandBuffer, MaterialInstanceRef overrideMaterial, bool bind)
	{
		MaterialInstanceRef material = overrideMaterial ? overrideMaterial : m_MaterialInstance;
		material->Bind(commandBuffer);
		if(bind)
        {
            commandBuffer->BindVertexBuffer(m_VulkanVertexBuffer);
            commandBuffer->BindIndexBuffer(m_VulkanIndexBuffer);
        }
		commandBuffer->RecordDraw(m_IndexSize);
	}

	Buffer& Mesh::GetVertexBuffer()
	{
		return m_VulkanVertexBuffer;
	}

	Buffer& Mesh::GetIndexBuffer()
	{
		return m_VulkanIndexBuffer;
	}

	void Mesh::SetIndexSize(uint32_t indexSize)
	{
		m_IndexSize = indexSize;
	}

	void Mesh::SetMaterial(MaterialRef material)
	{
		PL_ASSERT(material);
		m_MaterialInstance = MaterialInstance::CreateMaterialInstance(material);
	}

	std::vector<vk::Mesh*> Mesh::LoadFromFile(const std::string& fileName,
                        std::vector<VertexLayoutComponent> vertLayoutComponents,
		                std::string defaultDiffuseTexture,
		                std::string defaultNormalTexture)
    {
        std::vector<vk::Mesh*> meshes;

        const int flags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;
        
        Assimp::Importer Importer;
        const aiScene* scene;

        std::vector<char> fileContents = Helpers::ReadBinaryFile(fileName.c_str());
        scene = Importer.ReadFileFromMemory(fileContents.data(), fileContents.size(), flags);
        if (!scene)
        {
            Log::Error(Importer.GetErrorString());
        }
        
        std::vector<ModelPart> parts;
        
        if (scene)
        {
            parts.clear();
            parts.resize(scene->mNumMeshes);
            
            glm::vec3 scale(1.0f);
            glm::vec2 uvscale(1.0f);
			glm::vec3 center(0.0f);

            int vertexCount = 0;
            int indexCount = 0;
            
            for (unsigned int i = 0; i < scene->mNumMeshes; i++)
            {
                const aiMesh* paiMesh = scene->mMeshes[i];

                aiString diffusePath;
                aiReturn hasDiffuse = scene->mMaterials[paiMesh->mMaterialIndex]->Get(_AI_MATKEY_TEXTURE_BASE, aiTextureType_DIFFUSE, 0, diffusePath);
                aiString normalPath;
                aiReturn hasNormal = scene->mMaterials[paiMesh->mMaterialIndex]->Get(_AI_MATKEY_TEXTURE_BASE, aiTextureType_NORMALS, 0, normalPath);

                if (hasDiffuse != AI_SUCCESS)
                {
                    Log::Warn("No diffuse texture defined for submesh: %i, in file: %s", i, fileName.c_str());
                    diffusePath = defaultDiffuseTexture + Platform::GetTextureExtension();
                }

                if (hasNormal != AI_SUCCESS)
                {
                    Log::Warn("No normal texture defined for submesh: %i, in file: %s", i, fileName.c_str());
                    normalPath = defaultNormalTexture + Platform::GetTextureExtension();
                }

                vk::Mesh* newModel = new vk::Mesh();

                meshes.push_back(newModel);

                parts[i] = ModelPart();
                parts[i].m_VertexBase = vertexCount;
                parts[i].m_IndexBase = indexCount;
                
                vertexCount += scene->mMeshes[i]->mNumVertices;

				aiColor3D pColor(0.f, 0.f, 0.f);
				scene->mMaterials[paiMesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, pColor);
                
                meshes.back()->GetColourMap()->LoadTexture(Platform::GetTextureDirPath() + diffusePath.C_Str());
			    meshes.back()->GetNormalMap()->LoadTexture(Platform::GetTextureDirPath() + normalPath.C_Str());

                const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
                
                Dimension dim;
                for (unsigned int j = 0; j < paiMesh->mNumVertices; j++)
                {
                    const aiVector3D* pPos = &(paiMesh->mVertices[j]);
                    const aiVector3D* pNormal = &(paiMesh->mNormals[j]);
                    const aiVector3D* pTexCoord = (paiMesh->HasTextureCoords(0)) ? &(paiMesh->mTextureCoords[0][j]) : &Zero3D;
                    const aiVector3D* pTangent = (paiMesh->HasTangentsAndBitangents()) ? &(paiMesh->mTangents[j]) : &Zero3D;
                    const aiVector3D* pBiTangent = (paiMesh->HasTangentsAndBitangents()) ? &(paiMesh->mBitangents[j]) : &Zero3D;
                    
                    for (auto& component : vertLayoutComponents)
                    {
                        switch (component)
                        {
                            case VertexLayoutComponent::Position:
                                meshes.back()->GetStagingVertexBuffer().push_back(pPos->x * scale.x + center.x);
                                meshes.back()->GetStagingVertexBuffer().push_back(-pPos->y * scale.y + center.y);
                                meshes.back()->GetStagingVertexBuffer().push_back(pPos->z * scale.z + center.z);
                                break;
                            case VertexLayoutComponent::Normal:
                                meshes.back()->GetStagingVertexBuffer().push_back(pNormal->x);
                                meshes.back()->GetStagingVertexBuffer().push_back(-pNormal->y);
                                meshes.back()->GetStagingVertexBuffer().push_back(pNormal->z);
                                break;
                            case VertexLayoutComponent::UV:
                                meshes.back()->GetStagingVertexBuffer().push_back(pTexCoord->x * uvscale.s);
                                meshes.back()->GetStagingVertexBuffer().push_back(pTexCoord->y * uvscale.t);
                                break;
                            case VertexLayoutComponent::Colour:
                                meshes.back()->GetStagingVertexBuffer().push_back(pColor.r);
                                meshes.back()->GetStagingVertexBuffer().push_back(pColor.g);
                                meshes.back()->GetStagingVertexBuffer().push_back(pColor.b);
                                break;
                            case VertexLayoutComponent::Tangent:
                                meshes.back()->GetStagingVertexBuffer().push_back(pTangent->x);
                                meshes.back()->GetStagingVertexBuffer().push_back(pTangent->y);
                                meshes.back()->GetStagingVertexBuffer().push_back(pTangent->z);
                                break;
                            case VertexLayoutComponent::Bitangent:
                                meshes.back()->GetStagingVertexBuffer().push_back(pBiTangent->x);
                                meshes.back()->GetStagingVertexBuffer().push_back(pBiTangent->y);
                                meshes.back()->GetStagingVertexBuffer().push_back(pBiTangent->z);
                                break;
                            case VertexLayoutComponent::DummyFloat:
                                meshes.back()->GetStagingVertexBuffer().push_back(1.0f);
                                break;
                            case VertexLayoutComponent::DummyVec4:
                                meshes.back()->GetStagingVertexBuffer().push_back(0.0f);
                                meshes.back()->GetStagingVertexBuffer().push_back(0.0f);
                                meshes.back()->GetStagingVertexBuffer().push_back(0.0f);
                                meshes.back()->GetStagingVertexBuffer().push_back(0.0f);
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
                
                uint32_t indexBase = static_cast<uint32_t>(meshes.back()->GetStagingIndexBuffer().size());
                for (unsigned int j = 0; j < paiMesh->mNumFaces; j++)
                {
                    const aiFace& Face = paiMesh->mFaces[j];
                    if (Face.mNumIndices != 3)
                        continue;
                    meshes.back()->GetStagingIndexBuffer().push_back(indexBase + Face.mIndices[0]);
                    meshes.back()->GetStagingIndexBuffer().push_back(indexBase + Face.mIndices[1]);
                    meshes.back()->GetStagingIndexBuffer().push_back(indexBase + Face.mIndices[2]);
                    parts[i].m_IndexCount += 3;
                    indexCount += 3;
                }
            }
        }
        else
        {
            Log::Error("Error loading model");
        }

        return meshes;
    }

	std::vector<Mesh*> Mesh::LoadModel(const std::string& fileName, std::string defaultTexturePath, std::string defaultNormalPath)
	{
		std::vector<VertexLayoutComponent> vertLayoutComponents;
		vertLayoutComponents.push_back(VertexLayoutComponent::Position);
		vertLayoutComponents.push_back(VertexLayoutComponent::UV);
		vertLayoutComponents.push_back(VertexLayoutComponent::Colour);
		vertLayoutComponents.push_back(VertexLayoutComponent::Normal);
		vertLayoutComponents.push_back(VertexLayoutComponent::Tangent);

        std::vector<Mesh*> models = LoadFromFile(fileName, vertLayoutComponents, defaultTexturePath, defaultNormalPath);

        for (Mesh* model : models)
        {
            model->PostLoad();
        }

        return models;
	}

}

