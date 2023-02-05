#pragma once

#include "evk.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <unordered_map>
#include <filesystem>

#if defined(_DEBUG) || defined(EVK_DEBUG)
#define EVK_ASSERT(cond, message, ...)                                                    \
    if (!(cond)) {                                                                        \
        printf("\033[1;33m" __FUNCTION__ "() \033[1;31m" message "\033[0m", __VA_ARGS__); \
        abort();                                                                          \
    }

#define CHECK_VK(cmd) EVK_ASSERT(cmd == VK_SUCCESS, #cmd)  // printf("%s\n", #cmd);
#else
#define EVK_ASSERT(cond, message, ...)
#define CHECK_VK(cmd) cmd
#endif

namespace evk {

    struct FrameData {
        Image image;

        VkSemaphore imageReadySemaphore;

        VkCommandPool pool;
        VkCommandBuffer cmd;
        VkSemaphore cmdDoneSemaphore;
        bool doingPresent = false;
        bool insideRenderPass = false;
        VkFence fence;  // for the queue submit

        // performance queries
        VkQueryPool queryPool;
        std::vector<const char*> timestampNames;
        std::vector<uint64_t> queries;
        std::vector<TimestampEntry> timestampEntries;

        inline int timestampId(const char* name) {
            for (int i = 0; i < timestampNames.size(); i++) {
                if (std::strcmp(timestampNames[i], name) == 0) {
                    return i;
                }
            }
            timestampNames.push_back(name);
            return (int)timestampNames.size() - 1;
        }

        // deferred deletion
        std::vector<Resource*> toDelete;

        ~FrameData();
    };

    struct State {
        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkQueue queue;
        uint32_t queueFamily;
        VmaAllocator allocator;
        float timestampPeriod;

        // Descriptors
        VkDescriptorPool descriptorPool;
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSet descriptorSet;
        int descriptorIndexBuffer = 0;
        int descriptorIndexImage = 0;
        std::vector<int> freeIndexBuffer;
        std::vector<int> freeIndexImage;
        VkPipelineLayout pipelineLayout;

        // Swapchain
        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;
        uint32_t swapchainIndex = 0;
        uint32_t swapchainSemaphoreIndex = 0;

        std::vector<FrameData> frames = {};
        uint32_t frame = 0;
        uint32_t frame_total = 0;

        // Raytracing
        struct TLAS {
            VkAccelerationStructureKHR accel;
            Buffer buffer;
        } tlas;
        PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
        PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
        PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
        PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
        PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
        PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
        PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
        PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
        PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;

        // Pfns
        PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectNameEXT;
    };
    State& GetState();
    FrameData& GetFrame();

    struct Internal_Buffer : Resource {
        BufferDesc desc;

        VmaAllocation allocation = {};
        VkBuffer buffer = {};
        VkDeviceAddress deviceAddress = {};
        void* mappedData = {};
        ~Internal_Buffer() {
            vmaDestroyBuffer(GetState().allocator, buffer, allocation);
            // Free descriptor index
            EVK_ASSERT(resourceid != -1, "destroying buffer '%s' with RID = -1", desc.name.c_str());
            GetState().freeIndexBuffer.push_back(resourceid);
        }
    };
    struct Internal_Image : Resource {
        ImageDesc desc;

        VmaAllocation allocation = {};
        VkImage image = {};
        VkImageView view = {};
        VkSampler sampler = {};

        ~Internal_Image() {
            auto& S = GetState();
            vkDestroySampler(S.device, sampler, nullptr);
            vkDestroyImageView(S.device, view, nullptr);
            if (allocation != nullptr) {
                vmaFreeMemory(S.allocator, allocation);
                vkDestroyImage(S.device, image, nullptr);
                // Free descriptor index
                {
                    EVK_ASSERT(resourceid != -1, "destroying image '%s' with RID = -1", desc.name.c_str());
                    GetState().freeIndexImage.push_back(resourceid);
                    resourceid = -1;
                }
            }
        }
    };
    struct Internal_Pipeline : Resource {
        PipelineDesc desc;
        VkPipeline pipeline = {};
        bool isCompute = false;
        ~Internal_Pipeline() {
            vkDestroyPipeline(GetState().device, pipeline, nullptr);
        }
    };
#if EVK_RT
    struct Internal_BLAS : Resource {
        struct BLASInput {
            std::vector<VkAccelerationStructureGeometryKHR> asGeometry;
            std::vector<VkAccelerationStructureBuildRangeInfoKHR> asBuildOffsetInfo;
            VkBuildAccelerationStructureFlagsKHR flags{};
        };
        BLASInput blasInput;
        VkAccelerationStructureKHR accel = {};
        VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
        VkAccelerationStructureBuildSizesInfoKHR sizeInfo = {};
        VkAccelerationStructureBuildRangeInfoKHR rangeInfo = {};
        Buffer buffer;

        ~Internal_BLAS() {
            GetState().vkDestroyAccelerationStructureKHR(GetState().device, accel, nullptr);
        }
    };
    static inline Internal_BLAS& ToInternal(const rt::BLAS& ref) {
        return *((Internal_BLAS*)ref.res);
    }
#endif

#define DEFINE_TO_INTERNAL(libClass)                                     \
    static inline Internal_##libClass& ToInternal(const libClass& ref) { \
        return *((Internal_##libClass*)ref.res);                         \
    }

    DEFINE_TO_INTERNAL(Image)
    DEFINE_TO_INTERNAL(Buffer)
    DEFINE_TO_INTERNAL(Pipeline)
}  // namespace evk