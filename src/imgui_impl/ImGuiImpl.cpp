#include "imgui_impl/ImGuiImpl.h"
#include "Log.h"
#include "GameObject.h"
#include "TranslationComponent.h"
#include "ModelComponent.h"
#include "PointLightComponent.h"
#include "imgui/imgui_internal.h"

ImGUIImpl::ImGUIImpl(vk::VulkanDevice* device)
{
	//ImGui::CreateContext();
	m_Device = device;
}

ImGUIImpl::~ImGUIImpl()
{
	// Release all Vulkan resources required for rendering imGui
	m_VertexBuffer.Cleanup();
	m_IndexBuffer.Cleanup();
	vkDestroyImage(m_Device->GetDevice(), m_FontImage, nullptr);
	vkDestroyImageView(m_Device->GetDevice(), m_FontView, nullptr);
	vkFreeMemory(m_Device->GetDevice(), m_FontMemory, nullptr);
	vkDestroySampler(m_Device->GetDevice(), m_Sampler, nullptr);
	vkDestroyPipelineCache(m_Device->GetDevice(), m_PipelineCache, nullptr);
	vkDestroyPipeline(m_Device->GetDevice(), m_Pipeline, nullptr);
	vkDestroyPipelineLayout(m_Device->GetDevice(), m_PipelineLayout, nullptr);
	vkDestroyDescriptorPool(m_Device->GetDevice(), m_DescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_Device->GetDevice(), m_DescriptorSetLayout, nullptr);
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

    vk::VulkanRenderer* renderer = static_cast<vk::VulkanRenderer*>(Application::Get().GetRenderer());

	glfwSetScrollCallback(renderer->GetWindow(), OnMouseScolled);
	glfwSetKeyCallback(renderer->GetWindow(), OnKeyDown);
	glfwSetCharCallback(renderer->GetWindow(), OnChar);
}

void ImGUIImpl::InitResources(VkRenderPass renderPass, VkQueue copyQueue)
{
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
	CHECK_VK_RESULT(vkCreateImage(m_Device->GetDevice(), &imageInfo, nullptr, &m_FontImage));
	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(m_Device->GetDevice(), m_FontImage, &memReqs);
	VkMemoryAllocateInfo memAllocInfo{};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = m_Device->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	CHECK_VK_RESULT(vkAllocateMemory(m_Device->GetDevice(), &memAllocInfo, nullptr, &m_FontMemory));
	CHECK_VK_RESULT(vkBindImageMemory(m_Device->GetDevice(), m_FontImage, m_FontMemory, 0));

	// Image view
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = m_FontImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.layerCount = 1;
	CHECK_VK_RESULT(vkCreateImageView(m_Device->GetDevice(), &viewInfo, nullptr, &m_FontView));

	// Staging buffers for font data upload
	vk::Buffer stagingBuffer;

	CHECK_VK_RESULT(m_Device->CreateBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer,
		uploadSize));

	stagingBuffer.Map();
	memcpy(stagingBuffer.m_Mapped, fontData, uploadSize);
	stagingBuffer.Unmap();

	// Copy buffer data to font image
	VkCommandBuffer copyCmd = m_Device->CreateCommandBuffer(true);

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

	m_Device->FlushCommandBuffer(copyCmd, copyQueue);

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
	CHECK_VK_RESULT(vkCreateSampler(m_Device->GetDevice(), &samplerInfo, nullptr, &m_Sampler));

	VkDescriptorPoolSize descriptorPoolSize{};
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorPoolSize.descriptorCount = 2;
	// Descriptor pool
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		descriptorPoolSize
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = 2;
	CHECK_VK_RESULT(vkCreateDescriptorPool(m_Device->GetDevice(), &descriptorPoolInfo, nullptr, &m_DescriptorPool));

	// Descriptor set layout
	VkDescriptorSetLayoutBinding setLayoutBinding{};
	setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	setLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	setLayoutBinding.binding = 0;
	setLayoutBinding.descriptorCount = 1;
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
	{
		setLayoutBinding
	};

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pBindings = setLayoutBindings.data();
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());

	CHECK_VK_RESULT(vkCreateDescriptorSetLayout(m_Device->GetDevice(), &descriptorSetLayoutCreateInfo, nullptr, &m_DescriptorSetLayout));

	// Descriptor set
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = m_DescriptorPool;
	descriptorSetAllocateInfo.pSetLayouts = &m_DescriptorSetLayout;
	descriptorSetAllocateInfo.descriptorSetCount = 1;

	CHECK_VK_RESULT(vkAllocateDescriptorSets(m_Device->GetDevice(), &descriptorSetAllocateInfo, &m_DescriptorSet));

	VkDescriptorImageInfo descriptorImageInfo{};
	descriptorImageInfo.sampler = m_Sampler;
	descriptorImageInfo.imageView = m_FontView;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkDescriptorImageInfo fontDescriptor = descriptorImageInfo;

	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = m_DescriptorSet;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.pImageInfo = &fontDescriptor;
	writeDescriptorSet.descriptorCount = 1;

	std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
		writeDescriptorSet
	};
	vkUpdateDescriptorSets(m_Device->GetDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

	// Pipeline cache
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	CHECK_VK_RESULT(vkCreatePipelineCache(m_Device->GetDevice(), &pipelineCacheCreateInfo, nullptr, &m_PipelineCache));

	// Pipeline layout
	// Push constants for UI rendering parameters
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PushConstBlock);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
	CHECK_VK_RESULT(vkCreatePipelineLayout(m_Device->GetDevice(), &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout));

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

    shaderStages[0] = static_cast<vk::VulkanRenderer*>(Application::Get().GetRenderer())->LoadShader("shaders/ui.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = static_cast<vk::VulkanRenderer*>(Application::Get().GetRenderer())->LoadShader("shaders/ui.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	io.Fonts->TexID = m_DescriptorSet;

	CHECK_VK_RESULT(vkCreateGraphicsPipelines(m_Device->GetDevice(), m_PipelineCache, 1, &pipelineCreateInfo, nullptr, &m_Pipeline));
}

