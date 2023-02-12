#pragma once

#include "Graphics/Graphics.h"
#include "glm/glm.hpp"
#include "Graphics/Renderer/View.h"
#include "Core/Engine.h"
#include "Util/FileUtil.h"
#include <functional>

class ComposePipeline {
    Pipeline pipeline;

    struct PushConstant {
        int ViewBufferRID;
        int ColorTextureRID;
        int DepthTextureRID;
        int NormalTextureRID;
        int LightTextureRID;
        int ReflectionTextureRID;
        int Bloom0TextureRID;
        int Bloom1TextureRID;
        int Bloom2TextureRID;
        int Bloom3TextureRID;
        int Bloom4TextureRID;
    };

   public:
    ComposePipeline() {
        pipeline = CreatePipeline({.VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/Compose.vert.spv"), .FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/Compose.frag.spv"), .attachments = {Format::BGRA8Unorm}});
    }

    void Use(Buffer viewBuffer, GBuffer& gbuffer, Image light, Image reflection, std::vector<Image>& blooms) {
        CHECK(blooms.size() == 5);

        // Update Push Constances
        PushConstant pc = {};
        pc.ViewBufferRID = GetRID(viewBuffer);
        pc.ColorTextureRID = GetRID(gbuffer.color);
        pc.DepthTextureRID = GetRID(gbuffer.depth);
        pc.NormalTextureRID = GetRID(gbuffer.normal);
        pc.LightTextureRID = GetRID(light);
        pc.ReflectionTextureRID = GetRID(reflection);
        pc.Bloom0TextureRID = GetRID(blooms[0]);
        pc.Bloom1TextureRID = GetRID(blooms[1]);
        pc.Bloom2TextureRID = GetRID(blooms[2]);
        pc.Bloom3TextureRID = GetRID(blooms[3]);
        pc.Bloom4TextureRID = GetRID(blooms[4]);

        CmdPush(pc);
        CmdBind(pipeline);

        constexpr bool ENABLE_UPSAMPLING = false;
        CmdDraw(6, 1, 0, 0);
    }

    static ComposePipeline& Get() {
        static ComposePipeline Instance;
        return Instance;
    }
};