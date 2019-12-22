#include "renderer/vk/Window.h"
#include "imgui_impl/Log.h"

namespace vk
{
	void Window::Init(uint32_t width, uint32_t height)
	{
		glfwInit();
		//dont use OpenGl
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		m_Window = glfwCreateWindow(width, height, "VulkanRenderer", nullptr, nullptr);

		glfwSetWindowUserPointer(m_Window, this);
	}

	void Window::CreateSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) != VK_SUCCESS)
		{
			Log::Fatal("failed to create window surface!");
		}
	}

	void Window::Destroy()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	glm::vec2 Window::GetMousePos()
	{
		double xpos, ypos;
		glfwGetCursorPos(m_Window, &xpos, &ypos);

		return glm::vec2(xpos, ypos);
	}

	uint32_t Window::GetWidth()
	{
		int width;
		glfwGetWindowSize(m_Window, &width, nullptr);
		return width;
	}

	uint32_t Window::GetHeight()
	{
		int height;
		glfwGetWindowSize(m_Window, nullptr, &height);
		return height;
	}

	bool Window::ShouldClose()
	{
		return glfwWindowShouldClose(m_Window);
	}

}

