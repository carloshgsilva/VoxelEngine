#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class GBufferPass {
    Pipeline pipeline;
public:
    GBufferPass() {
        pipeline = CreatePipeline({
            .CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/GBuffer.comp.spv")
        });
    }

    void Use(GBuffer& gbuffer, Buffer& viewBuffer, Buffer& bvhBuffer, Buffer& bvhLeafsBuffer) {
        struct PushConstant {
            int ColorTextureRID;
            int NormalTextureRID;
            int VisibilityTextureRID;
            int MotionTextureRID;
            int DepthTextureRID;
            int ViewBufferRID;
            int BVHBufferRID;
            int BVHLeafsBufferRID;
        } pc {
            .ColorTextureRID = GetRID(gbuffer.color),
            .NormalTextureRID = GetRID(gbuffer.normal),
            .VisibilityTextureRID = GetRID(gbuffer.visibility),
            .MotionTextureRID = GetRID(gbuffer.motion),
            .DepthTextureRID = GetRID(gbuffer.depthf),
            .ViewBufferRID = GetRID(viewBuffer),
            .BVHBufferRID = GetRID(bvhBuffer),
            .BVHLeafsBufferRID = GetRID(bvhLeafsBuffer)
        };

        Extent extent = GetDesc(gbuffer.color).extent;
        CmdBind(pipeline);
        CmdPush(pc);
        CmdDispatch((extent.width+7)/8, (extent.height+7)/8, 1);
    }

    static GBufferPass& Get() {
        static GBufferPass Instance;
        return Instance;
    }
};