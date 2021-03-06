#ifndef VULKAN_COMMAND_BUFFER_H_
#define VULKAN_COMMAND_BUFFER_H_

#include "core/ehm/ehm.h"

// epoch Vulkan Framebuffer
// epoch/src/hal/vulkan/VKFramebuffer.h

#include "hal/command_buffer.h"

#include <vulkan/vulkan.h>

#include "core/types/EPVector.h"
#include "core/types/EPRef.h"
#include "core/types/EPFactoryMethod.h"
#include "core/types/EPString.h"

#include "VulkanUtilities.h"

class VKPipeline;
class VKSwapchain;
class VKCommandPool;
class VKBuffer;
class VKVertexBuffer;
class VKModel;
class VKDescriptorSet;

class VKCommandBuffers :
	public command_buffer,
	//public EPFactoryMethod<VKCommandBuffers, const EPRef<VKCommandPool>&>,
	public EPFactoryMethod<VKCommandBuffers, 
		const EPRef<VKCommandPool>&,
		const EPRef<VKVertexBuffer>&,
		const EPRef<VKDescriptorSet>&,
		const EPRef<VKPipeline>&,
		const EPRef<VKSwapchain>&
	>
{
public:
	enum CommandBufferState {
		UNINITIALIZED,
		INITIALIZED,
		RECORDING,
		READY,
		SUBMITTED
	};

public:
	VKCommandBuffers(const EPRef<VKCommandPool>&);

private:
	VKCommandBuffers(
		const EPRef<VKCommandPool>&, 
		const EPRef<VKVertexBuffer>&, 
		const EPRef<VKDescriptorSet>&,
		const EPRef<VKPipeline>&,
		const EPRef<VKSwapchain>&);

	VKCommandBuffers(
		const EPRef<VKCommandPool>&,
		const EPRef<VKModel>&,
		const EPRef<VKDescriptorSet>&,
		const EPRef<VKPipeline>&,
		const EPRef<VKSwapchain>&);


	virtual RESULT Initialize() override;
	virtual RESULT Kill() override;

public:
	// TODO: This allows an external object to do this
	// might want to convert into the only way
	RESULT ProtoInitialize(uint32_t numBuffers = 1);
	RESULT Begin(uint32_t index = 0);
	RESULT End(uint32_t index = 0);
	
	// Commands
	RESULT CopyBuffer(uint32_t index, VkBuffer vkSrcBuffer, VkBuffer vkDstBuffer, VkDeviceSize vkSize);
	RESULT PipelineBarrier(uint32_t index, VkImageMemoryBarrier &vkImageMemoryBarrier);
	RESULT CopyBufferToImage(uint32_t index, VkBuffer vkSrcBuffer, VkImage vkDstImage, const VkBufferImageCopy &vkBufferImageCopy);

	RESULT Submit(VkQueue vkQueue);
	RESULT Submit(VkQueue vkQueue, uint32_t index);

public:
	virtual ~VKCommandBuffers() override;

	static EPRef<VKCommandBuffers> InternalMake(
		const EPRef<VKCommandPool>&, 
		const EPRef<VKVertexBuffer>&, 
		const EPRef<VKDescriptorSet>&,
		const EPRef<VKPipeline>&,
		const EPRef<VKSwapchain>&
	);

	static EPRef<VKCommandBuffers> MakeFromModel(
		const EPRef<VKCommandPool>&,
		const EPRef<VKModel>&,
		const EPRef<VKDescriptorSet>&,
		const EPRef<VKPipeline>&,
		const EPRef<VKSwapchain>&
	);

	//static EPRef<VKCommandBuffers> InternalMake(const EPRef<VKCommandPool>&);

	RESULT RecordCommandBuffers();
	const VkCommandBuffer* GetCommandBufferHandle(uint32_t index) const {
		return &(m_vkCommandBuffers[index]);
	}

private:
	EPRef<VKCommandPool> m_pVKCommandPool = nullptr;
	EPRef<VKPipeline> m_pVKPipeline = nullptr;
	EPRef<VKSwapchain> m_pVKSwapchain = nullptr;

	VkCommandBufferAllocateInfo m_vkCommandBufferAllocateInfo = {};
	
	EPVector<VkCommandBuffer> m_vkCommandBuffers;
	EPVector<CommandBufferState> m_vkCommandBufferStates;

	// TODO: temp
	EPRef<VKDescriptorSet> m_pVKDescriptorSet = nullptr;
	EPRef<VKVertexBuffer> m_pVKVertexBuffer = nullptr;
	EPRef<VKModel> m_pVKModel = nullptr;

	VKQueueFamilies m_vkQueueFamilies = {};

};

#endif // ! VULKAN_COMMAND_BUFFER_H_