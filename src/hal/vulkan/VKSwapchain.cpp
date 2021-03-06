#include "VKSwapchain.h"

#include "VulkanUtilities.h"

#include <algorithm>

#include "VKPipeline.h"
#include "VKFramebuffer.h"

#include "VKDescriptorSet.h"

#include "VKImage.h"
#include "VKImageView.h"
#include "VKDepthAttachment.h"
#include "VKCommandPool.h"

RESULT VKSwapchain::Initialize() {
	RESULT r = R::OK;

	CNM(m_vkPhysicalDevice, "VKSwapchain needs a valid physical device");
	CNM(m_vkSurface, "VKSwapchain needs a valid surface");

	// Get Surface Capabilities
	CVKRM(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_vkPhysicalDevice, m_vkSurface, &m_vkSurfaceCapabilities),
		"Failed to get Physical device surface capabilities");

	// Query supported surface formats
	CVKRM(vkGetPhysicalDeviceSurfaceFormatsKHR(m_vkPhysicalDevice, m_vkSurface, &m_vkPhysicalDeviceSurfaceFormatCount, nullptr),
		"Failed to get physical device surface format count");

	CBM(m_vkPhysicalDeviceSurfaceFormatCount != 0, "Physical device has no surface formats");

	m_vkSurfaceFormats = EPVector<VkSurfaceFormatKHR>(m_vkPhysicalDeviceSurfaceFormatCount);
	CVKRM(vkGetPhysicalDeviceSurfaceFormatsKHR(m_vkPhysicalDevice, m_vkSurface, &m_vkPhysicalDeviceSurfaceFormatCount, m_vkSurfaceFormats.data(m_vkPhysicalDeviceSurfaceFormatCount)),
		"Failed to get physical device surface formats");

	// Query supported presentation modes
	
	CVKRM(vkGetPhysicalDeviceSurfacePresentModesKHR(m_vkPhysicalDevice, m_vkSurface, &m_vkPhysicalDevicePresenationModeCount, nullptr),
		"Failed to get physical device presentation mode count");

	CBM(m_vkPhysicalDeviceSurfaceFormatCount != 0, "Physical device has no presentation modes");

	m_vkPresentationModes = EPVector< VkPresentModeKHR>(m_vkPhysicalDevicePresenationModeCount);
	CVKRM(vkGetPhysicalDeviceSurfacePresentModesKHR(m_vkPhysicalDevice, m_vkSurface, &m_vkPhysicalDevicePresenationModeCount, m_vkPresentationModes.data(m_vkPhysicalDevicePresenationModeCount)),
		"Failed to get physical device presentation modes");

Error:
	return r;
}

RESULT VKSwapchain::InitializeFramebuffers(const EPRef<VKPipeline> &pVKPipeline) {
	RESULT r = R::OK;

	m_pVKPipeline = pVKPipeline;

	CNM(m_vkLogicalDevice, "Framebuffers need valid logical device");
	CNM(m_pVKPipeline, "Framebuffers need valid pipeline");

	for (uint32_t i = 0; i < GetSwapchainImageCount(); i++) {
		
		// TODO: this is nasty currently
		EPRef<VKFramebuffer> pVKFramebuffer = 
			VKFramebuffer::make(
				m_vkLogicalDevice, 
				m_pVKPipeline, 
				*(new EPRef<VKSwapchain>(this)), 
				i
		);

		CNM(pVKFramebuffer, "Failed to create framebuffer");

		m_vkFramebuffers.PushBack(pVKFramebuffer);
	}

Error:
	return r;
}

// This is split since this call can simply query the object
EPRef<VKSwapchain> VKSwapchain::make(
	VkPhysicalDevice vkPhysicalDevice, 
	VkSurfaceKHR vkSurface, 
	const EPRef<VKCommandPool>& pVKCommandPool
) {
	RESULT r = R::OK;
	EPRef<VKSwapchain> pVKSwapchain = nullptr;

	CNM(vkPhysicalDevice, "Cannot make vk swapchain without a valid physical device");
	CNM(vkSurface, "Cannot make vk swapchain without a valid surface");

	pVKSwapchain = new VKSwapchain(vkPhysicalDevice, vkSurface, pVKCommandPool);
	CNM(pVKSwapchain, "Failed to allocate swapchain");

	CRM(pVKSwapchain->Initialize(), "Failed to initialize VK swapchain");

Success:
	return pVKSwapchain;

Error:
	pVKSwapchain = nullptr;
	return nullptr;
}

