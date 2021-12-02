#define EVK_INTERNAL_STATE
#include "evk.h"

#include <fstream>

#define TIMESTAMP(stage, name) state->cmd.writeTimestamp(stage, state->queryPool, state->queryId(name));

namespace evk {
	uint32_t STORAGE_COUNT = 8096;
	uint32_t IMAGE_COUNT = 8096;


	/////////////
	// Helpers //
	/////////////
	std::vector<uint8_t> ShaderFromFile(const std::string& path) {
		std::vector<uint8_t> data;
		std::ifstream fs(path, std::ios::binary | std::ios::ate);
		assert(fs.is_open()); // wrong path or file doesn't exist
		size_t size = fs.tellg();
		fs.seekg(std::ios::beg);
		data.resize(size);
		fs.read((char*)data.data(), size);
		return data;
	}

	const VmaMemoryUsage MEMORY_TYPE_VMA[] = {
		VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY,
		VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY,
		VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU,
		VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_TO_CPU,
	};
	const vk::Format FORMAT_VK[] = {
		vk::Format::eR8Uint,
		vk::Format::eB8G8R8A8Unorm,
		vk::Format::eB8G8R8A8Snorm,
		vk::Format::eR8G8B8A8Unorm,
		vk::Format::eR8G8B8A8Snorm,
		vk::Format::eR16G16Sfloat,
		vk::Format::eR16G16B16A16Sfloat,
		vk::Format::eR16G16B16A16Unorm,
		vk::Format::eR16G16B16A16Snorm,
		vk::Format::eR32G32B32A32Sfloat,
		vk::Format::eD24UnormS8Uint
	};
	const vk::Filter FILTER_VK[] = {
		vk::Filter::eNearest,
		vk::Filter::eLinear
	};
	const vk::PipelineStageFlagBits STAGE_VK[] = {
		vk::PipelineStageFlagBits::eTopOfPipe,             //TopOfPipe,
		vk::PipelineStageFlagBits::eHost,                  //Host,
		vk::PipelineStageFlagBits::eTransfer,              //Transfer,
		vk::PipelineStageFlagBits::eComputeShader,         //Compute,
		vk::PipelineStageFlagBits::eDrawIndirect,          //DrawIndirect,
		vk::PipelineStageFlagBits::eVertexInput,           //VertexInput,
		vk::PipelineStageFlagBits::eVertexShader,          //VertexShader,
		vk::PipelineStageFlagBits::eEarlyFragmentTests,    //EarlyFragmentTest,
		vk::PipelineStageFlagBits::eEarlyFragmentTests,    //FragmentShader,
		vk::PipelineStageFlagBits::eLateFragmentTests,     //LateFragmentTest,
		vk::PipelineStageFlagBits::eColorAttachmentOutput, //ColorAttachmentOutput,
		vk::PipelineStageFlagBits::eBottomOfPipe,          //BottomOfPipe,
		vk::PipelineStageFlagBits::eAllGraphics,           //AllGraphics,
		vk::PipelineStageFlagBits::eAllCommands,           //AllCommands
	};
	const vk::ImageLayout IMAGE_LAYOUT_VK[] = {
		vk::ImageLayout::eUndefined,             // Undefined,
		vk::ImageLayout::eGeneral,               // General,
		vk::ImageLayout::eTransferSrcOptimal,    // TransferSrc,
		vk::ImageLayout::eTransferDstOptimal,    // TransferDst,
		vk::ImageLayout::eShaderReadOnlyOptimal, // ShaderReadOptimal,
	};
	const int FORMAT_SIZE[] = {
		1, //vk::Format::eR8Uint,
		4, //vk::Format::eB8G8R8A8Unorm,
		4, //vk::Format::eB8G8R8A8Snorm,
		4, //vk::Format::eR8G8B8A8Unorm,
		4, //vk::Format::eR8G8B8A8Snorm,
		4, //vk::Format::eR16G16Sfloat,
		8, //vk::Format::eR16G16B16A16Sfloat,
		4  //vk::Format::eD24UnormS8Uint
	};

	BufferUsage operator|(BufferUsage a, BufferUsage b) {
		return (BufferUsage)((uint32_t)a | (uint32_t)b);
	}
	ImageUsage operator|(ImageUsage a, ImageUsage b) {
		return (ImageUsage)((uint32_t)a | (uint32_t)b);
	}

	bool DoesFormatHaveDepth(Format format) {
		switch (format) {
		case evk::Format::D24UnormS8Uint: return true;
		default:                          return false;
		}
	}
	vk::ShaderModule CreateShader(const std::vector<uint8_t> spirv) {
		return GetState().device.createShaderModule(vk::ShaderModuleCreateInfo()
			.setPCode((const uint32_t*)spirv.data())
			.setCodeSize(spirv.size())
		);
	}

	bool RecreateSwapchain();

	//////////
	// Pass //
	//////////
	Pass Pass::Create(const std::vector<Attachment>& attachments) {
		auto& S = GetState();
		bool hasDepth = false;

		std::shared_ptr<Internal_Pass> state = std::make_shared<Internal_Pass>();

		std::vector<vk::AttachmentDescription> attachmentsDescs;
		std::vector<vk::AttachmentReference> attachmentsColors;
		vk::AttachmentReference attachmentDepth;

		int attachmentIndex = 0;
		for (auto& attach : attachments) {
			attachmentsDescs.push_back(vk::AttachmentDescription()
				.setFormat(FORMAT_VK[(int)attach.format])
				.setSamples(vk::SampleCountFlagBits::e1)//TODO: 
				.setLoadOp(vk::AttachmentLoadOp::eClear)//TODO: 
				.setStoreOp(vk::AttachmentStoreOp::eStore)//TODO:
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)//TODO: 
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)//TODO: 
				.setInitialLayout(vk::ImageLayout::eUndefined)//TODO: 
				.setFinalLayout(attach.present ? vk::ImageLayout::ePresentSrcKHR : vk::ImageLayout::eShaderReadOnlyOptimal)//TODO: 
			);

