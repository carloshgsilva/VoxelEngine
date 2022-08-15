#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

class ComposePass {
    Pipeline pipeline;
public:
    ComposePass() {
        pipeline = CreatePipeline({
            .CS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/Compose.comp.spv")
        });
    }

    void Use(Image& outRadiance, Image& inRadiance, GBuffer& gbuffer, Buffer& viewBuffer) {
        struct PushConstant {
            int OutRadianceTextureRID;
            int InRadianceTextureRID;
            int VisibilityTextureRID;
            int DepthTextureRID;
            int ViewBufferRID;
        } pc {
            .OutRadianceTextureRID = GetRID(outRadiance),
            .InRadianceTextureRID = GetRID(inRadiance),
            .VisibilityTextureRID = GetRID(gbuffer.visibility),
            .DepthTextureRID = GetRID(gbuffer.depthf),
            .ViewBufferRID = GetRID(viewBuffer),
        };

        Extent extent = GetDesc(outRadiance).extent;
        CmdBind(pipeline);
        CmdPush(pc);
        CmdDispatch((extent.width+7)/8, (extent.height+7)/8, 1);
    }

    static ComposePass& Get() {
        static ComposePass Instance;
        return Instance;
    }
};