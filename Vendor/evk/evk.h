
#include <vector>

#ifdef EVK_INTERNAL_STATE
#include "vulkan/vulkan.hpp"
#include "vma/vk_mem_alloc.h"
#endif

/*
////////////////////////////////////////
// GLSL to access bindless resources //
////////////////////////////////////////

#define RID int
#define IN(code) layout(location=0) in struct { code } In;
#define OUT(code) layout(location=0) out struct { code } Out;
#define PUSH_CONSTANT(code) layout(push_constant) uniform uPushConstant { code };
#define BINDING_BUFFER(name, code) layout(binding = 0) readonly buffer name##_t { code } name[];
layout(binding = 1) uniform sampler2D _BindingSampler2D[];
layout(binding = 1) uniform usampler3D _BindingUSampler3D[];

How to access push constant:
	PUSH_CONSTANT(
		float Time;
	)
	float t = Time;

How to access a buffer:
	BINDING_BUFFER(ViewBuffer,
		mat4 ViewMatrix;
		mat4 ProjectionMatrix;
	)

	To read just supply with the id
	mat4 view_matrix = ViewBuffer[resource_id].ViewMatrix;

How to access a sampler:
	vec4 color = texture(_BindingSampler2D(resource_id), uv);

;*/

namespace evk {
	/*
	src: https://developer.samsung.com/galaxy-gamedev/resources/articles/usage.html


	#-------------------------------------------------------------#
	|                        (AllCommands)                        |
	#-------------------------------------------------------------#
	| Host	    | Transfer | Compute	  | Graphics (AllGraphics)|
	|-----------|----------|--------------|-----------------------|
	| TopOfPipe |          |              |                       |
	| Host      |          |              |                       |
	|-----------| Transfer |              |                       |
	|           |----------| Compute      |                       |
	|           |          | DrawIndirect | DrawIndirect          |
	|           |          |--------------| VertexInput           |
	|           |          |              | VertexShader          |
	|           |          |              | EarlyFragmentTest     |
	|           |          |              | FragmentShader        |
	|           |          |              | LateFragmentTest      |
	|-----------#----------#--------------# ColorAttachmentOutput |
	| BottomOfPipe                                                |
	#-------------------------------------------------------------#
	Host = CPU->GPU Memory read/write

	*/

#ifndef __EVK_INCLUDE_
#define __EVK_INCLUDE_

	using RID = int;
	enum class Stage {
		TopOfPipe,
		Host,
		Transfer,
		Compute,
		DrawIndirect,
		VertexInput,
		VertexShader,
		EarlyFragmentTest,
		FragmentShader,
		LateFragmentTest,
		ColorAttachmentOutput,
		BottomOfPipe,
		AllGraphics,
		AllCommands
	};
	enum class Format {
		R8Uint,
		B8G8R8A8Unorm,
		B8G8R8A8Snorm,
		R8G8B8A8Unorm,
		R8G8B8A8Snorm,
		R16G16Sfloat,
		R16G16B16A16Sfloat,
		R16G16B16A16Unorm,
		R16G16B16A16Snorm,
		R32G32B32A32Sfloat,
		D24UnormS8Uint,
	};
	enum class Blend {
		Disabled,
		Alpha,
		Additive
	};
	enum class BufferUsage {
		TransferSrc = 1,
		TransferDst = 2,
		Vertex = 4,
		Index = 8,
		Indirect = 16,
		Storage = 32,
	};
	enum class ImageLayout {
		Undefined,
		General,
		TransferSrc,
		TransferDst,
		ShaderReadOptimal,
	};
	enum class ImageUsage {
		TransferSrc = 1,
		TransferDst = 2,
		Sampled = 4,
		Attachment = 8,
	};
	enum class Filter {
		Nearest,
		Linear
	};
	enum class MemoryType {
		CPU,       // lives in the mains CPU's memory
		GPU,       // lives in the GPU's memory
		CPU_TO_GPU, // data that change every frame 
		GPU_TO_CPU, // useful for read back
	};
	struct Extent {
		uint32_t width{ 0 };
		uint32_t height{ 0 };
		uint32_t depth{ 0 };
		Extent() { }
		Extent(uint32_t width, uint32_t height) : width(width), height(height), depth(1) { }
		Extent(uint32_t width, uint32_t height, uint32_t depth) : width(width), height(height), depth(depth) { }
	};
	struct Viewport {
		float x;
		float y;
		float width;
		float height;
	};
	struct ImageRegion {
		int x, y, z;
		uint32_t width, height, depth, mip{ 0 }, layer{ 0 };
	};
	struct TimestampEntry {
		double start, end;
		const char* name;
	};

