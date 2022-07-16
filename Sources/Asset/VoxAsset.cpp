#include "VoxAsset.h"

void VoxAsset::Upload() {

	// MipMaps
	Buffer buffer = Buffer::Create(SizeX * SizeY * SizeZ, BufferUsage::TransferSrc);
	int parentSizeX = SizeX, parentSizeY = SizeY, parentSizeZ = SizeZ;
	uint8_t* parentData = (uint8_t*)malloc(SizeX * SizeY * SizeZ);
	uint8_t* data = (uint8_t*)buffer.getData();

	//Copy First mip level
	std::memcpy(data, Data.data(), Data.size());

	//TODO: Correct Mip

	for (int mip = 0; mip < _Image.getMipCount(); mip++) {

		if (mip > 0) {
			int mipSizeX = parentSizeX > 1 ? parentSizeX / 2 : 1;
			int mipSizeY = parentSizeY > 1 ? parentSizeY / 2 : 1;
			int mipSizeZ = parentSizeZ > 1 ? parentSizeZ / 2 : 1;

			//Swap
			std::memcpy(parentData, data, parentSizeX * parentSizeY * parentSizeZ);

			for (int x = 0; x < mipSizeX; x++) {
				for (int y = 0; y < mipSizeY; y++) {
					for (int z = 0; z < mipSizeZ; z++) {
						uint8_t vox = 0;

						for (int vi = 0; vi < 9; vi++) {
							uint8_t upperVox = *(
								parentData +
								(2 * x + ((vi >> 0) & 1)) +
								(2 * y + ((vi >> 1) & 1)) * parentSizeX +
								(2 * z + ((vi >> 2) & 1)) * parentSizeX * parentSizeY
								);

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

		Graphics::Transfer([&](CmdBuffer& cmd){
			cmd.barrier(_Image, ImageLayout::Undefined, ImageLayout::TransferDst, mip);
			cmd.copy(buffer, _Image, mip);
			cmd.barrier(_Image, ImageLayout::TransferDst, ImageLayout::ShaderReadOptimal, mip);
		});

	}

	free(parentData);
}
