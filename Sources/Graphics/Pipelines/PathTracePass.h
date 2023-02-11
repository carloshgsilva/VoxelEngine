#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class PathTracePass {
    Pipeline pipeline;

   public:
    PathTracePass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/PathTrace.comp.spv")});
    }

    void Use(Image& dstColor, GBuffer& gbuffer, Buffer& viewBuffer, Buffer& bvhBuffer, Buffer& bvhLeafsBuffer, rt::TLAS& tlas, Buffer& voxInstancesBuffer) {
        Extent extent = GetDesc(dstColor).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(dstColor),
            GetRID(gbuffer.normal),
            GetRID(gbuffer.visibility),
            GetRID(gbuffer.depthf),
            GetRID(viewBuffer),
            GetRID(bvhBuffer),
            GetRID(bvhLeafsBuffer),
            GetRID(tlas),
            GetRID(voxInstancesBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static PathTracePass& Get() {
        static PathTracePass Instance;
        return Instance;
    }
};