#include "VKPipeline.h"

#include "VulkanUtilities.h"

#include "VKShader.h"
#include "VKSwapchain.h"
#include "VKVertex.h"
#include "VKTexture.h"
#include "VKBuffer.h"
#include "VKUniformBuffer.h"
#include "VKDescriptorPool.h"
#include "VKDescriptorSet.h"

#include "VKDepthAttachment.h"

#include "core/math/matrix/matrix.h"

#include "core/types/EPArray.h"

RESULT VKPipeline::InitializeDescriptors(const EPRef<VKTexture>& pVKTexture) {
	RESULT r = R::OK;;

	// Create the uniform buffer
	// TODO: Take out of the pipeline?
	m_pVKUniformBuffer = VKUniformBuffer::make(m_vkPhysicalDevice, m_vkLogicalDevice, m_pVKSwapchain);
	CNM(m_pVKUniformBuffer, "Failed to create valid uniform buffer");

	// Descriptor Pool for the uniform buffer
	m_pVKDescriptorPool = VKDescriptorPool::make(m_vkPhysicalDevice, m_vkLogicalDevice, m_pVKSwapchain);
	CNM(m_pVKDescriptorPool, "Failed to create valid descriptor pool");

	// Descriptor Set
	m_pVKDescriptorSet = m_pVKDescriptorPool->MakeDescriptorSet(
		m_vkDescriptorSetLayout,
		m_pVKUniformBuffer,
		pVKTexture
	);
	CNM(m_pVKDescriptorSet, "Failed to create descriptor set");

Error:
	return r;
}

