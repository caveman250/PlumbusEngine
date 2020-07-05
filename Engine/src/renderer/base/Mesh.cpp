#include "plumbus.h"

#include "renderer/base/Mesh.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/cimport.h"

#if VULKAN_RENDERER
#include "renderer/vk/Mesh.h"
#endif

#include "assimp/types.h"

namespace plumbus::base
{
	Mesh::Mesh()
		: m_ColourMap(nullptr)
		, m_NormalMap(nullptr)
		, m_Material(nullptr)
	{

	}

    std::vector<base::Mesh*> Mesh::LoadFromFile(const std::string& fileName,
                        std::vector<VertexLayoutComponent> vertLayoutComponents,
		                std::string defaultDiffuseTexture,
		                std::string defaultNormalTexture)
    {
        std::vector<base::Mesh*> meshes;

        const int flags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;
        
        Assimp::Importer Importer;
        const aiScene* scene;
        
        scene = Importer.ReadFile(fileName.c_str(), flags);
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
				scene->mMaterials[paiMesh->mMaterialIndex]->Get(_AI_MATKEY_TEXTURE_BASE, aiTextureType_DIFFUSE, 0, diffusePath);
				aiString normalPath;
				scene->mMaterials[paiMesh->mMaterialIndex]->Get(_AI_MATKEY_TEXTURE_BASE, aiTextureType_NORMALS, 0, normalPath);

				if (diffusePath.length == 0)
				{
                    Log::Warn("No diffuse texture defined for submesh: %i, in file: %s", i, fileName.c_str());
					diffusePath = defaultDiffuseTexture;
				}

				if (normalPath.length == 0)
				{
                    Log::Warn("No normal texture defined for submesh: %i, in file: %s", i, fileName.c_str());
                    normalPath = defaultNormalTexture;
				}

                base::Mesh* newModel;
#if VULKAN_RENDERER
                newModel = new vk::Mesh();
#else
                newModel = new mtl::Model();
#endif

                meshes.push_back(newModel);

                parts[i] = ModelPart();
                parts[i].m_VertexBase = vertexCount;
                parts[i].m_IndexBase = indexCount;
                
                vertexCount += scene->mMeshes[i]->mNumVertices;

				aiColor3D pColor(0.f, 0.f, 0.f);
				scene->mMaterials[paiMesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, pColor);
                
                meshes.back()->GetColourMap()->LoadTexture(std::string("textures/") + diffusePath.C_Str());
			    meshes.back()->GetNormalMap()->LoadTexture(std::string("textures/") + normalPath.C_Str());

                const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
                
                Dimension dim;
                
                VertexLayout layout = VertexLayout(vertLayoutComponents);
                
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
                                meshes.back()->GetVertexBuffer().push_back(pPos->x * scale.x + center.x);
                                meshes.back()->GetVertexBuffer().push_back(-pPos->y * scale.y + center.y);
                                meshes.back()->GetVertexBuffer().push_back(pPos->z * scale.z + center.z);
                                break;
                            case VERTEX_COMPONENT_NORMAL:
                                meshes.back()->GetVertexBuffer().push_back(pNormal->x);
                                meshes.back()->GetVertexBuffer().push_back(-pNormal->y);
                                meshes.back()->GetVertexBuffer().push_back(pNormal->z);
                                break;
                            case VERTEX_COMPONENT_UV:
                                meshes.back()->GetVertexBuffer().push_back(pTexCoord->x * uvscale.s);
                                meshes.back()->GetVertexBuffer().push_back(pTexCoord->y * uvscale.t);
                                break;
                            case VERTEX_COMPONENT_COLOR:
                                meshes.back()->GetVertexBuffer().push_back(pColor.r);
                                meshes.back()->GetVertexBuffer().push_back(pColor.g);
                                meshes.back()->GetVertexBuffer().push_back(pColor.b);
                                break;
                            case VERTEX_COMPONENT_TANGENT:
                                meshes.back()->GetVertexBuffer().push_back(pTangent->x);
                                meshes.back()->GetVertexBuffer().push_back(pTangent->y);
                                meshes.back()->GetVertexBuffer().push_back(pTangent->z);
                                break;
                            case VERTEX_COMPONENT_BITANGENT:
                                meshes.back()->GetVertexBuffer().push_back(pBiTangent->x);
                                meshes.back()->GetVertexBuffer().push_back(pBiTangent->y);
                                meshes.back()->GetVertexBuffer().push_back(pBiTangent->z);
                                break;
                            case VERTEX_COMPONENT_DUMMY_FLOAT:
                                meshes.back()->GetVertexBuffer().push_back(1.0f);
                                break;
                            case VERTEX_COMPONENT_DUMMY_VEC4:
                                meshes.back()->GetVertexBuffer().push_back(0.0f);
                                meshes.back()->GetVertexBuffer().push_back(0.0f);
                                meshes.back()->GetVertexBuffer().push_back(0.0f);
                                meshes.back()->GetVertexBuffer().push_back(0.0f);
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
                
                uint32_t indexBase = static_cast<uint32_t>(meshes.back()->GetIndexBuffer().size());
                for (unsigned int j = 0; j < paiMesh->mNumFaces; j++)
                {
                    const aiFace& Face = paiMesh->mFaces[j];
                    if (Face.mNumIndices != 3)
                        continue;
                    meshes.back()->GetIndexBuffer().push_back(indexBase + Face.mIndices[0]);
                    meshes.back()->GetIndexBuffer().push_back(indexBase + Face.mIndices[1]);
                    meshes.back()->GetIndexBuffer().push_back(indexBase + Face.mIndices[2]);
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

	Texture* Mesh::GetColourMap()
	{
		return m_ColourMap;
	}

	Texture* Mesh::GetNormalMap()
	{
		return m_NormalMap;
	}

    std::vector < base::Mesh*> Mesh::LoadModel(const std::string& fileName, std::string defaultTexturePath, std::string defaultNormalPath)
	{
		std::vector<VertexLayoutComponent> vertLayoutComponents;
		vertLayoutComponents.push_back(VERTEX_COMPONENT_POSITION);
		vertLayoutComponents.push_back(VERTEX_COMPONENT_UV);
		vertLayoutComponents.push_back(VERTEX_COMPONENT_COLOR);
		vertLayoutComponents.push_back(VERTEX_COMPONENT_NORMAL);
		vertLayoutComponents.push_back(VERTEX_COMPONENT_TANGENT);

        std::vector<base::Mesh*> models = LoadFromFile(fileName, vertLayoutComponents, defaultTexturePath, defaultNormalPath);

        for (Mesh* model : models)
        {
            model->PostLoad();
        }

        return models;
	}

}
