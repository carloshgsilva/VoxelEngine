#include "ImageAsset.h"

#include "stb/stb_image.h"

void ImageAsset::Serialize(Stream& S) {
	if (!S.IsLoading()) {
		Log::critical("ImageAsset saving not supported!");
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
		_Image = Image::Create(Image::Info(Format::R8G8B8A8Unorm, { (uint32)w, (uint32)h }));
		
		Buffer staging = Buffer::Create(w*h*c);
		staging.update(result, w*h*c);

		Graphics::Transfer([&](CmdBuffer& cmd) {
			cmd.barrier(_Image, ImageLayout::Undefined, ImageLayout::TransferDst);
			cmd.copy(staging, _Image);
			cmd.barrier(_Image, ImageLayout::TransferDst, ImageLayout::ShaderReadOptimal);
		});

		stbi_image_free(result);
	}

}