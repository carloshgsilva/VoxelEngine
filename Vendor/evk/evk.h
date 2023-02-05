#pragma once

#include <vector>
#include <string>

#define EVK_RT 0

namespace evk {
    constexpr int MAX_VERTEX_BINDING_COUNT = 4;
    constexpr int MAX_VERTEX_ATTRIBUTE_COUNT = 4;
    constexpr int MAX_ATTACHMENTS_COUNT = 8;

    using RID = int;
    enum class Stage { TopOfPipe, Host, Transfer, Compute, DrawIndirect, VertexInput, VertexShader, EarlyFragmentTest, FragmentShader, LateFragmentTest, ColorAttachmentOutput, BottomOfPipe, AllGraphics, AllCommands };
    enum class Format { Undefined, R8Uint, R16Uint, R32Uint, R64Uint, BGRA8Unorm, BGRA8Snorm, RGBA8Unorm, RGBA8Snorm, RG16Sfloat, RGBA16Sfloat, RGBA16Unorm, RGBA16Snorm, R32Sfloat, RG32Sfloat, RGB32Sfloat, RGBA32Sfloat, D24UnormS8Uint, D32Sfloat };
    enum class Primitive { Triangle, Line };
    enum class Blend { Disabled, Alpha, Additive };
    enum class BufferUsage {
        TransferSrc = 1,
        TransferDst = 2,
        Vertex = 4,     // used in cmd.vertex()
        Index = 8,      // used in cmd.drawIndex;ed() and cmd.drawIndexedIndirect()
        Indirect = 16,  // used in cmd.drawIndirect() and cmd.drawIndexedIndirect()
        Storage = 32,
        AccelerationStructure = 64,        // used internally for ray tracing acceleration structure
        AccelerationStructureInput = 128,  // used on vertex and indices buffer for ray tracing acceleration structure
    };
    enum class ImageLayout {
        Undefined,
        General,
        TransferSrc,
        TransferDst,
        ShaderRead,
        Attachment,
        Present,
    };
    enum class ImageUsage {
        TransferSrc = 1,  // used by CmdCopy()
        TransferDst = 2,  // used by CmdCopy()
        Sampled = 4,      // can be read by shader with texture() or texelFetch()
        Attachment = 8,   // used by CmdBeginRender()
        Storage = 16,     // can access with imageStore and imageLoad
    };
    struct ClearDepthStencil {
        float depth = 1.0f;
        uint32_t stencil = 0;
    };
    struct ClearColor {
        union {
            float float32[4];
            int32_t int32[4];
            uint32_t uint32[4];
        };
    };
    struct ClearValue {
        union {
            ClearDepthStencil depthStencil;
            ClearColor color;
        };
        ClearValue(ClearColor&& color) {
            this->color = color;
        }
        ClearValue(ClearDepthStencil&& depthStencil) {
            this->depthStencil = depthStencil;
        }
    };
    enum class Filter { Nearest, Linear };
    enum class MemoryType {
        CPU,         // lives in the mains CPU's memory
        GPU,         // lives in the GPU's memory
        CPU_TO_GPU,  // data that change every frame
        GPU_TO_CPU,  // useful for read back
    };
    enum class Op { Never, Less, Equal, LessOrEqual, Greater, NotEqual, GreaterOrEqual, Always };
    enum class Cull { None, Front, Back };
    struct Extent {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 1;
        Extent() {
        }
        Extent(uint32_t width, uint32_t height) : width(width), height(height), depth(1) {
        }
        Extent(uint32_t width, uint32_t height, uint32_t depth) : width(width), height(height), depth(depth) {
        }
    };
    struct ImageRegion {
        int x, y, z;
        int width, height, depth, mip = 0, layer = 0;
    };
    struct TimestampEntry {
        double start, end;
        const char* name;
    };

    BufferUsage operator|(BufferUsage a, BufferUsage b);
    ImageUsage operator|(ImageUsage a, ImageUsage b);

    struct Resource {
        RID resourceid = -1;
        uint32_t refCount = 0;
        void incRef() {
            refCount++;
        }
        void decRef();
        virtual ~Resource() {
        }
    };
    struct ResourceRef {
        Resource* res = nullptr;
        ResourceRef() {
        }
        ResourceRef(Resource* res) {
            if (res != nullptr) {
                this->res = res;
                res->incRef();
            }
        }
        ResourceRef& operator=(const ResourceRef& ref) {
            // if(ref.res == nullptr) return *this;
            if (res != nullptr) res->decRef();
            res = ref.res;
            if (res != nullptr) res->incRef();
            return *this;
        }
        ResourceRef(const ResourceRef& ref) {
            *this = ref;
        }
        ResourceRef& operator=(ResourceRef&& ref) {
            // if(ref.res == nullptr) return *this;
            if (res != nullptr) res->decRef();
            res = ref.res;
            if (res != nullptr) ref.res = nullptr;
            return *this;
        }
        ResourceRef(ResourceRef&& ref) {
            *this = std::move(ref);
        }

