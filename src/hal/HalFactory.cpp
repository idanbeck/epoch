#include "HALFactory.h"

#ifdef EPOCH_HAL_VULKAN
	#include "hal/vulkan/VKHAL.h"

	// TODO: This shouldn't be needed here
	// These are objects referenced in VKHAL
	#include "hal/vulkan/VKPipeline.h"
	#include "hal/vulkan/VKSwapchain.h"
	#include "hal/vulkan/VKCommandPool.h"
	#include "hal/vulkan/VKCommandBuffers.h"
	#include "hal/vulkan/VKVertexBuffer.h"
	#include "hal/vulkan/VKDescriptorSet.h"
	#include "hal/vulkan/VKTexture.h"
	#include "hal/vulkan/VKDepthAttachment.h"
	#include "hal/vulkan/VKModel.h"

	EPRef<HAL> HALFactory::InternalMakeVulkan(EPRef<SandboxWindowProcess> pSBWindowProcess) {
		RESULT r = R::OK;

		EPRef<HAL> pHAL = nullptr;

		pHAL = new VKHAL();
		CNM(pHAL, "Failed to allocate VulkanHAL");

		CRM(pHAL->SetSBWindowProcess(pSBWindowProcess), "Failed to set sandbox window process for HAL");

		// Initialize is handled in the process itself
		CRM(pHAL->Initialize(), "Failed to initialize Win64SandboxWindowProcess");

	Error:
		return pHAL;
	}
#else
	EPRef<HAL> HALFactory::InternalMakeVulkan(UNUSED EPRef<SandboxWindowProcess> pSBWindowProcess) {
		return nullptr;
	}
#endif

EPRef<HAL> HALFactory::InternalMake(HAL::type halType, UNUSED EPRef<SandboxWindowProcess> pSBWindowProcess) {
	RESULT r = R::OK;

	switch (halType) {
		#ifdef EPOCH_HAL_VULKAN
			case HAL::type::vulkan: {
				return HALFactory::InternalMakeVulkan(pSBWindowProcess);
			} break;
		#endif

		default: {
			CRM(R::NOT_IMPLEMENTED,
				"HAL %s not yet supported",
				HAL::GetTypeName(halType));
		} break;
	}

Error:
	return nullptr;
}