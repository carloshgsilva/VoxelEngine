#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class DenoiserDiscPass {
    Pipeline pipeline;
public:
    DenoiserDiscPass() {
        pipeline = CreatePipeline({
            .CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/DenoiserDisc.comp.spv")
        });
    }

    void Use(Image& outRadiance, Image& inRadiance, GBuffer& gbuffer, Buffer& viewBuffer) {
        struct PushConstant {
            int OutRadianceTextureRID;
            int InRadianceTextureRID;
            int NormalTextureRID;
            int DepthTextureRID;
            int ViewBufferRID;
        } pc {
            .OutRadianceTextureRID = GetRID(outRadiance),
            .InRadianceTextureRID = GetRID(inRadiance),
            .NormalTextureRID = GetRID(gbuffer.normal),
            .DepthTextureRID = GetRID(gbuffer.depthf),
            .ViewBufferRID = GetRID(viewBuffer),
        };

        Extent extent = GetDesc(gbuffer.color).extent;
        CmdBind(pipeline);
        CmdPush(pc);
        CmdDispatch((extent.width+7)/8, (extent.height+7)/8, 1);
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
        pipeline = CreatePipeline({
            .CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/DenoiserAtrous.comp.spv")
        });
    }

    void Use(Image& outRadiance, Image& inRadiance, GBuffer& gbuffer, Buffer& viewBuffer, int size) {
        struct PushConstant {
            int OutRadianceTextureRID;
            int InRadianceTextureRID;
            int NormalTextureRID;
            int DepthTextureRID;
            int ViewBufferRID;
            int Size;
        } pc {
            .OutRadianceTextureRID = GetRID(outRadiance),
            .InRadianceTextureRID = GetRID(inRadiance),
            .NormalTextureRID = GetRID(gbuffer.normal),
            .DepthTextureRID = GetRID(gbuffer.depthf),
            .ViewBufferRID = GetRID(viewBuffer),
            .Size = size
        };

        Extent extent = GetDesc(gbuffer.color).extent;
        CmdBind(pipeline);
        CmdPush(pc);
        CmdDispatch((extent.width+7)/8, (extent.height+7)/8, 1);
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
        pipeline = CreatePipeline({
            .CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/DenoiserTemporal.comp.spv")
        });
    }

    void Use(Image& outRadiance, Image& inRadiance, Image& inLastRadiance, GBuffer& gbuffer, Buffer& viewBuffer) {
        struct PushConstant {
            int OutRadianceTextureRID;
            int InRadianceTextureRID;
            int InLastRadianceTextureRID;
            int MotionTextureRID;
            int DepthTextureRID;
            int ViewBufferRID;
        } pc {
            .OutRadianceTextureRID = GetRID(outRadiance),
            .InRadianceTextureRID = GetRID(inRadiance),
            .InLastRadianceTextureRID = GetRID(inLastRadiance),
            .MotionTextureRID = GetRID(gbuffer.motion),
            .DepthTextureRID = GetRID(gbuffer.depthf),
            .ViewBufferRID = GetRID(viewBuffer)
        };

        Extent extent = GetDesc(gbuffer.color).extent;
        CmdBind(pipeline);
        CmdPush(pc);
        CmdDispatch((extent.width+7)/8, (extent.height+7)/8, 1);
    }

    static DenoiserTemporalPass& Get() {
        static DenoiserTemporalPass Instance;
        return Instance;
    }
};