        ~ResourceRef() {
            if (res != nullptr) res->decRef();
        }
        explicit operator bool() const {
            return res != nullptr;
        }
        void release() {
            if (res != nullptr) {
                res->decRef();
                res = nullptr;
            }
        }
        void swap(ResourceRef& other) {
            Resource* temp = other.res;
            other.res = res;
            res = temp;
        }
    };

    struct PipelineDesc {
        std::string name = {};

        std::vector<uint8_t> VS = {};
        std::vector<uint8_t> FS = {};
        std::vector<uint8_t> CS = {};

        std::vector<std::vector<Format>> bindings = {};
        std::vector<Format> attachments = {};
        std::vector<Blend> blends = {};

        Primitive primitive = Primitive::Triangle;
        Cull cull = Cull::None;
        bool wireframe = false;
        bool frontClockwise = false;

        Op depthOp = Op::Never;
        bool depthTest = false;
        bool depthWrite = false;
    };
    struct Pipeline : ResourceRef {
        Pipeline(Resource* res = nullptr) : ResourceRef(res) {
        }
    };
    Pipeline CreatePipeline(const PipelineDesc& desc);

    struct BufferDesc {
        std::string name = "";
        uint64_t size = 0;
        BufferUsage usage = BufferUsage::TransferSrc;
        MemoryType memoryType = MemoryType::CPU;
    };
    struct Buffer : ResourceRef {
        Buffer(Resource* res = nullptr) : ResourceRef(res) {
        }
        void* GetPtr();
    };
    Buffer CreateBuffer(const BufferDesc& desc);
    void WriteBuffer(Buffer& buffer, void* src, uint64_t size, uint64_t offset = 0u);
    void ReadBuffer(Buffer& buffer, void* dst, uint64_t size, uint64_t offset = 0u);

    struct ImageDesc {
        std::string name = "";
        Extent extent = {};
        Format format = Format::Undefined;
        Filter filter = Filter::Linear;
        ImageUsage usage = ImageUsage::Sampled;
        uint32_t mipCount = 1;
        uint32_t layerCount = 1;
        bool isCube = false;
    };
    struct Image : ResourceRef {
        Image(Resource* res = nullptr) : ResourceRef(res) {
        }
    };
    Image CreateImage(const ImageDesc& desc);

    struct EvkDesc {
        std::string applicationName = "";
        std::uint32_t applicationVersion = 0;
        std::string engineName = "";
        std::uint32_t engineVersion = 0;
        std::vector<std::string> instanceLayers = {};
        std::vector<std::string> instanceExtensions = {};
        uint32_t frameBufferingCount = 2;
    };

    RID GetRID(const ResourceRef& ref);
    const BufferDesc& GetDesc(const Buffer& res);
    const ImageDesc& GetDesc(const Image& res);

    bool InitializeEVK(const EvkDesc& info);
    void Shutdown();
    bool InitializeSwapchain(void* vulkanSurfaceKHR);

    // Returns the frame buffering count
    uint32_t GetFrameBufferingCount();
    // Returns the frame buffering index
    uint32_t GetFrameIndex();
    // Should be called at the end of each frame to submit the command buffer to the gpu
    void Submit();
    // Ensures that no command buffer are running or have access to any resource
    void Sync();
    // Returns the timestamps of the last frame
    const std::vector<TimestampEntry>& GetTimestamps();
    // Binds a graphics shader or a compute kernel
    // Eg. Path/To/Shader
    // Graphics and Compute shader will not affect each other
    void CmdBind(Pipeline pipeline);
    // Dispatches a compute shader
    void CmdDispatch(uint32_t countX, uint32_t countY, uint32_t countZ);
    // Transition a image to a layout with dependecy stages
    void CmdBarrier(Image& image, ImageLayout oldLayout, ImageLayout newLayout, uint32_t mip = 0, uint32_t mipCount = 1, uint32_t layer = 0, uint32_t layerCount = 1);
    void CmdBarrier(Buffer& buffer);
    // Fills buffer with value
    void CmdFill(Buffer dst, uint32_t data, uint64_t size, uint64_t offset = 0);
    // Updates buffer with small amout of data
    void CmdUpdate(Buffer& dst, uint64_t dstOffset, uint64_t size, void* src);
    // Blit a region of a Image to another Image
    void CmdBlit(Image& src, Image& dst, ImageRegion srcRegion, ImageRegion dstRegion, Filter filter = Filter::Linear);
    // Copy a Image to another Image of the same size
    void CmdCopy(Image& src, Image& dst, uint32_t srcMip = 0, uint32_t srcLayer = 0, uint32_t dstMip = 0, uint32_t dstLayer = 0, uint32_t layerCount = 1);
    // Copy a Buffer to a Image
    void CmdCopy(Buffer& src, Image& dst, uint32_t mip = 0, uint32_t layer = 0);
    // Copy regions from a Buffer to a Image
    void CmdCopy(Buffer& src, Image& dst, const std::vector<ImageRegion>& regions);
    // Copy regions of a Buffer to Buffer
    void CmdCopy(Buffer& src, Buffer& dst, uint64_t size, uint64_t srcOffset = 0, uint64_t dstOffset = 0);
    // Binds a vertex buffer
    void CmdVertex(Buffer& buffer, uint64_t offset = 0);
    // Binds a index buffer
    void CmdIndex(Buffer& buffer, bool useHalf = false, uint64_t offset = 0);