EPRef<VKSwapchain> VKSwapchain::make(
	VkPhysicalDevice vkPhysicalDevice, 
	VkSurfaceKHR vkSurface, 
	const EPRef<VKCommandPool>& pVKCommandPool,
	VkDevice vkLogicalDevice,
	VkFormat vkSurfaceFormat, 
	VkColorSpaceKHR vkColorSpaceKHR,
	VkPresentModeKHR vkPresentModeKHR,
	VkExtent2D vkExtent2D
) {
	RESULT r = R::OK;
	EPRef<VKSwapchain> pVKSwapchain = nullptr;

	CNM(vkPhysicalDevice, "Cannot make vk swapchain without a valid physical device");
	CNM(vkSurface, "Cannot make vk swapchain without a valid surface");
	CNM(vkLogicalDevice, "Cannot make vk swapchain without a valid logical device");

	pVKSwapchain = VKSwapchain::make(vkPhysicalDevice, vkSurface, pVKCommandPool);
	CNM(pVKSwapchain, "Failed to initialize VK swapchain");

	pVKSwapchain->m_vkLogicalDevice = vkLogicalDevice;

	// Select valid presentation mode and format
	CRM(pVKSwapchain->SelectSurfaceFormat(vkSurfaceFormat, vkColorSpaceKHR),
		"Failed to select surface format");

	CRM(pVKSwapchain->SelectPresentationMode(vkPresentModeKHR),
		"Failed to select presentation mode");

	CRM(pVKSwapchain->SelectSwapchainExtent(vkExtent2D),
		"Failed to set swap chain extends");

	CRM(pVKSwapchain->CreateSwapchain(), "Failed to create swapchain");

Success:
	return pVKSwapchain;

Error:
	pVKSwapchain = nullptr;
	return nullptr;
}

RESULT VKSwapchain::SelectSurfaceFormat(VkFormat vkSurfaceFormat, VkColorSpaceKHR vkColorSpaceKHR) {
	RESULT r = R::OK;
	bool fFound = false;

	for (const auto& availableFormat : m_vkSurfaceFormats) {
		if (availableFormat.format == vkSurfaceFormat &&
			availableFormat.colorSpace == vkColorSpaceKHR)
		{
			m_vkSelectedSurfaceFormat = availableFormat;
			fFound = true;
			break;
		}
	}

	CBM(fFound, "Failed to find selected surface format");

Error:
	return r;
}

RESULT VKSwapchain::SelectPresentationMode(VkPresentModeKHR vkPresentModeKHR) {
	RESULT r = R::OK;
	bool fFound = false;

	for (const auto& availablePresentationMode : m_vkPresentationModes) {
		if (availablePresentationMode == vkPresentModeKHR) {
			m_vkSelectedPresentationMode = availablePresentationMode;
			fFound = true;
			break;
		}
	}

	CBM(fFound, "Failed to find selected presentation mode");

Error:
	return r;
}

RESULT VKSwapchain::SelectSwapchainExtent(VkExtent2D vkExtent2D) {
	RESULT r = R::OK;

	if (m_vkSurfaceCapabilities.currentExtent.width != UINT32_MAX) {
		m_vkSelectedExtent2D = m_vkSurfaceCapabilities.currentExtent;
	}
	else {
		m_vkSelectedExtent2D = vkExtent2D;

		m_vkSelectedExtent2D.width = std::max(
			m_vkSurfaceCapabilities.minImageExtent.width,
			std::min(m_vkSurfaceCapabilities.maxImageExtent.width, m_vkSelectedExtent2D.width));
		m_vkSelectedExtent2D.height = std::max(
			m_vkSurfaceCapabilities.minImageExtent.height,
			std::min(m_vkSurfaceCapabilities.maxImageExtent.height, m_vkSelectedExtent2D.height));
	}

	DEBUG_LINEOUT("vkExtends: {%d, %d}", m_vkSelectedExtent2D.width, m_vkSelectedExtent2D.height);

Error:
	return r;
}

