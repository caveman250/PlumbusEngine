#include "plumbus.h"

#include "renderer/base/Model.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/cimport.h"

namespace plumbus::base
{
    void Model::LoadFromFile(const std::string& fileName,
                      std::vector<VertexLayoutComponent> vertLayoutComponents,
                      std::vector<float>& vertexBuffer,
                      std::vector<uint32_t>& indexBuffer)
    {
        const int flags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;
        
        Assimp::Importer Importer;
        const aiScene* scene;
        
        scene = Importer.ReadFile(fileName.c_str(), flags);
        if (!scene)
        {
            Log::Fatal(Importer.GetErrorString());
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
                
                parts[i] = ModelPart();
                parts[i].m_VertexBase = vertexCount;
                parts[i].m_IndexBase = indexCount;
                
                vertexCount += scene->mMeshes[i]->mNumVertices;
                
                aiColor3D pColor(0.f, 0.f, 0.f);
                scene->mMaterials[paiMesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, pColor);
                
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
                            case VERTEX_COMPONENT_DUMMY_FLOAT:
                                vertexBuffer.push_back(1.0f);
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
        }
        else
        {
            Log::Fatal("Error loading model");
        }
    }
}
