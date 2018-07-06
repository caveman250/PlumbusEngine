#pragma once

namespace base
{
	class Renderer
	{
	public:
		virtual void Init() = 0;
		virtual void Cleanup() = 0;

		virtual void DrawFrame() = 0;
		virtual bool WindowShouldClose() = 0;

		virtual void AwaitIdle() = 0;
	};
}