	BufferUsage operator |(BufferUsage a, BufferUsage b);
	ImageUsage operator |(ImageUsage a, ImageUsage b);

	struct Pass {
		std::shared_ptr<struct Internal_Pass> state;

		struct Attachment {
			Format format;
			float clearColor[4]{ 0.0f,0.0f,0.0f,0.0f };
			float clearDepth{ 1.0f };
			uint32_t clearStencil{ 0 };
			bool present{ false };

			Attachment(Format format) : format(format) {}
			/* default: r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f */
			Attachment& setClearColor(float r, float g, float b, float a) {
				clearColor[0] = r;
				clearColor[1] = g;
				clearColor[2] = b;
				clearColor[3] = a;
				return *this;
			}
			/* default: depth = 0.0f, stencil = 0 */
			Attachment& setClearDepthStencil(float depth, uint32_t stencil) {
				clearDepth = depth;
				clearStencil = stencil;
				return *this;
			}
			Attachment& setPresent(bool present) { this->present = present; return *this; }
		};

		static Pass Create(const std::vector<Attachment>& attachments);
	};


	//TODO: Probably going to remove
	struct Shader {
		std::shared_ptr<struct Internal_Shader> state;

		static Shader Vertex(const std::vector<uint8_t>& spirv);
		static Shader Fragment(const std::vector<uint8_t>& spirv);
	};

	struct GraphicsPipeline {
		std::shared_ptr<struct Internal_GraphicsPipeline> state;

		struct Info {
			Blend blend{ Blend::Disabled };
			std::vector<uint8_t> vertexSpirv;
			std::vector<uint8_t> fragmentSpirv;
			Pass pass;
			bool depthTest;
			bool depthWrite;
			//MeshLayout meshLayout;//TODO: 
			//std::vector<Shader> shaders;//TODO: 

			//default: Blend::Disabled
			Info& setBlend(Blend blend) { this->blend = blend; return *this; }
			//default: present pass
			Info& setPass(Pass pass) { this->pass = pass; return *this; }

			//default: depthTest: false, depthWrite: false
			Info& setDepth(bool depthTest = true, bool depthWrite = true) { this->depthTest = depthTest; this->depthWrite = depthWrite; return *this; }

			//default: false
			//default: No mesh
			//Info& setMeshLayout

			Info& vertexShader(const std::vector<uint8_t>& spirv) { vertexSpirv = spirv; return *this; }
			Info& fragmentShader(const std::vector<uint8_t>& spirv) { fragmentSpirv = spirv; return *this; }
		};

		static GraphicsPipeline Create(const Info& info);//TODO: 

	};

	struct MeshLayout {
		std::shared_ptr<struct Internal_MeshLayout> state;
		//TODO: MeshLayout Create();
	};

	struct Buffer {
		struct Info {
			MemoryType memoryType{ MemoryType::CPU };

			Info& setMemoryType(MemoryType memoryType) { this->memoryType = memoryType; return *this; }
		};
		std::shared_ptr<struct Internal_Buffer> state;

		static Buffer Create(uint64_t size, BufferUsage usage = BufferUsage::TransferSrc, MemoryType memoryType = MemoryType::CPU);

		void* getData();
		void update(void* src, uint64_t size, uint64_t offset = 0);
		RID getRID();
	};

	struct Image {
		std::shared_ptr<struct Internal_Image> state;

		struct Info {
			Format format;
			Extent extent;
			Filter filter{ Filter::Linear };
			ImageUsage usage{ ImageUsage::Sampled };
			int mipCount{ 1 };
			int layerCount{ 1 };
			bool isCube{ false };

			Info(Format format, Extent extent) : format(format), extent(extent) {}

			//default: ImageUsage::Sampled
			Info& setUsage(ImageUsage usage) { this->usage = usage; return *this; }
			//default: Filter::Linear
			Info& setFilter(Filter filter) { this->filter = filter; return *this; };
			//default: 1
			Info& setMipCount(int count) { this->mipCount = count; return *this; };
			//default: 1
			Info& setLayerCount(int count) { this->layerCount = count; return *this; };
			//default: false
			Info& setCube(int isCube) { this->isCube = isCube; return *this; };
		};
		static Image Create(const Image::Info& info);

