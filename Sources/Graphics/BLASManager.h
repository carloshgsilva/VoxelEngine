#pragma once

#include "Core/Core.h"
#include "Graphics/Graphics.h"
#include "Core/Module.h"
#include "Asset/Assets.h"
#include "Asset/VoxAsset.h"

class BLASManager : public ModuleDef<BLASManager> {
    std::vector<rt::BLAS> toBuildBLAS;

   public:
    struct VoxBLAS {
        rt::BLAS blas;
        Buffer geometry;
    };

    VoxBLAS GetBLAS(AssetRefT<VoxAsset>& asset);

    inline void EnsureAllBLASAreBuilt() {
        rt::CmdBuildBLAS(toBuildBLAS);
        toBuildBLAS.clear();
    }
};