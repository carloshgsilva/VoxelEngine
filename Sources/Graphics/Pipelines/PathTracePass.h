#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class PathTracePass {
    Pipeline pipeline;
public:
    PathTracePass() {
        pipeline = CreatePipeline({
            .CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/PathTrace.comp.spv")
        });
    }

    void Use(Image& dstColor, GBuffer& gbuffer, Buffer& viewBuffer, Buffer& bvhBuffer, Buffer& bvhLeafsBuffer) {
        struct PushConstant {
            int ColorTextureRID;
            int NormalTextureRID;
            int VisibilityTextureRID;
            int DepthTextureRID;
            int ViewBufferRID;
            int BVHBufferRID;
            int BVHLeafsBufferRID;
        } pc {
            .ColorTextureRID = GetRID(dstColor),
            .NormalTextureRID = GetRID(gbuffer.normal),
            .VisibilityTextureRID = GetRID(gbuffer.visibility),
            .DepthTextureRID = GetRID(gbuffer.depthf),
            .ViewBufferRID = GetRID(viewBuffer),
            .BVHBufferRID = GetRID(bvhBuffer),
            .BVHLeafsBufferRID = GetRID(bvhLeafsBuffer)
        };

        Extent extent = GetDesc(dstColor).extent;
        CmdBind(pipeline);
        CmdPush(pc);
        CmdDispatch((extent.width+7)/8, (extent.height+7)/8, 1);
    }

    static PathTracePass& Get() {
        static PathTracePass Instance;
        return Instance;
    }
};