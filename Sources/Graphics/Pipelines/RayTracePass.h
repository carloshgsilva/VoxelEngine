#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class RayTracePass {
    Pipeline pipeline;
public:
    RayTracePass() {
        pipeline = CreatePipeline({
            .CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/RayTrace.comp.spv")
        });
    }

    void Use(Image& dstColor, Buffer& viewBuffer, Buffer& bvhBuffer, Buffer& bvhLeafsBuffer) {
        struct PushConstant {
            int ColorTextureRID;
            int ViewBufferRID;
            int BVHBufferRID;
            int BVHLeafsBufferRID;
        } pc {
            .ColorTextureRID = GetRID(dstColor),
            .ViewBufferRID = GetRID(viewBuffer),
            .BVHBufferRID = GetRID(bvhBuffer),
            .BVHLeafsBufferRID = GetRID(bvhLeafsBuffer)
        };

        Extent extent = GetDesc(dstColor).extent;
        CmdBind(pipeline);
        CmdPush(pc);
        CmdDispatch((extent.width+7)/8, (extent.height+7)/8, 1);
    }

    static RayTracePass& Get() {
        static RayTracePass Instance;
        return Instance;
    }
};