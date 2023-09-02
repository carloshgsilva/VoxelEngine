#include "SkyBoxAsset.h"

#include "stb/stb_image.h"
#include "Graphics/Pipelines.h"

const int SKY_BOX_SIZE = 3096;
const int SKY_BOX_MIP_COUNT = 8;

void SkyBoxAsset::Serialize(Stream& S) {
    CHECK(S.IsLoading());

        void* data = malloc(S.GetSize());
        S.Serialize(data, S.GetSize());

        int w, h, c;
        float* result = stbi_loadf_from_memory((stbi_uc*)data, S.GetSize(), &w, &h, &c, 4);

        if (result == nullptr) {
            Log::error("Failed to load SkyBox {}", S.GetIdentifier());
        }
        else {
            {
                ImageDesc desc;
                desc.name = "SkyBox Light";
                desc.format = Format::RGBA16Sfloat;
                desc.extent = { SKY_BOX_SIZE, SKY_BOX_SIZE };
                desc.usage = ImageUsage::Sampled | ImageUsage::TransferSrc | ImageUsage::TransferDst;
                desc.layerCount = 6;
                desc.mipCount = SKY_BOX_MIP_COUNT;
                desc.isCube = true;
                hdriImage = CreateImage(desc);
            }

            ImageDesc desc;
            desc.name = "SkyBox Panorama";
            desc.format = Format::RGBA32Sfloat;
            desc.extent = { (uint32_t)w, (uint32_t)h };
            desc.usage = ImageUsage::Sampled|ImageUsage::TransferDst;
            Image panorama = CreateImage(desc);

            uint64_t size = w * h * 4 * sizeof(float);

            BufferDesc bufferDesc;
            bufferDesc.size = size;
            Buffer panoramaStaging = CreateBuffer(bufferDesc);
            WriteBuffer(panoramaStaging, result, size);

            std::vector<Image> faces;
            for (int i = 0; i < 6; i++) {
                ImageDesc desc;
                desc.name = "SkyBox Face";
                desc.format = Format::RGBA16Sfloat;
                desc.extent = {SKY_BOX_SIZE, SKY_BOX_SIZE};
                desc.usage = ImageUsage::Attachment|ImageUsage::Sampled| ImageUsage::TransferSrc;
                faces.push_back(CreateImage(desc));
            }

            //Copy Panorama
            CmdBarrier(panorama, ImageLayout::Undefined, ImageLayout::TransferDst);
            CmdCopy(panoramaStaging, panorama);
            CmdBarrier(panorama, ImageLayout::TransferDst, ImageLayout::ShaderRead);
                
            //Render faces to framebuffer
            for (int i = 0; i < 6; i++) {
                auto& face = faces[i];
                CmdBarrier(face, ImageLayout::Undefined, ImageLayout::ShaderRead);
                CmdRender(&face, nullptr, 1, [&] {
                    CubeMapFacePipeline::Get().Use(panorama, i);
                });
                CmdBarrier(face, ImageLayout::ShaderRead, ImageLayout::TransferSrc);
            }

            //Copy to cubemap
            CmdBarrier(hdriImage, ImageLayout::Undefined, ImageLayout::TransferDst, 0, SKY_BOX_MIP_COUNT, 0, 6);
                
            for (int i = 0; i < 6; i++) {
                for (int mip = 0; mip < SKY_BOX_MIP_COUNT; mip ++) {
                    auto& face = faces[i];
                    if (mip == 0) {
                        CmdCopy(face, hdriImage, 0, 0, 0, i);
                    } else {
                        CmdBarrier(hdriImage, ImageLayout::TransferDst, ImageLayout::TransferSrc, mip-1, 1, i, 1);
                        CmdBlit(hdriImage, hdriImage, { 0,0,0,0,0,0,mip-1,i }, { 0,0,0,0,0,0,mip,i });
                    }
                }
            }

            CmdBarrier(hdriImage, ImageLayout::TransferSrc, ImageLayout::ShaderRead, 0, SKY_BOX_MIP_COUNT-1, 0, 6);
            CmdBarrier(hdriImage, ImageLayout::TransferDst, ImageLayout::ShaderRead, SKY_BOX_MIP_COUNT-1, 1, 0, 6);


            stbi_image_free(result);
        }

        free(data);
}
