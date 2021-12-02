#pragma once

#include "Assets.h"
#include "IO/Stream.h"
#include "Vox/PalleteCache.h"

struct VoxMaterial {
	uint8 r, g, b, a;
	uint8 roughness, metallic, emit;

	VoxMaterial() {}
	VoxMaterial(uint8 r, uint8 g, uint8 b, uint8 roughness, uint8 metal, uint8 emit) {
		this->r = r;
		this->g = g;
		this->b = b;
		this->roughness = roughness;
		this->metallic = metal;
		this->emit = emit;
	}

	friend Stream& operator|(Stream& S, VoxMaterial& V) {
		S | V.r | V.g | V.b;
		return S;
	}
};

class PalleteAsset : public Asset {
	ASSET(PalleteAsset, p)

	//To access PalleteIndex
	friend class PalleteCache; 
	friend class GeometryVoxelPipeline;
	friend class OutlineVoxelPipeline;

	//Serialized Data
	std::vector<VoxMaterial> Data;

	//Runtime Data
	int32 PalleteIndex = -1; //Index in PalleteCache Global

	virtual void OnLoad() {
		PalleteIndex = PalleteCache::AllocatePalleteIndex(this);
		Upload();
	}
public:
	PalleteAsset(){
		Data.resize(256);
		OnLoad();
	}
	~PalleteAsset(){
		PalleteCache::FreePalleteIndex(this);
	}

	//Should be called after Pallete.Data has been changed
	void Upload() {
		PalleteCache::UploadPallete(this);
	}

	int32 GetPalleteIndex() { return PalleteIndex; }

	VoxMaterial& MaterialAt(int32 Index) { return Data[Index]; }

	virtual void Serialize(Stream& S) {
		if (S.IsLoading()) {
			Data.resize(256);
		}
		size_t sizeInBytes = Data.size() * sizeof(VoxMaterial);
		S.Serialize(Data.data(), sizeInBytes);
	}
};