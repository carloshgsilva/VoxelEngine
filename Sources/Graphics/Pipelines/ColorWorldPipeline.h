#pragma once

#include "Graphics/Graphics.h"
#include "Util/FileUtil.h"

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