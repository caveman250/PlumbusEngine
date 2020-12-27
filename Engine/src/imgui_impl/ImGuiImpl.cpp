#include "plumbus.h"
#include "imgui_impl/ImGuiImpl.h"
#include "Log.h"
#include "GameObject.h"
#include "components/TranslationComponent.h"
#include "components/ModelComponent.h"
#include "components/LightComponent.h"
#include "imgui/imgui_internal.h"
#include "Scene.h"
#include "renderer/vk/CommandBuffer.h"
#include "renderer/vk/Material.h"
#include "renderer/vk/DescriptorPool.h"
#include "renderer/vk/DescriptorSet.h"
#include "renderer/vk/MaterialInstance.h"
#include "renderer/vk/PipelineLayout.h"
#include "renderer/vk/vk_types_fwd.h"
#include "renderer/vk/VulkanRenderer.h"
#include "renderer/vk/shader_compiler/ShaderSettings.h"

#if ENABLE_IMGUI
namespace plumbus
{
	ImGUIImpl::ImGUIImpl()
	{
	}

	ImGUIImpl::~ImGUIImpl()
	{
		std::shared_ptr<vk::Device> device = vk::VulkanRenderer::Get()->GetDevice();

		// Release all Vulkan resources required for rendering imGui
		m_VertexBuffer.Cleanup();
		m_IndexBuffer.Cleanup();
		vkDestroyImage(device->GetVulkanDevice(), m_FontImage, nullptr);
		vkDestroyImageView(device->GetVulkanDevice(), m_FontView, nullptr);
		vkFreeMemory(device->GetVulkanDevice(), m_FontMemory, nullptr);
		vkDestroySampler(device->GetVulkanDevice(), m_Sampler, nullptr);
		
		m_MaterialInstance.reset();
		m_GameViewMaterialInstance.reset();
		m_Material.reset();
	}

	void ImGUIImpl::Init(float width, float height)
	{
		ImGui::CreateContext();

		// Color scheme
		//ImGuiStyle& style = ImGui::GetStyle();
		//style.Colors[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
		//style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
		//style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
		//style.Colors[ImGuiCol_Header] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
		//style.Colors[ImGuiCol_CheckMark] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
		//style.Colors[ImGuiCol_WindowBg] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
		//style.Colors[ImGuiCol_Button] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
        vk::Window* window = vk::VulkanRenderer::Get()->GetWindow();
        
		// Dimensions
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(width, height);
        io.DisplayFramebufferScale = ImVec2(window->GetContentScaleX(), window->GetContentScaleY());
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		InitGLFWCallbacks();
	}

	void ImGUIImpl::InitGLFWCallbacks()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;

		vk::VulkanRenderer* renderer = vk::VulkanRenderer::Get();

