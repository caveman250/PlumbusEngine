#pragma once
#include <memory>

namespace plumbus
{
	namespace base
	{
		class Material;
	}
	typedef std::shared_ptr<base::Material> MaterialRef;
}