// TODO: We may want to open this up as needed in the future
RESULT VKSwapchain::CreateSwapchain() {
	RESULT r = R::OK;

	VKQueueFamilies vkQueueFamilies;

	CNM(m_vkPhysicalDevice, "Cannot create swapchain without a valid physical device");
	CNM(m_vkSurface, "Cannot create swapchain without a valid surface");
	CNM(m_vkLogicalDevice, "Cannot create swapchain without a valid logcial device");

	m_swapchainImageCount = m_vkSurfaceCapabilities.minImageCount + 1;

	if (m_vkSurfaceCapabilities.maxImageCount > 0 && m_swapchainImageCount > m_vkSurfaceCapabilities.maxImageCount) {
		m_swapchainImageCount = m_vkSurfaceCapabilities.maxImageCount;
	}

	m_vkSwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	m_vkSwapchainCreateInfo.surface = m_vkSurface;
	m_vkSwapchainCreateInfo.minImageCount = m_swapchainImageCount;
	m_vkSwapchainCreateInfo.imageFormat = m_vkSelectedSurfaceFormat.format;
	m_vkSwapchainCreateInfo.imageColorSpace = m_vkSelectedSurfaceFormat.colorSpace;
	m_vkSwapchainCreateInfo.imageExtent = m_vkSelectedExtent2D;
	m_vkSwapchainCreateInfo.imageArrayLayers = 1;
	m_vkSwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	vkQueueFamilies = FindQueueFamilies(m_vkPhysicalDevice, m_vkSurface);

	// TODO: Actually use this
	//if (vkQueueFamilies.GetUniqueIndexes().size() > 1) {
	//	m_vkSwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	//	m_vkSwapchainCreateInfo.queueFamilyIndexCount = (uint32_t)vkQueueFamilies.GetUniqueIndexes().size();
	//	m_vkSwapchainCreateInfo.pQueueFamilyIndices = vkQueueFamilies.GetUniqueIndexes().data();
	//}
	//else {
		m_vkSwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		m_vkSwapchainCreateInfo.queueFamilyIndexCount = 0; // Optional
		m_vkSwapchainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
	//}

	m_vkSwapchainCreateInfo.preTransform = m_vkSurfaceCapabilities.currentTransform;
	m_vkSwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	m_vkSwapchainCreateInfo.presentMode = m_vkSelectedPresentationMode;
	m_vkSwapchainCreateInfo.clipped = VK_TRUE;
	m_vkSwapchainCreateInfo.oldSwapchain = nullptr;

	CVKRM(vkCreateSwapchainKHR(m_vkLogicalDevice, &m_vkSwapchainCreateInfo, nullptr, &m_vkSwapchain),
		"Failed to create swap chain");
	CNM(m_vkSwapchain, "Swapchain is invalid");

	// Set up the swapchain images
	CVKRM(vkGetSwapchainImagesKHR(m_vkLogicalDevice, m_vkSwapchain, &m_swapchainImageCount, nullptr),
		"Failed to get swapchain image count");
	CBM(m_swapchainImageCount != 0, "Found no swap chain images");


	// Do the same as VKImageView object below
	m_swapchainImages = EPVector<VkImage>(m_swapchainImageCount);
	CVKRM(vkGetSwapchainImagesKHR(
		m_vkLogicalDevice, 
		m_vkSwapchain, 
		&m_swapchainImageCount, 
		m_swapchainImages.data(m_swapchainImageCount)), 
	"Failed to get swapchain images");

	m_vkSwapchainImageFormat = m_vkSelectedSurfaceFormat.format;

	// Set up the swapchain image views
	for (size_t i = 0; i < m_swapchainImages.size(); i++) {	
		EPRef<VKImageView> pVKImageView = new VKImageView(m_vkPhysicalDevice, m_vkLogicalDevice);
		CNM(pVKImageView, "Failed to create swapchain image view %zu", i);
		CRM(pVKImageView->Initialize(m_swapchainImages[i], m_vkSwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT), 
			"Failed to initialize swapchain image view %zu", i);
		
		m_swapchainImageViews.PushBack(pVKImageView);
	}
	
	// Depth Attachment
	m_pVKDepthAttachment = VKDepthAttachment::make(
		m_vkPhysicalDevice, 
		m_vkLogicalDevice, 
		EPRef<VKSwapchain>(this),
		m_pVKCommandPool);
	CNM(m_pVKDepthAttachment, "Failed to create depth attachment");

Error:
	return r;
}

const VkImageView& VKSwapchain::GetSwapchainImageView(uint32_t index) { 
	return m_swapchainImageViews[index]->GetVKImageViewHandle();
}

RESULT VKSwapchain::KillFramebuffers() {
	RESULT r = R::OK;

	CNM(m_pVKPipeline, "Cannot destroy framebufers without valid pipeline");

	// Framebuffers
	for (int i = 0; i < m_vkFramebuffers.size(); i++) {
		m_vkFramebuffers[i] = nullptr;
	}
	m_vkFramebuffers.clear(true);

	m_pVKPipeline = nullptr;

Error:
	return r;
}

RESULT VKSwapchain::Kill() {
	RESULT r = R::OK;

	CNM(m_vkLogicalDevice, "Cannot kill swapchain without valid physical device");
	CNM(m_vkSwapchain, "Cannot destroy swapchain without valid swapchain");

	m_pVKDepthAttachment = nullptr;

	// Framebuffers done elsewhere 

	//for (auto imageView : m_swapchainImageViews) {
	//	vkDestroyImageView(m_vkLogicalDevice, imageView, nullptr);
	//}
	m_swapchainImageViews.destroy(true);

	vkDestroySwapchainKHR(m_vkLogicalDevice, m_vkSwapchain, nullptr);
	m_vkSwapchain = nullptr;

Error:
	return r;
}

const VkFramebuffer VKSwapchain::GetSwapchainFramebuffers(uint32_t i) const {
	return m_vkFramebuffers[i]->GetVKFrameBufferHandle();
}

VKSwapchain::VKSwapchain(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface, const EPRef<VKCommandPool>& pVKCommandPool) :
	m_vkPhysicalDevice(vkPhysicalDevice),
	m_vkSurface(vkSurface),
	m_pVKCommandPool(pVKCommandPool)
{
	//
}

VKSwapchain::~VKSwapchain() {
	Kill();
}