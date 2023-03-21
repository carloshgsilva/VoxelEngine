#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class PathTracePass {
    Pipeline pipeline;

   public:
    PathTracePass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/PathTrace.comp.spv")});
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

    static PathTracePass& Get() {
        static PathTracePass Instance;
        return Instance;
    }
};

class DITracePass {
    Pipeline pipeline;

   public:
    DITracePass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/DITrace.comp.spv")});
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

    static DITracePass& Get() {
        static DITracePass Instance;
        return Instance;
    }
};

class ReSTIRGITracePass {
    Pipeline pipeline;

   public:
    ReSTIRGITracePass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/ReSTIRGITrace.comp.spv")});
    }

    void Use(ReSTIRGIReservoir& temporalA, ReSTIRGIReservoir& temporalB, GBuffer& gbuffer, Buffer& viewBuffer, rt::TLAS& tlas, Buffer& voxInstancesBuffer) {
        Extent extent = GetDesc(gbuffer.color).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(temporalA.b0),
            GetRID(temporalA.b1),
            GetRID(temporalA.b2),
            GetRID(temporalA.b3),
            GetRID(temporalB.b0),
            GetRID(temporalB.b1),
            GetRID(temporalB.b2),
            GetRID(temporalB.b3),
            GetRID(gbuffer.normal),
            GetRID(gbuffer.previousNormal),
            GetRID(gbuffer.visibility),
            GetRID(gbuffer.depth),
            GetRID(gbuffer.previousDepth),
            GetRID(viewBuffer),
            GetRID(tlas),
            GetRID(voxInstancesBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static ReSTIRGITracePass& Get() {
        static ReSTIRGITracePass Instance;
        return Instance;
    }
};

class ReSTIRGISpatialPass {
    Pipeline pipeline;

   public:
    ReSTIRGISpatialPass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/ReSTIRGISpatial.comp.spv")});
    }

    void Use(ReSTIRGIReservoir& spatialA, ReSTIRGIReservoir& temporalB, GBuffer& gbuffer, Buffer& viewBuffer, rt::TLAS& tlas, Buffer& voxInstancesBuffer, int sampleSize) {
        Extent extent = GetDesc(gbuffer.color).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(spatialA.b0),
            GetRID(spatialA.b1),
            GetRID(spatialA.b2),
            GetRID(spatialA.b3),
            GetRID(temporalB.b0),
            GetRID(temporalB.b1),
            GetRID(temporalB.b2),
            GetRID(temporalB.b3),
            GetRID(gbuffer.normal),
            GetRID(gbuffer.previousNormal),
            GetRID(gbuffer.visibility),
            GetRID(gbuffer.depth),
            GetRID(gbuffer.previousDepth),
            GetRID(viewBuffer),
            GetRID(tlas),
            GetRID(voxInstancesBuffer),
            sampleSize,
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static ReSTIRGISpatialPass& Get() {
        static ReSTIRGISpatialPass Instance;
        return Instance;
    }
};

class ReSTIRGIResolvePass {
    Pipeline pipeline;

   public:
    ReSTIRGIResolvePass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/ReSTIRGIResolve.comp.spv")});
    }

    void Use(Image& dst, ReSTIRGIReservoir& reservoir, GBuffer& gbuffer, Buffer& viewBuffer) {
        Extent extent = GetDesc(dst).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(dst),
            GetRID(reservoir.b0),
            GetRID(reservoir.b1),
            GetRID(reservoir.b2),
            GetRID(reservoir.b3),
            GetRID(gbuffer.depth),
            GetRID(gbuffer.normal),
            GetRID(viewBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static ReSTIRGIResolvePass& Get() {
        static ReSTIRGIResolvePass Instance;
        return Instance;
    }
};
