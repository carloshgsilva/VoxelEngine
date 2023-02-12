#pragma once

#include <inttypes.h>
#include <tuple>

#include "Core/Core.h"
#include "Core/Module.h"

#include <evk/evk.h>

using namespace evk;

struct GBuffer {
    Image color;
    Image visibility;
    Image normal;
    Image material;
    Image motion;
    Image depth;
    Image previousDepth;

    GBuffer() {
    }
    GBuffer(uint32 width, uint32 height) {
        ImageDesc desc = {.extent = {width, height}, .usage = ImageUsage::Sampled | ImageUsage::Storage};

        desc.format = Format::BGRA8Unorm;
        color = CreateImage(desc);

        desc.format = Format::R32Uint;
        visibility = CreateImage(desc);

        desc.format = Format::RGBA8Snorm;
        normal = CreateImage(desc);

        desc.format = Format::BGRA8Unorm;
        material = CreateImage(desc);

        desc.format = Format::RG16Sfloat;
        motion = CreateImage(desc);

        desc.format = Format::R32Sfloat;
        depth = CreateImage(desc);
        previousDepth = CreateImage(desc);
    }
};

class Graphics : public ModuleDef<Graphics> {
   public:
    Graphics();

    template <typename T>
    static void Frame(T callback) {
        evk::Submit();
    }
};