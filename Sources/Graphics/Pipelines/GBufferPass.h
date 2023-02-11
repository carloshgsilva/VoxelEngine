#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class GBufferPass {
    Pipeline pipeline;

   public:
    GBufferPass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/GBuffer.comp.spv")});
    }

    void Use(GBuffer& gbuffer, Buffer& viewBuffer, Buffer& bvhBuffer, Buffer& bvhLeafsBuffer, rt::TLAS& tlas, Buffer& voxInstancesBuffer) {
        Extent extent = GetDesc(gbuffer.color).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(gbuffer.color),
            GetRID(gbuffer.normal),
            GetRID(gbuffer.visibility),
            GetRID(gbuffer.motion),
            GetRID(gbuffer.depthf),
            GetRID(viewBuffer),
            GetRID(bvhBuffer),
            GetRID(bvhLeafsBuffer),
            GetRID(tlas),
            GetRID(voxInstancesBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static GBufferPass& Get() {
        static GBufferPass Instance;
        return Instance;
    }
};