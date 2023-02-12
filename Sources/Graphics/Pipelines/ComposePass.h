#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class ComposePass {
    Pipeline pipeline;

   public:
    ComposePass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/Compose.comp.spv")});
    }

    void Use(Image& outRadiance, Image& inRadiance, GBuffer& gbuffer, Buffer& viewBuffer, Buffer& voxInstancesBuffer) {
        Extent extent = GetDesc(outRadiance).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(outRadiance),
            GetRID(inRadiance),
            GetRID(gbuffer.visibility),
            GetRID(gbuffer.depth),
            GetRID(viewBuffer),
            GetRID(voxInstancesBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static ComposePass& Get() {
        static ComposePass Instance;
        return Instance;
    }
};