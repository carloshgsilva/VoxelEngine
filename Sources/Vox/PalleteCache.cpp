#include "PalleteCache.h"

#include "Asset/PalleteAsset.h"

void PalleteCache::UploadPallete(PalleteAsset* InPallete) {
	CHECK(InPallete->PalleteIndex != -1);
	constexpr int32 BPP = 4;

	size_t offset = (PalleteWidth * InPallete->PalleteIndex) * BPP;
	uint8* color =  (uint8*)Get().ColorData.GetPtr() + offset;
	uint8* mat = (uint8*)Get().MaterialData.GetPtr() + offset;
	
	for (int i = 0; i < 256; i++) {
		VoxMaterial& Mat = InPallete->MaterialAt(i);
		color[i * BPP + 0] = Mat.r;
		color[i * BPP + 1] = Mat.g;
		color[i * BPP + 2] = Mat.b;
		color[i * BPP + 3] = 255;
		mat[i * BPP + 0] = Mat.roughness;
		mat[i * BPP + 1] = Mat.metallic;
		mat[i * BPP + 2] = Mat.emit;
		mat[i * BPP + 3] = 0;

		//Log::info("Material[{:3}] r = {:3} g = {:3} b = {:3} rough = {:3} metal = {:3} emit = {:3}", i, Mat.r, Mat.g, Mat.b, Mat.roughness, Mat.metallic, Mat.emit);
	}

	CmdBarrier(Get().ColorTexture, ImageLayout::Undefined, ImageLayout::TransferDst);
	CmdBarrier(Get().MaterialTexture, ImageLayout::Undefined, ImageLayout::TransferDst);
	CmdCopy(Get().ColorData, Get().ColorTexture);
	CmdCopy(Get().MaterialData, Get().MaterialTexture);
	CmdBarrier(Get().ColorTexture, ImageLayout::TransferDst, ImageLayout::ShaderRead);
	CmdBarrier(Get().MaterialTexture, ImageLayout::TransferDst, ImageLayout::ShaderRead);
}

int32 PalleteCache::AllocatePalleteIndex(PalleteAsset* InPallete) {
	int32 PalleteIndex = -1;

	//Reuse Index
	if (Get().FreePalletesIndex.size() > 0) {
		PalleteIndex = Get().FreePalletesIndex.front();
		Get().FreePalletesIndex.pop();
	}

	//Allocate new Index
	PalleteIndex = static_cast<int32>(Get().Palletes.size());
	InPallete->PalleteIndex = PalleteIndex;
	Get().Palletes.push_back(InPallete);

	CHECK(PalleteIndex != -1);
	CHECK(PalleteIndex < MaxPalleteCount);

	Get().UploadPallete(InPallete);

	return PalleteIndex;
}

void PalleteCache::FreePalleteIndex(PalleteAsset* InPallete) {
	CHECK(InPallete->PalleteIndex != -1);
	CHECK(Get().Palletes[InPallete->PalleteIndex] != nullptr);

	int32 Index = InPallete->PalleteIndex;

	Get().Palletes[Index] = nullptr;
	Get().FreePalletesIndex.push(Index);
}
