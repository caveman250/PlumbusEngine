#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "renderer/base/Window.h"

namespace vk
{
	class Window : public base::Window
	{
	public:
		Window() {}
		~Window() {}

		void Init(uint32_t width, uint32_t height) override;
		void CreateSurface(VkInstance instance, VkSurfaceKHR* surface);
		void Destroy() override;

		glm::vec2 GetMousePos();

		uint32_t GetWidth() override;
		uint32_t GetHeight() override;
		bool ShouldClose();

		GLFWwindow* GetWindow() { return m_Window; }

	private:
		GLFWwindow* m_Window;

	};
}