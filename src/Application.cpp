#include "Application.h"

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <set>
#include <algorithm>
#include "Helpers.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <cstring>

#include "ImageHelpers.h"
#include "GameObject.h"
#include "components/Component.h"
#include "components/ModelComponent.h"
#include "vk/Model.h"
#include "components/TranslationComponent.h"
#include "Camera.h"
#include "components/PointLightComponent.h"

bool debugDisplay = false;

const int WIDTH = 1280;
const int HEIGHT = 720;

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Vertex layout for the models
vk::VertexLayout vertexLayout = vk::VertexLayout(
	{
	vk::VERTEX_COMPONENT_POSITION,
	vk::VERTEX_COMPONENT_UV,
	vk::VERTEX_COMPONENT_COLOR,
	vk::VERTEX_COMPONENT_NORMAL,
	vk::VERTEX_COMPONENT_TANGENT,
	});

const std::vector<const char*> validationLayers = 
{
	//"VK_LAYER_LUNARG_standard_validation",
	//"VK_LAYER_RENDERDOC_Capture"
};

//#ifdef NDEBUG
//const bool enableValidationLayers = false;
//#else
//const bool enableValidationLayers = true;
//#endif
const bool enableValidationLayers = true;
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData)
{
	std::cerr << "validation layer: " << msg << std::endl;

	return VK_FALSE;
}

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}

Application* Application::m_Instance = nullptr;

Application::Application() :
	m_Scene(new Scene()),
	m_lastUpdateTime(glfwGetTime())
{
}

void Application::Run()
{
	InitWindow();
	InitVulkan();
	InitScene();
	MainLoop();
	Cleanup();
}

void Application::InitWindow()
{
	glfwInit();
	//dont use OpenGl
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Super Awesome Happy Times", nullptr, nullptr);

	glfwSetWindowUserPointer(m_Window, this);
	glfwSetWindowSizeCallback(m_Window, Application::OnWindowResized);
	glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void Application::InitScene()
{
	//probs have to do more here
	m_Scene->Init();
}

void Application::InitVulkan()
{
	CreateVulkanInstance();
	SetupDebugCallback();
	CreateSurface();
	PickPhysicalDevice();
	//Create the vulkan "side" of the device, what we use to speak to it and control it.
	m_VulkanDevice = new vk::VulkanDevice(m_PhysicalDevice, m_Surface);
	m_VulkanDevice->CreateLogicalDevice(deviceExtensions, validationLayers, true);

	vkGetDeviceQueue(m_VulkanDevice->GetDevice(), m_VulkanDevice->GetQueueFamilyIndices().m_GraphicsFamily, 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_VulkanDevice->GetDevice(), m_VulkanDevice->GetQueueFamilyIndices().m_PresentFamily, 0, &m_PresentQueue);

	CreateSwapChain();
	CreateImageViews();
	CreateCommandBuffers();
	CreateRenderPass();
	CreatePipelineCache();
	CreateDepthResources();
	CreateFrameBuffers();
	CreateSemaphores();

	GameObject* obj = new GameObject();
	m_Scene->AddGameObject(obj->
		AddComponent<ModelComponent>(new ModelComponent("models/armor.dae", "textures/color_bc3_unorm.ktx", "textures/normal_bc3_unorm.ktx"))->
		AddComponent<TranslationComponent>(new TranslationComponent())
	);

	GameObject* plane = new GameObject();
	m_Scene->AddGameObject(plane->
		AddComponent<ModelComponent>(new ModelComponent("models/plane.obj", "textures/stonefloor01_color_bc3_unorm.ktx", "textures/stonefloor01_normal_bc3_unorm.ktx"))->
		AddComponent<TranslationComponent>(new TranslationComponent())
	);

	plane->GetComponent<TranslationComponent>()->SetTranslation(glm::vec3(0, 2.3, 0));

	//Lights
	glm::vec3 colours[] =
	{
		glm::vec3(1.5f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 2.5f),
		glm::vec3(1.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.2f),
		glm::vec3(1.0f, 0.7f, 0.3f)
	};

	for (int i = 0; i < 6; ++i)
	{
		GameObject* light = new GameObject();
		m_Scene->AddGameObject(light->
			AddComponent<TranslationComponent>(new TranslationComponent())->
			AddComponent<PointLightComponent>(new PointLightComponent(colours[i], 25.0f)));
	}

	m_Scene->LoadModels(m_GraphicsQueue, vertexLayout);

	GenerateQuads();
	CreateVertexDescriptions();
	m_OffscreenFrameBuffer = new vk::FrameBuffer();
	m_OffscreenFrameBuffer->PrepareOffscreenFramebuffer();
	InitLightsVBO();
	CreateUniformBuffers();

	CreateDescriptorSetLayout();
	CreatePipelines();
	CreateDescriptorPool();
	CreateDescriptorSet();
	BuildCommandBuffers();
	BuildDefferedCommandBuffer();
}

void Application::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, devices.data());

	std::cout << "Found " << devices.size() << " device(s)" << std::endl;
	for (const auto& device : devices)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		std::cout << deviceProperties.deviceName << std::endl;
	}

	for (const auto& device : devices)
	{
		if (IsDeviceSuitable(device))
		{
			m_PhysicalDevice = device;
			break;
		}
	}

	if (m_PhysicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

bool Application::IsDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		std::cout << "Device: " << deviceProperties.deviceName << " Invalid! type is not a discrete gpu." << std::endl;
		return false;
	}

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	if (!deviceFeatures.geometryShader)
	{
		std::cout << "Device: " << deviceProperties.deviceName << " Invalid! no geometry shader support." << std::endl;
		return false;
	}

	if (!CheckDeviceExtensionSupport(device))
	{
		std::cout << "Device: " << deviceProperties.deviceName << " Invalid! missing required extension support" << std::endl;
		return false;
	}

	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
	if (swapChainSupport.m_Formats.empty() || swapChainSupport.m_PresentModes.empty())
	{
		std::cout << "Device: " << deviceProperties.deviceName << " Invalid! missing swap chain support." << std::endl;
		return false;
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
	if (!supportedFeatures.samplerAnisotropy)
	{
		std::cout << "Device: " << deviceProperties.deviceName << " Invalid! missing missing Anisotropy sampler support." << std::endl;
		return false;
	}

	return true;
}

