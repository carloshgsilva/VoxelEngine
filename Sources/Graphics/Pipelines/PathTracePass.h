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
            GetRID(gbuffer.packed),
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

class SpecularTracePass {
    Pipeline pipeline;

   public:
    SpecularTracePass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/SpecularTrace.comp.spv")});
    }

    void Use(Image& dstColor, GBuffer& gbuffer, Buffer& viewBuffer, rt::TLAS& tlas, Buffer& voxInstancesBuffer) {
        Extent extent = GetDesc(dstColor).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(dstColor),
            GetRID(gbuffer.packed),
            GetRID(viewBuffer),
            GetRID(tlas),
            GetRID(voxInstancesBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static SpecularTracePass& Get() {
        static SpecularTracePass Instance;
        return Instance;
    }
};

class IRCacheTracePass {
    Pipeline pipeline;

   public:
    IRCacheTracePass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/IRCacheTrace.comp.spv")});
    }

    void Use(Image& dstColor, GBuffer& gbuffer, Buffer& viewBuffer, rt::TLAS& tlas, Buffer& voxInstancesBuffer, Buffer& radianceCacheBuffer) {
        Extent extent = GetDesc(dstColor).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(dstColor),
            GetRID(gbuffer.packed),
            GetRID(viewBuffer),
            GetRID(tlas),
            GetRID(voxInstancesBuffer),
            GetRID(radianceCacheBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static IRCacheTracePass& Get() {
        static IRCacheTracePass Instance;
        return Instance;
    }
};
class IRCacheNormalizePass {
    Pipeline pipeline;

   public:
    IRCacheNormalizePass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/IRCacheNormalize.comp.spv")});
    }

    void Use(Buffer& viewBuffer, rt::TLAS& tlas, Buffer& voxInstancesBuffer, Buffer& radianceCacheBuffer) {
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(radianceCacheBuffer),
            GetRID(viewBuffer),
            GetRID(tlas),
            GetRID(voxInstancesBuffer),
        });
        CmdDispatch(65536 / 64, 1, 1);
    }

    static IRCacheNormalizePass& Get() {
        static IRCacheNormalizePass Instance;
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
            GetRID(gbuffer.packed),
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

    void Use(ReSTIRGIReservoir& temporalDst, ReSTIRGIReservoir& temporalSrc, GBuffer& gbuffer, Buffer& viewBuffer, rt::TLAS& tlas, Buffer& voxInstancesBuffer) {
        Extent extent = GetDesc(gbuffer.packed).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(temporalDst.b0),
            GetRID(temporalDst.b1),
            GetRID(temporalDst.b2),
            GetRID(temporalDst.b3),
            GetRID(temporalSrc.b0),
            GetRID(temporalSrc.b1),
            GetRID(temporalSrc.b2),
            GetRID(temporalSrc.b3),
            GetRID(gbuffer.packed),
            GetRID(gbuffer.previousPacked),
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
        Extent extent = GetDesc(gbuffer.packed).extent;
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
            GetRID(gbuffer.packed),
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
            GetRID(gbuffer.packed),
            GetRID(viewBuffer),
        });
        CmdDispatch((extent.width + 7) / 8, (extent.height + 7) / 8, 1);
    }

    static ReSTIRGIResolvePass& Get() {
        static ReSTIRGIResolvePass Instance;
        return Instance;
    }
};