		Format getFormat();
		Extent getExtent();
		int getMipCount();
		RID getRID();
	};

	struct Framebuffer {
		std::shared_ptr<struct Internal_Framebuffer> state;

		static Framebuffer Create(Pass& pass, uint32_t width, uint32_t height);
		Extent getExtent();
		Image& getAttachment(int index);
	};

	struct CmdBuffer {
		std::shared_ptr<struct Internal_CmdBuffer> state;
		static CmdBuffer Create();

		CmdBuffer& submit();
		CmdBuffer& wait();

		// Transition a image to a layout with dependecy stages
		CmdBuffer& barrier(Image& image, ImageLayout oldLayout, ImageLayout newLayout, uint32_t mip = 0, uint32_t mipCount = 1, uint32_t layer = 0, uint32_t layerCount = 1);

		// Blit a Image to another Image
		CmdBuffer& blit(Image& src, Image& dst, ImageRegion srcRegion, ImageRegion dstRegion, Filter filter = Filter::Linear);
		// Copy a Image to another Image of the same size
		CmdBuffer& copy(Image& src, Image& dst, uint32_t srcMip = 0, uint32_t srcLayer = 0, uint32_t dstMip = 0, uint32_t dstLayer = 0, uint32_t layerCount = 1);
		// Copy a Buffer to a Image
		CmdBuffer& copy(Buffer& buffer, Image& image, uint32_t mip = 0, uint32_t layer = 0);
		// Copy regions from a Buffer to a Image
		CmdBuffer& copy(Buffer& buffer, Image& image, const std::vector<ImageRegion>& regions);

		CmdBuffer& begin();
		CmdBuffer& end();
		CmdBuffer& beginPresent();
		CmdBuffer& endPresent();
		CmdBuffer& beginPass(Framebuffer& frambuffer);
		CmdBuffer& endPass();
		CmdBuffer& bind(GraphicsPipeline& pass);
		CmdBuffer& viewport(const Viewport& viewport);
		CmdBuffer& constant(void* data, uint32_t size, uint32_t offset);
		CmdBuffer& draw(int vertexCount, int instanceCount = 1, int firstVertex = 0, int firstInstance = 0);
		CmdBuffer& beginTimestamp(const char* name);
		CmdBuffer& endTimestamp(const char* name);

		const std::vector<TimestampEntry> timestamps();

		template<typename T> CmdBuffer& use(T callback) {
			begin();
			callback();
			end();
			return *this;
		}
		template<typename T> CmdBuffer& present(T callback) {
			beginPresent();
			callback();
			endPresent();
			return *this;
		}
		template<typename T> CmdBuffer& use(Framebuffer& framebuffer, T callback) {
			beginPass(framebuffer);
			callback();
			endPass();
			return *this;
		}
		template<typename T> CmdBuffer& timestamp(const char* name, T callback) {
			beginTimestamp(name);
			callback();
			endTimestamp(name);
			return *this;
		}
	};

	struct InitInfo {
		std::string applicationName{ 0 };
		std::uint32_t applicationVersion{ 0 };
		std::string engineName{ 0 };
		std::uint32_t engineVersion{ 0 };
		std::vector<std::string> instanceLayers;
		std::vector<std::string> instanceExtensions;
		void* surface;
	public:

		InitInfo& setApplicationName(const std::string& name) { applicationName = name; return *this; }
		InitInfo& setApplicationVersion(std::uint32_t major, std::uint32_t minor = 0, std::uint32_t patch = 0);
		InitInfo& setEngineName(const std::string& name) { engineName = name; return *this; }
		InitInfo& setEngineVersion(std::uint32_t major, std::uint32_t minor = 0, std::uint32_t patch = 0);
		InitInfo& addInstanceExtension(const std::string& name) { instanceExtensions.push_back(name.c_str()); return *this; }
		InitInfo& addInstanceLayer(const std::string& name) { instanceLayers.push_back(name.c_str()); return *this; }
		InitInfo& setSurface(void* vulkanSurface);
		InitInfo& setExtensionsFilter(std::function<bool(const std::string&)> filter);
		InitInfo& setLayersFilter(std::function<bool(const std::string&)> filter);
	};

	bool InitializeEVK(const InitInfo& info);
	bool InitializeSwapchain(void* vulkanSurfaceKHR);
	Pass& GetSwapchainPass();

