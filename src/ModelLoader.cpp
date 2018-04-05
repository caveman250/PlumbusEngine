#include "ModelLoader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj/tiny_obj_loader.h"
#include <unordered_map>
#include "Vertex.h"
#include "Model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "Application.h"
#include "ModelManager.h"
#include "ImageHelpers.h"
#include "Helpers.h"

ModelLoader* ModelLoader::m_Instance = nullptr;

Model ModelLoader::LoadModel(std::string modelPath, std::string texturePath)
{
	Model model;
	model.m_Texture = texturePath;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, modelPath.c_str()))
	{
		Helpers::LogFatal(err.data());
	}
	
	std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
	
	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex = {};
	
			vertex.m_Pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};
	
			vertex.m_TexCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};
	
			vertex.m_Colour = { 1.0f, 1.0f, 1.0f };
	
	
			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(model.m_Vertices.size());
				model.m_Vertices.push_back(vertex);
			}
	
			model.m_Indices.push_back(uniqueVertices[vertex]);
		}
	}

	uint32_t vBufferSize = static_cast<uint32_t>(model.m_Vertices.size()) * sizeof(Vertex);
	uint32_t iBufferSize = static_cast<uint32_t>(model.m_Indices.size()) * sizeof(uint32_t);

	vk::Buffer vertexStaging, indexStaging;

	// Vertex buffer staging
	if (Application::Get().CreateBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&vertexStaging,
		vBufferSize,
		model.m_Vertices.data()) != VK_SUCCESS)
		Helpers::LogFatal("failed to create vertex staging buffer");

	// Index buffer staging
	if (Application::Get().CreateBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&indexStaging,
		iBufferSize,
		model.m_Indices.data()) != VK_SUCCESS)
		Helpers::LogFatal("failed to create index staging buffer");
	// Create device local target buffers
	// Vertex buffer
	if (Application::Get().CreateBuffer(
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&model.m_VertexBuffer,
		vBufferSize) != VK_SUCCESS)
		Helpers::LogFatal("failed to create vertex buffer");

	// Index buffer
	if(Application::Get().CreateBuffer(
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&model.m_IndexBuffer,
		iBufferSize) != VK_SUCCESS)
		Helpers::LogFatal("failed to create index buffer");

	// Copy from staging buffers
	VkCommandBuffer copyCmd = Application::Get().CreateCommandBuffer();

	VkBufferCopy copyRegion{};

	copyRegion.size = model.m_VertexBuffer.m_Size;
	vkCmdCopyBuffer(copyCmd, vertexStaging.m_Buffer, model.m_VertexBuffer.m_Buffer, 1, &copyRegion);

	copyRegion.size = model.m_IndexBuffer.m_Size;
	vkCmdCopyBuffer(copyCmd, indexStaging.m_Buffer, model.m_IndexBuffer.m_Buffer, 1, &copyRegion);

	Application::Get().FlushCommandBuffer(copyCmd);

	// Destroy staging resources
	vkDestroyBuffer(Application::Get().GetDevice(), vertexStaging.m_Buffer, nullptr);
	vkFreeMemory(Application::Get().GetDevice(), vertexStaging.m_Memory, nullptr);
	vkDestroyBuffer(Application::Get().GetDevice(), indexStaging.m_Buffer, nullptr);
	vkFreeMemory(Application::Get().GetDevice(), indexStaging.m_Memory, nullptr);

	return model;
}

void ModelLoader::LoadTexture(std::string texturePath, VkImage& textureImage, VkDeviceMemory& textureImageMemory)
{
	Application& app = Application::Get();

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		Helpers::LogFatal("failed to load texture image!");
	}

	vk::Buffer stagingBuffer;

	app.CreateBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer,
		imageSize);

	void* data;
	vkMapMemory(app.GetDevice(), stagingBuffer.m_Memory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(app.GetDevice(), stagingBuffer.m_Memory);

	stbi_image_free(pixels);

	ImageHelpers::CreateImage(texWidth,
		texHeight,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		textureImage,
		textureImageMemory);

	ImageHelpers::TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	ImageHelpers::CopyBufferToImage(stagingBuffer.m_Buffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	ImageHelpers::TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(app.GetDevice(), stagingBuffer.m_Buffer, nullptr);
	vkFreeMemory(app.GetDevice(), stagingBuffer.m_Memory, nullptr);
}