bool Application::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());


	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	std::cout << "Device: " << deviceProperties.deviceName << ": Available extensions:" << std::endl;
	for (VkExtensionProperties prop : availableExtensions)
	{
		std::cout << "\t" << prop.extensionName << std::endl;
	}

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void Application::SetupDebugCallback()
{
	if (!enableValidationLayers)
		return;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = debugCallback;

	if (CreateDebugReportCallbackEXT(m_VulkanInstance, &createInfo, nullptr, &m_Callback) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug callback!");
	}
}

void Application::MainLoop()
{
	while (!glfwWindowShouldClose(m_Window))
	{
		glfwPollEvents();

		double currTime = glfwGetTime();
		m_DeltaTime = currTime - m_lastUpdateTime;
		m_lastUpdateTime = currTime;

		UpdateScene();
		DrawFrame();
	}

	vkDeviceWaitIdle(m_VulkanDevice->GetDevice());
}

void Application::UpdateScene()
{
	m_Scene->OnUpdate();
}

void Application::DrawFrame()
{
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_VulkanDevice->GetDevice(), m_SwapChain, std::numeric_limits<uint64_t>::max(), m_ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_OffScreenCmdBuffer;

	VkSemaphore signalSemaphores[] = { m_OffscreenSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	CHECK_VK_RESULT(vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

	submitInfo.pWaitSemaphores = &m_OffscreenSemaphore;
	submitInfo.pSignalSemaphores = &m_RenderFinishedSemaphore;

	submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex];
	CHECK_VK_RESULT(vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphore;

	VkSwapchainKHR swapChains[] = { m_SwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	vkQueueWaitIdle(m_PresentQueue);

	UpdateUniformBufferDeferredLights();
}

void Application::Cleanup()
{
	// Color attachments
	vkDestroyImageView(m_VulkanDevice->GetDevice(), m_OffscreenFrameBuffer->m_Position.m_ImageView, nullptr);
	vkDestroyImage(m_VulkanDevice->GetDevice(), m_OffscreenFrameBuffer->m_Position.m_Image, nullptr);
	vkFreeMemory(m_VulkanDevice->GetDevice(), m_OffscreenFrameBuffer->m_Position.m_Memory, nullptr);

	vkDestroyImageView(m_VulkanDevice->GetDevice(), m_OffscreenFrameBuffer->m_Normal.m_ImageView, nullptr);
	vkDestroyImage(m_VulkanDevice->GetDevice(), m_OffscreenFrameBuffer->m_Normal.m_Image, nullptr);
	vkFreeMemory(m_VulkanDevice->GetDevice(), m_OffscreenFrameBuffer->m_Normal.m_Memory, nullptr);

	vkDestroyImageView(m_VulkanDevice->GetDevice(), m_OffscreenFrameBuffer->m_Albedo.m_ImageView, nullptr);
	vkDestroyImage(m_VulkanDevice->GetDevice(), m_OffscreenFrameBuffer->m_Albedo.m_Image, nullptr);
	vkFreeMemory(m_VulkanDevice->GetDevice(), m_OffscreenFrameBuffer->m_Albedo.m_Memory, nullptr);

	vkDestroyImageView(m_VulkanDevice->GetDevice(), m_OffscreenFrameBuffer->m_Depth.m_ImageView, nullptr);
	vkDestroyImage(m_VulkanDevice->GetDevice(), m_OffscreenFrameBuffer->m_Depth.m_Image, nullptr);
	vkFreeMemory(m_VulkanDevice->GetDevice(), m_OffscreenFrameBuffer->m_Depth.m_Memory, nullptr);

	vkDestroySampler(m_VulkanDevice->GetDevice(), m_OffscreenFrameBuffer->m_ColourSampler, nullptr);

	vkDestroyFramebuffer(m_VulkanDevice->GetDevice(), m_OffscreenFrameBuffer->m_FrameBuffer, nullptr);

	vkDestroyPipeline(m_VulkanDevice->GetDevice(), m_Pipelines.m_Deferred, nullptr);
	vkDestroyPipeline(m_VulkanDevice->GetDevice(), m_Pipelines.m_Offscreen, nullptr);
	vkDestroyPipeline(m_VulkanDevice->GetDevice(), m_Pipelines.m_Debug, nullptr);

	vkDestroyPipelineLayout(m_VulkanDevice->GetDevice(), m_PipelineLayouts.m_Deferred, nullptr);
	vkDestroyPipelineLayout(m_VulkanDevice->GetDevice(), m_PipelineLayouts.m_Offscreen, nullptr);

	vkDestroyDescriptorSetLayout(m_VulkanDevice->GetDevice(), m_DescriptorSetLayout, nullptr);

	for (GameObject* obj : m_Scene->GetObjects())
	{
		if (ModelComponent* modelComp = obj->GetComponent<ModelComponent>())
		{
			modelComp->Cleanup(m_VulkanDevice->GetDevice());
		}
	}

	m_Quad.Cleanup(m_VulkanDevice->GetDevice());

	m_UniformBuffers.m_VertFullScreen.Cleanup();
	m_UniformBuffers.m_FragLights.Cleanup();

	vkFreeCommandBuffers(m_VulkanDevice->GetDevice(), m_VulkanDevice->GetCommandPool(), 1, &m_OffScreenCmdBuffer);

	vkDestroyRenderPass(m_VulkanDevice->GetDevice(), m_OffscreenFrameBuffer->m_RenderPass, nullptr);

	CleanupSwapChain();
	vkDestroyDescriptorPool(m_VulkanDevice->GetDevice(), m_DescriptorPool, nullptr);

	for (auto& shaderModule : m_ShaderModules)
	{
		vkDestroyShaderModule(m_VulkanDevice->GetDevice(), shaderModule, nullptr);
	}

	vkDestroyPipelineCache(m_VulkanDevice->GetDevice(), m_PipelineCache, nullptr);
	vkDestroyCommandPool(m_VulkanDevice->GetDevice(), m_VulkanDevice->GetCommandPool(), nullptr);

	vkDestroySemaphore(m_VulkanDevice->GetDevice(), m_RenderFinishedSemaphore, nullptr);
	vkDestroySemaphore(m_VulkanDevice->GetDevice(), m_ImageAvailableSemaphore, nullptr);
	vkDestroySemaphore(m_VulkanDevice->GetDevice(), m_OffscreenSemaphore, nullptr);

	delete m_VulkanDevice;

	DestroyDebugReportCallbackEXT(m_VulkanInstance, m_Callback, nullptr);
	vkDestroySurfaceKHR(m_VulkanInstance, m_Surface, nullptr);
	vkDestroyInstance(m_VulkanInstance, nullptr);

	glfwDestroyWindow(m_Window);

	glfwTerminate();
}

void Application::OnWindowResized(GLFWwindow* window, int width, int height)
{
	Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->RecreateSwapChain();
}

VkFormat Application::FindDepthFormat()
{
	return FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkFormat Application::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

void Application::CreateVulkanInstance()
{
	if (enableValidationLayers && !CheckValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	//App Info
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "SUPER AWESOME MEGA ENGINE YAY";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	//Create Info
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto reqExtensions = GetRequiredExtensions();

	std::cout << "required extensions:" << std::endl;

	for (const auto& extension : reqExtensions)
	{
		std::cout << "\t" << extension << std::endl;
	}

	createInfo.enabledExtensionCount = static_cast<uint32_t>(reqExtensions.size());
	createInfo.ppEnabledExtensionNames = reqExtensions.data();
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateInstance(&createInfo, nullptr, &m_VulkanInstance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << "available extensions:" << std::endl;

	for (const auto& extension : extensions)
	{
		std::cout << "\t" << extension.extensionName << std::endl;
	}
}

std::vector<const char*> Application::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

bool Application::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		std::cout << "Looking for validation layer: " << layerName << std::endl;

		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				std::cout << "Found." << std::endl;
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			std::cout << layerName << " not found." << std::endl;
			return false;
		}
	}

	return true;
}

void Application::CreateSurface()
{
	if (glfwCreateWindowSurface(m_VulkanInstance, m_Window, nullptr, &m_Surface) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create window surface!");
	}
}

void Application::CreatePipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	CHECK_VK_RESULT(vkCreatePipelineCache(m_VulkanDevice->GetDevice(), &pipelineCacheCreateInfo, nullptr, &m_PipelineCache));
}

void Application::GenerateQuads()
{
	// Setup vertices for multiple screen aligned quads
	// Used for displaying final result and debug 
	struct Vert {
		float pos[3];
		float uv[2];
		float col[3];
		float normal[3];
		float tangent[3];
	};

	std::vector<Vert> vertexBuffer;

	float x = 0.0f;
	float y = 0.0f;
	for (uint32_t i = 0; i < 3; i++)
	{
		// Last component of normal is used for debug display sampler index
		vertexBuffer.push_back({ { x + 1.0f, y + 1.0f, 0.0f },{ 1.0f, 1.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, (float)i } });
		vertexBuffer.push_back({ { x,      y + 1.0f, 0.0f },{ 0.0f, 1.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, (float)i } });
		vertexBuffer.push_back({ { x,      y,      0.0f },{ 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, (float)i } });
		vertexBuffer.push_back({ { x + 1.0f, y,      0.0f },{ 1.0f, 0.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, (float)i } });
		x += 1.0f;
		if (x > 1.0f)
		{
			x = 0.0f;
			y += 1.0f;
		}
	}

	CHECK_VK_RESULT(m_VulkanDevice->CreateBuffer(
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&m_Quad.m_VertexBuffer,
		vertexBuffer.size() * sizeof(Vert),
		vertexBuffer.data()));


	// Setup indices
	std::vector<uint32_t> indexBuffer = { 0,1,2, 2,3,0 };
	for (uint32_t i = 0; i < 3; ++i)
	{
		uint32_t indices[6] = { 0,1,2, 2,3,0 };
		for (auto index : indices)
		{
			indexBuffer.push_back(i * 4 + index);
		}
	}

	m_Quad.m_IndexSize = (uint32_t)indexBuffer.size();

	CHECK_VK_RESULT(m_VulkanDevice->CreateBuffer(
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&m_Quad.m_IndexBuffer,
		indexBuffer.size() * sizeof(uint32_t),
		indexBuffer.data()));
}

void Application::CreateVertexDescriptions()
{
	// Binding description
	VkVertexInputBindingDescription vInputBindDescription{};
	vInputBindDescription.binding = 0;
	vInputBindDescription.stride = vertexLayout.stride();
	vInputBindDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	m_VertexDescriptions.m_BindingDescriptions.resize(1);
	m_VertexDescriptions.m_BindingDescriptions[0] = vInputBindDescription;
	
	// Attribute descriptions
	m_VertexDescriptions.m_AttributeDescriptions.resize(5);

	// Location 0: Position
	VkVertexInputAttributeDescription posInput{};
	posInput.location = 0;
	posInput.binding = 0;
	posInput.format = VK_FORMAT_R32G32B32_SFLOAT;
	posInput.offset = 0;
	m_VertexDescriptions.m_AttributeDescriptions[0] = posInput;

	// Location 1: Texture coordinates
	VkVertexInputAttributeDescription texCoordInput{};
	texCoordInput.location = 1;
	texCoordInput.binding = 0;
	texCoordInput.format = VK_FORMAT_R32G32B32_SFLOAT;
	texCoordInput.offset = sizeof(float) * 3;
	m_VertexDescriptions.m_AttributeDescriptions[1] = texCoordInput;

	// Location 2: Colour
	VkVertexInputAttributeDescription colourInput{};
	colourInput.location = 2;
	colourInput.binding = 0;
	colourInput.format = VK_FORMAT_R32G32B32_SFLOAT;
	colourInput.offset = sizeof(float) * 5;
	m_VertexDescriptions.m_AttributeDescriptions[2] = colourInput;
		
	// Location 3: Normal
	VkVertexInputAttributeDescription normalInput{};
	normalInput.location = 3;
	normalInput.binding = 0;
	normalInput.format = VK_FORMAT_R32G32B32_SFLOAT;
	normalInput.offset = sizeof(float) * 8;
	m_VertexDescriptions.m_AttributeDescriptions[3] = normalInput;

	// Location 4: Tangent
	VkVertexInputAttributeDescription tangentInput{};
	tangentInput.location = 4;
	tangentInput.binding = 0;
	tangentInput.format = VK_FORMAT_R32G32B32_SFLOAT;
	tangentInput.offset = sizeof(float) * 11;
	m_VertexDescriptions.m_AttributeDescriptions[4] = tangentInput;

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
	pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	m_VertexDescriptions.m_InputState = pipelineVertexInputStateCreateInfo;
	m_VertexDescriptions.m_InputState.vertexBindingDescriptionCount = static_cast<uint32_t>(m_VertexDescriptions.m_BindingDescriptions.size());
	m_VertexDescriptions.m_InputState.pVertexBindingDescriptions = m_VertexDescriptions.m_BindingDescriptions.data();
	m_VertexDescriptions.m_InputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_VertexDescriptions.m_AttributeDescriptions.size());
	m_VertexDescriptions.m_InputState.pVertexAttributeDescriptions = m_VertexDescriptions.m_AttributeDescriptions.data();
}

void Application::InitLightsVBO()
{
	for (GameObject* obj : m_Scene->GetObjects())
	{
		int index = 0;
		if (PointLightComponent* comp = obj->GetComponent<PointLightComponent>())
		{
			if (index >= MAX_LIGHTS)
				Helpers::LogFatal("Light count exceeds MAX_LIGHTS");

			LightBufferInfo info = {};
			info.m_Colour = comp->GetColour();
			info.m_Radius = comp->GetRadius();

			TranslationComponent* trans = obj->GetComponent<TranslationComponent>();
			trans->SetTranslation(glm::vec3(5.f, 1.f, 0.f));
			info.m_Position = glm::vec4(trans->GetTranslation(), 0.f);

			m_LightsUBO.m_Lights[index] = info;
			++index;
		}
	}
}

void Application::CreateUniformBuffers()
{
	// Fullscreen vertex shader
	CHECK_VK_RESULT(m_VulkanDevice->CreateBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&m_UniformBuffers.m_VertFullScreen,
		sizeof(m_VertUBO)));


	for (GameObject* obj : m_Scene->GetObjects())
	{
		if (ModelComponent* comp = obj->GetComponent<ModelComponent>())
		{
			comp->CreateUniformBuffer(m_VulkanDevice);
		}
	}

	// Deferred fragment shader
	CHECK_VK_RESULT(m_VulkanDevice->CreateBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&m_UniformBuffers.m_FragLights,
		sizeof(m_LightsUBO)));

	// Map persistent
	CHECK_VK_RESULT(m_UniformBuffers.m_VertFullScreen.Map());
	
	CHECK_VK_RESULT(m_UniformBuffers.m_FragLights.Map());

	// Update
	UpdateUniformBuffersScreen();
	UpdateUniformBufferDeferredLights();
}

