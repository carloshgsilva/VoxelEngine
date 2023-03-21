#include "BLASManager.h"

struct Vox {
    uint8 x, y, z, id;
};

rt::BLAS CreateBLASFromVoxels(const std::vector<Vox>& voxels) {
    std::vector<rt::AABB> aabbs;
    aabbs.resize(voxels.size());
    for (int i = 0; i < voxels.size(); i++) {
        Vox v = voxels[i];
        aabbs[i] = {
            float(v.x), float(v.y), float(v.z), float(v.x + 1), float(v.y + 1), float(v.z + 1),
        };
    }
    Buffer aabbsBuffer = CreateBuffer({
        .name = "Voxels AABBs",
        .size = sizeof(rt::AABB) * aabbs.size(),
        .usage = BufferUsage::Storage | BufferUsage::AccelerationStructureInput,
        .memoryType = MemoryType::GPU,
    });
    CmdCopy((void*)aabbs.data(), aabbsBuffer, sizeof(rt::AABB) * aabbs.size());
    return rt::CreateBLAS({
        .geometry = rt::GeometryType::AABBs,
        .aabbs = aabbsBuffer,
        .aabbsCount = uint32(aabbs.size()),
    });
}

void CreateVoxelsForAsset(AssetRefT<VoxAsset>& asset, std::vector<Vox>& voxels) {
    Extent extent = GetDesc(asset->GetImage()).extent;
    std::vector<uint8>& volume = asset->GetData();

    for (uint32 z = 0; z < extent.depth; z++) {
        for (uint32 y = 0; y < extent.height; y++) {
            for (uint32 x = 0; x < extent.width; x++) {
                uint8 v = volume[x + (y + z * extent.height) * extent.width];
                if (v > 0) {
                    voxels.push_back(Vox{
                        .x = uint8(x),
                        .y = uint8(y),
                        .z = uint8(z),
                        .id = v,
                    });
                }
            }
        }
    }
}

BLASManager::VoxBLAS CreateBLASFromVoxAsset(AssetRefT<VoxAsset>& asset) {
    std::vector<Vox> voxels;
    CreateVoxelsForAsset(asset, voxels);
    if (voxels.empty()) {
        voxels.push_back({0, 0, 0, 1});
        Log::warn("Empty Voxel found, putting a voxel in there TODO: FIXME");
    }

    Buffer geometry = CreateBuffer({
        .size = sizeof(Vox) * voxels.size(),
        .usage = BufferUsage::Storage,
        .memoryType = MemoryType::GPU,
    });
    CmdCopy(voxels.data(), geometry, sizeof(Vox) * voxels.size());

    return BLASManager::VoxBLAS{
        .blas = CreateBLASFromVoxels(voxels),
        .geometry = geometry,
    };
}

BLASManager::VoxBLAS BLASManager::GetBLAS(AssetRefT<VoxAsset>& asset) {
    std::vector<Vox> voxels;

    VoxBLAS voxBlas = CreateBLASFromVoxAsset(asset);
    toBuildBLAS.push_back(voxBlas.blas);
    return voxBlas;
}