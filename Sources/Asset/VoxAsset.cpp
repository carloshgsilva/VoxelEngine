#include "VoxAsset.h"

void VoxAsset::Upload() {
    uint64_t bufferSize = SizeX * SizeY * SizeZ;

    // MipMaps
    Buffer buffer = CreateBuffer({.size = bufferSize, .usage = BufferUsage::TransferSrc});
    int parentSizeX = SizeX, parentSizeY = SizeY, parentSizeZ = SizeZ;
    uint8_t* parentData = (uint8_t*)malloc(bufferSize);
    uint8_t* data = (uint8_t*)malloc(bufferSize);

    // Copy First mip level
    std::memcpy(data, Data.data(), Data.size());

    // TODO: Correct Mip

    for (int mip = 0; mip < MIP_COUNT; mip++) {
        if (mip > 0) {
            int mipSizeX = parentSizeX > 1 ? parentSizeX / 2 : 1;
            int mipSizeY = parentSizeY > 1 ? parentSizeY / 2 : 1;
            int mipSizeZ = parentSizeZ > 1 ? parentSizeZ / 2 : 1;

            // Swap
            std::memcpy(parentData, data, parentSizeX * parentSizeY * parentSizeZ);

            for (int x = 0; x < mipSizeX; x++) {
                for (int y = 0; y < mipSizeY; y++) {
                    for (int z = 0; z < mipSizeZ; z++) {
                        uint8_t vox = 0;

                        for (int vi = 0; vi < 9; vi++) {
                            uint8_t upperVox = *(parentData + (2 * x + ((vi >> 0) & 1)) + (2 * y + ((vi >> 1) & 1)) * parentSizeX + (2 * z + ((vi >> 2) & 1)) * parentSizeX * parentSizeY);

                            if (upperVox) {
                                vox = upperVox;
                                break;
                            }
                        }

                        *(data + x + y * mipSizeX + z * mipSizeX * mipSizeY) = vox;
                    }
                }
            }

            if (parentSizeX > 1) parentSizeX /= 2;
            if (parentSizeY > 1) parentSizeY /= 2;
            if (parentSizeZ > 1) parentSizeZ /= 2;
        }

        WriteBuffer(buffer, data, bufferSize);

        // CommandBuffer cmd = CreateCommandBuffer({});
        // cmd.Barrier(_Image, ImageLayout::Undefined, ImageLayout::TransferDst, mip);
        // cmd.Copy(buffer, _Image, mip);
        // cmd.Barrier(_Image, ImageLayout::TransferDst, ImageLayout::ShaderRead, mip);
        // cmd.Submit();
        // cmd.Wait();

        CmdBarrier(_Image, ImageLayout::Undefined, ImageLayout::TransferDst, mip);
        CmdCopy(buffer, _Image, mip);
        CmdBarrier(_Image, ImageLayout::TransferDst, ImageLayout::ShaderRead, mip);
        Sync();
    }

    free(parentData);
    free(data);
}
