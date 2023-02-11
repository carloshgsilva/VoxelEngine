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
    Image depthf;

    GBuffer() {
    }
    GBuffer(uint32 width, uint32 height) {
        ImageDesc desc = {.extent = {width, height}, .usage = ImageUsage::Sampled | ImageUsage::Attachment | ImageUsage::Storage};

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
        depthf = CreateImage(desc);

        desc.format = Format::D24UnormS8Uint;
        desc.usage = ImageUsage::Sampled | ImageUsage::Attachment;
        depth = CreateImage(desc);
    }

    inline static std::vector<Format> Attachments() {
        return std::vector<Format>{Format::BGRA8Unorm, Format::RGBA8Snorm, Format::BGRA8Unorm, Format::RG16Sfloat, Format::D24UnormS8Uint};
    }

    template <typename T>
    void Render(T callback) {
        CmdRender({color, normal, material, motion, depth}, {ClearColor{}, ClearColor{}, ClearColor{}, ClearColor{}, ClearDepthStencil{}}, callback);
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