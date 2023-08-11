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
        Extent extent = GetDesc(gbuffer.packed).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(outRadiance),
            GetRID(inRadiance),
            GetRID(gbuffer.packed),
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
        Extent extent = GetDesc(gbuffer.packed).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(outRadiance),
            GetRID(inRadiance),
            GetRID(gbuffer.packed),
            GetRID(viewBuffer),
            extent.width,
            extent.height,
            size,
        });
        CmdDispatch((extent.width + 15) / 16, (extent.height + 7) / 8, 1);
    }

    static DenoiserAtrousPass& Get() {
        static DenoiserAtrousPass Instance;
        return Instance;
    }
};

class DenoiserSpecularPass {
    Pipeline pipeline;

   public:
    DenoiserSpecularPass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/DenoiserSpecular.comp.spv")});
    }

    void Use(Image& outRadiance, Image& inRadiance, GBuffer& gbuffer, Buffer& viewBuffer, int size) {
        Extent extent = GetDesc(gbuffer.packed).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(outRadiance),
            GetRID(inRadiance),
            GetRID(gbuffer.packed),
            GetRID(viewBuffer),
            size,
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static DenoiserSpecularPass& Get() {
        static DenoiserSpecularPass Instance;
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
        Extent extent = GetDesc(gbuffer.packed).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(outRadiance),
            GetRID(inRadiance),
            GetRID(inLastRadiance),
            GetRID(gbuffer.motion),
            GetRID(gbuffer.packed),
            GetRID(gbuffer.previousPacked),
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
        Extent extent = GetDesc(gbuffer.packed).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(outRadiance),
            GetRID(inRadiance),
            GetRID(inLastRadiance),
            GetRID(gbuffer.motion),
            GetRID(gbuffer.packed),
            GetRID(viewBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static TAAPass& Get() {
        static TAAPass Instance;
        return Instance;
    }
};