void ImGUIImpl::NewFrame()
{
    vk::VulkanRenderer* renderer = static_cast<vk::VulkanRenderer*>(Application::Get().GetRenderer());

	if (m_GameViewTextureDescSet == VK_NULL_HANDLE)
		m_GameViewTextureDescSet = AddTexture(renderer->m_OutputTexture.m_TextureSampler, renderer->m_OutputTexture.m_ImageView);

	ImGuiIO& io = ImGui::GetIO();

	io.DeltaTime = (float)Application::Get().GetDeltaTime();

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
			for (GameObject* obj : Application::Get().GetScene()->GetObjects())
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
						ImGui::Text(comp->m_ModelPath.c_str());
						ImGui::Text(comp->m_TexturePath.c_str());
						ImGui::Text(comp->m_NormalPath.c_str());
						ImGui::TreePop();
					}
				}
				if (PointLightComponent* comp = m_SelectedObject->GetComponent<PointLightComponent>())
				{
					if (ImGui::TreeNodeEx("Point Light", ImGuiTreeNodeFlags_DefaultOpen))
					{
						glm::vec3 colour = comp->GetColour();
						ImGui::DragFloat3("Colour", (float*)&colour, 0.05f);
						comp->SetColour(colour);

						float radius = comp->GetRadius();
						ImGui::DragFloat("Radius", &radius, 0.05f);
						comp->SetRadius(radius);

						ImGui::TreePop();
					}
				}
			}
		}
		ImGui::End();
		//ImGui::ShowTestWindow();

		ImGui::Begin("Game Window", nullptr, ImGuiWindowFlags_NoScrollbar);
		Application::Get().m_GameFocued = ImGui::IsWindowHovered();

		//TODO game size is fixed to initial window size, give more control over game resolution.
		ImGui::Image(m_GameViewTextureDescSet, ImVec2(1600, 900), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1,1,1,1), ImVec4(0,0,0,0), true);
		ImGui::End();

		Log::Draw("Log");

		ImGui::End(); //Dockspace
	}

	// Render to generate draw buffers
	ImGui::Render();
}

void ImGUIImpl::UpdateBuffers()
{
	ImDrawData* imDrawData = ImGui::GetDrawData();

	// Note: Alignment is done inside buffer creation
	VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
	VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

	// Update buffers only if vertex or index count has been changed compared to current buffer size

	// Vertex buffer
	if ((m_VertexBuffer.m_Buffer == VK_NULL_HANDLE) || (m_VertexCount != imDrawData->TotalVtxCount)) {
		m_VertexBuffer.Unmap();
		m_VertexBuffer.Cleanup();
		CHECK_VK_RESULT(m_Device->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &m_VertexBuffer, vertexBufferSize));
		m_VertexCount = imDrawData->TotalVtxCount;
		m_VertexBuffer.Unmap();
		m_VertexBuffer.Map();
	}

	// Index buffer
	VkDeviceSize indexSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
	if ((m_IndexBuffer.m_Buffer == VK_NULL_HANDLE) || (m_IndexCount < imDrawData->TotalIdxCount)) {
		m_IndexBuffer.Unmap();
		m_IndexBuffer.Cleanup();
		CHECK_VK_RESULT(m_Device->CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &m_IndexBuffer, indexBufferSize));
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

VkDescriptorSet ImGUIImpl::AddTexture(VkSampler sampler, VkImageView image_view)
{
	VkDescriptorSet descriptor_set;
	// Create Descriptor Set:
	{
		VkDescriptorSetAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = m_DescriptorPool;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &m_DescriptorSetLayout;
		CHECK_VK_RESULT(vkAllocateDescriptorSets(m_Device->GetDevice(), &alloc_info, &descriptor_set));
	}

	// Update the Descriptor Set:s
	{
		VkDescriptorImageInfo desc_image[1] = {};
		desc_image[0].sampler = sampler;
		desc_image[0].imageView = image_view;
		desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkWriteDescriptorSet write_desc[1] = {};
		write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_desc[0].dstSet = descriptor_set;
		write_desc[0].descriptorCount = 1;
		write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_desc[0].pImageInfo = desc_image;
		vkUpdateDescriptorSets(m_Device->GetDevice(), 1, write_desc, 0, NULL);
	}

	return descriptor_set;
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
