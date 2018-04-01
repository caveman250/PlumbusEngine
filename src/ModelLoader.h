#pragma once
#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Model;
class ModelLoader
{
public:
	ModelLoader() {}
	static void CreateInstance() { m_Instance = new ModelLoader(); }
	static ModelLoader& GetInstance() { return *m_Instance; }

	Model LoadModel(std::string modelPath, std::string texturePath);
	void LoadTexture(std::string texturePath, VkImage& textureImage, VkDeviceMemory& textureImageMemory);
private:
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	static ModelLoader* m_Instance;
};