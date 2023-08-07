#pragma once

#include <inttypes.h>
#include <tuple>

#include "Core/Core.h"
#include "Core/Module.h"

#include <evk/evk.h>

using namespace evk;

struct ReSTIRGIReservoir {
    Image b0;  // w, M, W (vec3)              16 bytes
    Image b1;  // xv (vec3), nv (u32 packed)  16 bytes
    Image b2;  // xs (vec3), ns (u32 packed)  16 bytes
    Image b3;  // radiance (vec3_f16)         8  bytes

    ReSTIRGIReservoir() {
    }
    ReSTIRGIReservoir(uint32 width, uint32 height) {
        ImageDesc desc = {
            .extent = {width, height},
            .format = Format::RGBA32Sfloat,
            .usage = ImageUsage::Sampled | ImageUsage::Storage,
        };

        b0 = CreateImage(desc);
        b1 = CreateImage(desc);
        b2 = CreateImage(desc);

        desc.format = Format::RGBA16Sfloat;
        b3 = CreateImage(desc);
    }
};

struct GBuffer {
    Image packed; // vec4(Depth, Normal, RoughMetalEmissivePADDING, Visibility)
    Image previousPacked;
    Image motion;
    ReSTIRGIReservoir reservoirTemporalA;
    ReSTIRGIReservoir reservoirTemporalB;
    ReSTIRGIReservoir reservoirSpatialA;
    ReSTIRGIReservoir reservoirSpatialB;
    ReSTIRGIReservoir previousGiReservoir;

    GBuffer() {
    }
    GBuffer(uint32 width, uint32 height) : reservoirTemporalA(width, height), reservoirTemporalB(width, height), previousGiReservoir(width, height), reservoirSpatialA(width, height), reservoirSpatialB(width, height) {
        ImageDesc desc = {
            .extent = {width, height},
            .usage = ImageUsage::Sampled | ImageUsage::Storage,
        };

        desc.format = Format::RGBA32Uint;
        packed = CreateImage(desc);
        previousPacked = CreateImage(desc);

        desc.format = Format::RG16Sfloat;
        motion = CreateImage(desc);
    }
};

class Graphics : public ModuleDef<Graphics> {
   public:
    Graphics();
};