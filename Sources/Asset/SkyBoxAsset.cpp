#include "SkyBoxAsset.h"

#include "stb/stb_image.h"
#include "Graphics/Pipelines/CubeMapFace.h"

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
		_Image = Image::Create(Image::Info(Format::R16G16B16A16Sfloat, { SKY_BOX_SIZE, SKY_BOX_SIZE }).setLayerCount(6).setMipCount(SKY_BOX_MIP_COUNT).setCube(true));

		Image panorama = Image::Create(Image::Info(Format::R32G32B32A32Sfloat, { (uint32)w, (uint32)h }));
		uint64_t size = w * h * 4 * sizeof(float);
		Buffer panoramaStaging = Buffer::Create(size);
		panoramaStaging.update(result, size);

		std::vector<Framebuffer> faces;
		for (int i = 0; i < 6; i++) {
			faces.push_back(Framebuffer::Create(Passes::CubeMapFace(), SKY_BOX_SIZE, SKY_BOX_SIZE));
		}

		Graphics::Transfer([&](CmdBuffer& cmd) {
			//Copy Panorama
			cmd.barrier(panorama, ImageLayout::Undefined, ImageLayout::TransferDst);
			cmd.copy(panoramaStaging, panorama);
			cmd.barrier(panorama, ImageLayout::TransferDst, ImageLayout::ShaderReadOptimal);
			
			//Render faces to framebuffer
			for (int i = 0; i < 6; i++) {
				auto& face = faces[i];
				cmd.use(face, [&] {
					CubeMapFacePipeline::Get().Use(cmd, panorama, i);
				});
				cmd.barrier(face.getAttachment(0), ImageLayout::ShaderReadOptimal, ImageLayout::TransferSrc);
			}

			//Copy to cubemap
			cmd.barrier(_Image, ImageLayout::Undefined, ImageLayout::TransferDst, 0, SKY_BOX_MIP_COUNT, 0, 6);
			
			for (uint32_t i = 0; i < 6; i++) {
				for (uint32_t mip = 0; mip < SKY_BOX_MIP_COUNT; mip ++) {
					auto& face = faces[i];
					if (mip == 0) {
						cmd.copy(face.getAttachment(0), _Image, 0, 0, 0, i);
					} else {
						cmd.barrier(_Image, ImageLayout::TransferDst, ImageLayout::TransferSrc, mip-1, 1, i, 1);
						cmd.blit(_Image, _Image, { 0,0,0,0,0,0,mip-1,i }, { 0,0,0,0,0,0,mip,i });
					}
				}
			}

			cmd.barrier(_Image, ImageLayout::TransferSrc, ImageLayout::ShaderReadOptimal, 0, SKY_BOX_MIP_COUNT-1, 0, 6);
			cmd.barrier(_Image, ImageLayout::TransferDst, ImageLayout::ShaderReadOptimal, SKY_BOX_MIP_COUNT-1, 1, 0, 6);

		});

		stbi_image_free(result);
	}

	free(data);
}
