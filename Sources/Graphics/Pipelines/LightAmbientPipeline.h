#pragma once

#include "Graphics/Graphics.h"
#include "glm/glm.hpp"
#include "Util/FileUtil.h"
#include <functional>
#include "Core/Engine.h"

#include "Graphics/Renderer/View.h"

class LightAmbientPipeline {
    Pipeline pipeline;

   public:
    LightAmbientPipeline() {
        pipeline = CreatePipeline({
            .VS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightAmbient.vert.spv"),
            .FS = FileUtil::ReadBytes("Assets/Mods/default/Shaders/LightAmbient.frag.spv"),
            .attachments = {Format::RGBA16Sfloat},
            .blends = {Blend::Additive},
        });
    }

    void Use(Buffer& viewBuffer, GBuffer& gbuffer, Buffer& bvhBuffer, Buffer& bvhLeafsBuffer, Image& blueNoise, Image& skyBox, rt::TLAS& tlas, Buffer& voxInstancesBuffer) {
        CmdPush(Constant{
            GetRID(viewBuffer), GetRID(gbuffer.color), GetRID(gbuffer.normal), GetRID(gbuffer.material), GetRID(gbuffer.depth), GetRID(blueNoise), GetRID(bvhBuffer), GetRID(bvhLeafsBuffer), GetRID(skyBox), GetRID(tlas),
            GetRID(voxInstancesBuffer),  // TODO: remove this
        });
        CmdBind(pipeline);
        CmdDraw(6, 1, 0, 0);
    }

    static LightAmbientPipeline& Get() {
        static LightAmbientPipeline Instance;
        return Instance;
    }
};