#include "VKDescriptorPool.h"

#include "VulkanUtilities.h"

#include "VKDescriptorSet.h"
#include "VKDepthAttachment.h"

RESULT VKDescriptorPool::Initialize() {
	RESULT r = R::OK;

	CNM(m_vkLogicalDevice, "Cannot initialize command pool without valid logical device");
	CNM(m_pVKSwapchain, "Cannot initialize command pool without valid swapchain");

	// TODO: generalize this... 
	m_vkDescriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	m_vkDescriptorPoolSizes[0].descriptorCount = static_cast<uint32_t>(m_pVKSwapchain->GetSwapchainImageCount());

	m_vkDescriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	m_vkDescriptorPoolSizes[1].descriptorCount = static_cast<uint32_t>(m_pVKSwapchain->GetSwapchainImageCount());

	m_vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	m_vkDescriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(m_vkDescriptorPoolSizes.size());
	m_vkDescriptorPoolCreateInfo.pPoolSizes = m_vkDescriptorPoolSizes.data;
	m_vkDescriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(m_pVKSwapchain->GetSwapchainImageCount());

	CVKRM(vkCreateDescriptorPool(m_vkLogicalDevice, &m_vkDescriptorPoolCreateInfo, nullptr, &m_vkDescriptorPool),
		"Failed to create descriptor pool");

Error:
	return r;
}

RESULT VKDescriptorPool::Kill() {
	RESULT r = R::OK;

	CN(m_vkDescriptorPool);

	vkDestroyDescriptorPool(m_vkLogicalDevice, m_vkDescriptorPool, nullptr);

Error:
	return r;
}

EPRef<VKDescriptorSet> VKDescriptorPool::MakeDescriptorSet(
	VkDescriptorSetLayout vkDescriptorSetLayout, 
	const EPRef<VKUniformBuffer>& pVKUniformBuffer,
	const EPRef<VKTexture>& pVKTexture
) {
	RESULT r = R::OK;
	EPRef<VKDescriptorSet> pVKDescriptorSet = nullptr;

	pVKDescriptorSet = VKDescriptorSet::make(
		m_vkPhysicalDevice,
		m_vkLogicalDevice,
		this,
		pVKUniformBuffer,
		pVKTexture,
		vkDescriptorSetLayout
	);

	CNM(pVKDescriptorSet, "Failed to create descriptor set");

Success:
	return pVKDescriptorSet;

Error:
	return nullptr;
}

EPRef<VKDescriptorPool> VKDescriptorPool::InternalMake(
	VkPhysicalDevice vkPhysicalDevice,
	VkDevice vkLogicalDevice,
	const EPRef<VKSwapchain>& pVKSwapchain
) {
	RESULT r = R::OK;
	EPRef<VKDescriptorPool> pVKDescriptorPool = nullptr;

	pVKDescriptorPool = new VKDescriptorPool(vkPhysicalDevice, vkLogicalDevice, pVKSwapchain);
	CNM(pVKDescriptorPool, "Failed to allocate vk command pool");

	CRM(pVKDescriptorPool->Initialize(), "Failed to initialize VK command pool");

Success:
	return pVKDescriptorPool;

Error:
	pVKDescriptorPool = nullptr;
	return nullptr;
}

VKDescriptorPool::VKDescriptorPool(
	VkPhysicalDevice vkPhysicalDevice,
	VkDevice vkLogicalDevice,
	const EPRef<VKSwapchain>& pVKSwapchain
) :
	m_vkPhysicalDevice(vkPhysicalDevice),
	m_vkLogicalDevice(vkLogicalDevice),
	m_pVKSwapchain(pVKSwapchain)
{
	//
}

VKDescriptorPool::~VKDescriptorPool() {
	Kill();
}