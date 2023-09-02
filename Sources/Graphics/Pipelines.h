#pragma once

#include "Graphics.h"
#include "Util/FileUtil.h"

#include <glm/glm.hpp>

class CubeMapFacePipeline {
    Pipeline pipeline;

    struct PushConstant {
        int PanoramaTextureRID;
        int FaceID;
    };

public:
    CubeMapFacePipeline() {
        pipeline = CreatePipeline({
            .VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/CubeMapFace.vert.spv"),
            .FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/CubeMapFace.frag.spv"),
            .attachments = {Format::RGBA16Sfloat}
        });
    }

    void Use(Image& panorama, int face) {
        CmdBind(pipeline);

        PushConstant pc;
        pc.PanoramaTextureRID = GetRID(panorama);
        pc.FaceID = face;

        CmdPush(&pc, sizeof(PushConstant), 0);
        CmdDraw(6, 1, 0, 0);
    }

    static CubeMapFacePipeline& Get() {
        static CubeMapFacePipeline Instance;
        return Instance;
    }
};

class GBufferPass {
    Pipeline pipeline;

   public:
    GBufferPass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/GBuffer.comp.spv")});
    }

    void Use(GBuffer& gbuffer, Buffer& viewBuffer, rt::TLAS& tlas, Buffer& voxInstancesBuffer) {
        Extent extent = GetDesc(gbuffer.packed).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(gbuffer.packed),
            GetRID(gbuffer.motion),
            GetRID(viewBuffer),
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

    void Use(Image& tempRadiance, Image& inoutRadiance, GBuffer& gbuffer, Buffer& viewBuffer) {
        Extent extent = GetDesc(gbuffer.packed).extent;
        uint32_t dispatchWidth = (extent.width + 7) / 8;
        uint32_t dispatchHeight = (extent.height + 7) / 8;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(tempRadiance),
            GetRID(inoutRadiance),
            GetRID(gbuffer.packed),
            GetRID(viewBuffer),
            1,
        });
        CmdDispatch(dispatchWidth, dispatchHeight, 1);
        CmdPush(Constant{
            GetRID(inoutRadiance),
            GetRID(tempRadiance),
            GetRID(gbuffer.packed),
            GetRID(viewBuffer),
            2,
        });
        CmdDispatch(dispatchWidth, dispatchHeight, 1);
        CmdPush(Constant{
            GetRID(tempRadiance),
            GetRID(inoutRadiance),
            GetRID(gbuffer.packed),
            GetRID(viewBuffer),
            4,
        });
        CmdDispatch(dispatchWidth, dispatchHeight, 1);
        CmdPush(Constant{
            GetRID(inoutRadiance),
            GetRID(tempRadiance),
            GetRID(gbuffer.packed),
            GetRID(viewBuffer),
            8,
        });
        CmdDispatch(dispatchWidth, dispatchHeight, 1);
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
class ColorWorldPipeline {
    Pipeline pipeline;

    struct PushConstant {
        int ColorTextureRID;
        int OutlineTextureRID;
    };

public:
    ColorWorldPipeline() {
        pipeline = CreatePipeline({
            .VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/ColorWorld.vert.spv"),
            .FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/ColorWorld.frag.spv"),
            .attachments = {Format::BGRA8Unorm},
        });
    }

    void Use(Image& compose, Image& outline) {
        CmdBind(pipeline);
        
        PushConstant pc;
        pc.ColorTextureRID = GetRID(compose);
        pc.OutlineTextureRID = GetRID(outline);
    
        CmdPush(pc);
        CmdDraw(6, 1, 0, 0);
    }

    static ColorWorldPipeline& Get() {
        static ColorWorldPipeline Instance;
        return Instance;
    }
};

class GlyphPass {
    Pipeline pipeline;

public:
    GlyphPass() {
        pipeline = CreatePipeline({
            .VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/Glyph.vert.spv"),
            .FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/Glyph.frag.spv"),
            .bindings = {{Format::RG32Sfloat}},
            .attachments = {Format::BGRA8Unorm},
            .blends = {Blend::Alpha},
        });
    }

    void Use(Buffer& glyphsBuffer, glm::vec2 screenSize, glm::vec2 position, glm::vec2 scale, float time, int offsetCoords, int numCoords) {
        CmdBind(pipeline);
        CmdPush(Constant{
            screenSize,
            position,
            scale,
            time,
            GetRID(glyphsBuffer),
            offsetCoords,
            numCoords
        });
        CmdDraw(6, 1, 0, 0);
    }