			if (DoesFormatHaveDepth(attach.format)) {
				assert(hasDepth == false); //Can only have one depth attachment
				hasDepth = true;
				attachmentDepth
					.setAttachment(attachmentIndex)
					.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				state->clearValues.push_back(vk::ClearDepthStencilValue().setDepth(attach.clearDepth).setStencil(attach.clearStencil));
			}
			else {
				attachmentsColors.push_back(vk::AttachmentReference()
					.setAttachment(attachmentIndex)
					.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
				);
				state->clearValues.push_back(vk::ClearColorValue().setFloat32({ attach.clearColor[0], attach.clearColor[1], attach.clearColor[2], attach.clearColor[3] }));
			}

			state->formats.push_back(attach.format);
			attachmentIndex++;
		}

		//Single SubPass
		std::vector<vk::SubpassDescription> subPasses = {
			vk::SubpassDescription()
				.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
				.setColorAttachments(attachmentsColors)
				.setPDepthStencilAttachment(hasDepth ? &attachmentDepth : nullptr)
		};

		state->pass = S.device.createRenderPass(vk::RenderPassCreateInfo()
			.setAttachments(attachmentsDescs)
			.setSubpasses(subPasses)

			.setDependencies(vk::SubpassDependency()
				.setSrcSubpass(VK_SUBPASS_EXTERNAL)
				.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
				.setSrcAccessMask((vk::AccessFlags)0)
				.setDstSubpass(0)
				.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
				.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
			)

		);

		return Pass{ state };
	}

	////////////
	// Buffer //
	////////////
	Buffer Buffer::Create(uint64_t size, BufferUsage usage, MemoryType memoryType) {
		auto& S = GetState();
		std::shared_ptr<Internal_Buffer> state = std::make_shared<Internal_Buffer>();

		//Immutable state
		state->size = size;

		vk::BufferUsageFlags usageBits{};
		if ((int)(usage | BufferUsage::TransferSrc)) usageBits |= vk::BufferUsageFlagBits::eTransferSrc;
		if ((int)(usage | BufferUsage::TransferDst)) usageBits |= vk::BufferUsageFlagBits::eTransferDst;
		if ((int)(usage | BufferUsage::Vertex))      usageBits |= vk::BufferUsageFlagBits::eVertexBuffer;
		if ((int)(usage | BufferUsage::Index))       usageBits |= vk::BufferUsageFlagBits::eIndexBuffer;
		if ((int)(usage | BufferUsage::Indirect))    usageBits |= vk::BufferUsageFlagBits::eIndirectBuffer;
		if ((int)(usage | BufferUsage::Storage))     usageBits |= vk::BufferUsageFlagBits::eStorageBuffer;

		VkBufferCreateInfo createInfo = vk::BufferCreateInfo()
			.setSize(size)
			.setQueueFamilyIndexCount(1)
			.setPQueueFamilyIndices(&S.queueFamily)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setUsage(usageBits);

		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = MEMORY_TYPE_VMA[(int)memoryType];
		allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		VmaAllocationInfo allocInfo;

		vmaCreateBuffer(S.allocator, &createInfo, &allocCreateInfo, (VkBuffer*)&state->buffer, &state->allocation, &allocInfo);
		state->mappedData = allocInfo.pMappedData;

		//Alloc descriptor index
		{
			int newRID = -1;
			if (S.freeIndexBuffer.size() > 0) {
				newRID = S.freeIndexBuffer.back();
				S.freeIndexBuffer.pop_back();
			}
			else {
				newRID = S.descriptorIndexBuffer++;
				assert(S.descriptorIndexBuffer < STORAGE_COUNT);
			}

			S.device.updateDescriptorSets({ vk::WriteDescriptorSet()
				.setBufferInfo(vk::DescriptorBufferInfo().setBuffer(state->buffer).setOffset(0).setRange(state->size))
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eStorageBuffer)
				.setDstSet(S.descriptorSet)
				.setDstBinding(0)
				.setDstArrayElement(newRID)
				}, {});
			state->resourceid = newRID;
		}

		return Buffer{ state };
	}
	void* Buffer::getData() {
		assert(state->mappedData != nullptr);
		return state->mappedData;
	}
	void Buffer::update(void* src, uint64_t size, uint64_t offset) {
		std::memcpy((void*)((size_t)getData() + offset), src, size);
	}
	RID Buffer::getRID() { return state->resourceid; }

	///////////
	// Image //
	///////////
	void InitializeImageView(std::shared_ptr<Internal_Image>& state) {
		vk::ImageAspectFlags aspects = {};
		if (DoesFormatHaveDepth(state->format)) {
			aspects |= vk::ImageAspectFlagBits::eDepth;
		}
		else {
			aspects |= vk::ImageAspectFlagBits::eColor;
		}

		//ImageView
		state->view = GetState().device.createImageView(vk::ImageViewCreateInfo()
			.setImage(state->image)
			.setFormat(FORMAT_VK[(int)state->format])
			.setComponents(vk::ComponentMapping()
				.setR(vk::ComponentSwizzle::eR)
				.setG(vk::ComponentSwizzle::eG)
				.setB(vk::ComponentSwizzle::eB)
				.setA(vk::ComponentSwizzle::eA)
			)
			.setViewType(state->isCube ? vk::ImageViewType::eCube : (state->extent.depth == 1 ? vk::ImageViewType::e2D : vk::ImageViewType::e3D))
			.setSubresourceRange(vk::ImageSubresourceRange()
				.setAspectMask(aspects)
				.setBaseMipLevel(0)
				.setLevelCount(state->mipCount)
				.setBaseArrayLayer(0)
				.setLayerCount(state->layerCount)
			)
		);

		//Sampler
		state->sampler = GetState().device.createSampler(vk::SamplerCreateInfo()
			.setMinFilter(FILTER_VK[(int)state->filter])
			.setMagFilter(FILTER_VK[(int)state->filter])
			.setAddressModeU(vk::SamplerAddressMode::eClampToBorder)
			.setAddressModeV(vk::SamplerAddressMode::eClampToBorder)
			.setAddressModeW(vk::SamplerAddressMode::eClampToBorder)
			.setAnisotropyEnable(false)
			.setMaxAnisotropy(0.0f)
			.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
			.setUnnormalizedCoordinates(false)
			.setCompareEnable(false)
			.setCompareOp(vk::CompareOp::eAlways)
			.setMipmapMode(vk::SamplerMipmapMode::eNearest)
			.setMinLod(0)
			.setMaxLod(state->mipCount)
		);
	}
	Image Image::Create(const Image::Info& info) {
		auto& S = GetState();
		std::shared_ptr<Internal_Image> state = std::make_shared<Internal_Image>();

		//Immutable state
		state->format = info.format;
		state->extent = info.extent;
		state->filter = info.filter;
		state->mipCount = info.mipCount;
		state->layerCount = info.layerCount;
		state->isCube = info.isCube;

		//Usage flags
		ImageUsage usage = info.usage;
		vk::ImageUsageFlags usageBits{};
		if ((int)(usage | ImageUsage::TransferSrc)) usageBits |= vk::ImageUsageFlagBits::eTransferSrc;
		if ((int)(usage | ImageUsage::TransferDst)) usageBits |= vk::ImageUsageFlagBits::eTransferDst;
		if ((int)(usage | ImageUsage::Sampled))     usageBits |= vk::ImageUsageFlagBits::eSampled;
		if ((int)(usage | ImageUsage::Attachment))  usageBits |= DoesFormatHaveDepth(state->format) ? vk::ImageUsageFlagBits::eDepthStencilAttachment : vk::ImageUsageFlagBits::eColorAttachment;

		//Create Image
		VkImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
			.setImageType(state->extent.depth == 1 ? vk::ImageType::e2D : vk::ImageType::e3D)
			.setFormat(FORMAT_VK[(int)state->format])
			.setUsage(usageBits)
			.setExtent(vk::Extent3D().setWidth(state->extent.width).setHeight(state->extent.height).setDepth(state->extent.depth))
			.setMipLevels(state->mipCount)
			.setArrayLayers(state->layerCount)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setTiling(vk::ImageTiling::eOptimal)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFlags(state->isCube ? vk::ImageCreateFlagBits::eCubeCompatible : (vk::ImageCreateFlagBits)0);

		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		VmaAllocationInfo allocInfo;

		if (vmaCreateImage(S.allocator, &imageCreateInfo, &allocCreateInfo, (VkImage*)&state->image, &state->allocation, &allocInfo) != VK_SUCCESS) {
			assert(false);
		}

		InitializeImageView(state);

		//Alloc descriptor index
		{
			int newRID = -1;
			if (S.freeIndexImage.size() > 0) {
				newRID = S.freeIndexImage.back();
				S.freeIndexImage.pop_back();
			}
			else {
				newRID = S.descriptorIndexImage++;
				assert(S.descriptorIndexImage < STORAGE_COUNT);
			}

			S.device.updateDescriptorSets({ vk::WriteDescriptorSet()
				.setImageInfo(vk::DescriptorImageInfo().setImageView(state->view).setSampler(state->sampler).setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal))
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDstSet(S.descriptorSet)
				.setDstBinding(1)
				.setDstArrayElement(newRID)
				}, {});
			state->resourceid = newRID;
		}

		return Image{ state };
	}
	Format Image::getFormat() { return state->format; }
	Extent Image::getExtent() { return state->extent; }
	int Image::getMipCount() { return state->mipCount; }
	RID Image::getRID() { return state->resourceid; }

	/////////////////
	// Framebuffer //
	/////////////////
	Framebuffer Framebuffer::Create(Pass& pass, uint32_t width, uint32_t height) {
		auto& S = GetState();
		std::shared_ptr<Internal_Framebuffer> state = std::make_shared<Internal_Framebuffer>();


		std::vector<vk::ImageView> attachments;
		for (auto format : pass.state->formats) {
			state->attachments.push_back(Image::Create(Image::Info(format, { width, height }).setUsage(ImageUsage::Attachment)));
			attachments.push_back(state->attachments.back().state->view);
		}

		state->pass = pass;
		state->extent = Extent(width, height);
		state->framebuffer = S.device.createFramebuffer(vk::FramebufferCreateInfo()
			.setRenderPass(pass.state->pass)
			.setWidth(width)
			.setHeight(height)
			.setLayers(1)
			.setAttachments(attachments)
		);

		return Framebuffer{ state };
	}
	Extent Framebuffer::getExtent() { return state->extent; }
	Image& Framebuffer::getAttachment(int index) {
		return state->attachments[index];
	}

	////////////
	// Shader //
	////////////
	Shader Shader::Vertex(const std::vector<uint8_t>& spirv) {
		auto& S = GetState();
		std::shared_ptr<Internal_Shader> state = std::make_shared<Internal_Shader>();

		state->shaderStage = vk::ShaderStageFlagBits::eVertex;
		state->shaderModule = S.device.createShaderModule(vk::ShaderModuleCreateInfo()
			.setPCode((const uint32_t*)spirv.data())
			.setCodeSize(spirv.size())
		);

		return Shader{ state };
	}
	Shader Shader::Fragment(const std::vector<uint8_t>& spirv) {
		auto& S = GetState();
		std::shared_ptr<Internal_Shader> state = std::make_shared<Internal_Shader>();

		state->shaderStage = vk::ShaderStageFlagBits::eFragment;
		state->shaderModule = S.device.createShaderModule(vk::ShaderModuleCreateInfo()
			.setPCode((const uint32_t*)spirv.data())
			.setCodeSize(spirv.size())
		);

		return Shader{ state };
	}

	//////////////////////
	// GraphicsPipeline //
	//////////////////////
	GraphicsPipeline GraphicsPipeline::Create(const Info& info) {
		auto& S = GetState();
		std::shared_ptr<Internal_GraphicsPipeline> state = std::make_shared<Internal_GraphicsPipeline>();

		auto& pass = info.pass.state ? info.pass : S.swapchainPass;

		assert(info.vertexSpirv.size() != 0);
		assert(info.fragmentSpirv.size() != 0);

		auto vertexModule = CreateShader(info.vertexSpirv);
		auto fragmentModule = CreateShader(info.fragmentSpirv);

		std::vector<vk::VertexInputBindingDescription> bindings;
		std::vector<vk::VertexInputAttributeDescription> attributes;
		std::vector<vk::Viewport> viewports = { vk::Viewport().setX(0).setY(0).setWidth(640).setHeight(480) };
		std::vector<vk::Rect2D> scissors = { vk::Rect2D().setOffset(vk::Offset2D{ 0, 0 }).setExtent(vk::Extent2D{ std::numeric_limits<int>::max(), std::numeric_limits<int>::max() }) };
		std::vector<vk::PipelineColorBlendAttachmentState> attachments;
		std::vector<vk::DynamicState> dynamicState = { vk::DynamicState::eViewport };

		std::vector<vk::PipelineShaderStageCreateInfo> stages = {
			vk::PipelineShaderStageCreateInfo().setModule(vertexModule).setStage(vk::ShaderStageFlagBits::eVertex).setPName("main"),
			vk::PipelineShaderStageCreateInfo().setModule(fragmentModule).setStage(vk::ShaderStageFlagBits::eFragment).setPName("main"),
		};
		vk::PipelineVertexInputStateCreateInfo vertexInfo; {
			vertexInfo
				.setVertexBindingDescriptions(bindings)
				.setVertexAttributeDescriptions(attributes);
		}
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo; {
			inputAssemblyInfo.setTopology(vk::PrimitiveTopology::eTriangleList);
		}
		vk::PipelineViewportStateCreateInfo viewportInfo; {
			viewportInfo
				.setViewports(viewports)
				.setScissors(scissors);
		}
		vk::PipelineRasterizationStateCreateInfo rasterizationInfo; {
			rasterizationInfo
				.setPolygonMode(vk::PolygonMode::eFill)
				.setCullMode(vk::CullModeFlagBits::eBack)
				.setFrontFace(vk::FrontFace::eCounterClockwise)
				.setLineWidth(1.0f);
		}
		vk::PipelineMultisampleStateCreateInfo multisampleInfo; {
			multisampleInfo.setRasterizationSamples(vk::SampleCountFlagBits::e1);
		}
		vk::PipelineDepthStencilStateCreateInfo depthStencilInfo; {
			depthStencilInfo
				.setDepthTestEnable(info.depthTest)
				.setDepthWriteEnable(info.depthWrite)
				.setDepthCompareOp(vk::CompareOp::eLess)
				.setDepthBoundsTestEnable(false)
				.setMinDepthBounds(0.0f)
				.setMaxDepthBounds(1.0f);
		}
		vk::PipelineColorBlendStateCreateInfo colorBlendInfo; {

			//A single blendAttach for all the attachments
			vk::PipelineColorBlendAttachmentState blendAttach{};
			blendAttach
				.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
			switch (info.blend) {
			case Blend::Disabled:
				blendAttach.setBlendEnable(false);
				break;
			case Blend::Alpha:
				blendAttach.setBlendEnable(true)
					.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
					.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
					.setColorBlendOp(vk::BlendOp::eAdd)
					.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
					.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
					.setAlphaBlendOp(vk::BlendOp::eAdd);
				break;
			case Blend::Additive:
				blendAttach.setBlendEnable(true)
					.setSrcColorBlendFactor(vk::BlendFactor::eOne)
					.setDstColorBlendFactor(vk::BlendFactor::eOne)
					.setColorBlendOp(vk::BlendOp::eAdd)
					.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
					.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
					.setAlphaBlendOp(vk::BlendOp::eAdd);
				break;
			default:
				break;
			}

			for (Format format : pass.state->formats) {
				if (!DoesFormatHaveDepth(format)) {
					attachments.push_back(blendAttach);
				}
			}
			colorBlendInfo.setAttachments(attachments);
		}
		vk::PipelineDynamicStateCreateInfo dynamicStateInfo; {
			dynamicStateInfo
				.setDynamicStates(dynamicState);
		}

		vk::GraphicsPipelineCreateInfo createInfo = vk::GraphicsPipelineCreateInfo()
			.setStages(stages)
			.setPVertexInputState(&vertexInfo)
			.setPInputAssemblyState(&inputAssemblyInfo)
			.setPViewportState(&viewportInfo)
			.setPRasterizationState(&rasterizationInfo)
			.setPMultisampleState(&multisampleInfo)
			.setPDepthStencilState(&depthStencilInfo)
			.setPColorBlendState(&colorBlendInfo)
			.setPDynamicState(&dynamicStateInfo)
			.setLayout(S.pipelineLayout)
			.setRenderPass(pass.state->pass);

		state->pipeline = S.device.createGraphicsPipeline(nullptr, createInfo).value;

		S.device.destroyShaderModule(vertexModule);
		S.device.destroyShaderModule(fragmentModule);

		return GraphicsPipeline{ state };
	}
	
	///////////////
	// CmdBuffer //
	///////////////
	CmdBuffer CmdBuffer::Create() {
		State S = GetState();
		std::shared_ptr<Internal_CmdBuffer> state = std::make_shared<Internal_CmdBuffer>();

		state->pool = S.device.createCommandPool(vk::CommandPoolCreateInfo()
			.setQueueFamilyIndex(S.queueFamily)
		);
		state->cmd = S.device.allocateCommandBuffers(vk::CommandBufferAllocateInfo()
			.setCommandBufferCount(1)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandPool(state->pool)
		)[0];
		state->fence = S.device.createFence(vk::FenceCreateInfo());
		state->semaphore = S.device.createSemaphore(vk::SemaphoreCreateInfo());

		state->queryPool = S.device.createQueryPool(vk::QueryPoolCreateInfo().setQueryType(vk::QueryType::eTimestamp).setQueryCount(64));

		return CmdBuffer{ state };
	}
	CmdBuffer& CmdBuffer::submit() {
		State& S = GetState();
		S.device.resetFences(state->fence);

		vk::PipelineStageFlags dstStage = vk::PipelineStageFlagBits::eAllCommands;
		S.queue.submit(vk::SubmitInfo()
			.setWaitSemaphoreCount(state->doingPresent ? 1 : 0)
			.setPWaitSemaphores(state->doingPresent ? &S.swapchainSemaphores[S.swapchainSemaphoreIndex] : nullptr)
			.setSignalSemaphoreCount(state->doingPresent ? 1 : 0)
			.setPSignalSemaphores(state->doingPresent ? &state->semaphore : nullptr)
			.setCommandBufferCount(1)
			.setPCommandBuffers(&state->cmd)
			.setPWaitDstStageMask(&dstStage)
			, state->fence);

		if (state->doingPresent) {
			try {
				S.queue.presentKHR(vk::PresentInfoKHR()
					.setWaitSemaphoreCount(1)
					.setPWaitSemaphores(&state->semaphore)
					.setSwapchainCount(1)
					.setPSwapchains(&S.swapchain)
					.setPImageIndices(&S.swapchainIndex)
				);
			}
			catch (vk::OutOfDateKHRError e) {
				RecreateSwapchain();
			}

			S.swapchainSemaphoreIndex++;
			S.swapchainSemaphoreIndex = S.swapchainIndex % S.swapchainFramebuffers.size();
		}

		return *this;
	}
	CmdBuffer& CmdBuffer::wait() {
		GetState().device.waitForFences(1, &state->fence, true, 99999999999999);
		state->queries = GetState().device.getQueryPoolResults<uint64_t>(state->queryPool, (uint32_t)0, (uint32_t)64, (size_t)64 * sizeof(uint64_t), (vk::DeviceSize)8, vk::QueryResultFlagBits::e64).value;

		//Build timestamps
		state->timestampEntries.clear();
		uint64_t start = state->queries[0];
		for (int i = 0; i < state->timestampNames.size(); i++) {
			TimestampEntry e;
			e.start = (state->queries[i * 2] - start) * GetState().timestampPeriod;
			e.end = (state->queries[i * 2 + 1] - start) * GetState().timestampPeriod;
			e.name = state->timestampNames[i];
			state->timestampEntries.push_back(e);
		}
		return *this;
	}
	CmdBuffer& CmdBuffer::barrier(Image& image, ImageLayout oldLayout, ImageLayout newLayout, uint32_t mip, uint32_t mipCount, uint32_t layer, uint32_t layerCount) {

		vk::ImageAspectFlags aspects = {};
		if (DoesFormatHaveDepth(image.state->format)) {
			aspects |= vk::ImageAspectFlagBits::eDepth;
		}
		else {
			aspects |= vk::ImageAspectFlagBits::eColor;
		}

		auto imageBarrier = vk::ImageMemoryBarrier()
			.setImage(image.state->image)
			.setOldLayout(IMAGE_LAYOUT_VK[(int)oldLayout])
			.setNewLayout(IMAGE_LAYOUT_VK[(int)newLayout])
			.setSubresourceRange(vk::ImageSubresourceRange()
				.setAspectMask(aspects)
				.setLayerCount(layerCount)
				.setBaseArrayLayer(layer)
				.setLevelCount(mipCount)
				.setBaseMipLevel(mip)
			);

		vk::PipelineStageFlags srcStage;
		vk::PipelineStageFlags dstStage;

		{
			if (oldLayout == ImageLayout::Undefined) {
				srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
				imageBarrier.setSrcAccessMask((vk::AccessFlagBits)0);
			}
			else if (oldLayout == ImageLayout::TransferSrc) {
				srcStage = vk::PipelineStageFlagBits::eTransfer;
				imageBarrier.setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
			}
			else if (oldLayout == ImageLayout::TransferDst) {
				srcStage = vk::PipelineStageFlagBits::eTransfer;
				imageBarrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
			}
			else if (oldLayout == ImageLayout::ShaderReadOptimal) {
				srcStage = vk::PipelineStageFlagBits::eFragmentShader;
				imageBarrier.setSrcAccessMask(vk::AccessFlagBits::eShaderRead);
			}
			else {
				throw std::invalid_argument("Unsurported layout transition!");
			}

			if (newLayout == ImageLayout::Undefined) {
				dstStage = vk::PipelineStageFlagBits::eTopOfPipe;
				imageBarrier.setDstAccessMask((vk::AccessFlagBits)0);
			}
			else if (newLayout == ImageLayout::TransferSrc) {
				dstStage = vk::PipelineStageFlagBits::eTransfer;
				imageBarrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);
			}
			else if (newLayout == ImageLayout::TransferDst) {
				dstStage = vk::PipelineStageFlagBits::eTransfer;
				imageBarrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
			}
			else if (newLayout == ImageLayout::ShaderReadOptimal) {
				dstStage = vk::PipelineStageFlagBits::eFragmentShader;
				imageBarrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
			}
			else {
				throw std::invalid_argument("Unsurported layout transition!");
			}
		}
		state->cmd.pipelineBarrier(srcStage, dstStage, {}, {}, {}, imageBarrier);
		return *this;
	}
	CmdBuffer& CmdBuffer::blit(Image& src, Image& dst, ImageRegion srcRegion, ImageRegion dstRegion, Filter filter) {
		if (srcRegion.width == 0)srcRegion.width   = std::max(src.getExtent().width >> srcRegion.mip , 1u);
		if (srcRegion.height == 0)srcRegion.height = std::max(src.getExtent().height >> srcRegion.mip, 1u);
		if (srcRegion.depth == 0)srcRegion.depth   = std::max(src.getExtent().depth >> srcRegion.mip , 1u);
		if (dstRegion.width == 0)dstRegion.width   = std::max(dst.getExtent().width >> dstRegion.mip , 1u);
		if (dstRegion.height == 0)dstRegion.height = std::max(dst.getExtent().height >> dstRegion.mip, 1u);
		if (dstRegion.depth == 0)dstRegion.depth   = std::max(dst.getExtent().depth >> dstRegion.mip , 1u);
		state->cmd.blitImage(src.state->image, vk::ImageLayout::eTransferSrcOptimal, dst.state->image, vk::ImageLayout::eTransferDstOptimal,
			{ vk::ImageBlit()
			.setSrcOffsets({ vk::Offset3D(srcRegion.x, srcRegion.y, srcRegion.z), vk::Offset3D(srcRegion.x + srcRegion.width, srcRegion.y + srcRegion.height, srcRegion.z + srcRegion.depth) })
			.setSrcSubresource(vk::ImageSubresourceLayers()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setMipLevel(srcRegion.mip)
				.setBaseArrayLayer(srcRegion.layer)
				.setLayerCount(1)
			)
			.setDstOffsets({ vk::Offset3D(dstRegion.x, dstRegion.y, dstRegion.z), vk::Offset3D(dstRegion.x + dstRegion.width, dstRegion.y + dstRegion.height, dstRegion.z + dstRegion.depth) })
			.setDstSubresource(vk::ImageSubresourceLayers()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setMipLevel(dstRegion.mip)
				.setBaseArrayLayer(dstRegion.layer)
				.setLayerCount(1)
			)
		},
		FILTER_VK[(int)filter]);
		return *this;
	}
	CmdBuffer& CmdBuffer::copy(Image& src, Image& dst, uint32_t srcMip, uint32_t srcLayer, uint32_t dstMip, uint32_t dstLayer, uint32_t layerCount) {
		state->cmd.copyImage(src.state->image, vk::ImageLayout::eTransferSrcOptimal, dst.state->image, vk::ImageLayout::eTransferDstOptimal,
			vk::ImageCopy()
			.setExtent(vk::Extent3D{ src.state->extent.width, src.state->extent.height, src.state->extent.depth })
			.setSrcOffset({})
			.setSrcSubresource(vk::ImageSubresourceLayers()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setMipLevel(srcMip)
				.setBaseArrayLayer(srcLayer)
				.setLayerCount(layerCount)
			)
			.setDstOffset({})
			.setDstSubresource(vk::ImageSubresourceLayers()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setMipLevel(dstMip)
				.setBaseArrayLayer(dstLayer)
				.setLayerCount(layerCount))
		);
		return *this;
	}
	CmdBuffer& CmdBuffer::copy(Buffer& buffer, Image& image, uint32_t mip, uint32_t layer) {

		state->cmd.copyBufferToImage(buffer.state->buffer, image.state->image, vk::ImageLayout::eTransferDstOptimal,
			vk::BufferImageCopy()
			.setBufferOffset(0)
			.setBufferRowLength(0)
			.setBufferImageHeight(0)
			.setImageSubresource(vk::ImageSubresourceLayers()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setMipLevel(mip)
				.setBaseArrayLayer(layer)
				.setLayerCount(1)//TODO: if expose its value maybe will improve hdr files loading
			)
			.setImageOffset(vk::Offset3D{ 0, 0, 0 })
			.setImageExtent(vk::Extent3D(image.getExtent().width >> mip, image.getExtent().height >> mip, image.getExtent().depth >> mip)) // extent for each mip
		);

		return *this;
	}
	CmdBuffer& CmdBuffer::copy(Buffer& buffer, Image& image, const std::vector<ImageRegion>& regions) {
		auto extent = image.getExtent();

		for (auto& region : regions) {
			state->cmd.copyBufferToImage(buffer.state->buffer, image.state->image, vk::ImageLayout::eTransferDstOptimal,
				vk::BufferImageCopy()
				.setBufferOffset(region.x + region.y * extent.width + region.z * extent.width * extent.height)
				.setBufferRowLength(extent.width)
				.setBufferImageHeight(extent.height)
				.setImageSubresource(vk::ImageSubresourceLayers()
					.setAspectMask(vk::ImageAspectFlagBits::eColor)
					.setMipLevel(region.mip)
					.setBaseArrayLayer(0)
					.setLayerCount(1)
				)
				.setImageOffset(vk::Offset3D{ region.x, region.y, region.z })
				.setImageExtent(vk::Extent3D(region.width >> region.mip, region.height >> region.mip, region.depth >> region.mip)) // size of each mip
			);
		}

		return *this;
	}
	CmdBuffer& CmdBuffer::begin() {
		auto& S = GetState();
		state->timestampNames.clear();
		state->doingPresent = false;
		S.device.resetCommandPool(state->pool, vk::CommandPoolResetFlagBits::eReleaseResources);
		state->cmd.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
		state->cmd.resetQueryPool(state->queryPool, 0, 64);
		state->cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, S.pipelineLayout, 0, 1, &S.descriptorSet, 0, nullptr);
		return *this;
	}
	CmdBuffer& CmdBuffer::end() {
		state->cmd.end();
		return *this;
	}
	CmdBuffer& CmdBuffer::beginPresent() {
		assert(!state->doingPresent); // already presenting in this CommandBuffer
		state->doingPresent = true;

		auto& S = GetState();
		auto scResult = S.device.acquireNextImageKHR(S.swapchain, 0, S.swapchainSemaphores[S.swapchainSemaphoreIndex]);
		if (scResult.result == vk::Result::eErrorOutOfDateKHR) {
			RecreateSwapchain();
		}
		S.swapchainIndex = scResult.value;

		float width = S.swapchainFramebuffers[0].getExtent().width;
		float height = S.swapchainFramebuffers[0].getExtent().height;
		state->cmd.setViewport(0, vk::Viewport{ 0, height, width, -height });

		Framebuffer& fb = S.swapchainFramebuffers[GetState().swapchainIndex];
		state->cmd.beginRenderPass(vk::RenderPassBeginInfo()
			.setRenderPass(fb.state->pass.state->pass)
			.setFramebuffer(fb.state->framebuffer)
			.setRenderArea(vk::Rect2D().setExtent(vk::Extent2D{ fb.getExtent().width, fb.getExtent().height }))
			.setClearValues(fb.state->pass.state->clearValues),
			vk::SubpassContents::eInline
		);
		return *this;
	}
	CmdBuffer& CmdBuffer::endPresent() {
		state->cmd.endRenderPass();
		return *this;
	}
	CmdBuffer& CmdBuffer::beginPass(Framebuffer& framebuffer) {
		state->cmd.beginRenderPass(vk::RenderPassBeginInfo()
			.setRenderPass(framebuffer.state->pass.state->pass)
			.setFramebuffer(framebuffer.state->framebuffer)
			.setRenderArea(vk::Rect2D().setExtent({ framebuffer.getExtent().width, framebuffer.getExtent().height }))
			.setClearValues(framebuffer.state->pass.state->clearValues),
			vk::SubpassContents::eInline
		);
		state->cmd.setViewport(0, vk::Viewport{ 0, (float)framebuffer.getExtent().height, (float)framebuffer.getExtent().width, -(float)framebuffer.getExtent().height });
		return *this;
	}
	CmdBuffer& CmdBuffer::endPass() {
		state->cmd.endRenderPass();
		return *this;
	}
	CmdBuffer& CmdBuffer::bind(GraphicsPipeline& pipeline) {
		state->cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.state->pipeline);
		return *this;
	}
	CmdBuffer& CmdBuffer::viewport(const Viewport& viewport) {
		//TODO: Implement Viewport
		return *this;
	}
	CmdBuffer& CmdBuffer::constant(void* data, uint32_t size, uint32_t offset) {
		assert(size % 4 == 0);
		assert(offset % 4 == 0);
		state->cmd.pushConstants(GetState().pipelineLayout, vk::ShaderStageFlagBits::eAllGraphics, offset, size, data);
		return *this;
	}
	CmdBuffer& CmdBuffer::draw(int vertexCount, int instanceCount, int firstVertex, int firstInstance) {
		state->cmd.draw(vertexCount, instanceCount, firstVertex, firstInstance);
		return *this;
	}
	CmdBuffer& CmdBuffer::beginTimestamp(const char* name) {
		state->cmd.writeTimestamp(vk::PipelineStageFlagBits::eTopOfPipe, state->queryPool, state->timestampId(name) * 2);
		return *this;
	}
	CmdBuffer& CmdBuffer::endTimestamp(const char* name) {
		state->cmd.writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe, state->queryPool, state->timestampId(name) * 2 + 1);
		return *this;
	}

	const std::vector<TimestampEntry> CmdBuffer::timestamps() { return state->timestampEntries; }

	State& GetState() {
		static State state;
		return state;
	}

	bool InitializeEVK(const InitInfo& info) {
		State& S = GetState();

		//Application and Instance
		{
			//Application Info
			vk::ApplicationInfo appInfo;
			appInfo
				.setApiVersion(VK_API_VERSION_1_2)
				.setPApplicationName(info.applicationName.c_str())
				.setApplicationVersion(info.applicationVersion)//VK_MAKE_VERSION(1, 0, 0))
				.setPEngineName(info.engineName.c_str())
				.setEngineVersion(info.engineVersion);

			//Instance Info
			//TODO: filter vk::enumerateInstanceExtensionProperties
			std::vector<const char*> extensions;
			for (auto& name : info.instanceExtensions) { extensions.push_back(name.c_str()); }
			std::vector<const char*> layers;
			for (auto& name : info.instanceLayers) { layers.push_back(name.c_str()); }

			vk::InstanceCreateInfo instanceCreateInfo;
			instanceCreateInfo
				.setPApplicationInfo(&appInfo)
				.setPEnabledExtensionNames(extensions)
				.setPEnabledLayerNames(layers);

			S.instance = vk::createInstance(instanceCreateInfo);
		}

		//Device and Queues
		{
			S.physicalDevice = S.instance.enumeratePhysicalDevices()[0];

			//PhysicalDevice properties
			auto deviceProperties = S.physicalDevice.getProperties();
			assert(deviceProperties.limits.timestampComputeAndGraphics);
			S.timestampPeriod = deviceProperties.limits.timestampPeriod;

			S.queueFamily = 0;
			for (const auto& queueFamily : S.physicalDevice.getQueueFamilyProperties()) {
				if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) { break; }
				S.queueFamily++;
			}

			vk::DeviceQueueCreateInfo deviceQueueCreateInfo0;
			float priority0 = 1.0f;
			deviceQueueCreateInfo0
				.setQueueFamilyIndex(S.queueFamily)
				.setQueueCount(1)
				.setPQueuePriorities(&priority0);

			auto queuesCreateInfos = { deviceQueueCreateInfo0 };

			//Feature Descriptor Indexing Feature
			vk::PhysicalDeviceDescriptorIndexingFeatures feature_descriptorIndexing = {};
			feature_descriptorIndexing
				.setShaderSampledImageArrayNonUniformIndexing(true)
				.setRuntimeDescriptorArray(true)
				.setDescriptorBindingPartiallyBound(true)
				.setDescriptorBindingStorageBufferUpdateAfterBind(true)
				.setDescriptorBindingSampledImageUpdateAfterBind(true);

			//Create device and get queue
			std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			vk::DeviceCreateInfo deviceCreateInfo;
			deviceCreateInfo
				.setQueueCreateInfos(queuesCreateInfos)
				.setEnabledExtensionCount(0)
				.setPpEnabledExtensionNames(deviceExtensions.data())
				.setEnabledExtensionCount(deviceExtensions.size())
				.setPNext(&feature_descriptorIndexing);
			S.device = S.physicalDevice.createDevice(deviceCreateInfo);
			S.queue = S.device.getQueue(S.queueFamily, 0);
		}

		//Surface
		if (info.surface) {
			S.surface = (VkSurfaceKHR)info.surface;
		}

		//Vma Allocator
		{
			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
			allocatorInfo.instance = S.instance;
			allocatorInfo.physicalDevice = S.physicalDevice;
			allocatorInfo.device = S.device;
			vmaCreateAllocator(&allocatorInfo, &S.allocator);
		}

		//Descriptors
		{

			//Create descriptor pool
			std::vector<vk::DescriptorPoolSize> poolSizes = {
				vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer, STORAGE_COUNT},
				vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, IMAGE_COUNT},
			};
			S.descriptorPool = S.device.createDescriptorPool(vk::DescriptorPoolCreateInfo()
				.setPoolSizes(poolSizes)
				.setMaxSets(1)
				.setFlags(vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind)
			);

			//Create layout set
			auto bindings = {
				vk::DescriptorSetLayoutBinding()
					.setBinding(0)
					.setDescriptorType(vk::DescriptorType::eStorageBuffer)
					.setDescriptorCount(STORAGE_COUNT)
					.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics),
				 vk::DescriptorSetLayoutBinding()
					.setBinding(1)
					.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
					.setDescriptorCount(IMAGE_COUNT)
					.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics),
			};

			//Flag the set's bindings as partially bound and update after bind
			vk::DescriptorSetLayoutBindingFlagsCreateInfo setLayoutBindingsFlags = {};
			std::vector<vk::DescriptorBindingFlags> bindingFlags = {
				vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind,
				vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind
			};
			setLayoutBindingsFlags.setBindingFlags(bindingFlags);

			S.descriptorSetLayout = S.device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo()
				.setBindings(bindings)
				.setFlags(vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool)
				.setPNext(&setLayoutBindingsFlags)
			);

			auto sets = { S.descriptorSetLayout };
			auto pushConstant = { vk::PushConstantRange().setOffset(0).setSize(128).setStageFlags(vk::ShaderStageFlagBits::eAllGraphics) };
			S.pipelineLayout = S.device.createPipelineLayout(vk::PipelineLayoutCreateInfo()
				.setSetLayouts(sets)
				.setPushConstantRanges(pushConstant)
			);

			//Allocate single global descriptor set
			S.descriptorSet = S.device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo()
				.setDescriptorPool(S.descriptorPool)
				.setDescriptorSetCount(1)
				.setSetLayouts(sets)
			)[0];
		}

		return true;
	}

	///////////////
	// Swapchain //
	///////////////
	Image CreateImageForSwapchain(vk::Image image, uint32_t width, uint32_t height) {
		auto& S = GetState();
		std::shared_ptr<Internal_Image> state = std::make_shared<Internal_Image>();

		state->format = Format::B8G8R8A8Unorm;
		state->extent = { width, height };
		state->filter = Filter::Linear;
		state->image = image;

		InitializeImageView(state);

		return Image{ state };
	}
	Framebuffer CreateFramebufferForSwapchain(Image image) {
		auto& S = GetState();
		std::shared_ptr<struct Internal_Framebuffer> state = std::make_shared<struct Internal_Framebuffer>();

		std::vector<vk::ImageView> attachments = { image.state->view };

		state->pass = S.swapchainPass;
		state->extent = image.getExtent();
		state->framebuffer = S.device.createFramebuffer(vk::FramebufferCreateInfo()
			.setRenderPass(state->pass.state->pass)
			.setWidth(state->extent.width)
			.setHeight(state->extent.height)
			.setLayers(1)
			.setAttachments(attachments)
		);

		return Framebuffer{ state };
	}
	bool RecreateSwapchain() {
		auto& S = GetState();
		S.device.waitIdle();

		auto oldSwapchain = S.swapchain;
		//Reset state
		{
			S.swapchainImages.clear();
			S.swapchainFramebuffers.clear();
			S.swapchainPass = {};
			for (auto s : S.swapchainSemaphores) {
				S.device.destroySemaphore(s);
			}
			S.swapchainSemaphores.clear();
		}

		vk::SurfaceKHR surface = (VkSurfaceKHR)S.surface;

		if (!S.physicalDevice.getSurfaceSupportKHR(S.queueFamily, surface)) {
			return false;
		}

		//Check if surface supports format and colorSpace
		{
			bool foundSurfaceFormat = false;
			auto surfaceFormats = S.physicalDevice.getSurfaceFormatsKHR(surface);
			for (auto surfaceFormat : surfaceFormats) {
				if (surfaceFormat.format == vk::Format::eB8G8R8A8Unorm && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
					foundSurfaceFormat = true;
				}
			}
			assert(foundSurfaceFormat);
		}

		auto capabilities = S.physicalDevice.getSurfaceCapabilitiesKHR(surface);
		auto format = vk::Format::eB8G8R8A8Unorm;
		auto colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
		auto imageCount = capabilities.minImageCount;
		auto transform = capabilities.currentTransform;
		auto extent = capabilities.currentExtent;

		vk::SwapchainCreateInfoKHR swapchainCreateInfo;
		swapchainCreateInfo
			.setOldSwapchain(oldSwapchain)
			.setSurface(surface)
			.setPresentMode(vk::PresentModeKHR::eMailbox)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setClipped(false)

			.setImageFormat(format)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setImageSharingMode(vk::SharingMode::eExclusive)
			.setImageArrayLayers(1)

			.setQueueFamilyIndexCount(1)
			.setPQueueFamilyIndices(&S.queueFamily)

			.setMinImageCount(imageCount)
			.setImageExtent(extent)
			.setImageColorSpace(colorSpace)
			.setPreTransform(transform);

		S.swapchain = S.device.createSwapchainKHR(swapchainCreateInfo);
		S.swapchainPass = Pass::Create({ Pass::Attachment(Format::B8G8R8A8Unorm).setPresent(true) });
		S.swapchainIndex = imageCount - 1;

		std::vector<vk::Image> images = S.device.getSwapchainImagesKHR(S.swapchain);
		for (int i = 0; i < imageCount; i++) {
			Image img = CreateImageForSwapchain(images[i], extent.width, extent.height);
			S.swapchainImages.push_back(img);
			Framebuffer fb = CreateFramebufferForSwapchain(img);
			S.swapchainFramebuffers.push_back(fb);
			S.swapchainSemaphores.push_back(S.device.createSemaphore(vk::SemaphoreCreateInfo()));
		}

		S.device.destroySwapchainKHR(oldSwapchain);

		return true;
	}
	bool InitializeSwapchain(void* vulkanSurfaceKHR) {
		GetState().surface = (VkSurfaceKHR)vulkanSurfaceKHR;
		RecreateSwapchain();
		return true;
	}
	Pass& GetSwapchainPass() {
		return GetState().swapchainPass;
	}
}