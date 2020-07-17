#pragma once
#include "plumbus.h"

#include "renderer/base/Window.h"

namespace plumbus::vk
{
	class Window : public base::Window
	{
	public:
		Window() {}
		~Window() {}

		void Init(uint32_t width, uint32_t height) override;
		void CreateSurface();
		void Destroy() override;

		glm::vec2 GetMousePos() override;
		bool IsKeyDown(const int key) override;

		uint32_t GetWidth() override;
		uint32_t GetHeight() override;
		bool ShouldClose();

		GLFWwindow* GetWindow() { return m_Window; }
		VkSurfaceKHR GetSurface() { return m_Surface; }

	private:
		GLFWwindow* m_Window;
		VkSurfaceKHR m_Surface;
	};
}