void Application::CreateDescriptorSetLayout()
{
	// Binding 0 : Vertex shader uniform buffer
	VkDescriptorSetLayoutBinding vertUniformBufferBinding{};
	vertUniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vertUniformBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	vertUniformBufferBinding.binding = 0;
	vertUniformBufferBinding.descriptorCount = 1;

	// Binding 1 : Position
	VkDescriptorSetLayoutBinding positionBinding{};
	positionBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	positionBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	positionBinding.binding = 1;
	positionBinding.descriptorCount = 1;

	// Binding 2 : Normal
	VkDescriptorSetLayoutBinding normalBinding{};
	normalBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	normalBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	normalBinding.binding = 2;
	normalBinding.descriptorCount = 1;

	// Binding 3 : Albedo
	VkDescriptorSetLayoutBinding albedoBinding{};
	albedoBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	albedoBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	albedoBinding.binding = 3;
	albedoBinding.descriptorCount = 1;

	VkDescriptorSetLayoutBinding descriptorSetBinding{};
	descriptorSetBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorSetBinding.binding = 4;
	descriptorSetBinding.descriptorCount = 1;

	// Deferred shading layout
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
	{
		vertUniformBufferBinding,
		positionBinding,
		normalBinding,
		albedoBinding,
		descriptorSetBinding,
	};

	VkDescriptorSetLayoutCreateInfo descriptorLayout{};
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pBindings = setLayoutBindings.data();
	descriptorLayout.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());

	CHECK_VK_RESULT(vkCreateDescriptorSetLayout(m_VulkanDevice->GetDevice(), &descriptorLayout, nullptr, &m_DescriptorSetLayout));

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;

	CHECK_VK_RESULT(vkCreatePipelineLayout(m_VulkanDevice->GetDevice(), &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayouts.m_Deferred));

	// Offscreen (scene) rendering pipeline layout
	CHECK_VK_RESULT(vkCreatePipelineLayout(m_VulkanDevice->GetDevice(), &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayouts.m_Offscreen));
}

