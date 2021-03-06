#pragma once

#include "Core/Core.h"
#include "Graphics/Graphics.h"
#include "Asset/Assets.h"
#include "Core/Module.h"

#include <queue>

class PalleteAsset;

class PalleteCache : public ModuleDef<PalleteCache> {
	static inline const int32 MaxPalleteCount = 2048;
	static inline const int32 PalleteWidth = 256;

	//Color and Material Texture Have both 4 BytesPerPixel
	Image ColorTexture;
	Image MaterialTexture;
	Buffer ColorData;
	Buffer MaterialData;

	//It's safe to use pointer here because when an PalleteAsset is unloaded it removes from here
	//Abviously you should not cache the pointer use only in the current frame
	std::vector<PalleteAsset*> Palletes;
	std::queue<int32> FreePalletesIndex;

public:
	PalleteCache() {
		uint64_t size = PalleteWidth * MaxPalleteCount * 4;
		ColorTexture = Image::Create(Image::Info(Format::R8G8B8A8Unorm, {PalleteWidth, MaxPalleteCount}));
		ColorData = Buffer::Create(size);
		MaterialTexture = Image::Create(Image::Info(Format::R8G8B8A8Unorm, { PalleteWidth, MaxPalleteCount }));
		MaterialData = Buffer::Create(size);
	}

	static int32 AllocatePalleteIndex(PalleteAsset* InPallete);
	static void UploadPallete(PalleteAsset* InPallete);
	static void FreePalleteIndex(PalleteAsset* InPallete);

	static Image& GetColorTexture() { return Get().ColorTexture; }
	static Image& GetMaterialTexture() { return Get().MaterialTexture; }
};