RESULT VKPipeline::Initialize() {
	RESULT r = R::OK;
	EPArray<VkVertexInputAttributeDescription, 3> vkVertexAttributeDescriptions = {};
	EPArray<VkAttachmentDescription, 2> vkAttachments = {};

	// Uniform Descriptor Set Layout
	VkDescriptorSetLayoutBinding vkDescriptorLayoutBindingUniformBufferObject = {};
	VkDescriptorSetLayoutBinding vkDescriptorLayoutBindingSampler = {};
	EPArray<VkDescriptorSetLayoutBinding, 2> vkDescriptorSetLayoutBindings;

	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo = {};

	CNM(m_vkPhysicalDevice, "Cannot create pipeline without a valid physical device");
	CNM(m_vkLogicalDevice, "Cannot create pipeline without a valid logical device");
	CNM(m_pVKSwapchain, "Cannot create pipeline without a valid swapchain");

	m_pVertexShader = VKShader::make(m_vkLogicalDevice, "vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	CNM(m_pVertexShader, "Failed to create vertex shader");

	m_pFragmentShader = VKShader::make(m_vkLogicalDevice, "frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	CNM(m_pFragmentShader, "Failed to create fragment shader");

	// Uniform Buffer Object

	//struct UniformBufferObject {
	//	matrix<float, 4, 4> m_mat4Model;
	//	matrix<float, 4, 4> m_mat4View;
	//	matrix<float, 4, 4> m_mat4Projection;
	//} uboTransforms;

	// TODO: Generalize these descriptor sets 
	// Create descriptor set layout (for vertex shader)
	// TODO: Move this into the shaders bruv / object or something
	
	// Descriptor Layout

	vkDescriptorLayoutBindingUniformBufferObject.binding = 0;
	vkDescriptorLayoutBindingUniformBufferObject.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vkDescriptorLayoutBindingUniformBufferObject.descriptorCount = 1;
	vkDescriptorLayoutBindingUniformBufferObject.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	vkDescriptorLayoutBindingUniformBufferObject.pImmutableSamplers = nullptr; // optional

	// Image Sampler
	vkDescriptorLayoutBindingSampler.binding = 1;
	vkDescriptorLayoutBindingSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	vkDescriptorLayoutBindingSampler.descriptorCount = 1;
	vkDescriptorLayoutBindingSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	vkDescriptorLayoutBindingSampler.pImmutableSamplers = nullptr; // optional
	
	vkDescriptorSetLayoutBindings[0] = vkDescriptorLayoutBindingUniformBufferObject;
	vkDescriptorSetLayoutBindings[1] = vkDescriptorLayoutBindingSampler;

	vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	vkDescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(vkDescriptorSetLayoutBindings.size());
	vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBindings.data;

	// create the descriptor layout set
	CVKRM(vkCreateDescriptorSetLayout(
		m_vkLogicalDevice, 
		&vkDescriptorSetLayoutCreateInfo, 
		nullptr, 
		&m_vkDescriptorSetLayout),
		"Failed to create descriptor set layout");

	// Set up the vertex input description (TODO: generalize this)
	VkVertexInputBindingDescription vkVertexBindingDescription = VKVertex<float, 4>::GetVKVertexBindingDescription();
	vkVertexAttributeDescriptions = VKVertex<float, 4>::GetVKVertexAttributeDescriptions();

	// Vertex Input Stage
	m_vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	m_vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	m_vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &vkVertexBindingDescription;
	m_vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = (uint32_t)vkVertexAttributeDescriptions.size();
	m_vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexAttributeDescriptions.data; // Optional

	// Fixed stages
	// TODO: This should all be moved into objects

	// Input assembly
	m_vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	m_vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	// Viewport
	m_vkViewport.x = 0.0f;
	m_vkViewport.y = 0.0f;
	m_vkViewport.width = (float)m_pVKSwapchain->GetExtentsWidth();
	m_vkViewport.height = (float)m_pVKSwapchain->GetExtentsHeight();
	m_vkViewport.minDepth = 0.0f;
	m_vkViewport.maxDepth = 1.0f;

	// viewport Scissor
	m_vkrect2DScissor.offset = { 0, 0 };
	m_vkrect2DScissor.extent = { m_pVKSwapchain->GetExtentsWidth(), m_pVKSwapchain->GetExtentsHeight() };

	// Viewport state 
	m_vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	m_vkPipelineViewportStateCreateInfo.viewportCount = 1;
	m_vkPipelineViewportStateCreateInfo.pViewports = &m_vkViewport;
	m_vkPipelineViewportStateCreateInfo.scissorCount = 1;
	m_vkPipelineViewportStateCreateInfo.pScissors = &m_vkrect2DScissor;

	// Rasterizer
	m_vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	m_vkPipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	m_vkPipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	m_vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	m_vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
	//m_vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
	m_vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	//m_vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	m_vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	m_vkPipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	m_vkPipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f; // Optional
	m_vkPipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f; // Optional
	m_vkPipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f; // Optional

	// Multisampling
	m_vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_vkPipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	m_vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	m_vkPipelineMultisampleStateCreateInfo.minSampleShading = 1.0f; // Optional
	m_vkPipelineMultisampleStateCreateInfo.pSampleMask = nullptr; // Optional
	m_vkPipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
	m_vkPipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // Optional

	// Color blending

	//fBlendEnabled
	//	finalColor.rgb = (srcColorBlendFactor * newColor.rgb) < colorBlendOp > (dstColorBlendFactor * oldColor.rgb);
	//	finalColor.a = (srcAlphaBlendFactor * newColor.a) < alphaBlendOp > (dstAlphaBlendFactor * oldColor.a);
	//else
	//	finalColor = newColor;
	//finalColor = finalColor & colorWriteMask;

	m_vkPipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	m_vkPipelineColorBlendAttachmentState.blendEnable = m_fBlendEnabled;
	m_vkPipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	m_vkPipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	m_vkPipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	m_vkPipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	m_vkPipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	m_vkPipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	// Alternative - TODO: move into an object
	//finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
	//finalColor.a = newAlpha.a;
	//
	//m_vkPipelineColorBlendAttachmentState.blendEnable = VK_TRUE;
	//m_vkPipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	//m_vkPipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	//m_vkPipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	//m_vkPipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	//m_vkPipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//m_vkPipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	m_vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	m_vkPipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	m_vkPipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
	m_vkPipelineColorBlendStateCreateInfo.attachmentCount = 1;
	m_vkPipelineColorBlendStateCreateInfo.pAttachments = &m_vkPipelineColorBlendAttachmentState;
	m_vkPipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f; // Optional
	m_vkPipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f; // Optional
	m_vkPipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f; // Optional
	m_vkPipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f; // Optional

	// Dynamic States
	m_vkPipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	m_vkPipelineDynamicStateCreateInfo.dynamicStateCount = 2;
	m_vkPipelineDynamicStateCreateInfo.pDynamicStates = m_vkDynamicStates.data();

	// Create the pipeline layout
	m_vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	
	// TODO: automate
	m_vkPipelineLayoutCreateInfo.setLayoutCount = 1; 
	m_vkPipelineLayoutCreateInfo.pSetLayouts = &m_vkDescriptorSetLayout; 

	// TODO: automate and use
	m_vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0; // Optional
	m_vkPipelineLayoutCreateInfo.pPushConstantRanges = nullptr; // Optional

	CVKRM(vkCreatePipelineLayout(m_vkLogicalDevice, &m_vkPipelineLayoutCreateInfo, nullptr, &m_vkPipelineLayout),
		"Failed to creat the pipeline layout");
	CNM(m_vkPipelineLayout, "Failed to create pipeline layout");

	// Color attachment TODO: Split into object
	m_vkColorAttachmentDescription.format = m_pVKSwapchain->GetVKSwapchainImageFormat();
	m_vkColorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	m_vkColorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	m_vkColorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	m_vkColorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	m_vkColorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	m_vkColorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	m_vkColorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Color attachment reference
	m_vkColorAttachmentReference.attachment = 0;
	m_vkColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Depth Attachment
	m_vkDepthAttachmentDescription = m_pVKSwapchain->GetVKDepthAttachment()->GetVKAttachmentDescription();
	m_vkDepthAttachmentReference.attachment = 1;
	m_vkDepthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Subpass
	m_vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	m_vkSubpassDescription.colorAttachmentCount = 1;
	m_vkSubpassDescription.pColorAttachments = &m_vkColorAttachmentReference;
	m_vkSubpassDescription.pDepthStencilAttachment = &m_vkDepthAttachmentReference;

	// Subpadd dependency
	m_vkSubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	m_vkSubpassDependency.dstSubpass = 0;
	m_vkSubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	m_vkSubpassDependency.srcAccessMask = 0;
	m_vkSubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	m_vkSubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// Render pass
	// TODO: Automate this
	vkAttachments[0] = m_vkColorAttachmentDescription;
	vkAttachments[1] = m_vkDepthAttachmentDescription;

	m_vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;	
	m_vkRenderPassCreateInfo.pAttachments = vkAttachments.data;
	m_vkRenderPassCreateInfo.attachmentCount = static_cast<uint32_t>(vkAttachments.size());

	//m_vkRenderPassCreateInfo.pAttachments = &m_vkColorAttachmentDescription;
	//m_vkRenderPassCreateInfo.attachmentCount = 1;

	m_vkRenderPassCreateInfo.subpassCount = 1;
	m_vkRenderPassCreateInfo.pSubpasses = &m_vkSubpassDescription;
	m_vkRenderPassCreateInfo.dependencyCount = 1;
	m_vkRenderPassCreateInfo.pDependencies = &m_vkSubpassDependency;

	CVKRM(vkCreateRenderPass(m_vkLogicalDevice, &m_vkRenderPassCreateInfo, nullptr, &m_vkRenderPass),
		"Failed to create render pass");
	CNM(m_vkRenderPass, "Failed to create render pass");

	// Depth Stencil 
	m_vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	m_vkPipelineDepthStencilStateCreateInfo.depthTestEnable = true;
	m_vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = true;
	m_vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	m_vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = false;
	m_vkPipelineDepthStencilStateCreateInfo.minDepthBounds = 0.0f;	// opt
	m_vkPipelineDepthStencilStateCreateInfo.maxDepthBounds = 0.0f;	// opt
	m_vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = false;
	m_vkPipelineDepthStencilStateCreateInfo.front= {};					// opt
	m_vkPipelineDepthStencilStateCreateInfo.back = {};					// opt

	// Create the graphics pipeline 

	VkPipelineShaderStageCreateInfo vkPipelineShaderStages[] = {
		m_pVertexShader->GetShaderStageCreateInfo(), 
		m_pFragmentShader->GetShaderStageCreateInfo()
	};

	m_vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	m_vkGraphicsPipelineCreateInfo.stageCount = 2;
	m_vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStages;
	m_vkGraphicsPipelineCreateInfo.pVertexInputState = &m_vkPipelineVertexInputStateCreateInfo;
	m_vkGraphicsPipelineCreateInfo.pInputAssemblyState = &m_vkPipelineInputAssemblyStateCreateInfo;
	m_vkGraphicsPipelineCreateInfo.pViewportState = &m_vkPipelineViewportStateCreateInfo;
	m_vkGraphicsPipelineCreateInfo.pRasterizationState = &m_vkPipelineRasterizationStateCreateInfo;
	m_vkGraphicsPipelineCreateInfo.pMultisampleState = &m_vkPipelineMultisampleStateCreateInfo;
	m_vkGraphicsPipelineCreateInfo.pDepthStencilState = &m_vkPipelineDepthStencilStateCreateInfo; 
	m_vkGraphicsPipelineCreateInfo.pColorBlendState = &m_vkPipelineColorBlendStateCreateInfo;
	m_vkGraphicsPipelineCreateInfo.pDynamicState = nullptr; // Optional
	m_vkGraphicsPipelineCreateInfo.layout = m_vkPipelineLayout;
	m_vkGraphicsPipelineCreateInfo.renderPass = m_vkRenderPass;
	m_vkGraphicsPipelineCreateInfo.subpass = 0;
	m_vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	m_vkGraphicsPipelineCreateInfo.basePipelineIndex = -1; // Optional

	CVKRM(vkCreateGraphicsPipelines(m_vkLogicalDevice, VK_NULL_HANDLE, 1, &m_vkGraphicsPipelineCreateInfo, nullptr, &m_vkGraphicsPipeline),
		"Failed to create vk graphics pipeline");

	CNM(m_vkGraphicsPipeline, "Failed to create graphics pipeline");

Error:
	return r;
}

RESULT VKPipeline::Update(uint32_t index) {
	RESULT r = R::OK;

	// TODO: This needs to be more general
	CRM(m_pVKUniformBuffer->Update(index), "Failed to update uniform buffer");

Error:
	return r;
}

RESULT VKPipeline::Kill() {
	RESULT r = R::OK;

	m_pVertexShader = nullptr;
	m_pFragmentShader = nullptr;

	m_pVKDescriptorSet = nullptr;
	m_pVKDescriptorPool = nullptr;
	m_pVKUniformBuffer = nullptr;

	CN(m_vkLogicalDevice);
	CN(m_vkGraphicsPipeline);

	vkDestroyPipeline(m_vkLogicalDevice, m_vkGraphicsPipeline, nullptr);

	CN(m_vkPipelineLayout);

	vkDestroyPipelineLayout(m_vkLogicalDevice, m_vkPipelineLayout, nullptr);

	CN(m_vkRenderPass);

	vkDestroyRenderPass(m_vkLogicalDevice, m_vkRenderPass, nullptr);

	CN(m_vkDescriptorSetLayout)

	vkDestroyDescriptorSetLayout(m_vkLogicalDevice, m_vkDescriptorSetLayout, nullptr);

Error:
	return r;
}

EPRef<VKPipeline> VKPipeline::InternalMake(VkPhysicalDevice vkPhysicalDevice, VkDevice vkLogicalDevice, const EPRef<VKSwapchain>& pVKSwapchain) {
	RESULT r = R::OK;
	EPRef<VKPipeline> pVKPipeline = nullptr;

	pVKPipeline = new VKPipeline(vkPhysicalDevice, vkLogicalDevice, pVKSwapchain);
	CNM(pVKPipeline, "Failed to allocate pipeline");

	CRM(pVKPipeline->Initialize(), "Failed to initialize VK pipeline");

Success:
	return pVKPipeline;

Error:
	pVKPipeline = nullptr;
	return nullptr;
}

VKPipeline::VKPipeline(VkPhysicalDevice vkPhysicalDevice, VkDevice vkLogicalDevice, const EPRef<VKSwapchain>& pVKSwapchain) :
	m_vkPhysicalDevice(vkPhysicalDevice),
	m_vkLogicalDevice(vkLogicalDevice),
	m_pVKSwapchain(pVKSwapchain)
{
	//
}

VKPipeline::~VKPipeline() {
	Kill();
}