    static GlyphPass& Get() {
        static GlyphPass Instance;
        return Instance;
    }

};

class OutlineVoxelPipeline {
    Pipeline pipeline;

    struct PushConstant {
        int ViewBufferRID;
        int VolumeRID;
        int GBufferRID;
        int PADDING_0;
        glm::mat4 WorldMatrix;
        glm::vec3 Color;
    };

public:

    struct Cmd {
        glm::mat4 WorldMatrix;
        glm::vec3 Color;
        int VolumeRID;
    };

    OutlineVoxelPipeline() {
        pipeline = CreatePipeline({
            .VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/OutlineVoxel.vert.spv"),
            .FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/OutlineVoxel.frag.spv"),
            .attachments = {Format::BGRA8Unorm}
        });
    }

    void Use(Buffer& viewBuffer, GBuffer& gbuffer, std::vector<Cmd>& instances) {

        CmdBind(pipeline);
        PushConstant pc{};
        pc.ViewBufferRID = GetRID(viewBuffer);
        pc.GBufferRID = GetRID(gbuffer.packed);

        //Upload the buffered draw data
        for (auto& c : instances) {
            pc.VolumeRID = c.VolumeRID;
            pc.WorldMatrix = c.WorldMatrix;
            pc.Color = c.Color;

            CmdPush(pc);
            CmdDraw(36, 1, 0, 0);
        }
    }

    static OutlineVoxelPipeline& Get() {
        static OutlineVoxelPipeline Instance;
        return Instance;
    }

};

class LightBloomStepPipeline {
    Pipeline pipeline;

    struct PushConstant {
        int LightTextureRID;
    };

public:
    LightBloomStepPipeline() {
        pipeline = CreatePipeline({
            .VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightBloomStep.vert.spv"),
            .FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightBloomStep.frag.spv"),
            .attachments = {Format::RGBA16Sfloat},
        });
    }

    void Use(Image& light) {
        CmdBind(pipeline);

        PushConstant pc;
        pc.LightTextureRID = GetRID(light);

        CmdPush(pc);
        CmdDraw(6, 1, 0, 0);
    }

    static LightBloomStepPipeline& Get() {
        static LightBloomStepPipeline Instance;
        return Instance;
    }
};
class LightBlurPipeline {
    Pipeline pipeline;

    struct PushConstant {
        int LightTextureRID;
        int Horizontal;
    };

public:
    LightBlurPipeline() {
        pipeline = CreatePipeline({
            .VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightBlur.vert.spv"),
            .FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightBlur.frag.spv"),
            .attachments = {Format::RGBA16Sfloat}
        });
    }

    void Use(Image& light, bool horizontal) {
        CmdBind(pipeline);

        PushConstant pc;
        pc.LightTextureRID = GetRID(light);
        pc.Horizontal = horizontal ? 1 : 0;

        CmdPush(pc);
        CmdDraw(6, 1, 0, 0);
    }

    static LightBlurPipeline& Get() {
        static LightBlurPipeline Instance;
        return Instance;
    }
};

class ComposePass {
    Pipeline pipeline;

   public:
    ComposePass() {
        pipeline = CreatePipeline({.CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/Compose.comp.spv")});
    }

    void Use(Image& outRadiance, Image& diffuse, Image& specular, GBuffer& gbuffer, Buffer& viewBuffer, Buffer& voxInstancesBuffer) {
        Extent extent = GetDesc(outRadiance).extent;
        CmdBind(pipeline);
        CmdPush(Constant{
            GetRID(outRadiance),
            GetRID(diffuse),
            GetRID(specular),
            GetRID(gbuffer.packed),
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

class PresentPipeline {
    Pipeline pipeline;

    struct PushConstant {
        int ColorTextureRID;
    };

public:
    PresentPipeline() {
        pipeline = CreatePipeline({
            .VS = FileUtil::ReadBytes("Sources/Shaders/Bin/Present.vert.spv"),
            .FS = FileUtil::ReadBytes("Sources/Shaders/Bin/Present.frag.spv"),
            .attachments = {Format::BGRA8Unorm},
        });
    }

    void Use(Image& color) {
        CmdBind(pipeline);
        PushConstant pc;
        pc.ColorTextureRID = GetRID(color);

        CmdPush(pc);
        CmdDraw(6, 1, 0, 0);
    }

    static PresentPipeline& Get() {
        static PresentPipeline Instance;
        return Instance;
    }
};