void Application::CreatePipelines()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyState.flags = 0;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo rasterizationState{};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationState.flags = 0;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f;

	VkPipelineColorBlendAttachmentState blendAttachmentState{};
	blendAttachmentState.colorWriteMask = 0xf;
	blendAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendState{};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &blendAttachmentState;

	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilState.front = depthStencilState.back;
	depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	viewportState.flags = 0;

	VkPipelineMultisampleStateCreateInfo multisampleState{};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.flags = 0;

	std::vector<VkDynamicState> dynamicStateEnables = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
	dynamicState.flags = 0;

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.layout = m_PipelineLayouts.m_Deferred;
	pipelineCreateInfo.renderPass = m_RenderPass;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.basePipelineIndex = -1;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	// Final fullscreen composition pass pipeline
	shaderStages[0] = LoadShader("shaders/deferred.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("shaders/deferred.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	// Empty vertex input state, quads are generated by the vertex shader
	VkPipelineVertexInputStateCreateInfo emptyInputState{};
	emptyInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineCreateInfo.pVertexInputState = &emptyInputState;
	pipelineCreateInfo.layout = m_PipelineLayouts.m_Deferred;
	CHECK_VK_RESULT(vkCreateGraphicsPipelines(m_VulkanDevice->GetDevice(), m_PipelineCache, 1, &pipelineCreateInfo, nullptr, &m_Pipelines.m_Deferred));

	// Debug display pipeline
	pipelineCreateInfo.pVertexInputState = &m_VertexDescriptions.m_InputState;
	shaderStages[0] = LoadShader("shaders/debug.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("shaders/debug.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	CHECK_VK_RESULT(vkCreateGraphicsPipelines(m_VulkanDevice->GetDevice(), m_PipelineCache, 1, &pipelineCreateInfo, nullptr, &m_Pipelines.m_Debug));

	// Offscreen pipeline
	shaderStages[0] = LoadShader("shaders/shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader("shaders/shader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	// Separate render pass
	pipelineCreateInfo.renderPass = m_OffscreenFrameBuffer->m_RenderPass;

	// Separate layout
	pipelineCreateInfo.layout = m_PipelineLayouts.m_Offscreen;

	// Blend attachment states required for all color attachments
	// This is important, as color write mask will otherwise be 0x0 and you
	// won't see anything rendered to the attachment
	VkPipelineColorBlendAttachmentState blendAttachmentState1{};
	blendAttachmentState1.colorWriteMask = 0xf;
	blendAttachmentState1.blendEnable = VK_FALSE;
	VkPipelineColorBlendAttachmentState blendAttachmentState2{};
	blendAttachmentState2.colorWriteMask = 0xf;
	blendAttachmentState2.blendEnable = VK_FALSE;
	VkPipelineColorBlendAttachmentState blendAttachmentState3{};
	blendAttachmentState3.colorWriteMask = 0xf;
	blendAttachmentState3.blendEnable = VK_FALSE;

	std::array<VkPipelineColorBlendAttachmentState, 3> blendAttachmentStates = {
		blendAttachmentState1,
		blendAttachmentState2,
		blendAttachmentState3
	};

	colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
	colorBlendState.pAttachments = blendAttachmentStates.data();

	CHECK_VK_RESULT(vkCreateGraphicsPipelines(m_VulkanDevice->GetDevice(), m_PipelineCache, 1, &pipelineCreateInfo, nullptr, &m_Pipelines.m_Offscreen));
}

void Application::CreateDescriptorPool()
{
	VkDescriptorPoolSize uniformPoolSize{};
	uniformPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformPoolSize.descriptorCount = 8;

	VkDescriptorPoolSize imageSamplerPoolSize{};
	imageSamplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	imageSamplerPoolSize.descriptorCount = 9;

	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		uniformPoolSize,
		imageSamplerPoolSize
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = 3;

	CHECK_VK_RESULT(vkCreateDescriptorPool(m_VulkanDevice->GetDevice(), &descriptorPoolInfo, nullptr, &m_DescriptorPool));
}

void Application::CreateDescriptorSet()
{
	std::vector<VkWriteDescriptorSet> writeDescriptorSets;

	// Textured quad descriptor set
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.pSetLayouts = &m_DescriptorSetLayout;
	allocInfo.descriptorSetCount = 1;

	CHECK_VK_RESULT(vkAllocateDescriptorSets(m_VulkanDevice->GetDevice(), &allocInfo, &m_DescriptorSet));

	// Image descriptors for the offscreen color attachments
	VkDescriptorImageInfo texDescriptorPosition{};
	texDescriptorPosition.sampler = m_OffscreenFrameBuffer->m_ColourSampler;
	texDescriptorPosition.imageView = m_OffscreenFrameBuffer->m_Position.m_ImageView;
	texDescriptorPosition.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkDescriptorImageInfo texDescriptorNormal{};
	texDescriptorNormal.sampler = m_OffscreenFrameBuffer->m_ColourSampler;
	texDescriptorNormal.imageView = m_OffscreenFrameBuffer->m_Normal.m_ImageView;
	texDescriptorNormal.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkDescriptorImageInfo texDescriptorAlbedo{};
	texDescriptorAlbedo.sampler = m_OffscreenFrameBuffer->m_ColourSampler;
	texDescriptorAlbedo.imageView = m_OffscreenFrameBuffer->m_Albedo.m_ImageView;
	texDescriptorAlbedo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	// Binding 0 : Vertex shader uniform buffer
	VkWriteDescriptorSet uniformWriteSet{};
	uniformWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformWriteSet.dstSet = m_DescriptorSet;
	uniformWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformWriteSet.dstBinding = 0;
	uniformWriteSet.pBufferInfo = &m_UniformBuffers.m_VertFullScreen.m_Descriptor;
	uniformWriteSet.descriptorCount = 1;

	// Binding 1 : Position texture target
	VkWriteDescriptorSet positionWriteSet{};
	positionWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	positionWriteSet.dstSet = m_DescriptorSet;
	positionWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	positionWriteSet.dstBinding = 1;
	positionWriteSet.pImageInfo = &texDescriptorPosition;
	positionWriteSet.descriptorCount = 1;

	// Binding 2 : Normals texture target
	VkWriteDescriptorSet normalWriteSet{};
	normalWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	normalWriteSet.dstSet = m_DescriptorSet;
	normalWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	normalWriteSet.dstBinding = 2;
	normalWriteSet.pImageInfo = &texDescriptorNormal;
	normalWriteSet.descriptorCount = 1;

	// Binding 3 : Albedo texture target
	VkWriteDescriptorSet albedoWriteSet{};
	albedoWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	albedoWriteSet.dstSet = m_DescriptorSet;
	albedoWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	albedoWriteSet.dstBinding = 3;
	albedoWriteSet.pImageInfo = &texDescriptorAlbedo;
	albedoWriteSet.descriptorCount = 1;

	// Binding 4 : Fragment shader uniform buffer
	VkWriteDescriptorSet fragWriteSet{};
	fragWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	fragWriteSet.dstSet = m_DescriptorSet;
	fragWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	fragWriteSet.dstBinding = 4;
	fragWriteSet.pBufferInfo = &m_UniformBuffers.m_FragLights.m_Descriptor;
	fragWriteSet.descriptorCount = 1;

	writeDescriptorSets = {
		uniformWriteSet,
		positionWriteSet,
		normalWriteSet,
		albedoWriteSet,
		fragWriteSet,
	};

	vkUpdateDescriptorSets(m_VulkanDevice->GetDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);

	// Offscreen (scene)
	for (GameObject* obj : m_Scene->GetObjects())
	{
		if (ModelComponent* comp = obj->GetComponent<ModelComponent>())
			comp->CreateDescriptorSet(allocInfo);
	}

}

void Application::BuildCommandBuffers()
{
	VkCommandBufferBeginInfo cmdBufInfo{};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkClearValue clearValues[2];
	clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_RenderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = m_SwapChainExtent.width;
	renderPassBeginInfo.renderArea.extent.height = m_SwapChainExtent.height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	for (int32_t i = 0; i < m_CommandBuffers.size(); ++i)
	{
		// Set target frame buffer
		renderPassBeginInfo.framebuffer = m_Framebuffers[i];

		CHECK_VK_RESULT(vkBeginCommandBuffer(m_CommandBuffers[i], &cmdBufInfo));

		vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.width = (float)m_SwapChainExtent.width;
		viewport.height = (float)m_SwapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vkCmdSetViewport(m_CommandBuffers[i], 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.extent.width = m_SwapChainExtent.width;
		scissor.extent.height = m_SwapChainExtent.height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(m_CommandBuffers[i], 0, 1, &scissor);

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayouts.m_Deferred, 0, 1, &m_DescriptorSet, 0, NULL);

		if (debugDisplay)
		{
			vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.m_Debug);
			vkCmdBindVertexBuffers(m_CommandBuffers[i], 0, 1, &m_Quad.m_VertexBuffer.m_Buffer, offsets);
			vkCmdBindIndexBuffer(m_CommandBuffers[i], m_Quad.m_IndexBuffer.m_Buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(m_CommandBuffers[i], m_Quad.m_IndexSize, 1, 0, 0, 1);
			// Move viewport to display final composition in lower right corner
			viewport.x = viewport.width * 0.5f;
			viewport.y = viewport.height * 0.5f;
			viewport.width = viewport.width * 0.5f;
			viewport.height = viewport.height * 0.5f;
			vkCmdSetViewport(m_CommandBuffers[i], 0, 1, &viewport);
		}

		// Final composition as full screen quad
		vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.m_Deferred);
		vkCmdBindVertexBuffers(m_CommandBuffers[i], 0, 1, &m_Quad.m_VertexBuffer.m_Buffer, offsets);
		vkCmdBindIndexBuffer(m_CommandBuffers[i], m_Quad.m_IndexBuffer.m_Buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(m_CommandBuffers[i], 6, 1, 0, 0, 1);

		vkCmdEndRenderPass(m_CommandBuffers[i]);

		CHECK_VK_RESULT(vkEndCommandBuffer(m_CommandBuffers[i]));
	}
}

void Application::BuildDefferedCommandBuffer()
{
	if (m_OffScreenCmdBuffer == VK_NULL_HANDLE)
	{
		m_OffScreenCmdBuffer = m_VulkanDevice->CreateCommandBuffer(false);
	}

	// Create a semaphore used to synchronize offscreen rendering and usage
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	CHECK_VK_RESULT(vkCreateSemaphore(m_VulkanDevice->GetDevice(), &semaphoreCreateInfo, nullptr, &m_OffscreenSemaphore));

	VkCommandBufferBeginInfo cmdBufInfo{};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	// Clear values for all attachments written in the fragment sahder
	std::array<VkClearValue, 4> clearValues;
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[3].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_OffscreenFrameBuffer->m_RenderPass;
	renderPassBeginInfo.framebuffer = m_OffscreenFrameBuffer->m_FrameBuffer;
	renderPassBeginInfo.renderArea.extent.width = m_OffscreenFrameBuffer->m_Width;
	renderPassBeginInfo.renderArea.extent.height = m_OffscreenFrameBuffer->m_Height;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	CHECK_VK_RESULT(vkBeginCommandBuffer(m_OffScreenCmdBuffer, &cmdBufInfo));

	vkCmdBeginRenderPass(m_OffScreenCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.width = (float)m_OffscreenFrameBuffer->m_Width;
	viewport.height = (float)m_OffscreenFrameBuffer->m_Height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_OffScreenCmdBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.extent.width = m_OffscreenFrameBuffer->m_Width;
	scissor.extent.height = m_OffscreenFrameBuffer->m_Height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(m_OffScreenCmdBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(m_OffScreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.m_Offscreen);

	for (GameObject* obj : m_Scene->GetObjects())
	{
		if (ModelComponent* comp = obj->GetComponent<ModelComponent>())
		{
			comp->SetupCommandBuffer(m_OffScreenCmdBuffer, m_PipelineLayouts.m_Offscreen);
		}
	}

	vkCmdEndRenderPass(m_OffScreenCmdBuffer);

	CHECK_VK_RESULT(vkEndCommandBuffer(m_OffScreenCmdBuffer));
}

void Application::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.m_Formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.m_PresentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.m_Capabilities);

	uint32_t imageCount = swapChainSupport.m_Capabilities.minImageCount + 1;
	if (swapChainSupport.m_Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.m_Capabilities.maxImageCount)
	{
		
		imageCount = swapChainSupport.m_Capabilities.maxImageCount;
		std::cout << "Max image count: " << imageCount << std::endl;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_Surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	vk::VulkanDevice::QueueFamilyIndices indices = m_VulkanDevice->GetQueueFamilyIndices();
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.m_GraphicsFamily, (uint32_t)indices.m_PresentFamily };

	if (indices.m_GraphicsFamily != indices.m_PresentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.m_Capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(m_VulkanDevice->GetDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(m_VulkanDevice->GetDevice(), m_SwapChain, &imageCount, nullptr);
	m_SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_VulkanDevice->GetDevice(), m_SwapChain, &imageCount, m_SwapChainImages.data());
	m_SwapChainImageFormat = surfaceFormat.format;
	m_SwapChainExtent = extent;
}

void Application::RecreateSwapChain()
{
	int width, height;
	glfwGetWindowSize(m_Window, &width, &height);
	if (width == 0 || height == 0) return;

	CleanupSwapChain();

	vkDeviceWaitIdle(m_VulkanDevice->GetDevice());

	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	//CreateGraphicsPipeline();
	//CreateDepthResources();
	//CreateFrameBuffers();
	//CreateCommandBuffers();
}

void Application::CleanupSwapChain()
{
	vkDestroyImageView(m_VulkanDevice->GetDevice(), m_DepthImageView, nullptr);
	vkDestroyImage(m_VulkanDevice->GetDevice(), m_DepthImage, nullptr);
	vkFreeMemory(m_VulkanDevice->GetDevice(), m_DepthImageMemory, nullptr);

	for (size_t i = 0; i < m_Framebuffers.size(); i++) 
	{
		vkDestroyFramebuffer(m_VulkanDevice->GetDevice(), m_Framebuffers[i], nullptr);
	}

	vkFreeCommandBuffers(m_VulkanDevice->GetDevice(), m_VulkanDevice->GetCommandPool(), static_cast<uint32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());
	vkDestroyRenderPass(m_VulkanDevice->GetDevice(), m_RenderPass, nullptr);

	for (size_t i = 0; i < m_SwapChainImageViews.size(); i++) 
	{
		vkDestroyImageView(m_VulkanDevice->GetDevice(), m_SwapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(m_VulkanDevice->GetDevice(), m_SwapChain, nullptr);
}

void Application::CreateImageViews()
{
	m_SwapChainImageViews.resize(m_SwapChainImages.size());
	for (size_t i = 0; i < m_SwapChainImages.size(); i++) 
	{
		m_SwapChainImageViews[i] = ImageHelpers::CreateImageView(m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void Application::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = m_SwapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = FindDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(m_VulkanDevice->GetDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

void Application::UpdateUniformBuffersScreen()
{
	if (debugDisplay)
	{
		m_VertUBO.m_Projection = glm::ortho(0.0f, 2.0f, 0.0f, 2.0f, -1.0f, 1.0f);
	}
	else
	{
		m_VertUBO.m_Projection = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
	}

	m_VertUBO.m_Model = glm::mat4(1.0f);

	memcpy(m_UniformBuffers.m_VertFullScreen.m_Mapped, &m_VertUBO, sizeof(m_VertUBO));
}

void Application::UpdateUniformBufferDeferredLights()
{
	//TEMP should be controlled by a scene
	int index = 0;

	glm::vec3 modelTranslation;
	for (GameObject* obj : m_Scene->GetObjects())
	{
		if (ModelComponent* comp = obj->GetComponent<ModelComponent>())
		{
			modelTranslation = obj->GetComponent<TranslationComponent>()->GetTranslation();
			break;
		}
	}

	float degrees = 0.f;
	m_LightTime += GetDeltaTime();
	for (GameObject * obj : m_Scene->GetObjects())
	{
		if (PointLightComponent* lightComp = obj->GetComponent<PointLightComponent>())
		{
			TranslationComponent* comp = obj->GetComponent<TranslationComponent>();
			float radians = (glm::radians(degrees)) + m_LightTime;
			float x = 5 * cos(radians);
			float z = 5 * sin(radians);

			comp->SetTranslation(glm::vec3(x, 0.f, z) + modelTranslation);

			m_LightsUBO.m_Lights[index].m_Position = glm::vec4(comp->GetTranslation(), 0.f);
			m_LightsUBO.m_Lights[index].m_Colour = lightComp->GetColour();
			m_LightsUBO.m_Lights[index].m_Radius = lightComp->GetRadius();
			index++;
			degrees += 60.f;
		}
	}
	
	// Current view position
	m_LightsUBO.m_ViewPos = glm::vec4(m_Scene->GetCamera()->GetPosition(), 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

	memcpy(m_UniformBuffers.m_FragLights.m_Mapped, &m_LightsUBO, sizeof(m_LightsUBO));
}

void Application::CreateFrameBuffers()
{
	VkImageView attachments[2];

	// Depth/Stencil attachment is the same for all frame buffers
	attachments[1] = m_DepthImageView;

	VkFramebufferCreateInfo frameBufferCreateInfo = {};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.pNext = NULL;
	frameBufferCreateInfo.renderPass = m_RenderPass;
	frameBufferCreateInfo.attachmentCount = 2;
	frameBufferCreateInfo.pAttachments = attachments;
	frameBufferCreateInfo.width = m_SwapChainExtent.width;
	frameBufferCreateInfo.height = m_SwapChainExtent.height;
	frameBufferCreateInfo.layers = 1;

	// Create frame buffers for every swap chain image
	m_Framebuffers.resize(m_SwapChainImages.size());
	for (uint32_t i = 0; i < m_Framebuffers.size(); i++)
	{
		attachments[0] = m_SwapChainImageViews[i];
		CHECK_VK_RESULT(vkCreateFramebuffer(m_VulkanDevice->GetDevice(), &frameBufferCreateInfo, nullptr, &m_Framebuffers[i]));
	}
}

void Application::CreateCommandBuffers()
{
	m_CommandBuffers.resize(m_SwapChainImageViews.size());


	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = m_VulkanDevice->GetCommandPool();
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = (uint32_t)m_SwapChainImageViews.size();

	CHECK_VK_RESULT(vkAllocateCommandBuffers(m_VulkanDevice->GetDevice(), &commandBufferAllocateInfo, m_CommandBuffers.data()));
}


void Application::CreateSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(m_VulkanDevice->GetDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(m_VulkanDevice->GetDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore) != VK_SUCCESS) 
	{

		throw std::runtime_error("failed to create semaphores!");
	}
}

void Application::CreateDepthResources()
{
	VkFormat depthFormat = FindDepthFormat();
	ImageHelpers::CreateImage(m_SwapChainExtent.width,
		m_SwapChainExtent.height,
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_DepthImage,
		m_DepthImageMemory);

	m_DepthImageView = ImageHelpers::CreateImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	ImageHelpers::TransitionImageLayout(m_DepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, m_GraphicsQueue);
}

Application::SwapChainSupportDetails Application::QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.m_Capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

	if (formatCount != 0) 
	{
		details.m_Formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.m_Formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) 
	{
		details.m_PresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.m_PresentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR Application::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) 
	{
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& availableFormat : availableFormats) 
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR Application::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes)
{
	//standard double buffering.
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : availablePresentModes) 
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
		{
			//double buffering but the queue doesnt get blocked if we fill it up to fast unlike VK_PRESENT_MODE_FIFO_KHR
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) 
		{
			//everything gets sent to screen straight away, not ideal but most widely supported, 
			//this should be settings driven at some point.
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

VkExtent2D Application::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
	{
		return capabilities.currentExtent;
	}
	else 
	{
		int width, height;
		glfwGetWindowSize(m_Window, &width, &height);

		VkExtent2D actualExtent = { (uint32_t)width, (uint32_t)height };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

VkPipelineShaderStageCreateInfo Application::LoadShader(std::string fileName, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
	shaderStage.module = CreateShaderModule(Helpers::ReadFile(fileName));
	shaderStage.pName = "main"; // todo : make param
	assert(shaderStage.module != VK_NULL_HANDLE);
	m_ShaderModules.push_back(shaderStage.module);
	return shaderStage;
}

VkShaderModule Application::CreateShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_VulkanDevice->GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}