    void CmdBeginRender(Image* attachments, ClearValue* clearValues, int attachmentCount);
    void CmdEndRender();
    void CmdBeginPresent();
    void CmdEndPresent();
    void CmdViewport(float x, float y, float w, float h, float minDepth = 0.0f, float maxDepth = 1.0f);
    void CmdScissor(int32_t x, int32_t y, uint32_t w, uint32_t h);
    void CmdPush(void* data, uint32_t size, uint32_t offset = 0);
    void CmdClear(Image image, ClearValue value);
    void CmdDraw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    void CmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);
    void CmdDrawIndirect(Buffer& buffer, uint64_t offset, uint32_t drawCount, uint32_t stride);
    void CmdDrawIndexedIndirect(Buffer& buffer, uint64_t offset, uint32_t drawCount, uint32_t stride);
    void CmdDrawIndirectCount(Buffer& buffer, uint64_t offset, Buffer& countBuffer, uint64_t countBufferOffset, uint32_t drawCount, uint32_t stride);
    void CmdDrawIndexedIndirectCount(Buffer& buffer, uint64_t offset, Buffer& countBuffer, uint64_t countBufferOffset, uint32_t drawCount, uint32_t stride);
    void CmdBeginTimestamp(const char* name);
    void CmdEndTimestamp(const char* name);

    template <typename T>
    void CmdPush(T& data) {
        CmdPush((void*)(&data), sizeof(T), 0);
    }
    template <typename T>
    void CmdPresent(T callback) {
        CmdBeginPresent();
        callback();
        CmdEndPresent();
    }
    template <typename T>
    void CmdRender(std::initializer_list<Image> attachments, std::initializer_list<ClearValue> clearValues, T callback, ImageLayout finalLayout = ImageLayout::ShaderRead) {
        for (auto a : attachments) {
            CmdBarrier(a, ImageLayout::Undefined, ImageLayout::Attachment);
        }
        CmdBeginRender((Image*)attachments.begin(), (ClearValue*)clearValues.begin(), (int)attachments.size());
        callback();
        CmdEndRender();
        for (auto a : attachments) {
            CmdBarrier(a, ImageLayout::Attachment, finalLayout);
        }
    }
    template <typename T>
    void CmdRender(Image* attachments, ClearValue* clearValues, int attachmentCount, T callback) {
        CmdBeginRender(attachments, clearValues, attachmentCount);
        callback();
        CmdEndRender();
    }
    template <typename T>
    void CmdTimestamp(const char* name, T callback) {
        CmdBeginTimestamp(name);
        callback();
        CmdEndTimestamp(name);
    }
    template <typename T>
    class PerFrame {
        T data[3];

       public:
        operator T&() {
            return data[GetFrameIndex()];
        }
        T& operator[](int i) {
            // assert(i < 3 && "Only triple buffering supported at max!");
            return data[i];
        }
        T* operator->() {
            return &data[GetFrameIndex()];
        }
        template <typename Cb>
        void Build(Cb cb) {
            for (uint32_t i = 0; i < GetFrameBufferingCount(); i++) {
                data[i] = cb(i);
            }
        }
        void release() {
            for (uint32_t i = 0; i < GetFrameBufferingCount(); i++) {
                data[i].release();
            }
        }
    };

#if EVK_RT
    namespace rt {
        struct BLASDesc {
            Buffer vertices;
            uint32_t vertexCount;
            Buffer indices;
            uint32_t indexCount;
            uint32_t stride;
        };
        struct BLAS : ResourceRef {
            BLAS(Resource* res = nullptr) : ResourceRef(res) {
            }
        };
        BLAS CreateBLAS(const BLASDesc& desc);

        struct BLASInstance {
            BLAS blas;
            uint32_t customId;
            float transform[12];
        };

        void Initialize();
        void CmdBuildBLAS(BLAS blas, bool update = false);
        void CmdBuildTLAS(std::vector<BLASInstance>& instances, bool update = false);
    }  // namespace rt
#endif
}  // namespace evk