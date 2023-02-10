#pragma once

#include "Assets.h"
#include "Graphics/Graphics.h"

#include <vector>

class VoxAsset : public Asset {
    ASSET(VoxAsset, v)

    inline static constexpr int MIP_COUNT = 3;

    // Serialized data
    uint32 SizeX;
    uint32 SizeY;
    uint32 SizeZ;
    std::vector<uint8> Data;

    // Runtime Data
    Image _Image;

    void NormalizeSize() {
    }

   public:
    VoxAsset() {
    }
    VoxAsset(int32 InSizeX, int32 InSizeY, int32 InSizeZ) : SizeX(InSizeX), SizeY(InSizeY), SizeZ(InSizeZ) {
        SizeX = ((SizeX - 1) & (~0b11)) + 4;
        SizeY = ((SizeY - 1) & (~0b11)) + 4;
        SizeZ = ((SizeZ - 1) & (~0b11)) + 4;

        Data.resize(SizeX * SizeY * SizeZ);
        _Image = CreateImage({
            .extent = {SizeX, SizeY, SizeZ},
            .format = Format::R8Uint,
            .filter = Filter::Nearest,
            .usage = ImageUsage::Sampled | ImageUsage::TransferDst,
            .mipCount = MIP_COUNT,
        });
    }

    void Upload();

    virtual void OnLoad() {
        _Image = CreateImage({
            .extent = {SizeX, SizeY, SizeZ},
            .format = Format::R8Uint,
            .filter = Filter::Nearest,
            .usage = ImageUsage::Sampled | ImageUsage::TransferDst,
            .mipCount = 3,
        });
        Upload();
    }

    virtual void Serialize(Stream& S) {
        S | SizeX | SizeY | SizeZ;
        // TODO: Compress
        if (S.IsLoading()) {
            Data.resize(SizeX * SizeY * SizeZ);
        }

        S.Serialize(Data.data(), Data.size());
    }

    inline uint8* PixelAt(int32 X, int32 Y, int32 Z) {
        uint8* pos = Data.data() + ((size_t)X + ((size_t)Y * (size_t)SizeX) + ((size_t)Z * (size_t)SizeX * (size_t)SizeY));
        // CHECK(pos >= Data.begin()._Ptr && pos < Data.end()._Ptr);
        return pos;
    }

    inline Image& GetImage() {
        return _Image;
    }
    inline std::vector<uint8>& GetData() {
        return Data;
    }
};