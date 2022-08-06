#include "ImageAsset.h"

#include "stb/stb_image.h"

void ImageAsset::Serialize(Stream& S) {
	if (!S.IsLoading()) {
		Log::error("ImageAsset saving not supported!");
		CHECK(0);
	}

	void* data = malloc(S.GetSize());
	S.Serialize(data, S.GetSize());

	int w = 0;
	int h = 0;
	int c = 0;
	stbi_uc* result = stbi_load_from_memory((const stbi_uc*)data, S.GetSize(), &w, &h, &c, 4);

	if (result == nullptr) {
		Log::error("Failed to load Image {}", S.GetIdentifier());
	}
	else {
		_Image = CreateImage({
			.extent = { (uint32)w, (uint32)h },
			.format = Format::RGBA8Unorm,
			.usage = ImageUsage::Sampled | ImageUsage::TransferDst
		});
		
		Buffer staging = CreateBuffer({
			.size = (uint64_t)w*h*c,
			.usage = BufferUsage::TransferSrc,
			.memoryType = MemoryType::CPU
		});
		WriteBuffer(staging, result, w*h*c);

		CmdBarrier(_Image, ImageLayout::Undefined, ImageLayout::TransferDst);
		CmdCopy(staging, _Image);
		CmdBarrier(_Image, ImageLayout::TransferDst, ImageLayout::ShaderRead);

		stbi_image_free(result);
	}

}