		glfwSetScrollCallback(renderer->GetWindow()->GetWindow(), OnMouseScolled);
		glfwSetKeyCallback(renderer->GetWindow()->GetWindow(), OnKeyDown);
		glfwSetCharCallback(renderer->GetWindow()->GetWindow(), OnChar);
	}

	void ImGUIImpl::InitResources(VkRenderPass renderPass, VkQueue copyQueue)
	{
		std::shared_ptr<vk::Device> device = vk::VulkanRenderer::Get()->GetDevice();

		ImGuiIO& io = ImGui::GetIO();

		// Create font texture
		unsigned char* fontData;
		int texWidth, texHeight;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
		VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

		// Create target image for copy
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageInfo.extent.width = texWidth;
		imageInfo.extent.height = texHeight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		CHECK_VK_RESULT(vkCreateImage(device->GetVulkanDevice(), &imageInfo, nullptr, &m_FontImage));
		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(device->GetVulkanDevice(), m_FontImage, &memReqs);
		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = device->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		CHECK_VK_RESULT(vkAllocateMemory(device->GetVulkanDevice(), &memAllocInfo, nullptr, &m_FontMemory));
		CHECK_VK_RESULT(vkBindImageMemory(device->GetVulkanDevice(), m_FontImage, m_FontMemory, 0));

		// Image view
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_FontImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.layerCount = 1;
		CHECK_VK_RESULT(vkCreateImageView(device->GetVulkanDevice(), &viewInfo, nullptr, &m_FontView));

		// Staging buffers for font data upload
		vk::Buffer stagingBuffer;

		CHECK_VK_RESULT(device->CreateBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&stagingBuffer,
			uploadSize));

		stagingBuffer.Map();
		memcpy(stagingBuffer.m_Mapped, fontData, uploadSize);
		stagingBuffer.Unmap();

		// Copy buffer data to font image
		VkCommandBuffer copyCmd = device->CreateCommandBuffer(true);

		// Prepare for transfer
		ImageHelpers::SetImageLayout(
			copyCmd,
			m_FontImage,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_HOST_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT);

		// Copy
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = texWidth;
		bufferCopyRegion.imageExtent.height = texHeight;
		bufferCopyRegion.imageExtent.depth = 1;

		vkCmdCopyBufferToImage(
			copyCmd,
			stagingBuffer.m_Buffer,
			m_FontImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&bufferCopyRegion
		);

		// Prepare for shader read
		ImageHelpers::SetImageLayout(
			copyCmd,
			m_FontImage,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		device->FlushCommandBuffer(copyCmd, copyQueue);

		stagingBuffer.Cleanup();

		// Font texture Sampler
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		CHECK_VK_RESULT(vkCreateSampler(device->GetVulkanDevice(), &samplerInfo, nullptr, &m_Sampler));

		m_Material = std::make_shared<vk::Material>("shaders/ui.vert", "shaders/ui.frag", renderPass, true);
		m_Material->Setup();
		m_MaterialInstance = vk::MaterialInstance::CreateMaterialInstance(m_Material);
		m_MaterialInstance->SetTextureUniform("imageSampler", m_Sampler, m_FontView, false);
		io.Fonts->TexID = (void*)m_MaterialInstance.get();
	}

	void ImGUIImpl::OnGui()
	{
		vk::VulkanRenderer* renderer = vk::VulkanRenderer::Get();

		if (!m_GameViewMaterialInstance)
		{
			m_GameViewMaterialInstance = CreateImGuiTextureMaterialInstance(renderer->GetDeferredOutputFramebuffer()->GetSampler(), renderer->GetDeferredOutputFramebuffer()->GetAttachment("colour").m_ImageView, false);
		}

		ImGuiIO& io = ImGui::GetIO();

		io.DeltaTime = (float)BaseApplication::Get().GetDeltaTime();

		double xpos, ypos;
		glfwGetCursorPos(renderer->GetWindow()->GetWindow(), &xpos, &ypos);
		io.MousePos = ImVec2((float)xpos, (float)ypos);
		io.MouseDown[0] = glfwGetMouseButton(renderer->GetWindow()->GetWindow(), GLFW_MOUSE_BUTTON_1);
		io.MouseDown[1] = glfwGetMouseButton(renderer->GetWindow()->GetWindow(), GLFW_MOUSE_BUTTON_2);

		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
		if (ImGui::Begin("DockSpace", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			ImGuiID docspace_id = ImGui::GetCurrentWindow()->GetID("dockspace");
			ImGui::DockSpace(docspace_id);
			// Init imGui windows and elements

			ImGui::Begin("Objects", 0);
			if (ImGui::CollapsingHeader("Objects", ImGuiTreeNodeFlags_DefaultOpen))
			{
				static int listbox_item_current = 1;
				for (GameObject* obj : BaseApplication::Get().GetScene()->GetObjects())
				{
					if (ImGui::Button(obj->GetID().c_str()))
					{
						m_SelectedObject = obj;
					}
				}
			}

			ImGui::End();

			ImGui::Begin("Properties", 0);
			BaseApplication::Get().GetScene()->GetCamera()->OnGui();
			if (m_SelectedObject)
			{
				if (ImGui::CollapsingHeader("Components", ImGuiTreeNodeFlags_DefaultOpen))
				{
					if (TranslationComponent* comp = m_SelectedObject->GetComponent<TranslationComponent>())
					{
						if (ImGui::TreeNodeEx("Translation", ImGuiTreeNodeFlags_DefaultOpen))
						{
							glm::vec3 translation = comp->GetTranslation();
							ImGui::DragFloat3("Position", (float*)&translation, 0.05f);
							comp->SetTranslation(translation);
							glm::vec3 rotation = comp->GetRotation();
							ImGui::DragFloat3("Rotation", (float*)&rotation, 0.05f);
							comp->SetRotation(rotation);
							glm::vec3 scale = comp->GetScale();
							ImGui::DragFloat3("Scale", (float*)&scale, 0.05f);
							comp->SetScale(scale);
							ImGui::TreePop();
						}
					}
					if (ModelComponent* comp = m_SelectedObject->GetComponent<ModelComponent>())
					{
						if (ImGui::TreeNodeEx("Model", ImGuiTreeNodeFlags_DefaultOpen))
						{
							ImGui::Text("%s", comp->GetModelPath().c_str());
							ImGui::Text("%s", comp->GetTexturePath().c_str());
							ImGui::Text("%s", comp->GetNormalPath().c_str());
							ImGui::TreePop();
						}
					}
					if (LightComponent* comp = m_SelectedObject->GetComponent<LightComponent>())
					{
						for (Light* light : comp->GetLights())
						{
							if (light->GetType() == LightType::Point)
							{
								if(PointLight* pointLight = static_cast<PointLight*>(light))
								{
									if (ImGui::TreeNodeEx("Point Light", ImGuiTreeNodeFlags_DefaultOpen))
									{
										glm::vec3 colour = pointLight->GetColour();
										ImGui::DragFloat3("Colour", (float*)&colour, 0.05f);
										pointLight->SetColour(colour);

										float radius = pointLight->GetRadius();
										ImGui::DragFloat("Radius", &radius, 0.05f);
										pointLight->SetRadius(radius);

										ImGui::TreePop();
									}
								}
							}
							else if (light->GetType() == LightType::Directional)
							{
								if(DirectionalLight* directionalLight = static_cast<DirectionalLight*>(light))
								{
									if (ImGui::TreeNodeEx("Directional Light", ImGuiTreeNodeFlags_DefaultOpen))
									{
										glm::vec3 colour = directionalLight->GetColour();
										ImGui::DragFloat3("Colour", (float*)&colour, 0.05f);
										directionalLight->SetColour(colour);

										glm::vec3 target = directionalLight->GetDirection();
										ImGui::DragFloat3("Target", (float*)&target, 0.005f, -1.f, 1.f);
										directionalLight->SetDirection(target);

										ImGui::TreePop();
									}
								}
							}
						}
					}
				}
			}
			ImGui::End();
			//ImGui::ShowTestWindow();

			ImGui::Begin("Game Window", nullptr, ImGuiWindowFlags_NoScrollbar);
			BaseApplication::Get().m_GameWindowFocused = ImGui::IsWindowHovered();

			//TODO game size is fixed to initial window size, give more control over game resolution.
			ImGui::Image((void*)m_GameViewMaterialInstance.get(), ImVec2(1600, 900), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0), true);
			ImGui::End();

			Log::Draw("Log");

			BaseApplication::Get().OnGui();

			ImGui::End(); //Dockspace
		}

		// Render to generate draw buffers
		ImGui::Render();
	}

	void ImGUIImpl::UpdateBuffers()
	{
		std::shared_ptr<vk::Device> device = vk::VulkanRenderer::Get()->GetDevice();

		ImDrawData* imDrawData = ImGui::GetDrawData();

		// Note: Alignment is done inside buffer creation
		VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
		VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

		// Update buffers only if vertex or index count has been changed compared to current buffer size

		// Vertex buffer
		if ((m_VertexBuffer.m_Buffer == VK_NULL_HANDLE) || (m_VertexCount != imDrawData->TotalVtxCount)) {
			m_VertexBuffer.Unmap();
			m_VertexBuffer.Cleanup();
			CHECK_VK_RESULT(device->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &m_VertexBuffer, vertexBufferSize));
			m_VertexCount = imDrawData->TotalVtxCount;
			m_VertexBuffer.Unmap();
			m_VertexBuffer.Map();
		}

		// Index buffer
		VkDeviceSize indexSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
		if ((m_IndexBuffer.m_Buffer == VK_NULL_HANDLE) || (m_IndexCount < imDrawData->TotalIdxCount)) {
			m_IndexBuffer.Unmap();
			m_IndexBuffer.Cleanup();
			CHECK_VK_RESULT(device->CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &m_IndexBuffer, indexBufferSize));
			m_IndexCount = imDrawData->TotalIdxCount;
			m_IndexBuffer.Map();
		}

		// Upload data
		ImDrawVert* vtxDst = (ImDrawVert*)m_VertexBuffer.m_Mapped;
		ImDrawIdx* idxDst = (ImDrawIdx*)m_IndexBuffer.m_Mapped;

		for (int n = 0; n < imDrawData->CmdListsCount; n++) {
			const ImDrawList* cmd_list = imDrawData->CmdLists[n];
			memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtxDst += cmd_list->VtxBuffer.Size;
			idxDst += cmd_list->IdxBuffer.Size;
		}

		// Flush to make writes visible to GPU
		m_VertexBuffer.Flush();
		m_IndexBuffer.Flush();
	}

	void ImGUIImpl::DrawFrame(vk::CommandBufferRef commandBuffer)
	{
		ImGuiIO& io = ImGui::GetIO();

		// Bind vertex and index buffer
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer->GetVulkanCommandBuffer(), 0, 1, &m_VertexBuffer.m_Buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer->GetVulkanCommandBuffer(), m_IndexBuffer.m_Buffer, 0, VK_INDEX_TYPE_UINT16);
        
		VkViewport viewport{};
        viewport.width = io.DisplaySize.x * io.DisplayFramebufferScale.x;
		viewport.height = io.DisplaySize.y * io.DisplayFramebufferScale.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer->GetVulkanCommandBuffer(), 0, 1, &viewport);
		// Render commands
		ImDrawData* imDrawData = ImGui::GetDrawData();
		int32_t vertexOffset = 0;
		int32_t indexOffset = 0;
		for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
		{
			const ImDrawList* cmd_list = imDrawData->CmdLists[i];
			for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];

				// UI scale and translate via push constants
				m_PushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
				m_PushConstBlock.translate = glm::vec2(-1.0f);

				vk::MaterialInstance* materialInstance;
				if(pcmd->TextureId)
				{
					materialInstance = static_cast<vk::MaterialInstance*>(pcmd->TextureId);
				}
				else
				{
					materialInstance = m_MaterialInstance.get();
				}

				materialInstance->Bind(commandBuffer);
				vkCmdPushConstants(commandBuffer->GetVulkanCommandBuffer(), materialInstance->GetMaterial()->GetPipelineLayout()->GetVulkanPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &m_PushConstBlock);
			
				VkRect2D scissorRect;
				scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
				scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
				scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
				scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
                
                scissorRect.offset.x*= io.DisplayFramebufferScale.x;
                scissorRect.offset.y*= io.DisplayFramebufferScale.y;
                scissorRect.extent.width*=io.DisplayFramebufferScale.x;
                scissorRect.extent.height=scissorRect.extent.height*io.DisplayFramebufferScale.y+1;// FIXME: Why +1 here?
                
				vkCmdSetScissor(commandBuffer->GetVulkanCommandBuffer(), 0, 1, &scissorRect);
				
				vkCmdDrawIndexed(commandBuffer->GetVulkanCommandBuffer(), pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
				indexOffset += pcmd->ElemCount;
			}
			vertexOffset += cmd_list->VtxBuffer.Size;
		}
	}

	vk::MaterialInstanceRef ImGUIImpl::CreateImGuiTextureMaterialInstance(VkSampler sampler, VkImageView image_view, bool isDepth) const
	{
		vk::MaterialRef material = std::make_shared<vk::Material>("shaders/ui.vert", isDepth ? "shaders/ui_depth.frag" : "shaders/ui.frag", vk::VulkanRenderer::Get()->GetSwapChain()->GetRenderPass());
		material->Setup();
		vk::MaterialInstanceRef materialInstance = vk::MaterialInstance::CreateMaterialInstance(material);
		materialInstance->SetTextureUniform("imageSampler", sampler, image_view, isDepth);
		return materialInstance;
	}

	void ImGUIImpl::OnMouseScolled(GLFWwindow* window, double xoffset, double yoffset)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheel += (float)yoffset;
	}

	void ImGUIImpl::OnKeyDown(GLFWwindow*, int key, int, int action, int mods)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (action == GLFW_PRESS)
			io.KeysDown[key] = true;
		if (action == GLFW_RELEASE)
			io.KeysDown[key] = false;

		(void)mods; // Modifiers are not reliable across systems
		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
	}

	void ImGUIImpl::OnChar(GLFWwindow*, unsigned int c)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (c > 0 && c < 0x10000)
			io.AddInputCharacter((unsigned short)c);
	}
}
#endif
