#include "plumbus.h"
#include "imgui_impl/ImGuiImpl.h"
#include "Log.h"
#include "GameObject.h"
#include "components/TranslationComponent.h"
#include "components/ModelComponent.h"
#include "components/LightComponent.h"
#include "imgui/imgui_internal.h"
#include "Scene.h"
#include "renderer/vk/Material.h"
#include "renderer/vk/DescriptorPool.h"
#include "renderer/vk/DescriptorSet.h"

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
		vkDestroyPipelineCache(device->GetVulkanDevice(), m_PipelineCache, nullptr);
		vkDestroyPipeline(device->GetVulkanDevice(), m_Pipeline, nullptr);
		vkDestroyPipelineLayout(device->GetVulkanDevice(), m_PipelineLayout, nullptr);

		m_DescriptorSet.reset();
		m_GameViewTextureDescSet.reset();
		m_DescriptorSetLayout.reset();
		m_DescriptorPool.reset();
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
		// Dimensions
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(width, height);
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		InitGLFWCallbacks();
	}

	void ImGUIImpl::InitGLFWCallbacks()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;

		vk::VulkanRenderer* renderer = vk::VulkanRenderer::Get();

		glfwSetScrollCallback(renderer->GetWindow(), OnMouseScolled);
		glfwSetKeyCallback(renderer->GetWindow(), OnKeyDown);
		glfwSetCharCallback(renderer->GetWindow(), OnChar);
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

		m_DescriptorPool = vk::DescriptorPool::CreateDescriptorPool(0, 5, 5);

		m_DescriptorSetLayout = vk::DescriptorSetLayout::CreateDescriptorSetLayout();
		m_DescriptorSetLayout->AddBinding(vk::DescriptorSetLayout::BindingUsage::FragmentShader, vk::DescriptorSetLayout::BindingType::ImageSampler, 0);
		m_DescriptorSetLayout->Build();

		m_DescriptorSet = vk::DescriptorSet::CreateDescriptorSet(m_DescriptorPool, m_DescriptorSetLayout);
		m_DescriptorSet->AddTexture(m_Sampler, m_FontView, vk::DescriptorSet::BindingUsage::FragmentShader);
		m_DescriptorSet->Build();

		// Pipeline cache
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		CHECK_VK_RESULT(vkCreatePipelineCache(device->GetVulkanDevice(), &pipelineCacheCreateInfo, nullptr, &m_PipelineCache));

		// Pipeline layout
		// Push constants for UI rendering parameters
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstBlock);

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout->GetVulkanDescriptorSetLayout();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
		CHECK_VK_RESULT(vkCreatePipelineLayout(device->GetVulkanDevice(), &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout));

		// Setup graphics pipeline for UI rendering
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.flags = 0;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterizationState{};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.flags = 0;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.lineWidth = 1.0f;

		// Enable blending
		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.blendEnable = VK_TRUE;
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &blendAttachmentState;

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_FALSE;
		depthStencilState.depthWriteEnable = VK_FALSE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.front = depthStencilState.back;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		viewportState.flags = 0;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleState.flags = 0;

		std::vector<VkDynamicState> dynamicStateEnables =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
		dynamicState.flags = 0;

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = m_PipelineLayout;
		pipelineCreateInfo.renderPass = renderPass;
		pipelineCreateInfo.flags = 0;
		pipelineCreateInfo.basePipelineIndex = -1;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();

		// Vertex bindings an attributes based on ImGui vertex definition
		VkVertexInputBindingDescription vertexInputBindingDescription{};
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof(ImDrawVert);
		vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::vector<VkVertexInputBindingDescription> vertexInputBindings =
		{
			vertexInputBindingDescription
		};

		//position desc
		VkVertexInputAttributeDescription positionDesc{};
		positionDesc.location = 0;
		positionDesc.binding = 0;
		positionDesc.format = VK_FORMAT_R32G32_SFLOAT;
		positionDesc.offset = offsetof(ImDrawVert, pos);

		//uv desc
		VkVertexInputAttributeDescription uvDesc{};
		uvDesc.location = 1;
		uvDesc.binding = 0;
		uvDesc.format = VK_FORMAT_R32G32_SFLOAT;
		uvDesc.offset = offsetof(ImDrawVert, uv);

		//colour desc
		VkVertexInputAttributeDescription colourDesc{};
		colourDesc.location = 2;
		colourDesc.binding = 0;
		colourDesc.format = VK_FORMAT_R8G8B8A8_UNORM;
		colourDesc.offset = offsetof(ImDrawVert, col);

		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
			positionDesc,
			uvDesc,
			colourDesc
		};

		VkPipelineVertexInputStateCreateInfo vertexInputState{};
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
		vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
		vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
		vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

		pipelineCreateInfo.pVertexInputState = &vertexInputState;

		std::vector<vk::DescriptorSetLayout::Binding> bindingInfo;
		shaderStages[0] = vk::VulkanRenderer::Get()->LoadShader("shaders/ui.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, bindingInfo);
		shaderStages[1] = vk::VulkanRenderer::Get()->LoadShader("shaders/ui.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, bindingInfo);

		io.Fonts->TexID = (void*)m_DescriptorSet->GetVulkanDescriptorSet();

		CHECK_VK_RESULT(vkCreateGraphicsPipelines(device->GetVulkanDevice(), m_PipelineCache, 1, &pipelineCreateInfo, nullptr, &m_Pipeline));
	}

	void ImGUIImpl::NewFrame()
	{
		vk::VulkanRenderer* renderer = vk::VulkanRenderer::Get();

		if (!m_GameViewTextureDescSet)
		{
			m_GameViewTextureDescSet = AddTexture(renderer->GetOutputFramebuffer()->GetSampler(), renderer->GetOutputFramebuffer()->GetAttachment("colour").m_ImageView);
		}

		ImGuiIO& io = ImGui::GetIO();

		io.DeltaTime = (float)BaseApplication::Get().GetDeltaTime();

		double xpos, ypos;
		glfwGetCursorPos(renderer->GetWindow(), &xpos, &ypos);
		io.MousePos = ImVec2((float)xpos, (float)ypos);
		io.MouseDown[0] = glfwGetMouseButton(renderer->GetWindow(), GLFW_MOUSE_BUTTON_1);
		io.MouseDown[1] = glfwGetMouseButton(renderer->GetWindow(), GLFW_MOUSE_BUTTON_2);

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

										glm::vec3 direction = directionalLight->GetDirection();
										ImGui::DragFloat3("Direction", (float*)&direction, 0.005f, -1.f, 1.f);
										directionalLight->SetDirection(direction);

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
			ImGui::Image((void*)m_GameViewTextureDescSet->GetVulkanDescriptorSet(), ImVec2(1600, 900), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0, 0, 0, 0), true);
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

	void ImGUIImpl::DrawFrame(VkCommandBuffer commandBuffer)
	{
		ImGuiIO& io = ImGui::GetIO();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

		// Bind vertex and index buffer
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_VertexBuffer.m_Buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer.m_Buffer, 0, VK_INDEX_TYPE_UINT16);

		VkViewport viewport{};
		viewport.width = ImGui::GetIO().DisplaySize.x;
		viewport.height = ImGui::GetIO().DisplaySize.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		// UI scale and translate via push constants
		m_PushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		m_PushConstBlock.translate = glm::vec2(-1.0f);
		vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &m_PushConstBlock);

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
				VkRect2D scissorRect;
				scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
				scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
				scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
				scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
				vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);

				VkDescriptorSet desc_set[1] = { (VkDescriptorSet)pcmd->TextureId };
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, desc_set, 0, nullptr);


				vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
				indexOffset += pcmd->ElemCount;
			}
			vertexOffset += cmd_list->VtxBuffer.Size;
		}
	}

	vk::DescriptorSetRef ImGUIImpl::AddTexture(VkSampler sampler, VkImageView image_view)
	{
		vk::DescriptorSetRef descriptorSet = vk::DescriptorSet::CreateDescriptorSet(m_DescriptorPool, m_DescriptorSetLayout);
		descriptorSet->AddTexture(sampler, image_view, vk::DescriptorSet::BindingUsage::FragmentShader);
		descriptorSet->Build();
		return descriptorSet;
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