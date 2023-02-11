#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class DenoiserDiscPass {
    Pipeline pipeline;

   public:
    DenoiserDiscPass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/DenoiserDisc.comp.spv")});
    }

    void Use(Image& outRadiance, Image& inRadiance, GBuffer& gbuffer, Buffer& viewBuffer) {
        Extent extent = GetDesc(gbuffer.color).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(outRadiance),
            GetRID(inRadiance),
            GetRID(gbuffer.normal),
            GetRID(gbuffer.depthf),
            GetRID(viewBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static DenoiserDiscPass& Get() {
        static DenoiserDiscPass Instance;
        return Instance;
    }
};

class DenoiserAtrousPass {
    Pipeline pipeline;

   public:
    DenoiserAtrousPass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/DenoiserAtrous.comp.spv")});
    }

    void Use(Image& outRadiance, Image& inRadiance, GBuffer& gbuffer, Buffer& viewBuffer, int size) {
        Extent extent = GetDesc(gbuffer.color).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(outRadiance),
            GetRID(inRadiance),
            GetRID(gbuffer.normal),
            GetRID(gbuffer.depthf),
            GetRID(viewBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static DenoiserAtrousPass& Get() {
        static DenoiserAtrousPass Instance;
        return Instance;
    }
};

class DenoiserTemporalPass {
    Pipeline pipeline;

   public:
    DenoiserTemporalPass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/DenoiserTemporal.comp.spv")});
    }

    void Use(Image& outRadiance, Image& inRadiance, Image& inLastRadiance, GBuffer& gbuffer, Buffer& viewBuffer) {
        Extent extent = GetDesc(gbuffer.color).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(outRadiance),
            GetRID(inRadiance),
            GetRID(inLastRadiance),
            GetRID(gbuffer.motion),
            GetRID(gbuffer.depthf),
            GetRID(gbuffer.previousDepthf),
            GetRID(viewBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static DenoiserTemporalPass& Get() {
        static DenoiserTemporalPass Instance;
        return Instance;
    }
};

class TAAPass {
    Pipeline pipeline;

   public:
    TAAPass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/TAA.comp.spv")});
    }

    void Use(Image& outRadiance, Image& inRadiance, Image& inLastRadiance, GBuffer& gbuffer, Buffer& viewBuffer) {
        Extent extent = GetDesc(gbuffer.color).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(outRadiance),
            GetRID(inRadiance),
            GetRID(inLastRadiance),
            GetRID(gbuffer.motion),
            GetRID(gbuffer.depthf),
            GetRID(gbuffer.previousDepthf),
            GetRID(viewBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static TAAPass& Get() {
        static TAAPass Instance;
        return Instance;
    }
};