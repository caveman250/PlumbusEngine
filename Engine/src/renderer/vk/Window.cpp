#include "plumbus.h"

#include "renderer/vk/Window.h"
#include "imgui_impl/Log.h"
#include "VulkanRenderer.h"
#include "Instance.h"

#if PL_PLATFORM_ANDROID
#include "android_native_app_glue.h"
extern android_app* Android_application;
#endif

extern float androidWindowWidth;
extern float androidWindowHeight;

namespace plumbus::vk
{
	void Window::Init(uint32_t& width, uint32_t& height, std::string name)
	{
#if PL_PLATFORM_ANDROID
		width = androidWindowWidth;
		height = androidWindowHeight;
#else
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		GLFWmonitor* monitor =  glfwGetPrimaryMonitor();
   		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		width = (uint32_t)(mode->width * 0.5f);
		height = (uint32_t)(mode->height * 0.5f);

		m_Window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
        
        glfwGetWindowContentScale(m_Window, &m_ContentScaleX, &m_ContentScaleY);

		glfwSetWindowUserPointer(m_Window, this);
#endif
	}

	void Window::CreateSurface()
	{
#if !PL_PLATFORM_ANDROID
		if (glfwCreateWindowSurface(VulkanRenderer::Get()->GetInstance()->GetVulkanInstance(), m_Window, nullptr, &m_Surface) != VK_SUCCESS)
		{
			Log::Fatal("failed to create window surface!");
		}
#else
        VkAndroidSurfaceCreateInfoKHR info = { VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR };
        info.window = Android_application->window;
        CHECK_VK_RESULT(vkCreateAndroidSurfaceKHR(VulkanRenderer::Get()->GetInstance()->GetVulkanInstance(), &info, nullptr, &m_Surface));
#endif
	}

	void Window::Destroy()
	{
#if !PL_PLATFORM_ANDROID
		glfwDestroyWindow(m_Window);
		glfwTerminate();
#endif
	}

	glm::vec2 Window::GetMousePos()
	{
#if !PL_PLATFORM_ANDROID
		double xpos, ypos;
		glfwGetCursorPos(m_Window, &xpos, &ypos);

		return glm::vec2(xpos * m_ContentScaleX, ypos * m_ContentScaleY);
#else
		return glm::vec2(0, 0);
#endif
	}

	uint32_t Window::GetWidth()
	{
#if !PL_PLATFORM_ANDROID
		int width;
		glfwGetWindowSize(m_Window, &width, nullptr);
		return width * m_ContentScaleX;
#else
		return androidWindowWidth;
#endif
	}

	uint32_t Window::GetHeight()
	{
#if !PL_PLATFORM_ANDROID
		int height;
		glfwGetWindowSize(m_Window, nullptr, &height);
		return height * m_ContentScaleY;
#else
		return androidWindowHeight;
#endif
	}

	bool Window::ShouldClose()
	{
#if !PL_PLATFORM_ANDROID
		return glfwWindowShouldClose(m_Window);
#else
		return false;
#endif
	}

	bool Window::IsKeyDown(const int key)
	{
#if !PL_PLATFORM_ANDROID
		return glfwGetKey(GetWindow(), key);
#else
		return false;
#endif
	}

}

