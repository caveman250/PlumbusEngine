#pragma once
#include <string>

namespace base
{
	class Texture
	{
	public:
		virtual void LoadTexture(std::string fileName) = 0;
		virtual void Cleanup() = 0;
	};
}