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
            GetRID(gbuffer.depth),
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
            GetRID(gbuffer.depth),
            GetRID(viewBuffer),
            size,
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
            GetRID(gbuffer.depth),
            GetRID(gbuffer.previousDepth),
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
            GetRID(gbuffer.depth),
            GetRID(gbuffer.previousDepth),
            GetRID(viewBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static TAAPass& Get() {
        static TAAPass Instance;
        return Instance;
    }
};

class ScreenProbesTracePass {
    Pipeline pipeline;

   public:
    ScreenProbesTracePass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/ScreenProbesTrace.comp.spv")});
    }

    void Use(Image& dstColor, GBuffer& gbuffer, Buffer& viewBuffer, rt::TLAS& tlas, Buffer& voxInstancesBuffer) {
        Extent extent = GetDesc(dstColor).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(dstColor),
            GetRID(gbuffer.normal),
            GetRID(gbuffer.visibility),
            GetRID(gbuffer.depth),
            GetRID(viewBuffer),
            GetRID(tlas),
            GetRID(voxInstancesBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static ScreenProbesTracePass& Get() {
        static ScreenProbesTracePass Instance;
        return Instance;
    }
};

class ScreenProbesSamplePass {
    Pipeline pipeline;

   public:
    ScreenProbesSamplePass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/ScreenProbesSample.comp.spv")});
    }

    void Use(Image& dstIrradiance, Image& srcProbes, GBuffer& gbuffer, Buffer& viewBuffer, rt::TLAS& tlas, Buffer& voxInstancesBuffer) {
        Extent extent = GetDesc(dstIrradiance).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(dstIrradiance),
            GetRID(srcProbes),
            GetRID(gbuffer.normal),
            GetRID(gbuffer.visibility),
            GetRID(gbuffer.depth),
            GetRID(viewBuffer),
            GetRID(tlas),
            GetRID(voxInstancesBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static ScreenProbesSamplePass& Get() {
        static ScreenProbesSamplePass Instance;
        return Instance;
    }
};

class ScreenProbesFilterPass {
    Pipeline pipeline;

   public:
    ScreenProbesFilterPass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/ScreenProbesFilter.comp.spv")});
    }

    void Use(Image& dstIrradiance, Image& srcProbes, GBuffer& gbuffer, Buffer& viewBuffer, rt::TLAS& tlas, Buffer& voxInstancesBuffer, int axis) {
        Extent extent = GetDesc(dstIrradiance).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(dstIrradiance),
            GetRID(srcProbes),
            GetRID(gbuffer.normal),
            GetRID(gbuffer.visibility),
            GetRID(gbuffer.depth),
            GetRID(viewBuffer),
            GetRID(tlas),
            GetRID(voxInstancesBuffer),
            axis,
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static ScreenProbesFilterPass& Get() {
        static ScreenProbesFilterPass Instance;
        return Instance;
    }
};

class RadianceProbesTracePass {
    Pipeline pipeline;

   public:
    RadianceProbesTracePass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/RadianceProbesTrace.comp.spv")});
    }

    void Use(Image& dstRadiance, GBuffer& gbuffer, Buffer& viewBuffer, rt::TLAS& tlas, Buffer& voxInstancesBuffer) {
        Extent extent = GetDesc(dstRadiance).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(dstRadiance),
            GetRID(gbuffer.normal),
            GetRID(gbuffer.visibility),
            GetRID(gbuffer.depth),
            GetRID(viewBuffer),
            GetRID(tlas),
            GetRID(voxInstancesBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static RadianceProbesTracePass& Get() {
        static RadianceProbesTracePass Instance;
        return Instance;
    }
};