	std::vector<uint8_t> ShaderFromFile(const std::string& path);

#endif

#ifdef EVK_INTERNAL_STATE
#ifndef __EVK_INCLUDE_INTERNAL
#define __EVK_INCLUDE_INTERNAL
	struct State {
		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::Queue queue;
		uint32_t queueFamily;
		VmaAllocator allocator;
		float timestampPeriod;

		//Descriptors
		vk::DescriptorPool descriptorPool;
		vk::DescriptorSetLayout descriptorSetLayout;
		vk::DescriptorSet descriptorSet;
		int descriptorIndexBuffer{ 0 };
		int descriptorIndexImage{ 0 };
		std::vector<int> freeIndexBuffer;
		std::vector<int> freeIndexImage;
		vk::PipelineLayout pipelineLayout;

		//Swapchain
		vk::SurfaceKHR surface;
		vk::SwapchainKHR swapchain;
		uint32_t swapchainIndex{ 0 };
		uint32_t swapchainSemaphoreIndex{ 0 };
		Pass swapchainPass;
		std::vector<Framebuffer> swapchainFramebuffers;
		std::vector<Image> swapchainImages;
		std::vector<vk::Semaphore> swapchainSemaphores;
	};
	State& GetState();

	struct Internal_Pass {
		vk::RenderPass pass;
		std::vector<vk::ClearValue> clearValues;
		std::vector<Format> formats;
		~Internal_Pass() {
			GetState().device.destroyRenderPass(pass);
		}
	};
	struct Internal_Buffer {
		uint64_t size;
		//TODO: Usage, Types 

		VmaAllocation allocation;
		int resourceid{ -1 };
		vk::Buffer buffer;
		void* mappedData{ nullptr };

		~Internal_Buffer() {
			vmaDestroyBuffer(GetState().allocator, buffer, allocation);
			//Free descriptor index
			{
				assert(resourceid != -1);
				GetState().freeIndexBuffer.push_back(resourceid);
				resourceid = -1;
			}
		}
	};
	struct Internal_Image {
		Format format;
		Extent extent;
		Filter filter;
		int mipCount{ 1 };
		int layerCount{ 1 };
		bool isCube;

		VmaAllocation allocation;
		int resourceid;
		vk::Image image;
		vk::ImageView view;
		vk::Sampler sampler;

		~Internal_Image() {
			auto& S = GetState();
			S.device.destroySampler(sampler);
			S.device.destroyImageView(view);
			if (allocation != nullptr) {
				vmaFreeMemory(S.allocator, allocation);
				S.device.destroyImage(image);
				//Free descriptor index
				{
					assert(resourceid != -1);
					GetState().freeIndexImage.push_back(resourceid);
					resourceid = -1;
				}
			}

		}
	};
	struct Internal_Framebuffer {
		Pass pass{};
		Extent extent;
		vk::Framebuffer framebuffer;
		std::vector<Image> attachments;

		~Internal_Framebuffer() {
			attachments.clear();
			GetState().device.destroyFramebuffer(framebuffer);
		}
	};
	struct Internal_Shader {
		vk::ShaderModule shaderModule;
		vk::ShaderStageFlags shaderStage;

		~Internal_Shader() {
			GetState().device.destroyShaderModule(shaderModule);
		}
	};
	struct Internal_GraphicsPipeline {
		vk::Pipeline pipeline;

		~Internal_GraphicsPipeline() {
			GetState().device.destroyPipeline(pipeline);
		}
	};
	struct Internal_CmdBuffer {
		vk::CommandPool pool;
		vk::CommandBuffer cmd;
		vk::Fence fence;
		vk::Semaphore semaphore;
		vk::QueryPool queryPool;

		std::vector<const char*> timestampNames;
		std::vector<uint64_t> queries;
		std::vector<TimestampEntry> timestampEntries;

		Viewport viewport;
		bool doingPresent{ false };

		inline int timestampId(const char* name) {
			for (int i = 0; i < timestampNames.size(); i++) {
				if (std::strcmp(timestampNames[i], name) == 0) {
					return i;
				}
			}
			timestampNames.push_back(name);
			return timestampNames.size() - 1;
		}

		~Internal_CmdBuffer() {
			auto& D = GetState().device;
			D.destroyCommandPool(pool);
			D.destroyFence(fence);
			D.destroySemaphore(semaphore);
		}
	};
#endif
#endif
}