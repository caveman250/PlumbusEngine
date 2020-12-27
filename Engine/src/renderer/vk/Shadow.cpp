#include "Shadow.h"

#include "ShadowManager.h"

namespace plumbus::vk
{
	Shadow::~Shadow()
	{
		ShadowManager::Get()->UnregisterShadow(this);
	}
}
