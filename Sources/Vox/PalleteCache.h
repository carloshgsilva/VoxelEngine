#pragma once

#include "Core/Core.h"
#include "Graphics/Graphics.h"
#include "Asset/Assets.h"
#include "Core/Module.h"

#include <queue>

class PalleteAsset;

class PalleteCache : public ModuleDef<PalleteCache> {
    static inline const int32 MaxPalleteCount = 2048;
    static inline const int32 PalleteWidth = 256;

    // Color and Material Texture Have both 4 BytesPerPixel
    Image ColorTexture;
    Image MaterialTexture;
    std::vector<uint8> colors;
    std::vector<uint8> materials;

    // It's safe to use pointer here because when an PalleteAsset is unloaded it removes from here
    // Abviously you should not cache the pointer use only in the current frame
    std::vector<PalleteAsset*> Palletes;
    std::queue<int32> FreePalletesIndex;

   public:
    PalleteCache() {
        uint64_t size = PalleteWidth * MaxPalleteCount * 4;
        colors.resize(size);
        materials.resize(size);
        ColorTexture = CreateImage({
            .extent = {PalleteWidth, MaxPalleteCount},
            .format = Format::RGBA8Unorm,
            .usage = ImageUsage::Sampled | ImageUsage::TransferDst,
        });
        MaterialTexture = CreateImage({
            .extent = {PalleteWidth, MaxPalleteCount},
            .format = Format::RGBA8Unorm,
            .usage = ImageUsage::Sampled | ImageUsage::TransferDst,
        });
    }

    static int32 AllocatePalleteIndex(PalleteAsset* InPallete);
    static void UploadPallete(PalleteAsset* InPallete);
    static void FreePalleteIndex(PalleteAsset* InPallete);

    static Image& GetColorTexture() {
        return Get().ColorTexture;
    }
    static Image& GetMaterialTexture() {
        return Get().MaterialTexture;
    }
};