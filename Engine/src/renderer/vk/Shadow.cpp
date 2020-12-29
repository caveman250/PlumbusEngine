#include "Shadow.h"

#include "ShadowManager.h"
#include "VulkanRenderer.h"

namespace plumbus::vk
{
	Shadow::~Shadow()
	{
		ShadowManager::Get()->UnregisterShadow(this);
		m_CommandBuffer.reset();
		m_FrameBuffer.reset();
		vkDestroySemaphore(VulkanRenderer::Get()->GetDevice()->GetVulkanDevice(), m_Semaphore, nullptr);
	}
}
