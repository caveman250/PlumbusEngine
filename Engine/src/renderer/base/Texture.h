#pragma once
#include "plumbus.h"

namespace plumbus::base
{
	class Texture
	{
	public:
		virtual void LoadTexture(std::string fileName) = 0;
		virtual void Cleanup() = 0;
	};
}